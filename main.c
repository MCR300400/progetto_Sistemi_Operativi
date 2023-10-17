#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

int semid;  // ID del semaforo
int N = 5;
typedef struct {
    int id;
    time_t lastMeal;
} Philosopher;

bool shouldTerminate = false;

void init() {
    semid = semget(IPC_PRIVATE, N, 0600);  // Crea un semaforo per ogni filosofo
    if (semid == -1) {
        perror("Errore nella creazione del semaforo");
        exit(1);
    }
    for (int i = 0; i < N; i++) {
        if (semctl(semid, i, SETVAL, 1) == -1) {  // Inizializza tutti i semafori a 1
            perror("Errore nell'inizializzazione del semaforo");
            exit(1);
        }
    }
}

void handleSignal(int signal) {
    shouldTerminate = true;
}

void philosopher(Philosopher* philosopher, int deadlockDetection, int deadlockSolution, int starvationDetection) {
    struct sembuf op[2];
    int leftFork = philosopher->id;  // Forchetta a sinistra
    int rightFork = (philosopher->id+1) % N;  // Forchetta a destra
    if (deadlockSolution && philosopher->id == N-1) {
        // Inverti l'ordine delle forchette per l'ultimo filosofo
        int temp = leftFork;
        leftFork = rightFork;
        rightFork = temp;
    }

    while (true) {
        printf("-----------INIZIO WHILE-----------");
        printf("Philosopher %d is thinking...\n", philosopher->id);
        sleep(rand() % 5 + 1);

        printf("Philosopher %d is waiting to pick up left fork\n", philosopher->id);
        semop(semid, &op[leftFork], 1);

        printf("Philosopher %d picked up left fork\n", philosopher->id);

        printf("Philosopher %d is waiting to pick up right fork\n", philosopher->id);
        semop(semid, &op[rightFork], 1);

        printf("Philosopher %d picked up right fork\n", philosopher->id);

        printf("Philosopher %d is eating...\n", philosopher->id);
        sleep(rand() % 5 + 1);

        printf("Philosopher %d is putting down right fork\n", philosopher->id);
        semop(semid, &op[rightFork + N], 1);

        printf("Philosopher %d is putting down left fork\n", philosopher->id);
        semop(semid, &op[leftFork + N], 1);

        if (shouldTerminate) {
            printf("Terminating philosopher %d\n", philosopher->id);
            exit(0);
        }
        printf("-----------FINE WHILE-----------");
    }
}


int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Uso: %s <numero di filosofi> <rilevamento di stallo> <soluzione anti-stallo> <rilevamento di starvation>\n", argv[0]);
        exit(1);
    }
    N = atoi(argv[1]);
    int deadlockDetection = atoi(argv[2]);
    int deadlockSolution = atoi(argv[3]);
    int starvationDetection = atoi(argv[4]);
    Philosopher philosophers[N];

    for (int i = 0; i < N; i++) {
        philosophers[i].id = i;
        if (fork() == 0) {
            philosopher(&philosophers[i], deadlockDetection, deadlockSolution, starvationDetection);
            exit(0);
        }
    }

    // Wait for all child processes to terminate
    for (int i = 0; i < N; i++) {
        wait(NULL);
    }

    // Cleanup resources and terminate gracefully
    // ...

    return 0;
}
