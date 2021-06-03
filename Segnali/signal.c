#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int canGo = 0;

void customHandler(int signo) {
    switch(signo) {
        case SIGTSTP:
            printf("CTRL+Z Boy.\n");
            canGo = 1;
            break;
        case SIGINT:
            printf("CTRL+C Boy.\n");
            canGo = 1;
            break;
        default:
            printf("Bro %d\n", signo);
    }
}

void childHandler(int signo) {
    if (signo == SIGCHLD) {
        printf("Bro tuo figlio è morto.\n");
    }
}

int main(int argc, char *arv[]) {
    signal(SIGALRM, SIG_DFL);  // default handler
    signal(SIGINT, customHandler); // custom handler
    signal(SIGTERM, SIG_IGN); // ignore signal
    signal(SIGTSTP, customHandler); // custom handler

    signal(SIGCHLD, childHandler);

    while(!canGo);

    int pid = fork();
    if (!pid) {
        return 0;
    }

    while(wait(NULL) > 0);

    return 0;
}