#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void* routine(void *param) {
    int index = *(int*)param;

    if (index == 1) {
        pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    }
    else {
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    }

    printf("Tread avviato!\n");
    sleep(m3);
    printf("Thread terminato\n");

    return (void*)0;
}

int main() {
    pthread_t first;
    pthread_t second;
    int firstArg = 1;
    int secondArg = 0;

    pthread_create(&first, NULL, routine, &firstArg);
    sleep(1);
    printf("Termino thread 1\n");
    pthread_cancel(first);
    sleep(5);

    pthread_create(&second, NULL, routine, &secondArg);
    sleep(1);
    printf("Termino thread 2\n");
    pthread_cancel(second);
    sleep(5);

    return 0;
}