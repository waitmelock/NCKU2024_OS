#include "sender.h"
struct timespec start,end;
double time_taken;
/*  TODO: 
    1. Use flag to determine the communication method
    2. According to the communication method, send the message
*/
mailbox_t mailbox;
message_t message;
sem_t *empty;
sem_t *full;
sem_t *mutex;
char *shm_ptr;
void send(message_t message, mailbox_t* mailbox_ptr){    
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
/*  TODO: 
        1) Call send(message, &mailbox) according to the flow in slide 4
        2) Measure the total sending time
        3) Get the mechanism and the input file from command line arguments
            • e.g. ./sender 1 input.txt
                    (1 for Message Passing, 2 for Shared Memory)
        4) Get the messages to be sent from the input file
        5) Print information on the console according to the output format
        6) If the message form the input file is EOF, send an exit message to the receiver.c
        7) Print the total sending time and terminate the sender.c
    */
int main(int argc,char* argv[]){
     //./sender 1 input.txt
    mailbox.flag=atoi(argv[1]);
    FILE *file = fopen(argv[2], "r");
    if(mailbox.flag==2){
        empty = sem_open(SEM_EMPTY, O_CREAT, 0666, 1);
        full = sem_open(SEM_FULL, O_CREAT, 0666, 0);
        mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
        int shm_fd = sem_open(SHM_NAME, O_RDWR, 0666);
        ftruncate(shm_fd, SHM_SIZE); 
        mailbox.storage.shm_addr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
        shm_ptr = mailbox.storage.shm_addr;
        while(fgets(message.content, message.content, file)){
            clock_gettime(CLOCK_MONOTONIC_RAW, &start);
            send(message, &mailbox);
            clock_gettime(CLOCK_MONOTONIC_RAW, &end);
            message.timestamp = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) * 1e-9;
            time_taken+=message.timestamp;
            printf("%5f",time_taken);
        }
        perror("End of inout file! exit!");
        fclose(file);
        munmap(shm_ptr, SHM_SIZE);
        close(shm_fd);
        sem_close(empty);
        sem_close(full);
        sem_close(mutex);
    }
    return 0;
}