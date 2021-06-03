#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

void customUser1(int signo) {
    printf("Sigusr1 invoked\n");
}


// possiamo notare che SIGUSR1 viene bloccato, infatti il figlio 
// non riceve il segnale
int main() {
    signal(SIGUSR1, customUser1);

    sigset_t new;
    sigset_t old;
    sigset_t pending;

    // set the new set, blocking the SIGUSR1 signal
    sigemptyset(&new);
    sigaddset(&new, SIGUSR1);

    // set the new mask
    sigprocmask(SIG_SETMASK, &new, &old);

    // print the differences
    printf("New set: %d\nOld set: %d\n", new, old);

    // verify the new set
    int pid = fork();
    if (! pid) {
        printf("Child sleeping.\n");
        sleep(5);

        // looking for pending signals
        sigpending(&pending);

        printf("Child pending signals -> %d\n", pending);
        printf("Is SIGUSR1? %d\n", sigismember(&pending, SIGUSR1));

        printf("Child ending.\n");
        return 0;
    }

    printf("Father sleeping\n");
    sleep(2);
    printf("Father sending signal.\n");
    kill(pid, SIGUSR1);
    printf("Signal sent.\nWaiting for child\n");

    while(wait(NULL) > 0);

    return 0;
}