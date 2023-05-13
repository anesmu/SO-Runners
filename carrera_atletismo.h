// Previene la inclusión múltiple del archivo de encabezado
#ifndef CARRERA_ATLETISMO_H
#define CARRERA_ATLETISMO_H

// Incluye las bibliotecas necesarias para el programa
#include <stdio.h>     // Para las funciones de entrada y salida estándar (por ejemplo, printf y scanf)
#include <stdlib.h>    // Para funciones de uso general (por ejemplo, exit y srand)
#include <unistd.h>    // Para las funciones de Unix (por ejemplo, fork y usleep)
#include <sys/types.h> // Para definiciones de tipos de datos (por ejemplo, pid_t)
#include <sys/ipc.h>   // Para el uso de memoria compartida y colas de mensajes
#include <sys/shm.h>   // Para funciones de memoria compartida (por ejemplo, shmget y shmat)
#include <sys/wait.h>  // Para funciones de espera (por ejemplo, wait)
#include <semaphore.h> // Para funciones de semáforo (por ejemplo, sem_init y sem_wait)
#include <time.h>      // Para funciones de tiempo (por ejemplo, time)

#define MAX_RUNNERS 100

typedef struct Runner
{
    int distance;
    int pid;
} Runner;

// Estructura para el monitor que controla la carrera
typedef struct Monitor
{
    int winner;
    int number_runners;          // Número de competidores en la carrera
    sem_t mutex;                 // Semáforo para controlar el acceso a la memoria compartida
    Runner runners[MAX_RUNNERS]; // Array de competidores
} Monitor;

// Declaración de funciones utilizadas en el programa
int compare(const void *a, const void *b); // Función de comparación para ordenar las posiciones de los competidores
int main();                                // Función principal del programa

// Fin de la prevención de inclusión múltiple
#endif // CARRERA_ATLETISMO_H
