#include "receiver.h"
struct timespec start,end;
double time_taken;

mailbox_t mailbox;
message_t message;
sem_t *mutex_send;
sem_t *mutex_rece;
mqd_t mq;

char *shm_ptr;

/*  TODO: 
    1. Use flag to determine the communication method
    2. According to the communication method, receive the message
*/
void receive(message_t* message_ptr, mailbox_t* mailbox_ptr){
    if(mailbox_ptr->flag==1){// message passing
        key_t key = ftok(".", 65);
        int msgid = msgget(key, 0666);
        
        // 定義訊息結構
        struct msg_buffer {
            long msg_type;
            char msg_text[SHM_SIZE];
        } message;
        // 接收訊息
        ssize_t receive_size = msgrcv(msgid, &message, sizeof(message.msg_text), 1, 0);
    
        // 複製訊息內容並加上換行符
        snprintf(message_ptr->content, SHM_SIZE, "%s\n", message.msg_text);
        
    }
    if(mailbox_ptr->flag==2){// share memory
        sem_wait(mutex_rece);
        strncpy(message_ptr->content,mailbox_ptr->storage.shm_addr,SHM_SIZE);
        sem_post(mutex_send);
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
    if(mailbox.flag==1){
        printf("\033[34mMessage Passing\033[0m\n");
        receive(&message, &mailbox);
    
    while(1) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        receive(&message, &mailbox);
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        
        message.timestamp = (end.tv_sec - start.tv_sec) + 
                          (end.tv_nsec - start.tv_nsec) * 1e-9;
        time_taken += message.timestamp;
        
        if(strcmp(message.content, "EOF\n") == 0) {
            break;
        }
        
        printf("\033[34mReceiving message:\033[0m %s",message.content);
    }
    printf("\033[31mSender exit!\033[0m\n");

    printf("Total time taken in receiving msg: %6f s\n", time_taken);
    
    // 清理 message queue
    key_t key = ftok(".", 65);
    int msgid = msgget(key, 0666);
    if(msgid != -1) {
        msgctl(msgid, IPC_RMID, NULL);
    }
    }
    if(mailbox.flag==2){
        // empty = sem_open(SEM_EMPTY, O_RDWR, 0666, 1);
        // full = sem_open(SEM_FULL, O_RDWR, 0666, 0);
        mutex_send = sem_open(SEM_MUTEX_send, O_RDWR, 0666);
        mutex_rece = sem_open(SEM_MUTEX_rece, O_RDWR, 0666);
        int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
        ftruncate(shm_fd, SHM_SIZE); 
        mailbox.storage.shm_addr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
        shm_ptr = mailbox.storage.shm_addr;
        printf("\033[34mMessage Passing\033[0m\n");
        while(1){
            clock_gettime(CLOCK_MONOTONIC_RAW, &start);
            receive(&message, &mailbox);
            clock_gettime(CLOCK_MONOTONIC_RAW, &end);
            message.timestamp = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) * 1e-9;
            time_taken+=message.timestamp;
            if(!strcmp(message.content,"EOF")){
                break;
            }
            printf("\033[34mReceiving message:\033[0m %s",message.content);
            
        }
        printf("\n\033[31mSender exit!\033[0m\n");
        printf("Total time taken in receiving msg: %6f s\n",time_taken);

        munmap(shm_ptr, SHM_SIZE);
        close(shm_fd);
        sem_close(mutex_send);
        sem_close(mutex_rece);
    }
    return 0;
}