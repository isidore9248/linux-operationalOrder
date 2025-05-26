/*
 * @Description:
 * @Version: v1.0.0
 * @Author: isidore-chen
 * @Date: 2025-05-02 11:27:18
 * @Copyright: Copyright (c) 2025 CAUC
 */
#include "epoll_server.h"
#include <stdio.h>

#define MAX_CLIENTS 1024 // 定义最大客户端数量
int client_fds[MAX_CLIENTS];
int client_count = 0;

const int SERVER_PORT = 8080;

/// <summary>
/// 服务器通信函数，处理客户端连接和请求。
/// </summary>
/// <param name="epoll_fd">epoll实例的文件描述符</param>
/// <param name="server_sockfd">服务器套接字文件描述符</param>
/// <param name="triggermode">触发模式（边沿触发或水平触发，搜索 enum TriggerMode）</param>
/// <param name="blockmode">
/// 阻塞模式（阻塞或非阻塞，搜索 enum BlockingMode） ！！仅在ET模式下生效！！
/// </param>
void server_communicate(int epoll_fd, int server_sockfd,
	enum TriggerMode triggermode, enum BlockingMode blockmode)
{
	struct epoll_event evs[1024];
	int size = sizeof(evs) / sizeof(struct epoll_event);

	while (1)
	{
		/// <summary>
		/// 阻塞并等待就绪的文件描述符
		/// </summary>
		/// <param name="evs">epoll事件结构体数组	！！地址！！	</param>
		/// <param name="size">epoll事件结构体数组大小</param>
		/// <returns>就绪的文件描述符数量</returns>
		int readyfd_num = epoll_wait(epoll_fd, evs, size, -1);
		for (int i = 0; i < readyfd_num; ++i)
		{
			int current_fd = evs[i].data.fd;
			if (current_fd == server_sockfd)
			{
				// 接受新的客户端连接
				int cfd = accept(current_fd, NULL, NULL);
				if (blockmode == NONBLOCK && triggermode == ET)
				{
					int flag = fcntl(cfd, F_GETFL);
					flag |= O_NONBLOCK;
					fcntl(cfd, F_SETFL, flag);
					//设置完成之后, 读写都变成了非阻塞模式
				}
				// 将新的客户端文件描述符添加到epoll实例中
				add_fd_to_epoll(epoll_fd, cfd, EPOLLIN, triggermode);

				// 将新客户端添加到列表中
				if (client_count < MAX_CLIENTS) {
					client_fds[client_count++] = cfd;
					printf("新客户端连接: fd=%d, 当前总数: %d\n", cfd, client_count);
				}
				else {
					printf("客户端连接已满，拒绝新连接: fd=%d\n", cfd);
					close(cfd); // 关闭连接
				}
			}
			else //与客户端通信
			{
				// 处理客户端请求，传递客户端列表和计数器指针
				handle_client(epoll_fd, current_fd, triggermode, client_fds, &client_count);
			}
		}
	}
}

int main(int argc, const char* argv[])
{
	int server_sockfd = -1;
	struct sockaddr_in serv_addr;
	// 设置服务器套接字
	server_socket_init(&server_sockfd, &serv_addr, SERVER_PORT);

	// epoll相关初始化
	int epoll_fd = -1;
	// 注意：epoll 初始化应该在 main 函数中，而不是在循环里
	server_epoll_init(&epoll_fd, server_sockfd, EPOLLIN, 100, ET); // 使用 ET 模式

	// 服务器通信循环
	// 注意：阻塞模式参数 blockmode 仅在 ET 模式下有意义，这里设为 NONBLOCK
	server_communicate(epoll_fd, server_sockfd, ET, NONBLOCK);

	// 关闭监听套接字 (理论上服务器无限循环时不会执行到这里，除非有退出机制)
	close(server_sockfd);
	// 关闭 epoll 文件描述符 (同上)
	close(epoll_fd);

	return 0;
}