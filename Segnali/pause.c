#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void customHandler(int signo) {
    printf("Child can go\n");
}

int main() {
    signal(SIGUSR1, customHandler);

    int pid = fork();
    if (! pid) {
        printf("Child paused\n");
        pause();
        printf("Child doing stuff...\n");
        sleep(3);
        printf("Child done...\n");
        return 0;
    }

    printf("Doing stuff...\n");
    sleep(2);
    printf("Done, free the child\n");
    kill(pid, SIGUSR1);

    while(wait(NULL) > 0);

    return 0;
}