#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex;

int main() {
    pthread_mutex_init(&mutex, NULL);

    // Inizializza e fa cosa con thread

    pthread_mutex_destroy(&mutex);

    return 0;
}