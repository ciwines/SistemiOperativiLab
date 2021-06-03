#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

// Crea una pipe fifo di nome argv[1]
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./mkfifo <pipe name>\n");
        exit(1);
    }

    mkfifo(argv[1], S_IRUSR | S_IWUSR);

    int pid = fork();
    if (!pid) {
        int pipeFd = open(argv[1], O_RDONLY);

        // .....

        close(pipeFd);
    }
    else {
        int pipeFd = open(argv[1], O_WRONLY);

        // .....

        close(pipeFd);
    }

    return 0;
}