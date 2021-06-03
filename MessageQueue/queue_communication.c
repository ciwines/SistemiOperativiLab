#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#define MSG_LEN 80
#define MSG_TYPE 33

typedef struct {
    long mtype;
    char mtext[MSG_LEN];
} msgp_t;

int main() {
    const char *text = "Ciao sono un messaggio in una queue";
    msgp_t msgSnd;
    msgp_t msgRcv;

    msgSnd.mtype = MSG_TYPE;
    strncpy(msgSnd.mtext, text, strlen(text));

    // Creo la coda
    remove("/tmp/queue");
    creat("/tmp/queue", 0777);

    // creo la chiave e ricavo il queue id
    key_t queueKey = ftok("/tmp/queue", 1);
    int queueId = msgget(queueKey, 0777 | IPC_CREAT | IPC_EXCL);

    // Scrivo e poi leggo
    int fail = msgsnd(queueId, &msgSnd, sizeof(msgSnd.mtext), 0);
    if (fail == -1) {
        printf("Errore durante l'invio del messaggio\n");
        perror("Dio");
        exit(1);
    }

    fail = msgrcv(queueId, &msgRcv, sizeof(msgRcv.mtext), MSG_TYPE, 0);
    if (fail == -1) {
        printf("Errore durante la lettura dalla queue\n");
        perror("Dio due");
        exit(1);
    }

    printf("Nella coda è stato scritto -> %s\n", msgRcv.mtext);

    return 0;
}