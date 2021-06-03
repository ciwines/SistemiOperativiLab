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

#define FIFO_PATH           "/tmp/communication"
#define BUFFER_DIM          (256 * 4)
#define MSG_DIM             256

typedef struct {
    int fromPid;
    int toPid;
    char message[MSG_DIM];
} command_t;

// Ritorna il 1 se il buffer è un ordine di EXEC, 0 altrimenti
int isExec(const char *buffer) {
    // Controllo che contenga 
    if(strstr(buffer, "EXEC") != NULL) {
        return 1;
    }

    return 0;
}

// Ritorna il 1 se il buffer è un ordine di MSG, 0 altrimenti
int isMsg(const char *buffer) {
    // Controllo che contenga 
    if(strstr(buffer, "MSG") != NULL) {
        return 1;
    }

    return 0;
}

// Parsa un comando EXEC
command_t parseExec(const char *buffer, int bufferLen) {
    int index = 0;
    char firstNum[MSG_DIM] = {0};
    char secondNum[MSG_DIM] = {0};
    command_t ret;

    // Setup
    memset(ret.message, 0, MSG_DIM);

    // Arrivo al primo numero
    while(buffer[index] != ' ') {
        index++;
    }
    index++;

    // A questo punto parte il primo numero
    int numIndex = 0;
    while(buffer[index] != ' ') {
        firstNum[numIndex] = buffer[index];
        numIndex++;
        index++;
    }
    index++;

    // A questo punto parte il secondo numero
    numIndex = 0;
    while(buffer[index] != ' ') {
        secondNum[numIndex] = buffer[index];
        numIndex++;
        index++;
    }
    index++;

    // A questo punto parte il messaggio
    numIndex = 0;
    while(buffer[index] != '\n') {
        ret.message[numIndex] = buffer[index];
        numIndex++;
        index++;
    }

    // converto i numeri in int
    int nPid = atoi(firstNum);
    int mPid = atoi(secondNum);

    ret.fromPid = nPid;
    ret.toPid = mPid;

    return ret;
}

void sendMsgOtherSlave(int pipefd, int toPid, const char *message) {
    char buffer[BUFFER_DIM] = {0}; 

    snprintf(buffer, BUFFER_DIM, "MSG %d %s\n", toPid, message);
    // printf("[SLAVE] MSG eseguita -> %s\n", buffer);

    write(pipefd, buffer, strlen(buffer));

    // printf("[SLAVE] Msg Order sent\n");
}

command_t parseMsg(const char *buffer, int bufferLen) {
    int index = 0;
    char num[MSG_DIM] = {0};
    command_t ret;

    // Setup
    memset(ret.message, 0, MSG_DIM);

    // Arrivo al primo numero
    while(buffer[index] != ' ') {
        index++;
    }
    index++;

    // A questo punto parte il primo numero
    int numIndex = 0;
    while(buffer[index] != ' ') {
        num[numIndex] = buffer[index];
        numIndex++;
        index++;
    }
    index++;

    // A questo punto parte il messaggio
    numIndex = 0;
    while(buffer[index] != '\n') {
        ret.message[numIndex] = buffer[index];
        numIndex++;
        index++;
    }

    // converto i numeri in int
    int toPid = atoi(num);

    ret.fromPid = 0;
    ret.toPid = toPid;

    return ret;
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Bro devi dirmi qual è il mio indice please :(\n");
        exit(0);
    }

    // Apro la fifo in scrittura
    int pipefd = open(FIFO_PATH, O_RDWR);
    char buffer[BUFFER_DIM] = {0};
    int currentPid = getpid();
    int slaveIndex = atoi(argv[1]);

    // printf("[SLAVE] Il mio pid è %d\n", currentPid);

    while(1) {
        memset(buffer, 0, BUFFER_DIM);

        int bytes = read(pipefd, buffer, BUFFER_DIM);
        if (bytes == -1) {
            perror("Errore durante la lettura da FIFO: ");
            continue;
        } 
        else if(bytes == 0) {
            continue;
        }

        // printf("[SLAVE] Letto %d bytes\n", bytes);

        if(isExec(buffer)) {
            // printf("[SLAVE] Is exec\n");
            command_t cmd = parseExec(buffer, strlen(buffer));

            // printf("Slave legge dalla coda -> EXEC %d %d %s\n", cmd.fromPid, cmd.toPid, cmd.message);
            if(currentPid == cmd.fromPid) {
                printf("[%d] Sent '%s' to %d\n", slaveIndex, cmd.message, cmd.toPid);
                sendMsgOtherSlave(pipefd, cmd.toPid, cmd.message);
            }
        }
        else if(isMsg(buffer)) {
            command_t cmd = parseMsg(buffer, strlen(buffer));

            // printf("Slave legge dalla coda -> MSG %d %s\n", cmd.toPid, cmd.message);

            if(cmd.toPid == currentPid) {
                printf("[%d] Received '%s'\n", slaveIndex, cmd.message);
            }
        }
    }

    // Chiudo la pipe
    close(pipefd);

    return 0;
}