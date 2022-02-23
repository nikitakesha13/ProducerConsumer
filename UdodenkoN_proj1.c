#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>

#define BUFFER_SIZE 10
#define MSG_SIZE 8192
#define SEM_FULL_NAME "/full"
#define SEM_EMPTY_NAME "/empty"
#define SEM_MUTEX_PROD_NAME "/mutex_prod"
#define SEM_MUTEX_CONS_NAME "/mutex_cons"
#define SEM_LOCK_SEND "/lock_send"
#define SEM_LOCK_RECV "/lock_recv"
#define MQD_CONS "/mq_cons"
#define MQD_PROD "/mq_prod"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define FULL_VALUE 0
#define EMPTY_VALUE 10
#define LOCK_SEND_VALUE 1
#define LOCK_RECV_VALUE 0
#define MUTEX_VALUE 1

sem_t* empty;
sem_t* full;
sem_t* mutex_prod;
sem_t* mutex_cons;
sem_t* lock_send;
sem_t* lock_recv;
mqd_t mqd_cons;
mqd_t mqd_prod;


void producer(int process_num){
    full = sem_open(SEM_FULL_NAME, O_RDWR);
    empty = sem_open(SEM_EMPTY_NAME, O_RDWR);
    mutex_prod = sem_open(SEM_MUTEX_PROD_NAME, O_RDWR);
    lock_send = sem_open(SEM_LOCK_SEND, O_RDWR);
    lock_recv = sem_open(SEM_LOCK_RECV, O_RDWR);
    mqd_cons = mq_open(MQD_CONS, O_RDWR);
    mqd_prod = mq_open(MQD_PROD, O_RDWR);
    char message_send[MSG_SIZE];
    char message_recv[MSG_SIZE];
    char timestamp[30];
    struct timeval tv;

    memset(message_send, 0, MSG_SIZE);
    sprintf(message_send, " --- PRODUCER %d has a RESOURCE --- ", process_num);

    while(1){

        memset(message_recv, 0, MSG_SIZE);
        memset(timestamp, 0, 30);

        sem_wait(full);
        sem_wait(mutex_prod);

        // Read the message 
        // print the message from the consumer x
        // Send the message to the Consumer x

        ssize_t num_bytes_received = mq_receive(mqd_cons, message_recv, MSG_SIZE, 0);
        gettimeofday(&tv,NULL);
        sprintf(timestamp, "TIMESTAMP %ld --- ", tv.tv_usec);
        strcat(message_recv, timestamp);

        sem_post(empty);

        if (num_bytes_received == -1){
            printf("PRODUCER %d: message is not received\n\n", process_num);
        }
        else {
            printf("PRODUCER %d: message received: %s\n\n", process_num, message_recv);
        }

        sem_wait(lock_send);

        int status = mq_send(mqd_prod, message_send, MSG_SIZE, 0);

        sem_post(lock_recv);

        sleep(1);

        sem_post(mutex_prod);

        sleep(0.01);
    }

    exit(0);
}

void consumer(int process_num){
    full = sem_open(SEM_FULL_NAME, O_RDWR);
    empty = sem_open(SEM_EMPTY_NAME, O_RDWR);
    mutex_cons = sem_open(SEM_MUTEX_CONS_NAME, O_RDWR);
    lock_send = sem_open(SEM_LOCK_SEND, O_RDWR);
    lock_recv = sem_open(SEM_LOCK_RECV, O_RDWR);
    mqd_cons = mq_open(MQD_CONS, O_RDWR);
    mqd_prod = mq_open(MQD_PROD, O_RDWR);
    char message_recv[MSG_SIZE];
    char message_send[MSG_SIZE];

    memset(message_send, 0, MSG_SIZE);
    sprintf(message_send, " --- CONSUMER %d needs a resource ", process_num);

    while(1){

        memset(message_recv, 0, MSG_SIZE);

        sem_wait(empty);
        sem_wait(mutex_cons);

        // Consumer needs to send something to producer 
        // The same consumer recieves the message producer x

        int status = mq_send(mqd_cons, message_send, MSG_SIZE, 0);

        sem_post(full);

        sem_wait(lock_recv);

        ssize_t num_bytes_received = mq_receive(mqd_prod, message_recv, MSG_SIZE, 0);

        sem_post(lock_send);
        
        sleep(1);

        if (num_bytes_received == -1){
            printf("CONSUMER %d: message is not received\n", process_num);
        }
        else {
            printf("CONSUMER %d: message received: %s\n\n", process_num, message_recv);
        }

        printf("-------------------------------------\n\n");

        sem_post(mutex_cons);

        sleep(0.01);
    }

    exit(0);
}

