#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// definizioni
#define MAX_PROC    10

// variabili GLOBBBBALI
int numSigusr1[MAX_PROC] = {0};
int numSigusr2[MAX_PROC] = {0};
int indexToProcMap[MAX_PROC] = {0}; // index -> pid
int lastMaxpIndex = 0;
int shouldRun = 1;

// From Naimoliz
// check if application run in foreground
int foreground() {
    // tcgetpgrp(fd):
    // process group ID of foreground process group for terminal of “fd”
    // (should be the controlling terminal of the calling process)
    // getpgrp():
    // returns process group ID of the calling process
    int fg = 0;
    if (getpgrp() == tcgetpgrp(STDOUT_FILENO)) {
        fg = 1;
    }
    return fg;
}

void customHandler(int signo, siginfo_t *si, void *data) {
    int procPid = si->si_pid;
    int index = -1;

    // cerca il pid nella ""mappa""
    // se lo trova, assegna ad index il suo indice
    for(int i = 0; i < lastMaxpIndex; i++) {
        if (indexToProcMap[i] == procPid) {
            index = i;
            break;
        }
    }

    // altrimenti ne crea uno nuovo
    if (index == -1 && lastMaxpIndex < MAX_PROC) {
        index = lastMaxpIndex;
        lastMaxpIndex++;

        indexToProcMap[index] = procPid;
    }

    if (signo == SIGUSR1) {
        numSigusr1[index]++;
        printf("Bro %d ha detto SIGUSR1\n", procPid);
    }
    else if (signo == SIGUSR2) {
        numSigusr2[index]++;
        printf("Bro %d ha detto SIGUSR2\n", procPid);
    }
}

void stopCycling(int signo) {
    shouldRun = 0;
}

int main() {
    printf("Buongiorno Naimoliz, il mio PID è %d\n", getpid());
    
    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = customHandler;

    sigaction(SIGUSR1, &action, 0);
    sigaction(SIGUSR2, &action, 0);

    signal(SIGTERM, stopCycling);
    signal(SIGKILL, stopCycling);

    while(shouldRun);

    for(int i = 0; i < lastMaxpIndex; i++) {
        printf("PID[%d] ha ricevuto %d SIGUSR1 e %d SIGUSR2\n", indexToProcMap[i], numSigusr1[i], numSigusr2[i]);
    }

    return 0;
}