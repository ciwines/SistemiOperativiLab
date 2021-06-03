#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    int pid = fork();

    if (!pid) {
        printf("Figlio creato.\n");

        setpgid(0, getpid());

        sleep(5);

        printf("Figlio terminato.\n");
        return 0;
    }

    sleep(1); // necessario altrimenti il padre potrebbe terminare prima che il figlio cambi gid
    waitpid(-pid, NULL, 0);

    return 0;
}