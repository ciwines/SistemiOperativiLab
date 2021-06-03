#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_DIM 256

int main() {
    int pid;
    int pipefd[2];

    // creo la pipe
    // pipefd[0] lettura - pipefd[1] scrittura
    pipe(pipefd);

    // forko
    // il padre scrive al figlio
    pid = fork();
    if (!pid) {
        close(pipefd[1]); // buona norma all'inizio

        char buffer[BUFFER_DIM] = {0};

        // leggo in maniera bloccante dal padre
        int bytes = read(pipefd[0], buffer, BUFFER_DIM);

        printf("Il padre ha inviato al figlio: %s\n", buffer);

        close(pipefd[0]); // necessario

        return 0;
    }
    
    else {
        close(pipefd[0]); // buona norma all'inizio
        char buffer[BUFFER_DIM] = {0};

        printf("Cosa vuoi dire al figlio? ");
        scanf("%s", buffer);

        write(pipefd[1], buffer,strlen(buffer));

        printf("Test inviato al figlio.\n");

        close(pipefd[1]); // necessario
    }

    while(wait(NULL) > 0);

    return 0;
}