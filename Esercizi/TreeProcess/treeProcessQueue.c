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
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>

///////////////////////////////////////////////////////////////////////////////

#define MAX_DEPTH               5
#define MAX_CHILDREN_PER_LEVEL  16
#define BUFFER_DIM              256

///////////////////////////////////////////////////////////////////////////////

typedef struct {
    long mtype;
    char mtext[BUFFER_DIM];
} msg_t;

///////////////////////////////////////////////////////////////////////////////

int processLevel = 0;
int childrenNum = 0;

///////////////////////////////////////////////////////////////////////////////

void childProcessRoutine(int depth, int queueId);
int createChild(int depth, int queueId) ;
void masterProcessRoutine(int depth, int masterQueueId, int childrenQueueId);
void printTab(int depth);
int createQueue(const char *path, int index);
int sendMsg(int queueid, int depth, const char *buffer, int maxLen);
int recvMsg(int queueid, int depth, char *buffer, int maxLen);

///////////////////////////////////////////////////////////////////////////////

void printTab(int depth) {
    for(int i = 0; i < depth; i++) {
        printf("\t");
    }
}

int createQueue(const char *path, int index) {
    unlink(path);
    creat(path, 0777 | O_CREAT | O_RDWR);

    key_t key = ftok(path, index);
    if (key == -1) {
        perror("Error on ftok");
        exit(1);
    }

    int id = msgget(key, 0777 | IPC_CREAT);

    return id;
}

int sendMsg(int queueid, int depth, const char *buffer, int maxLen) {
    msg_t message;

    message.mtype = depth;
    strcpy(message.mtext, buffer);

    // printf("Sending %s to type %d\n", buffer, depth);

    return msgsnd(queueid, &message, maxLen, 0);
}

int sendMultipleMsg(int queueid, int depth, const char *buffer, int maxLen, int times) {
    // printf("Brodo to %d\n", depth);
    for (int i = 0; i < times; i++) {
        if (sendMsg(queueid, depth, buffer, maxLen) == -1) {
            return -1;
        }
    }

    return 1;
}

int recvMsg(int queueid, int depth, char *buffer, int maxLen) {
    msg_t message;

    memset(message.mtext, 0, maxLen);

    int ret = msgrcv(queueid, &message, maxLen, depth, 0);
    strcpy(buffer, message.mtext);

    return ret;
}

void updateChildrenAtLevel(int *map, int depth) {
    if (depth == 0) {
        map[depth] += 1;
    }
    else {
        if (map[depth - 1]!= 0) {
            map[depth] += map[depth - 1];
        }
    }
    
    // for(int i = 0; i < MAX_DEPTH; i++) {
    //     printf("Children at level %d = %d\n", i, map[i]);
    // }
}

///////////////////////////////////////////////////////////////////////////////

void childProcessRoutine(int depth, int queueId) {
    processLevel = depth;
    char buffer[BUFFER_DIM] = {0};
    int queueType = depth + 1;
    int shouldTerminate = 0;

    printf("[CHILD] Created at depth %d with type %d\n", depth, queueType);

    while (!shouldTerminate) {
        memset(buffer, 0, BUFFER_DIM);

        // printf("[CHILD at %d] Waiting input\n", depth);

        if (recvMsg(queueId, queueType, buffer, BUFFER_DIM) == -1) {
            perror("[CHILD] Error on recv");
            continue;
        }

        // printf("[CHILD at %d] Received %s\n", depth, buffer);
        
        // Parso l'azione
        switch(buffer[0]) {
            // Create new child
            case 'c':
                createChild(depth + 1, queueId);
                break;
            // Terminate
            case 'k':
                shouldTerminate = 1;
                break;
            case 'p':
                printTab(depth);
                printf("[CHILD %d at depth %d]\n", getpid(), depth);
                break;
            case 'q':
                shouldTerminate = 1;
                break;
        }

    }

    exit(0);
}

int createChild(int depth, int queueId) {
    int pid = fork();

    // Child
    if (!pid) {
        childProcessRoutine(depth, queueId);
        exit(0);
    }
    // Parent
    else {
        // ...
    }

    return pid;
}

