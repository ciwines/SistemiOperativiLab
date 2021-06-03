#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void customAlarm(int signo) {
    printf("Process taking too long...\n");
    exit(1);
}

int main() {
    signal(SIGALRM, customAlarm);

    // set the alarm to 3 seconds
    alarm(3);

    // start an heavy load
    printf("Starting computation\n");
    unsigned long int sum = 0;
    for (int i = 0; i < 100000000000; i++) {
        sum += i;
    }
    printf("Computation ended.\n");

    return 0;
}