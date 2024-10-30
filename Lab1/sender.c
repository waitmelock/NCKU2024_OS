#include "sender.h"
#include <time.h>
struct timespec start,end;
double time_taken;
/*  TODO: 
    1. Use flag to determine the communication method
    2. According to the communication method, send the message
*/
mailbox_t mailbox;
message_t message;
// sem_t *empty;
// sem_t *full;
sem_t *mutex_send;
sem_t *mutex_rece;
mqd_t mq;
char *shm_ptr;
void send(message_t message, mailbox_t* mailbox_ptr){    
    if(mailbox_ptr->flag==1){// message passing
        key_t key = ftok(".", 65);
        int msgid = msgget(key, 0666 | IPC_CREAT);
    
        struct msg_buffer {
            long msg_type;
            char msg_text[SHM_SIZE];
        } message_buf;
        
        // 設定訊息
        message_buf.msg_type = 1;
        char *newline = strchr(message.content, '\n');
        if (newline) *newline = '\0';
        
        strcpy(message_buf.msg_text, message.content);
        message_buf.msg_text[SHM_SIZE - 1] = '\0';
        
        // 發送訊息
        if(msgsnd(msgid, &message_buf, sizeof(message_buf.msg_text), 0) == -1) {
            perror("msgsnd failed");
            exit(1);
        }
    }

    if(mailbox_ptr->flag==2){// share memory
        // sem_wait(empty);
        snprintf(mailbox_ptr->storage.shm_addr,SHM_SIZE,"%s",message.content);
        // sem_post(full);
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
    if(argc != 3) {
        printf("Usage: %s <flag> <input_file>\n", argv[0]);
        return 1;
    }
    mailbox.flag=atoi(argv[1]);
    FILE *file = fopen(argv[2], "r");
    // printf("%d",mailbox.flag);
    char* result;
    if(mailbox.flag==1){// message passing
        printf("\033[34mMessage Passing\033[0m\n");
        key_t key = ftok(".", 65);
        int msgid = msgget(key, 0666 | IPC_CREAT);
        // 發送開始訊號
        strcpy(message.content, "START\n");
        send(message, &mailbox);
    
        char* result;
        while((result = fgets(message.content, SHM_SIZE, file)) != NULL) {
            clock_gettime(CLOCK_MONOTONIC, &start);
            send(message, &mailbox);
            clock_gettime(CLOCK_MONOTONIC, &end);
            
            message.timestamp = (end.tv_sec - start.tv_sec) + 
                            (end.tv_nsec - start.tv_nsec) * 1e-9;
            time_taken += message.timestamp;
            printf("\033[34mSending message:\033[0m %s",message.content);
        }
    
    // 發送結束訊號
    strcpy(message.content, "EOF\n");
    send(message, &mailbox);
    printf("\n\033[31mEnd of input file! exit!\033[0m\n");
    printf("Total time taken in sending msg: %6f s\n", time_taken);
    fclose(file);

    }
    if(mailbox.flag==2){
        printf("\033[34mMessage Passing\033[0m\n");  
        mutex_send = sem_open(SEM_MUTEX_send, O_CREAT|O_RDWR, 0666, 1);
        mutex_rece = sem_open(SEM_MUTEX_rece, O_CREAT|O_RDWR, 0666, 0);
        int shm_fd = shm_open(SHM_NAME, O_CREAT|O_RDWR, 0666);
        if(shm_fd== -1) {
            perror("shm fail");
            return 1;
        }
        ftruncate(shm_fd, SHM_SIZE); 
        mailbox.storage.shm_addr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
        shm_ptr = mailbox.storage.shm_addr;
    
        while((result=fgets(message.content,SHM_SIZE, file))!=NULL){
            sem_wait(mutex_send);
            clock_gettime(CLOCK_MONOTONIC_RAW, &start);
            send(message, &mailbox);
            clock_gettime(CLOCK_MONOTONIC_RAW, &end);
            sem_post(mutex_rece);
            message.timestamp = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) * 1e-9;
            time_taken+=message.timestamp;
            printf("\033[34mSending message:\033[0m %s",message.content);
        }
        strcpy(message.content,"EOF");
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        send(message, &mailbox);
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        message.timestamp = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) * 1e-9;
        time_taken+=message.timestamp;
        printf("\n\033[31mEnd of input file! exit!\033[0m\n");
        printf("Total time taken in sending msg: %6f s\n",time_taken);

        //free space
        fclose(file);
        munmap(shm_ptr, SHM_SIZE);
        close(shm_fd);
        sem_close(mutex_send);
        sem_close(mutex_rece);
    }
    return 0;
    }