#include "sender.h"
struct timespec start,end;
double time_taken;
void send(message_t message, mailbox_t* mailbox_ptr){
    /*  TODO: 
        1. Use flag to determine the communication method
        2. According to the communication method, send the message
    */
    if(mailbox_ptr->flag==1){// message passing

    }
    if(mailbox_ptr->flag==2){// share memory

    }
}

int main(){
    mailbox_t mailbox;
    message_t message;
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
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    send(message, &mailbox);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    time_taken = (end.tv_sec - start.tv_sec)+(end.tv_nsec - start.tv_nsec) * 1e-9;
}