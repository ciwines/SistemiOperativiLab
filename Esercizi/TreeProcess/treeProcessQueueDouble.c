// Creare un’applicazione in C che gestisca un albero di processi tramite dei segnali. In particolare, il programma,
// una volta lanciato, deve accettare i seguenti comandi:
// ● c n : crea nuovi figli al livello n (ogni processo esistente a livello n-1 ne genera un altro)
// ● k n : termina i figli al livello n
// ● p : stampa in output l’albero dei processi (anche in modo “non ordinato”)
// ● q : termina il processo dopo aver terminato tutti i figli
// L’intera comunicazione deve avvenire esclusivamente tramite segnali inviati dal processo principale. L’output
// del comando ‘p’ non deve essere ordinato ma deve essere ben chiaro il livello di ogni processo (per esempio
// usando la tabulazione).
// Trasformare il programma precedente per poter gestire due alberi separati. Il parametro accettato sarà dunque
// preceduto da 1 o 2 (esempio: 1c2 à crea figli a profondità 2 sull’albero 1, 2p à stampa il secondo albero).
// Creare un’applicazione in C che gestisca un albero di processi tramite dei segnali. In particolare, il programma,
// una volta lanciato, deve accettare i seguenti comandi:
// ● c n : crea nuovi figli al livello n (ogni processo esistente a livello n-1 ne genera un altro)
// ● k n : termina i figli al livello n
// ● p : stampa in output l’albero dei processi (anche in modo “non ordinato”)
// ● q : termina il processo dopo aver terminato tutti i figli
// L’intera comunicazione deve avvenire esclusivamente tramite segnali inviati dal processo principale. L’output
// del comando ‘p’ non deve essere ordinato ma deve essere ben chiaro il livello di ogni processo (per esempio
// usando la tabulazione).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define MAX_DEPTH               5
#define MAX_CHILDREN_PER_LEVEL  16
#define BUFFER_DIM              256

typedef struct {
    long mtype;
    char mtext[BUFFER_DIM];
} msg_t;

int processShouldTerminateFirst = 0;
int processShouldTerminateSecond = 0;
int processLevel = 0;
int masterProcessesGroupsFirst[MAX_DEPTH] = {0};
int masterProcessesGroupsSecond[MAX_DEPTH] = {0};
int childrenAtLevel[MAX_CHILDREN_PER_LEVEL] = {0};

void printTab(int depth) {
    for(int i = 0; i < depth; i++) {
        printf("\t");
    }
}

void printSignalHandler(int signo) {
    if (signo != SIGALRM) {
        return;
    }

    printTab(processLevel);
    printf("[CHILD %d at LEVEL %d]\n", getpid(), processLevel);
}

// Con SIGUSR1 genera un nuovo processo
// COn SIGUSR2 termina
void childProcessSignalHandlerFirst(int signo) {
    if (signo == SIGUSR1) {
        int pid = fork();

        // Child
        if (!pid) {
            printf("Setting Child gid at %d at level %d\n", masterProcessesGroupsFirst[processLevel + 1], processLevel);
            setpgid(0, masterProcessesGroupsFirst[processLevel + 1]);

            signal(SIGUSR1, childProcessSignalHandlerFirst);
            signal(SIGUSR2, childProcessSignalHandlerFirst);
            signal(SIGALRM, printSignalHandler);

            while(!processShouldTerminateFirst);

            exit(0);
        }
        // Parent
        else {
        }
    }
    else if (signo == SIGUSR2) {
        printf("Child at level %d received SIGUSR2\n", processLevel);
        processShouldTerminateFirst = 1;
    }
}

// Con SIGUSR1 dice ai processi del suo livello di creare un figlio
// Con SIGUSR2 uccide tutti i figli del suo livello
void masterProcessSignalHandlerFirst(int signo) {
    if (signo == SIGUSR1) {
        printf("Master at %d received SIGUSR1\n", processLevel);

        // Ad ogni figlio creato invio un SIGUSR1. Questo segnale indica loro di 
        // un nuovo figlio 
        int noOne = 1;
        for(int i = 0; i < MAX_CHILDREN_PER_LEVEL; i++) {
            if (childrenAtLevel[i] != 0) {
                noOne = 0;
                kill(childrenAtLevel[i], SIGUSR1);
            }
        }
        // Se non ci sono figli, crea il primo
        if (noOne) {
            int gid = getpgid(0);
            int pid = fork();

            int index = 0;
            for(index = 0; index < MAX_CHILDREN_PER_LEVEL; index++) {
                if (childrenAtLevel[index] == 0) {
                    break;
                }
            }

            if (index == MAX_CHILDREN_PER_LEVEL) {
                printf("[ERROR] Max children per level at level %d\n", processLevel);
                return;
            }

            // Child
            if (!pid) {
                setpgid(0, gid);

                signal(SIGUSR1, childProcessSignalHandlerFirst);
                signal(SIGUSR2, childProcessSignalHandlerFirst);
                signal(SIGALRM, printSignalHandler);

                while(!processShouldTerminateFirst);

                exit(0);
            }
            else {
                childrenAtLevel[index] = pid;
            }
        }
    }
    else if (signo == SIGUSR2) {
        printf("MASTER at level %d received SIGUSR2\n", processLevel);
        for(int i = 0; i < MAX_CHILDREN_PER_LEVEL; i++) {
            if(childrenAtLevel[i] != 0) {
                printf("MASTER sending SIGUSR at %d at level %d\n", childrenAtLevel[i], processLevel);
                kill(childrenAtLevel[i], SIGUSR2);
            }
        }
    }
}

