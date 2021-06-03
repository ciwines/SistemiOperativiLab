#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int writeToPipe(int fd, const char *who, int *maxSize) {
    char* buffer; 
    int ret = 1;

    printf("[Srcittura] %s > ");
    getline(buffer, maxSize, stdin);
    printf("Hai scritto -> %s\n", buffer);

    write(fd, buffer, strlen(buffer));

    if(!strncmp(buffer, "quit", 4)) {
        ret = 0;
    }

    free(buffer);

    return ret;
}

int readFromPipe(int fd, const char *who, int maxSize) {
    char *buffer = malloc(maxSize);
    int bytes = read(fd, buffer, maxSize);
    int ret = 1;

    if(bytes < 0) {
        free(buffer);
        return 0;
    }

    printf("[Lettura] %s | %s\n", who, buffer);
    
    if(!strncmp(buffer, "quit", 4)) {
        ret = 0;
    }

    free(buffer);

    return ret;
}

int main() {
    int pid;
    int ppipe[2]; // parent pipe
    int cpipe[2]; // child pipe
    int cycle = 1;
    const int BUFFER_DIM = 256;

    // creo 2 pipe
    // pipefd[0] lettura - pipefd[1] scrittura
    pipe(ppipe);
    pipe(cpipe);

    // forko
    // il padre scrive al figlio
    pid = fork();
    if (!pid) {
        close(ppipe[1]); // buona norma all'inizio
        close(cpipe[0]); // buona norma all'inizio

        do {
            cycle = readFromPipe(ppipe[0], "Figlio", BUFFER_DIM);
            cycle = writeToPipe(cpipe[1], "Figlio", &BUFFER_DIM);
        } while(cycle);

        close(ppipe[0]); // necessario
        close(cpipe[1]); // necessario

        return 0;
    }
    
    else {
        close(ppipe[0]); // buona norma all'inizio
        close(cpipe[1]); // buona norma all'inizio

        char buffer[BUFFER_DIM];

        do {
            cycle = writeToPipe(ppipe[1], "Padre", &BUFFER_DIM);
            cycle = readFromPipe(cpipe[0], "Padre", BUFFER_DIM);
        } while(cycle);

        close(ppipe[1]); // necessario
        close(cpipe[0]); // necessario
    }

    while(wait(NULL) > 0);

    return 0;
}