/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-05-02 11:27:18
 * @Copyright: Copyright (c) 2025 CAUC
 */
#pragma once

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

enum TriggerMode
{
	LT = 0,
	ET = 1,
};

enum BlockingMode
{
	BLOCK = 0,
	NONBLOCK = 1
};

/// 默认为水平LT触发模式,当缓冲区有数据时,就会触发epoll_wait,直到缓冲区为空
void server_socket_init(int *lfd, struct sockaddr_in *serv_addr, int port);
void server_epoll_init(int *epoll_fd, int server_sockfd, uint32_t events, int max_events, enum TriggerMode mode);
void add_fd_to_epoll(int epoll_fd, int server_sockfd, uint32_t events, enum TriggerMode mode);
// 修改 handle_client 签名以包含客户端列表信息
void handle_client(int epoll_fd, int curfd, enum TriggerMode triggermode, int *client_fds, int *client_count_ptr);
// 添加广播函数声明
void broadcast_message(int epoll_fd, int *client_fds, int *client_count_ptr, int sender_fd, const char *message, int msg_len);

/// 边缘ET触发模式,当缓冲区有数据时,就会触发epoll_wait,但只会触发一次
/// 如果缓冲区有数据未读取完毕，也不会再触发epoll_wait
/// 设置epoll事件结构体中的 events
///  eg:
///  struct epoll_event ev;
///  ev.events = events | EPOLLET;
///
///
/// 添加到epollfd中, 读写都设置为非阻塞模式
/// 设置客户端文件描述符为非阻塞模式
///  int flag = fcntl(cfd, F_GETFL);
///  flag |= O_NONBLOCK;
///  fcntl(cfd, F_SETFL, flag);
///  设置完成之后, 读写都变成了非阻塞模式
