#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define N_CHILD 5
#define TARGET_GROUP 9944

void customHandler(int signo) {
    printf("Child now free\n");
}

int main() {    
    signal(SIGUSR1, customHandler);

    int pids[N_CHILD];

    for (int i = 0; i < N_CHILD; i++) {
        int pid = fork();

        if (!pid) {
            if(setpgid(0, getpid()) != 0) {
                perror("Error");
                return 1;
            }

            pid_t currentGid;

            // get the current gid of the child
            currentGid = getpgid(0); // 0 means the process itself
            printf("Child gid -> %d\n", currentGid);

            printf("Child created\n");
            pause();

            return 0;
        }
        else {
            pids[i] = pid;
        }
    }

    sleep(3);

    for(int i = 0; i < N_CHILD; i++) {
        printf("Father killing %d\n", pids[i]);
        kill(-pids[i], SIGUSR1);
    }

    while(wait(NULL) > 0);

    return 0;
}