#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// const int MAX_CONNECT = 5;
#define MAX_CONNECT 5
#define BUFFER_SIZE 1024
const int server_port = 9734;

struct client_info
{
    int client_sockfd;
    struct sockaddr_in client_addr;
};
struct client_info client_info_list[MAX_CONNECT];

//*declaration
struct client_info *GetClientInfo(struct client_info client_info_list[]);
void SetClientinfo(struct client_info *p, int fd, struct sockaddr_in client_addr);
void *server_receive(void *arg);
void *server_send(void *arg);

int main(int argc, char **argv)
{
    int server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sockfd == -1)
    {
        perror("socket error");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    int bind_ret = bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if (bind_ret == -1)
    {
        perror("bind error");
        close(server_sockfd);
        return -1;
    }

    int listen_ret = listen(server_sockfd, MAX_CONNECT);
    if (listen_ret == -1)
    {
        perror("listen error");
        close(server_sockfd);
        return -1;
    }

    for (int i = 0; i < MAX_CONNECT; i++)
    {
        bzero(&client_info_list[i], sizeof(struct client_info));
        client_info_list[i].client_sockfd = -1;
    }

    while (true)
    {
        struct client_info *p_client_info = GetClientInfo(client_info_list);

        struct sockaddr_in client_addr;
        socklen_t clientaddr_len = sizeof(client_addr);
        //*返回客户端的socket描述符
        int accept_ret = accept(server_sockfd, (struct sockaddr *)&client_addr, &clientaddr_len);
        if (accept_ret == -1)
        {
            perror("accept error");
            continue;
        }

        SetClientinfo(p_client_info, accept_ret, client_addr);

        printf("accept client %s\n", inet_ntoa(client_addr.sin_addr));
        int pid = fork();
        if (pid == 0)
        {
            close(server_sockfd); // 子进程不需要服务器套接字

            pthread_t recv_thread, send_thread;
            pthread_create(&recv_thread, NULL, server_receive, p_client_info);
            pthread_create(&send_thread, NULL, server_send, p_client_info);

            pthread_detach(recv_thread);
            pthread_detach(send_thread);

            while (1)
                ;

            // pthread_join(tid1, NULL);
            // pthread_join(tid2, NULL);

            // exit(0);
        }
        else if (pid > 0)
        {
            // 父进程关闭客户端套接字并继续等待新的连接
            close(accept_ret);
        }
        else
        {
            perror("fork error");
            close(accept_ret);
        }
    }
    close(server_sockfd);
    return 0;
}

struct client_info *GetClientInfo(struct client_info client_info_list[])
{
    struct client_info *p_client_info = NULL;
    for (int i = 0; i < MAX_CONNECT; i++)
    {
        if (client_info_list[i].client_sockfd == -1)
        {
            p_client_info = &client_info_list[i];
            break;
        }
    }
    return p_client_info;
}

void SetClientinfo(struct client_info *p, int fd, struct sockaddr_in client_addr)
{
    p->client_sockfd = fd;
    p->client_addr = client_addr;
}

void *server_receive(void *arg)
{

    struct client_info *p = (struct client_info *)arg;
    char recv_buf[1024];

    while (true)
    {
        int recv_ret = recv(p->client_sockfd, recv_buf, sizeof(recv_buf), 0);
        if (recv_ret == -1)
        {
            perror("recv error");
            break;
        }
        else if (recv_ret == 0)
        {
            printf("client %s close\n", inet_ntoa(p->client_addr.sin_addr));
            break;
        }
        else
        {
            printf("recv from:%s\n", inet_ntoa(p->client_addr.sin_addr));
            printf("recv data:%s\n", recv_buf);
            printf("Enter message: \n");
        }
    }

    close(p->client_sockfd);
    p->client_sockfd = -1;
    return NULL;
}

void *server_send(void *arg)
{
    struct client_info *p = (struct client_info *)arg;
    const char *message = "Hello from server";
    while (true)
    {
        static char buffer[BUFFER_SIZE];
        printf("Enter message: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        send(p->client_sockfd, buffer, strlen(buffer), 0);

        // printf("Enter message: \n");
        // send(p->client_sockfd, message, strlen(message), 0);
        // sleep(5); // 每5秒发送一次消息
    }

    close(p->client_sockfd);
    p->client_sockfd = -1;
    return NULL;
}
