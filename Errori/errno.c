#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>

#define MAX_GID 100000

int main() {
    srand(time(NULL));

    int pid = fork();

    if (!pid) {
        // genero un gid a caso
        int newGid = rand() % MAX_GID;

        // setto il nuovo gid
        if (setpgid(0, newGid) != 0) {
            // stampo a schermo l'errore
            perror("Errore nel figlio");
            return 1;
        }

        printf("Figlio completato.\n");
    }

    waitpid(pid, NULL, 0);

    return 0;
}