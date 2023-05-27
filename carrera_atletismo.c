#include "carrera_atletismo.h"

// Función de comparación para ordenar las posiciones de los competidores
int compare(const void *a, const void *b)
{
    return ((Runner *)b)->distance - ((Runner *)a)->distance;
}

// Funcion para encontrar el índice del proceso hijo
int get_runner_index(Monitor *monitor, pid_t pid)
{
    for (int i = 0; i < monitor->number_runners; i++)
    {
        if (monitor->runners[i].pid == pid)
        {
            return i;
        }
    }
    // Si el corredor no se encuentra, se añade a la primera posición vacía
    for (int i = 0; i < monitor->number_runners; i++)
    {
        if (monitor->runners[i].pid == 0)
        {
            monitor->runners[i].pid = pid;
            return i;
        }
    }
    return -1; // Devuelve -1 si no se encuentra espacio para el corredor
}

// Función que simula a un corredor en la carrera
void run(Monitor *monitor, int race_length)
{
    srand(time(NULL) ^ (getpid() << 16));

    while (monitor->winner == -1)
    {
        // El corredor espera su turno para correr
        sem_wait(&monitor->run);

        int i = get_runner_index(monitor, getpid()); // Obtengo el índice del corredor
        if (i == -1)
        {
            // Si no se encuentra el corredor o no puede asignarlo, se sale de la función
            printf("Error: Corredor no encontrado.\n");
            return;
        }

        // El corredor corre una distancia aleatoria de 1 a 3 metros
        monitor->runners[i].distance += rand() % 3 + 1;

        if (monitor->runners[i].distance >= race_length)
        {
            monitor->winner = monitor->runners[i].pid;
            sem_post(&monitor->print_distance);
            break;
        }

        // Se libera el semáforo para el próximo corredor
        sem_post(&monitor->print_distance);

        // El corredor descansa durante medio segundo antes de la próxima iteración
        usleep(1000 * 500);
    }
}

// Función que simula al monitor de la carrera
void print_status(Monitor *monitor)
{
    // El monitor funciona hasta que hay un ganador
    while (monitor->winner == -1)
    {
        // El monitor espera su turno para imprimir
        sem_wait(&monitor->print_distance);

        // Ordena la posición de los corredores
        qsort(monitor->runners, monitor->number_runners, sizeof(Runner), compare);

        // El monitor imprime el estado actual de la carrera
        printf("\nEstado de la carrera:\n");

        // Para cada corredor, el monitor imprime su distancia actual
        for (int i = 0; i < monitor->number_runners; i++)
        {
            printf("#%d Corredor %d: %d metros\n", i + 1, monitor->runners[i].pid, monitor->runners[i].distance);
        }

        // El monitor libera el semáforo para el próximo corredor
        sem_post(&monitor->run);

        // El monitor espera un segundo antes de la próxima iteración
        usleep(1000 * 1000);
    }
}

void kill_childs(Monitor *monitor)
{
    printf("\n¡Corredor %d ha ganado la carrera!\n", monitor->winner);

    for (int i = 0; i < monitor->number_runners; i++)
    {
        if (i != monitor->winner)
        {
            kill(monitor->runners[i].pid, SIGKILL);
        }
    }

    while (wait(NULL) > 0)
        ;

    printf("\n¡La carrera ha finalizado!\n");
}

void start_race(Monitor *monitor, int race_length)
{
    pid_t pid;

        // Crear un proceso hijo para cada competidor
    for (int i = 0; i < monitor->number_runners; i++)
    {
        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(1);
        }
        else if (pid == 0)
        {
            run(monitor, race_length);
            exit(0);
        }
    }

    // El padre imprime los resultados
    print_status(monitor);

    // El padre elimina todos los hijos de forma ordenada
    kill_childs(monitor);
}

void initialize_monitor(Monitor *monitor, int number_runners)
{
    monitor->number_runners = number_runners;
    monitor->winner = -1;
    for (int i = 0; i < monitor->number_runners; i++)
    {
        monitor->runners[i].pid = 0;
        monitor->runners[i].distance = 0;
    }

    // Initialize los semáforos
    sem_init(&monitor->run, 1, 1);
    sem_init(&monitor->print_distance, 1, 0);
}

int main()
{
    int shmid;
    Monitor *monitor; // Declaración global del puntero al monitor
    int number_runners, race_length;

    // Solicitar al usuario el número de competidores y la longitud de la carrera
    printf("Ingrese el número de competidores: ");
    scanf("%d", &number_runners);
    while (number_runners > MAX_RUNNERS || number_runners <= 0)
    {
        printf("Número de competidores inválido. Debe ser un número entre 1 y %d.\n", MAX_RUNNERS);
        printf("Ingrese el número de competidores: ");
        scanf("%d", &number_runners);
    }

    // Solicita longitud de la carrera
    printf("Ingrese la longitud de la carrera: ");
    scanf("%d", &race_length);

    // Crear la memoria compartida
    if ((shmid = shmget(IPC_PRIVATE, sizeof(Monitor), IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        exit(1);
    }

    // Asignar la memoria compartida al puntero del monitor
    if ((monitor = shmat(shmid, NULL, 0)) == (Monitor *)-1)
    {
        perror("shmat");
        exit(1);
    }

    // Inicializar el monitor con los valores proporcionados por el usuario
    initialize_monitor(monitor, number_runners);

    // Empieza la carrera
    start_race(monitor, race_length);

    // Destruir el semáforo
    sem_destroy(&monitor->run);
    sem_destroy(&monitor->print_distance);

    // Desasociar la memoria compartida
    shmdt(monitor);

    // Liberar la memoria compartida
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}