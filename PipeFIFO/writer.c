#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int readline(char *buffer, int maxlen) {
    int chars = 0;
    char c;
    
    while((c = getchar()) != EOF) {
        if (chars == maxlen) {
            break;
        }

        buffer[chars] = c;
        chars++;

        if (c == '\n') {
            break;
        }
    }

    return chars;
}

int writeFIFO(const char* fifoName, const char *message, int len) {
    int fd = open(fifoName, O_WRONLY);
    write(fd, message, len);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: writer <named pipe>\n");
        exit(1);
    }

    // Se non esiste la pipe, la crea
    mkfifo(argv[1], S_IWUSR | S_IRUSR);

    printf("oke\n");

    // Loop di comunicazione
    size_t dim = 256;
    int len;
    char *buffer = malloc(dim);
    while(1) {
        memset(buffer, 0, dim);

        // Leggo l'input
        printf("> ");
        readline(buffer, dim);

        // Invio il messaggio alla pipe
        writeFIFO(argv[1], buffer, strlen(buffer));

        break;
    }

    // Libero la memoria allocata
    free(buffer);

    return 0;
}