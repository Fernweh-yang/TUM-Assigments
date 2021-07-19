/*
Concepts and Software Design for Cyber-Physical Systems
*
Assignment 6 - POSIX Message Queues
*
Authors: Lim, Seokkyun; Mijacevic, Matej; Xu, Yang
*
########################################################
----------------------- SERVER -------------------------
########################################################
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define SERVER_QUEUE_NAME   "/SRVR"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

// Signal handler
void signal_handler(int sig) {
    printf("SIGNAL: SIGINT received\n");
    if (mq_close (SERVER_QUEUE_NAME) == -1) {
        perror ("SERVER: mq_close");
        exit (1);
    }

    if (mq_unlink (SERVER_QUEUE_NAME) == -1) {
        perror ("SERVER: mq_unlink");
        exit (1);
    }
}

// Setup of signal handler
void signal_handler_setup() {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    sigemptyset(&action.sa_mask);

    action.sa_handler = signal_handler_setup;
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, NULL) != 0) {
        perror("sigaction");
        exit(1);
    }
}

int main (int argc, char **argv) {

    mqd_t qd_server, qd_client;   // queue descriptors

    sigset_t waitset;
    int signum;

    signal_handler_setup();

    sigemptyset(&waitset);
    sigaddset(&waitset, SIGINT);

    printf ("SERVER: Establishing Connection ...\n");

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    //创建消息队列去接收信号
    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("SERVER: mq_open (server)");
        exit (1);
    }
    char in_buffer [MSG_BUFFER_SIZE];
    char out_buffer [MSG_BUFFER_SIZE];
    char temp_buffer [MSG_BUFFER_SIZE];

    while (1) {
        // get the oldest message with highest priority
        if (mq_receive (qd_server, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("SERVER: mq_receive");
            exit (1);
        }

        if (mq_receive (qd_server, temp_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("SERVER: mq_receive");
            exit (1);
        }

        printf ("SERVER: message received.\n");
        printf ("SERVER: %s sent by %s\n", temp_buffer, in_buffer);

        // send reply message to client
        // 创建消息队列发送消息
        if ((qd_client = mq_open (in_buffer, O_WRONLY)) == 1) {
            perror ("SERVER: Not able to open client queue");
            continue;
        }

        sprintf (out_buffer, "Your message is: %s", temp_buffer);

        if (mq_send (qd_client, out_buffer, strlen(out_buffer) + 1, 0) == -1) {
            perror ("SERVER: Not able to send message to client");
            continue;
        }

        printf ("SERVER: response sent to client.\n");
    }
}