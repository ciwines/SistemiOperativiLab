#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex;
int counter = 0;

void* firstRoutine(void *params) {
    pthread_mutex_lock(&mutex);
    counter = 0;
    int offset = 1;
    printf("[T1] Si aspetta %d | Ricevuto -> Counter = %d\n", 0, counter);

    for(int i = 0; i < 0x00FF0000; i++);

    counter += offset;

    pthread_mutex_unlock(&mutex);

    printf("[T1] Si aspetta %d | Ricevuto -> Counter = %d\n", 1, counter);

    return (void*)0;
}

void* secondRoutine(void *params) {
    pthread_mutex_lock(&mutex);
    counter = 0;
    int offset = 10;
    printf("[T2] Si aspetta %d | Ricevuto -> Counter = %d\n", 0, counter);

    for(int i = 0; i < 0x0FFF0000; i++);

    counter += offset;

    pthread_mutex_unlock(&mutex);
    printf("[T2] Si aspetta %d | Ricevuto -> Counter = %d\n", 10, counter);

    return (void*)0;
}

int main() {
    pthread_t first;
    pthread_t second;

    pthread_mutex_init(&mutex, NULL);

    pthread_create(&first, NULL, firstRoutine, NULL);
    pthread_create(&second, NULL, secondRoutine, NULL);

    pthread_join(first, NULL);
    pthread_join(second, NULL);

    pthread_mutex_destroy(&mutex);

    return 0;
}