// Con SIGUSR1 genera un nuovo processo
// COn SIGUSR2 termina
void childProcessSignalHandlerSecond(int signo) {
    if (signo == SIGUSR1) {
        int pid = fork();

        // Child
        if (!pid) {
            printf("Setting Child gid at %d at level %d\n", masterProcessesGroupsSecond[processLevel + 1], processLevel);
            setpgid(0, masterProcessesGroupsSecond[processLevel + 1]);

            signal(SIGUSR1, childProcessSignalHandlerSecond);
            signal(SIGUSR2, childProcessSignalHandlerSecond);
            signal(SIGALRM, printSignalHandler);

            while(!processShouldTerminateSecond);

            exit(0);
        }
        // Parent
        else {
        }
    }
    else if (signo == SIGUSR2) {
        printf("Child at level %d received SIGUSR2\n", processLevel);
        processShouldTerminateSecond = 1;
    }
}

// Con SIGUSR1 dice ai processi del suo livello di creare un figlio
// Con SIGUSR2 uccide tutti i figli del suo livello
void masterProcessSignalHandlerSecond(int signo) {
    if (signo == SIGUSR1) {
        printf("Master at %d received SIGUSR1\n", processLevel);

        // Ad ogni figlio creato invio un SIGUSR1. Questo segnale indica loro di 
        // un nuovo figlio 
        int noOne = 1;
        for(int i = 0; i < MAX_CHILDREN_PER_LEVEL; i++) {
            if (childrenAtLevel[i] != 0) {
                noOne = 0;
                kill(childrenAtLevel[i], SIGUSR1);
            }
        }
        // Se non ci sono figli, crea il primo
        if (noOne) {
            int gid = getpgid(0);
            int pid = fork();

            int index = 0;
            for(index = 0; index < MAX_CHILDREN_PER_LEVEL; index++) {
                if (childrenAtLevel[index] == 0) {
                    break;
                }
            }

            if (index == MAX_CHILDREN_PER_LEVEL) {
                printf("[ERROR] Max children per level at level %d\n", processLevel);
                return;
            }

            // Child
            if (!pid) {
                setpgid(0, gid);

                signal(SIGUSR1, childProcessSignalHandlerSecond);
                signal(SIGUSR2, childProcessSignalHandlerSecond);
                signal(SIGALRM, printSignalHandler);

                while(!processShouldTerminateSecond);

                exit(0);
            }
            else {
                childrenAtLevel[index] = pid;
            }
        }
    }
    else if (signo == SIGUSR2) {
        printf("MASTER at level %d received SIGUSR2\n", processLevel);
        for(int i = 0; i < MAX_CHILDREN_PER_LEVEL; i++) {
            if(childrenAtLevel[i] != 0) {
                printf("MASTER sending SIGUSR at %d at level %d\n", childrenAtLevel[i], processLevel);
                kill(childrenAtLevel[i], SIGUSR2);
            }
        }
    }
}

int main() {
    // Setup
    int shouldCycle = 1;
    char buffer[BUFFER_DIM] = {0};

    // Primo albero
    // Genero MAX_DEPTH master process
    // Ogni master process gestirà gli eventi del suo livello
    for(int i = 0; i < MAX_DEPTH; i++) {
        int pid = fork();

        // Master Process
        if(!pid) {
            processLevel = i;

            setpgid(getpid(), getpid());

            printf("MASTER at %d | My gid is %d\n", processLevel, getpgid(0));

            signal(SIGUSR1, masterProcessSignalHandlerFirst);
            signal(SIGUSR2, masterProcessSignalHandlerFirst);

            while(!processShouldTerminateFirst);

            return 0;
        }
        // Parent
        else {
            sleep(1);
            masterProcessesGroupsFirst[i] = getpgid(pid);
        }
    }

    // Secondo albero
    // Genero MAX_DEPTH master process
    // Ogni master process gestirà gli eventi del suo livello
    for(int i = 0; i < MAX_DEPTH; i++) {
        int pid = fork();

        // Master Process
        if(!pid) {
            processLevel = i;

            setpgid(getpid(), getpid());

            printf("MASTER at %d | My gid is %d\n", processLevel, getpgid(0));

            signal(SIGUSR1, masterProcessSignalHandlerSecond);
            signal(SIGUSR2, masterProcessSignalHandlerSecond);

            while(!processShouldTerminateSecond);

            return 0;
        }
        // Parent
        else {
            sleep(1);
            masterProcessesGroupsSecond[i] = getpgid(pid);
        }
    }

    // Command loop
    while(shouldCycle) {
        memset(buffer, 0, BUFFER_DIM);

        printf("Next command: ");
        fflush(stdout);

        // Leggo l'input
        int bytes = read(0, buffer, BUFFER_DIM);
        if (bytes == -1) {
            printf("Error on read\n");
            continue;
        }

        // Processo l'input
        int level = atoi(&buffer[1]);
        switch(buffer[0]) {
            // generate child at level
            case 'c':
                // kill(-masterProcessesGroups[level - 1], SIGUSR1);
                break;

            // Kill children at level
            case 'k':
                // kill(-masterProcessesGroups[level], SIGUSR2);
                break;
            // Print the process tree
            case 'p':
                for(int i = 0; i < MAX_DEPTH; i++) {
                    // kill(-masterProcessesGroups[i], SIGALRM);
                }
                break;
            case 'q':
                shouldCycle = 0;
                break;
            default:
                printf("Invalid parameter.\n");
                break;
        }
        sleep(1);
    }

    for(int i = 0; i < MAX_DEPTH; i++) {
        kill(-masterProcessesGroupsFirst[i], SIGUSR2);
        kill(-masterProcessesGroupsSecond[i], SIGUSR2);
    }

    while(wait(NULL) > 0);

    return 0;
}