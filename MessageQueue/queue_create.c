#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    // Creo il file /tmp/queue1
    open("/tmp/queue1", O_CREAT);

    // Genero la chiave partendo dalla path desiderata
    // A questo punto però il file specificato come path
    // deve essere già stato creato!
    key_t queueKey = ftok("/tmp/queue1", 1); 
    // key_t queueKey2 = ftok("/tmp/queue2", 2); .....

    // Ricavo il queue id
    int queueId = msgget(queueKey, 0777); // Tutti i permessi

    return 0;
}   