int main(){

    pid_t pid;
    pid_t pid_2;

    sem_unlink(SEM_FULL_NAME);
    sem_unlink(SEM_EMPTY_NAME);
    sem_unlink(SEM_MUTEX_PROD_NAME);
    sem_unlink(SEM_MUTEX_CONS_NAME);
    sem_unlink(SEM_LOCK_SEND);
    sem_unlink(SEM_LOCK_RECV);
    mq_unlink(MQD_CONS);
    mq_unlink(MQD_PROD);

    full = sem_open(SEM_FULL_NAME, O_CREAT | O_EXCL, SEM_PERMS, FULL_VALUE);
    if (full == SEM_FAILED) {
        perror("sem_open(3) full error");
        exit(EXIT_FAILURE);
    }
    sem_close(full);

    empty = sem_open(SEM_EMPTY_NAME, O_CREAT | O_EXCL, SEM_PERMS, EMPTY_VALUE);
    if (empty == SEM_FAILED) {
        perror("sem_open(3) empty error");
        exit(EXIT_FAILURE);
    }
    sem_close(empty);

    mutex_prod = sem_open(SEM_MUTEX_PROD_NAME, O_CREAT | O_EXCL, SEM_PERMS, MUTEX_VALUE);
    if (mutex_prod == SEM_FAILED) {
        perror("sem_open(3) mutex_prod erro");
        exit(EXIT_FAILURE);
    }
    sem_close(mutex_prod);

    mutex_cons = sem_open(SEM_MUTEX_CONS_NAME, O_CREAT | O_EXCL, SEM_PERMS, MUTEX_VALUE);
    if (mutex_cons == SEM_FAILED) {
        perror("sem_open(3) mutex_cons erro");
        exit(EXIT_FAILURE);
    }
    sem_close(mutex_cons);

    lock_send = sem_open(SEM_LOCK_SEND, O_CREAT | O_EXCL, SEM_PERMS, LOCK_SEND_VALUE);
    if (lock_send == SEM_FAILED) {
        perror("sem_open(3) lock_send error");
        exit(EXIT_FAILURE);
    }
    sem_close(lock_send);

    lock_recv = sem_open(SEM_LOCK_RECV, O_CREAT | O_EXCL, SEM_PERMS, LOCK_RECV_VALUE);
    if (lock_recv == SEM_FAILED) {
        perror("sem_open(3) lock_recv error");
        exit(EXIT_FAILURE);
    }
    sem_close(lock_recv);

    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = MSG_SIZE,
        .mq_curmsgs = 0
    };

    mqd_cons = mq_open(MQD_CONS, O_CREAT | O_RDWR, S_IRWXU, &attr);
    if (mqd_cons == -1){
        perror("ORIGINAL MQD_CONS not open -----\n");
        exit(0);
    }
    mq_close(mqd_cons);

    mqd_prod = mq_open(MQD_PROD, O_CREAT | O_RDWR, S_IRWXU, &attr);
    if (mqd_prod == -1){
        perror("ORIGINAL MQD_PROD not open -----\n");
        exit(0);
    }
    mq_close(mqd_prod);

    for (int i = 0; i < 5; i++){
        pid = fork();
        if (pid == 0){
            // Child Process
            consumer(i);
            exit(0);
        }
        else {
            pid_2 = fork();
            if (pid_2 == 0){
                // Another Child Process
                producer(i);
                exit(0);
            }
            else {
                // Parent Process
            }
        }
    }
    wait(NULL);
    return 0;
}