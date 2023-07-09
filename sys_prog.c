#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define TOBACCO_PAPER_SEM_KEY 1234
#define PAPER_MATCHES_SEM_KEY 2345
#define TOBACCO_MATCHES_SEM_KEY 3456
#define AGENT_SEM_KEY 4567
#define SMOKER_SEM_KEY 5678
#define SHARED_MEM_KEY 6789

void wait_semaphore(int semid, int sem_num) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = -1;
    sb.sem_flg = 0;
    semop(semid, &sb, 1);
}

void signal_semaphore(int semid, int sem_num) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = 1;
    sb.sem_flg = 0;
    semop(semid, &sb, 1);
}

int main() {
    int tobacco_paper_sem, paper_matches_sem, tobacco_matches_sem, agent_sem, smoker_sem;
    int shared_mem;

    // Create or get the semaphores
    tobacco_paper_sem = semget(TOBACCO_PAPER_SEM_KEY, 1, IPC_CREAT | 0666);
    paper_matches_sem = semget(PAPER_MATCHES_SEM_KEY, 1, IPC_CREAT | 0666);
    tobacco_matches_sem = semget(TOBACCO_MATCHES_SEM_KEY, 1, IPC_CREAT | 0666);
    agent_sem = semget(AGENT_SEM_KEY, 1, IPC_CREAT | 0666);
    smoker_sem = semget(SMOKER_SEM_KEY, 1, IPC_CREAT | 0666);

    // Initialize the semaphores
    semctl(tobacco_paper_sem, 0, SETVAL, 0);
    semctl(paper_matches_sem, 0, SETVAL, 0);
    semctl(tobacco_matches_sem, 0, SETVAL, 0);
    semctl(agent_sem, 0, SETVAL, 1);
    semctl(smoker_sem, 0, SETVAL, 0);

    // Create or get the shared memory segment
    shared_mem = shmget(SHARED_MEM_KEY, sizeof(int), IPC_CREAT | 0666);
    int *ingredient = (int *)shmat(shared_mem, NULL, 0);

    if (fork() == 0) {
        // Agent process
        while (1) {
            wait_semaphore(agent_sem, 0);
            *ingredient = rand() % 3;
            switch (*ingredient) {
                case 0: // Tobacco and paper
                    printf("Agent puts tobacco and paper on the table.\n");
                    signal_semaphore(tobacco_paper_sem, 0);
                    break;
                case 1: // Paper and matches
                    printf("Agent puts paper and matches on the table.\n");
                    signal_semaphore(paper_matches_sem, 0);
                    break;
                case 2: // Tobacco and matches
                    printf("Agent puts tobacco and matches on the table.\n");
                    signal_semaphore(tobacco_matches_sem, 0);
                    break;
            }
        }
    } else {
        // Smoker processes
        int smoker_id = 0; // Smoker with tobacco
        while (1) {
            if (smoker_id == 0) {
                wait_semaphore(tobacco_paper_sem, 0);
                printf("Smoker with tobacco is smoking.\n");
                signal_semaphore(smoker_sem, 0);
                printf("Smoker with tobacco releases ingredients.\n");
                smoker_id = 1;
            } else if (smoker_id == 1) {
                wait_semaphore(paper_matches_sem, 0);
                printf("Smoker with paper is smoking.\n");
                signal_semaphore(smoker_sem, 0);
                printf("Smoker with paper releases ingredients.\n");
                smoker_id = 2;
            } else {
                wait_semaphore(tobacco_matches_sem, 0);
                printf("Smoker with matches is smoking.\n");
                signal_semaphore(smoker_sem, 0);
                printf("Smoker with matches releases ingredients.\n");
                smoker_id = 0;
            }
            usleep(1000000); // smoking time 1 sec
        }
    }

    return 0;
}