void masterProcessRoutine(int depth, int masterQueueId, int childrenQueueId) {
    processLevel = depth;
    char buffer[BUFFER_DIM] = {0};
    int queueType = depth + 1;

    printf("[MASTER] Created at depth %d with type %d\n", depth, queueType);

    while (1) {
        memset(buffer, 0, BUFFER_DIM);

        if (recvMsg(masterQueueId, queueType, buffer, BUFFER_DIM) == -1) {
            perror("[MASTER] Error on recv");
            continue;
        }

        // printf("[MASTER] Received %c\n", buffer[0]);
        
        // Parso l'azione
        switch(buffer[0]) {
            // Create new child - Case where the master process is at level 0
            case 'c':
                if (queueType == 1) {
                    createChild(depth, childrenQueueId);
                }
                break;
            // Create new child at level processLevel + 1 - Case where the master process has already at least one child
            case 'n':
                if (sendMsg(childrenQueueId, queueType, "c", 1) == -1) {
                    perror("[MASTER] Error on sendMsg");
                    continue;
                }
                break;
            // Create children at level
            case 'k':
                if (sendMsg(childrenQueueId, queueType, "k", 1) == -1) {
                    perror("[MASTER] Error on sendMsg");
                    continue;
                }

                break;
            case 'p':
                if (sendMsg(childrenQueueId, queueType, "p", 1) == -1) {
                    perror("[MASTER] Error on sendMsg");
                    continue;
                }
                break;
            case 'q':
                if (sendMsg(childrenQueueId, queueType, "q", 1) == -1) {
                    perror("[MASTER] Error on sendMsg");
                    continue;
                }
                break;
        }

        fflush(stdout);
    }

    exit(0);
}

///////////////////////////////////////////////////////////////////////////////

int main() {
    // Creo due code. 
    // /tmp/masterQueue per la comunicazione MAIN -> CHILDREN
    // /tmp/childrenQueue per la comunicazione MASTER -> CHILDREN
    const char *masterQueuePath = "/tmp/masterQueue";
    const char *childrenQueuePath = "/tmp/childrenQueue";

    int masterQueueId = createQueue(masterQueuePath, 0);
    int childrenQueueId = createQueue(childrenQueuePath, 0);

    printf("Master %d | Children %d\n", masterQueueId, childrenQueueId);

    // Creo i processi master
    int masterProcessPids[MAX_DEPTH] = {0};

    for (int i = 0; i < MAX_DEPTH; i++) {
        int pid = fork();

        // Master
        if (!pid) {
            masterProcessRoutine(i, masterQueueId, childrenQueueId);
        }
        // Main
        else {
            masterProcessPids[i] = pid;
        }
    }
    sleep(1);

    // Loop
    int shouldCycle = 1;
    char buffer[BUFFER_DIM] = {0};
    int childrenAtLevel[MAX_DEPTH] = {0};

    while (shouldCycle) {
        // Setup
        memset(buffer, 0, BUFFER_DIM);
        
        printf("> ");
        fflush(stdout);

        // Richiedo l'input
        int bytes = read(0, buffer, BUFFER_DIM);

        if (bytes == -1) {
            perror("Error on read");
            continue;
        }
        else if (bytes == 0) {
            continue;
        }

        // Parso l'input
        int depth = atoi(&buffer[1]); // salto il primo carattere
        int type = depth + 1;

        switch(buffer[0]) {
            case 'c':
                // printf("Sending c to depth %d with type %d\n", depth, type);
                if (type == 1 && sendMsg(masterQueueId, type, "c", 1) == -1) {
                    perror("Error on c command");
                    continue;
                } else if (type > 1 && sendMultipleMsg(masterQueueId, type - 1, "n", 1, childrenAtLevel[depth - 1]) == -1) {
                    perror("Error on c command");
                    continue;
                }

                updateChildrenAtLevel(childrenAtLevel, depth);
                break;
            case 'k':
                if (sendMultipleMsg(masterQueueId, type, "k", 1, childrenAtLevel[depth]) == -1) {
                    perror("Error on k command");
                    continue;
                }
                childrenAtLevel[depth] = 0;
                break;
            case 'p':
                for (int i = 1; i < MAX_DEPTH + 1; i++) {
                    if (sendMultipleMsg(masterQueueId, i, "p", 1, childrenAtLevel[i - 1]) == -1) {
                        perror("Error on c command");
                    }
                }
                break;
            case 'q':
                for (int i = 1; i < MAX_DEPTH + 1; i++) {
                    if (sendMultipleMsg(masterQueueId, i, "q", 1, childrenAtLevel[i - 1]) == -1) {
                        perror("Error on c command");
                    }
                }
                shouldCycle = 0;
                break;
        }

        sleep(1);
    }

    // Termino tutti i master process
    for (int i = 0; i < MAX_DEPTH; i++) {
        kill(masterProcessPids[i], SIGTERM);
    }

    while(wait(NULL) > 0);

    return 0;
}