#include "carrera_atletismo.h"

// Función de comparación para ordenar las posiciones de los competidores
int compare(const void *a, const void *b)
{
    return ((Runner *)a)->distance - ((Runner *)b)->distance;
}

int main()
{
    int shmid;
    Monitor *monitor;
    pid_t pid;
    int i, number_runners, race_length, traveled_distance;

    // Solicitar al usuario el número de competidores y la longitud de la carrera
    printf("Ingrese el número de competidores: ");
    scanf("%d", &number_runners);

    while (number_runners > MAX_RUNNERS || number_runners <= 0)
    {
        printf("Número de competidores inválido. Debe ser un número entre 1 y %d.\n", MAX_RUNNERS);
        printf("Ingrese el número de competidores: ");
        scanf("%d", &number_runners);
    }

    printf("Ingrese la longitud de la carrera: ");
    scanf("%d", &race_length);

    // Inicializa la semilla para la función rand()
    srand(time(NULL));

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

    // Inicializar el monitor con los valores ingresados por el usuario
    monitor->number_runners = number_runners;
    monitor->winner = -1;
    for (i = 0; i < number_runners; i++)
    {
        monitor->runners[i].pid = 0;
        monitor->runners[i].distance = 0;
    }

    // Inicializar el semáforo para controlar el acceso a la memoria compartida
    if (sem_init(&monitor->mutex, 1, 1) == -1)
    {
        perror("sem_init");
        exit(1);
    }

    // Crear un proceso hijo para cada competidor
    for (i = 0; i < number_runners; i++)
    {
        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(1);
        }
        else if (pid == 0)
        {
            // Código del proceso hijo (competidor)
            while (monitor->winner == -1)
            {
                // Adquirir el semáforo antes de modificar la memoria compartida
                sem_wait(&monitor->mutex);

                // Calcular la distancia recorrida en esta iteración (entre 1 y 3 metros)
                traveled_distance = rand() % 3 + 1;
                monitor->runners[i].distance += traveled_distance;
                monitor->runners[i].pid = getpid();

                // Comprobar si el competidor ha llegado a la meta y notificar al proceso padre
                if (monitor->runners[i].distance >= race_length)
                {
                    monitor->winner = i;
                }
                else
                {
                    qsort(monitor->runners, number_runners, sizeof(Runner), compare);
                }
                
                // Liberar el semáforo después de modificar la memoria compartida
                sem_post(&monitor->mutex);

                // Dormir por un tiempo
                usleep(1000);
            }

            exit(0);
        }
    }

    // Esperar a que todos los procesos hijos (competidores) terminen
    while (wait(NULL) > 0)
        ;

    // Imprimir los resultados de la carrera
    printf("\nResultados de la carrera:\n");
    printf("Ganador: Competidor (PID: %d)\n", monitor->runners[monitor->winner].pid);
    for (i = 0; i < number_runners; i++)
    {
        printf("Competidor #%d - Distancia: %d\n", monitor->runners[i].pid, monitor->runners[i].distance);
    }

    // Destruir el semáforo
    sem_destroy(&monitor->mutex);

    // Desasociar la memoria compartida
    shmdt(monitor);

    // Liberar la memoria compartida
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
