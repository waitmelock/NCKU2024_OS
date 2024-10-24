#include "receiver.h"
struct timespec start,end;
double time_taken;

mailbox_t mailbox;
message_t message;
sem_t *empty;
sem_t *full;
sem_t *mutex;
char *shm_ptr;

/*  TODO: 
    1. Use flag to determine the communication method
    2. According to the communication method, receive the message
*/
void receive(message_t* message_ptr, mailbox_t* mailbox_ptr){
    // if(mailbox_ptr->flag==1){// message passing

    // }
    if(mailbox_ptr->flag==2){// share memory
        sem_wait(empty);
        sem_wait(mutex);
        snprintf(mailbox_ptr->storage.shm_addr,SHM_SIZE,"%s\n",message.content);
        sem_post(mutex);
        sem_post(full);
    }
}

int main(int argc,char* argv[]){
    /*  TODO: 
        1) Call receive(&message, &mailbox) according to the flow in slide 4
        2) Measure the total receiving time
        3) Get the mechanism from command line arguments
            • e.g. ./receiver 1
        4) Print information on the console according to the output format
        5) If the exit message is received, print the total receiving time and terminate the receiver.c
    */
    mailbox.flag=atoi(argv[1]);
    if(mailbox.flag==2){
        empty = sem_open(SEM_EMPTY, O_CREAT, 0666, 1);
        full = sem_open(SEM_FULL, O_CREAT, 0666, 0);
        mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
        int shm_fd = sem_open(SHM_NAME, O_RDWR, 0666);
        ftruncate(shm_fd, SHM_SIZE); 
        mailbox.storage.shm_addr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
        shm_ptr = mailbox.storage.shm_addr;
        while(strcmp(message.content,"EOF")){
            clock_gettime(CLOCK_MONOTONIC_RAW, &start);
            send(message, &mailbox);
            clock_gettime(CLOCK_MONOTONIC_RAW, &end);
            message.timestamp = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) * 1e-9;
            time_taken+=message.timestamp;
            printf("%5f",time_taken);
        }
        perror("Sender exit!");

        munmap(shm_ptr, SHM_SIZE);
        close(shm_fd);
        sem_close(empty);
        sem_close(full);
        sem_close(mutex);
    }
    return 0;
}