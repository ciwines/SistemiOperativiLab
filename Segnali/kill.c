#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int shouldWait = 1;

void alarmHandler(int signo) {
    printf("You're now free!\n");
    shouldWait = 0;
}

int main(int argc, char *arv[]) {
    signal(SIGALRM, alarmHandler);

    int pid = fork();
    if (!pid) {
        while(shouldWait);
        printf("Grazie amaaa\n");
        return 0;
    }
    else {
        printf("aspetto 5 secondi\n");
        sleep(5);
        printf("ora sei libero, figlio mio!\n");
        kill(pid, SIGALRM);
    }

    while(wait(NULL) > 0);

    return 0;
}