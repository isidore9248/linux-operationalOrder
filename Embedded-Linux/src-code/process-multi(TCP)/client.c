//? 客户端程序框架
//? socket -- connect -- send -- recv -- close

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>

#define SERVER_IP "192.168.225.128"
const int server_port = 9734;
#define BUFFER_SIZE 1024

struct server_info
{
    int client_sockfd;
};

void *client_receive(void *arg);
void *client_send(void *arg);

int main(int argc, char **argv)
{
    int client_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sockfd == -1)
    {
        perror("socket error");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    int pton_ret = inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);
    if (pton_ret <= 0)
    {
        printf("inet_pton error");
        close(client_sockfd);
        return -1;
    }
    else if (pton_ret == 0)
    {
        printf("invalid address\n");
        close(client_sockfd);
        return -1;
    }

    int connect_ret = connect(client_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect_ret == -1)
    {
        printf("connect error\n");
        close(client_sockfd);
        return -1;
    }

    printf("connect to server %s:%d\n", SERVER_IP, server_port);

    pthread_t send_thread, recv_thread;

    pthread_create(&send_thread, NULL, client_receive, &client_sockfd);
    pthread_create(&recv_thread, NULL, client_send, &client_sockfd);

    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    while (1)
        ;

    return 0;
}

void *client_receive(void *arg)
{
    while (true)
    {
        int *sockfd = (int *)arg;
        char recv_buf[BUFFER_SIZE];
        ssize_t recv_ret = recv(*sockfd, recv_buf, sizeof(recv_buf), 0);
        if (recv_ret == -1)
        {
            printf("recv error");
            continue;
        }
        else if (recv_ret == 0)
        {
            printf("server close\n");
            return NULL;
        }
        else
        {
            recv_buf[recv_ret] = '\0';
            printf("recv from server:%s\n", recv_buf);
            printf("Enter message: \n");
        }
    }
    return NULL;
}

void *client_send(void *arg)
{
    while (true)
    {
        int *sockfd = (int *)arg;
        char send_buf[BUFFER_SIZE];
        printf("Enter message: ");
        fgets(send_buf, BUFFER_SIZE, stdin);
        send(*sockfd, send_buf, strlen(send_buf), 0);
    }
    return NULL;
}