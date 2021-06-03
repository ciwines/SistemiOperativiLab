/**
 * Nella pipe verranno messi due comandi:
 *  - EXEC nPid mPid msg\n
 *  - ORDER to msg\n
 * EXEC viene usata dal master per dire a nPid di iniviare a mPid il messaggio msg.
 * MSG viene usata da nPid per comunicare a mPid di leggere il messaggio msg
 * Il terminatore di frase è sempre \n
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FIFO_PATH       "/tmp/communication"
#define BUFFER_DIM      256
#define EXEC_BUFFER_DIM (3*BUFFER_DIM + 5) // 3 * BUFFER_DIM + strlen("EXEC ")
#define MAX_CHILDREN    20

int pids[MAX_CHILDREN] = {0};

void usage() {
    printf("Usage: ./master.out <children num>\n");
    exit(1);
}

void myGetline(char *buffer, size_t len) {
    char c;
    int i = 0;

    while((c = getchar()) != '\n' && i < len) {
        buffer[i] = c;
        i++;
    }
}

void sendOrderToSlave(int pipefd, int nPid, int mPid, const char *text) {
    char buffer[EXEC_BUFFER_DIM] = {0}; 

    snprintf(buffer, EXEC_BUFFER_DIM, "EXEC %d %d %s\n", nPid, mPid, text);
    printf("EXEC eseguita -> %s\n", buffer);

    write(pipefd, buffer, strlen(buffer));

    printf("[MASTER] Order sent\n");
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        usage();
    }

    // Creo le fifo
    remove(FIFO_PATH);
    creat(FIFO_PATH, 0777);
    mkfifo(FIFO_PATH, S_IRUSR | S_IWUSR);

    // La apro in scrittura
    int pipefd = open(FIFO_PATH, O_WRONLY);

    // Creo argv[1] figli
    int children = atoi(argv[1]);

    printf("Creating slaves\n");
    for(int i = 0; i < children; i++) {
        int pid = fork();

        // Figlio
        if (!pid) {
            execlp("./slave.out", i, NULL);
            return 0;
        }
        // Padre
        else {
            pids[i] = pid;
        }
    }

    // Master loop
    int cycle = 1;
    char firstNum[BUFFER_DIM] = {0};
    char secondNum[BUFFER_DIM] = {0};
    char direction;
    char message[BUFFER_DIM] = {0};
    int cmdValid = 1;
    int firstPid = 0;
    int secondPid = 0;

    while(cycle) {  
        memset(firstNum, 0, BUFFER_DIM);
        memset(secondNum, 0, BUFFER_DIM);
        memset(message, 0, BUFFER_DIM);
        cmdValid = 1;

        // Processa il primo numero e la direzione
        char c;
        int i = 0;
        
        while((c = getchar()) != EOF) {
            if(c == '<' || c == '>') {
                direction = c;
                break;
            }

            if (c == '\n') {
                printf("Comando non valido.\n");
                cmdValid = 0;
                break;
            }

            firstNum[i] = c;
            i++;
        } 

        if (!cmdValid) {
            continue;
        }

        // Processa il secondo numero e la direzione
        i = 0;

        while((c = getchar()) != EOF) {
            if(c == '\n') {
                printf("Comando non valido.\n");
                cmdValid = 0;
                break;
            }

            if (c == ' ') {
                break;
            }

            secondNum[i] = c;
            i++;
        } 

        if (!cmdValid) {
            continue;
        }

        // Processo il messaggio
        i = 0;

        while((c = getchar()) != '\n') {
            message[i] = c;
            i++;
        } 

        // Ricavo i numeri
        int first = atoi(firstNum);
        int second = atoi(secondNum);

        if (first >= children || first < 0 || second >= children || second < 0) {
            printf("Comando non valido.\n");
            continue;
        }

        ///////////////////////////////////////////////////////////////////////
        
        // printf("CMD = %d %c %d %s\n", first, direction, second, message);
        
        if (direction == '>') {
            // firstPid = pids[first];
            // secondPid = pids[second];
            firstPid = first;
            secondPid = second;
        } 
        else {
            // firstPid = pids[second];
            // secondPid = pids[first];

            firstPid = second;
            secondPid = first;
        }

        sendOrderToSlave(pipefd, firstPid, secondPid, message);
    }   

    // Chiudo la pipe
    close(pipefd);

    return 0;
}