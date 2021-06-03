#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    pid_t fatherGid;
    
    int pid = fork();

    if (!pid) {
        pid_t currentGid;

        // get the current gid of the child
        currentGid = getpgid(0); // 0 means the process itself
        printf("Child gid -> %d\n", currentGid);

        // set a new gid
        pid_t currentPid = getpid();
        setpgid(currentPid, currentPid);

        // get the current gid of the child
        currentGid = getpgid(0); // 0 means the process itself
        printf("Child gid -> %d\n", currentGid);

        return 0;
    }

    // get the current gid of the child
    fatherGid = getpgid(0);
    printf("Parent gid -> %d\n", fatherGid);

    while(wait(NULL) > 0);

    return 0;
}