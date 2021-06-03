#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_DIM      32
#define MAX_CHILDREN    15
#define PIPE_READ       0
#define PIPE_WRITE      1  
#define MAX_RAND        1000000

void usage() {
    printf("Usage: communication <child_num>\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    // Controllo sui parametri
    if (argc != 2) {
        usage();
    }

    int children = atoi(argv[1]);
    int pids[MAX_CHILDREN] = {0};
    char buffer[BUFFER_DIM] = {0};
    int inputCycle = 1;

    int parentReadFromChild[MAX_CHILDREN][2] = {0};
    int parentWritesToChild[MAX_CHILDREN][2] = {0};
    int childReadFromParent[MAX_CHILDREN][2] = {0};
    int childWritesToParent[MAX_CHILDREN][2] = {0};

    // Controllo su argv[1]
    if (!children || children > MAX_CHILDREN) {
        usage();
    }

    // Genera i argv[i] figli
    printf("Creating %d processes\n\n", children);

    for(int i = 0; i < children; i++) {
        // Creo le pipe relative al processo i
        pipe(parentReadFromChild[i]);
        pipe(parentWritesToChild[i]);
        pipe(childReadFromParent[i]);
        pipe(childWritesToParent[i]);

        // Creo il figlio
        int currPid = fork();

        // Figlio
        if(!currPid) {
            // Chiude le pipe non necessarie
            close(parentWritesToChild[i][PIPE_WRITE]);
            close(parentReadFromChild[i][PIPE_READ]);
            close(childWritesToParent[i][PIPE_READ]);
            close(childReadFromParent[i][PIPE_WRITE]);

            // Inizializza il RNG
            srand(time(NULL));

            // Fa cosa
            while (1) {
                memset(buffer, 0, BUFFER_DIM);

                // Leggo il comando dal parent
                int bytes = read(parentWritesToChild[i][PIPE_READ], buffer, 1);

                if (bytes == -1) {
                    printf("Error on read\n\n");
                    continue;
                }
                
                // Numero casuale
                if(! strncmp(buffer, "r", 1)) {
                    // Genera e invia il numero casuale
                    printf("Child computing random number.\n");
                    int random = rand() % MAX_RAND;

                    // Lo converto in stringa
                    memset(buffer, 0, BUFFER_DIM);
                    snprintf(buffer, BUFFER_DIM, "%d", random);
                }
                else if (! strncmp(buffer, "i", 1)) {
                    // Genera e invia il numero casuale
                    printf("Child sending own pid.\n");
                    int pid = getpid();

                    // Lo converto in stringa
                    memset(buffer, 0, BUFFER_DIM);
                    snprintf(buffer, BUFFER_DIM, "%d", pid);
                }
                else {
                    printf("Comando non riconosciuto -> %s\n", buffer);
                }

                // Invio il dato al parent
                write(childWritesToParent[i][PIPE_WRITE], buffer, strlen(buffer));
            }

            // Chiude le pipe rimaste
            close(parentWritesToChild[i][PIPE_READ]);
            close(parentReadFromChild[i][PIPE_WRITE]);
            close(childWritesToParent[i][PIPE_WRITE]);
            close(childReadFromParent[i][PIPE_READ]);

            // Termina il processo
            return 0;
        }
        // Padre
        else {
            pids[i] = currPid;

            // Chiude le pipe non necessarie
            close(parentWritesToChild[i][PIPE_READ]);
            close(parentReadFromChild[i][PIPE_WRITE]);
            close(childWritesToParent[i][PIPE_WRITE]);
            close(childReadFromParent[i][PIPE_READ]);
        }
    }

    // Legge input
    char cmd;
    int arg;
    while(inputCycle) {
        memset(buffer, 0, BUFFER_DIM);

        printf("> ");
        scanf("%s", buffer);

        cmd = buffer[0];
        arg = atoi(&buffer[1]);

        // prevent bof
        if (arg >= children) {
            printf("Hai generato %d figli!\n\n", children);
            continue;
        }
        else if (arg < 0) {
            printf("L'argomento deve essere non nullo.\n\n");
            continue;
        }

        switch(cmd) {
            // Child sends random number
            case 'r':
                write(parentWritesToChild[arg][PIPE_WRITE], "r", 1);

                memset(buffer, 0, BUFFER_DIM);
                sleep(1);

                read(childWritesToParent[arg][PIPE_READ], buffer, BUFFER_DIM);

                printf("Child %d told me: '%s'\n\n", pids[arg], buffer);
                break;

            // Child sends pid
            case 'i':
                write(parentWritesToChild[arg][PIPE_WRITE], "i", 1);

                memset(buffer, 0, BUFFER_DIM);
                sleep(1);

                read(childWritesToParent[arg][PIPE_READ], buffer, BUFFER_DIM);

                printf("Child %d told me: '%s'\n\n", pids[arg], buffer);
                break;

            // Quit
            case 'q':
                inputCycle = 0;

                // invio SIGTERM a tutti i figli
                for(int i = 0; i < children; i++) {
                    kill(pids[i], SIGTERM);
                }
                break;

            // invalid
            default:
                continue;
        }
    }

    // Aspetto tutti i figli
    while(wait(NULL) > 0);

    // Chiude tutte le pipe rimaste
    for(int i = 0; i < children; i++) {
        for(int j = 0; j < 2; j++) {
            close(parentReadFromChild[i][j]);
            close(parentWritesToChild[i][i]);
            close(childReadFromParent[i][j]);
            close(childWritesToParent[i][j]);
        }
    }

    return 0;
}