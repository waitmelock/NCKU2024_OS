#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <time.h>

#define SHM_NAME "/shm_share"
#define SHM_SIZE 1024
#define SEM_EMPTY "/empty"
#define SEM_FULL "/full"
#define SEM_MUTEX "/mutex"

typedef struct {
    int flag;      // 1 for message passing, 2 for shared memory
    union{
        int msqid; //for system V api. You can replace it with struecture for POSIX api
        char* shm_addr;
    }storage;
} mailbox_t;


typedef struct {
    /*  TODO: 
        Message structure for wrapper
    */
    char content[1024];
    double timestamp;
} message_t;

void send(message_t message, mailbox_t* mailbox_ptr);