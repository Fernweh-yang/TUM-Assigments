/*
Concepts and Software Design for Cyber-Physical Systems
*
Assignment 6 - POSIX Message Queues
*
Authors: Lim, Seokkyun; Mijacevic, Matej; Xu, Yang
*
########################################################
----------------------- CLIENT -------------------------
########################################################
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define SERVER_QUEUE_NAME   "/SRVR"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

int main (int argc, char **argv) {

    char client_queue_name [64];
    //memset将变量指针（第一个参数）指向的前n个内存单元(第三个参数)用一个整数(第二个参数)替代
    memset(client_queue_name, '\0', sizeof(client_queue_name));

    //消息队列描述符
    mqd_t qd_server, qd_client;   // queue descriptors


    // create the client queue for receiving messages from server
    //sprintf()将格式化的数据写入字符串(第一个参数中)。
    //getpid()获得目前进程的识别码
    sprintf (client_queue_name, "/CLNT_%d", getpid ());

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    //创建消息队列接收消息O_RDONLY
    //O_CREAT指定该消息队列的参数为attr
    if ((qd_client = mq_open (client_queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("CLIENT: mq_open (client)");
        exit (1);
    }

    //创建消息队列发送消息,O_WRONLY
    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        perror ("CLIENT: mq_open (server)");
        exit (1);
    }

    char in_buffer [MSG_BUFFER_SIZE];
    memset(in_buffer, '\0', sizeof(in_buffer));

    printf ("CLIENT: Enter message (Press <ENTER>): ");

    char temp_buf [100];
    memset(temp_buf, '\0', sizeof(temp_buf));
    //从标准输入流stdin中读取1000个数据到temp_buf所指向的内存空间
    while (fgets (temp_buf, 1000, stdin)) {

        // send message to server
        if (mq_send (qd_server, client_queue_name, strlen(client_queue_name) + 1, 0) == -1) {
            perror ("CLIENT: Not able to send message to server");
            continue;
        }

        // send message to server
        if (mq_send (qd_server, temp_buf, strlen(temp_buf) + 1, 0) == -1) {
            perror ("CLIENT: Not able to send message to server");
            continue;
        }

        // receive response from server

        if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("CLIENT: mq_receive");
            exit (1);
        }
        // display token received from server
        printf ("CLIENT: %s sent by %s\n\n", in_buffer, SERVER_QUEUE_NAME);

        printf ("CLIENT: Enter Message (Press ): ");
        memset(temp_buf, '\0', sizeof(temp_buf));
    }

    //关闭消息队列
    if (mq_close (qd_client) == -1) {
        perror ("CLIENT: mq_close");
        exit (1);
    }
    //删除消息队列
    if (mq_unlink (client_queue_name) == -1) {
        perror ("Client: mq_unlink");
        exit (1);
    }

    exit (0);
}