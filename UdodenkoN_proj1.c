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
#define MQD "/mq"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define FULL_VALUE 0
#define EMPTY_VALUE 10
#define MUTEX_VALUE 1

sem_t* empty;
sem_t* full;
sem_t* mutex_prod;
sem_t* mutex_cons;
mqd_t mqd;


void producer(int process_num){
    full = sem_open(SEM_FULL_NAME, O_RDWR);
    empty = sem_open(SEM_EMPTY_NAME, O_RDWR);
    mutex_prod = sem_open(SEM_MUTEX_PROD_NAME, O_RDWR);
    mqd = mq_open(MQD, O_RDWR);
    char message[MSG_SIZE];
    char timestamp[30];
    struct timeval tv;

    while(1){

        memset(message, 0, MSG_SIZE);
        memset(timestamp, 0, 30);
        sprintf(message, " --- HELLO from PRODUCER %d and TIMESTAMP ", process_num);

        sem_wait(empty);
        sem_wait(mutex_prod);

        gettimeofday(&tv,NULL);
        sprintf(timestamp, "%ld --- ", tv.tv_usec);
        strcat(message, timestamp);

        // Read the message 
        // print the message from the consumer x
        // Send the message to the Consumer x

        int status = mq_send(mqd, message, MSG_SIZE, 0);
        // if (status == -1){
        //     printf("PRODUCER %d: message is not sent\n\n", process_num);
        // }
        // else {
        //     printf("PRODUCER %d: message was succesfully sent\n\n", process_num);
        // }
        sleep(1);

        sem_post(mutex_prod);
        sem_post(full);
        
        sleep(0.5);
    }
}

void consumer(int process_num){
    full = sem_open(SEM_FULL_NAME, O_RDWR);
    empty = sem_open(SEM_EMPTY_NAME, O_RDWR);
    mutex_cons = sem_open(SEM_MUTEX_CONS_NAME, O_RDWR);
    mqd = mq_open(MQD, O_RDWR);
    char message[MSG_SIZE];

    while(1){

        memset(message, 0, MSG_SIZE);

        sem_wait(full);
        sem_wait(mutex_cons);

        // Consumer needs to send something to producer 
        // The same consumer recieves the message producer x

        ssize_t num_bytes_received = mq_receive(mqd, message, MSG_SIZE, 0);
        
        sleep(1);

        sem_post(mutex_cons);
        sem_post(empty);

        if (num_bytes_received == -1){
            printf("CONSUMER %d: message is not received\n\n", process_num);
        }
        else {
            printf("CONSUMER %d: message received: %s\n\n", process_num, message);
        }
    }
}

int main(){

    pid_t pid;
    pid_t pid_2;

    sem_unlink(SEM_FULL_NAME);
    sem_unlink(SEM_EMPTY_NAME);
    sem_unlink(SEM_MUTEX_PROD_NAME);
    sem_unlink(SEM_MUTEX_CONS_NAME);
    mq_unlink(MQD);

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

    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = MSG_SIZE,
        .mq_curmsgs = 0
    };

    mqd = mq_open(MQD, O_CREAT | O_RDWR, S_IRWXU, &attr);
    if (mqd == -1){
        perror("ORIGINAL MQD not open -----\n");
        exit(0);
    }
    mq_close(mqd);

    for (int i = 0; i < 5; i++){
        pid = fork();
        if (pid == 0){
            // Child Process
            producer(i);
            exit(0);
        }
        else {
            pid_2 = fork();
            if (pid_2 == 0){
                // Another Child Process
                consumer(i);
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