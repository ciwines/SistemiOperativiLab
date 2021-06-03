#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void* routine(void *param) {
    int index = *(int*)param;

    for(int i = 0; i < 10; i++) {
        printf("Ciao sono il thread %d\n", index);
    }

    return (void*)0;
}

int main() {
    pthread_t tid[5];
    int args[5];

    for(int i = 0; i < 5; i++) {
        args[i] = i;

        // crea ed esegue il thread
        pthread_create(&tid[i], NULL, routine, (void*)&args[i]);
    }

    sleep(5);

    return 0;
}