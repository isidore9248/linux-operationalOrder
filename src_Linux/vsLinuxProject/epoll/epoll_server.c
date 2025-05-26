#include "epoll_server.h"

// Helper function to remove fd from array
void remove_client_fd(int fd_to_remove, int* fds, int* count_ptr) {
	int i;
	for (i = 0; i < *count_ptr; ++i) {
		if (fds[i] == fd_to_remove) {
			// 找到 fd，将其后的元素前移覆盖
			for (int j = i; j < (*count_ptr - 1); ++j) {
				fds[j] = fds[j + 1];
			}
			(*count_ptr)--; // 减少计数
			printf("客户端断开: fd=%d, 剩余总数: %d\n", fd_to_remove, *count_ptr);
			break; // 找到并移除后退出循环
		}
	}
}

void broadcast_message(int epoll_fd, int* client_fds, int* client_count_ptr, int sender_fd, const char* message, int msg_len) {
	printf("广播消息给 %d 个客户端 (来自 fd=%d): %.*s\n", *client_count_ptr, sender_fd, msg_len, message);

	// 遍历客户端列表进行发送
	// 注意：遍历时需要小心，因为列表可能在发送过程中因客户端断开而改变
	for (int i = 0; i < *client_count_ptr; ++i) {
		int target_fd = client_fds[i];

		// 不发送给消息的原始发送者
		if (target_fd == sender_fd) {
			continue;
		}

		printf("  发送给 fd=%d...\n", target_fd);
		int sent_len = send(target_fd, message, msg_len, 0);

		if (sent_len == -1) {
			// 发送失败，检查错误类型
			if (errno == EPIPE || errno == ECONNRESET || errno == EBADF) {
				// 这些错误通常表示对端已关闭或无效
				perror("send error during broadcast, client likely disconnected");
				printf("  检测到 fd=%d 在广播时断开，正在移除...\n", target_fd);
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, target_fd, NULL);
				close(target_fd);
				// 从列表中移除，并调整循环变量
				remove_client_fd(target_fd, client_fds, client_count_ptr);
				i--; // 因为移除了一个元素，需要将索引减一以检查下一个元素（现在位于当前索引）
			}
			else if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// 对于非阻塞套接字，发送缓冲区可能满了
				// 简单处理：暂时跳过这个客户端，或者实现更复杂的缓冲机制
				printf("  发送给 fd=%d 时缓冲区满 (EAGAIN/EWOULDBLOCK)，暂时跳过。\n", target_fd);
			}
			else {
				// 其他发送错误
				perror("send error during broadcast");
				// 根据具体错误决定是否移除客户端
			}
		}
		else if (sent_len < msg_len) {
			// 消息未完全发送（对于非阻塞套接字可能发生）
			// 简单处理：打印警告，或者实现更复杂的重试/缓冲逻辑
			printf("  警告: 消息未能完全发送给 fd=%d (%d/%d bytes)\n", target_fd, sent_len, msg_len);
		}
		else {
			printf("  成功发送给 fd=%d\n", target_fd);
		}
	}
	printf("广播完成。\n");
}

/**
 * @brief 初始化服务器套接字
 *
 * @param lfd 指向监听文件描述符的指针
 * @param serv_addr 指向服务器地址结构的指针
 * @param port 端口号
 */
void server_socket_init(int* lfd, struct sockaddr_in* serv_addr, int port)
{
	// 创建套接字
	*lfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*lfd == -1) { perror("socket error"); exit(1); }

	memset(serv_addr, 0, sizeof(*serv_addr));
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_port = htons(port);
	serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);

	int opt = 1;
	// 设置套接字选项
	setsockopt(*lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	int ret = bind(*lfd, (struct sockaddr*)serv_addr, sizeof(*serv_addr));
	if (ret == -1) { perror("bind error"); exit(1); }
	ret = listen(*lfd, 64);
	if (ret == -1) { perror("listen error"); exit(1); }
}

/**
 * @brief 将文件描述符添加到epoll实例中
 *
 * @param epoll_fd epoll实例的文件描述符
 * @param server_sockfd 服务器套接字文件描述符
 * @param events 事件类型
 * @param mode 触发模式（边沿触发或水平触发，搜索 enum TriggerMode）
 */
void add_fd_to_epoll(int epoll_fd, int server_sockfd, uint32_t events, enum TriggerMode mode)
{
	struct epoll_event ev;
	ev.events = events;	if (mode == ET) { ev.events |= EPOLLET; }
	ev.data.fd = server_sockfd;
	// 将文件描述符添加到epoll实例中
	int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sockfd, &ev);
	if (ret == -1) { perror("epoll_ctl"); exit(0); }
}

/**
 * @brief 初始化epoll实例并添加服务器套接字
 *
 * @param epoll_fd 指向epoll实例文件描述符的指针
 * @param server_sockfd 服务器套接字文件描述符
 * @param events 事件类型
 * @param max_events 最大事件数
 * @param mode 触发模式（边沿触发或水平触发，搜索 enum TriggerMode）
 */
void server_epoll_init(int* epoll_fd, int server_sockfd, uint32_t events, int max_events, enum TriggerMode mode)
{
	// 创建epoll实例
	*epoll_fd = epoll_create(max_events);
	if (*epoll_fd == -1) { perror("epoll_create"); exit(0); }

	enum TriggerMode flag = mode;
	add_fd_to_epoll(*epoll_fd, server_sockfd, EPOLLIN, flag);
}

/**
 * @brief 处理客户端连接
 *
 * @param epoll_fd epoll实例的文件描述符
 * @param curfd 当前客户端的文件描述符
 * @param triggermode 触发模式（边沿触发或水平触发，搜索 enum TriggerMode）
 * @param client_fds 客户端文件描述符列表
 * @param client_count_ptr 指向客户端数量的指针
 */
void handle_client(int epoll_fd, int curfd, enum TriggerMode triggermode, int* client_fds, int* client_count_ptr)
{
	if (triggermode == LT)
	{
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		// 接收客户端数据
		int len = recv(curfd, buf, sizeof(buf), 0);
		printf("recv over\n");
		if (len > 0)
		{
			// printf("客户端say: %s\n", buf); // 原回显逻辑
			// send(curfd, buf, len, 0); // 原回显逻辑
			// 调用广播函数
			printf("收到来自 fd=%d 的消息 (LT模式)，准备广播...\n", curfd);
			broadcast_message(epoll_fd, client_fds, client_count_ptr, curfd, buf, len);
		}
		else if (len == 0)
		{
			printf("客户端已经断开了连接: fd=%d\n", curfd);
			// 从epoll实例中删除文件描述符
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, curfd, NULL);
			close(curfd);
			// 从列表中移除
			remove_client_fd(curfd, client_fds, client_count_ptr);
		}
		else
		{
			perror("recv LT");
			// 发生错误也需要移除客户端
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, curfd, NULL);
			close(curfd);
			remove_client_fd(curfd, client_fds, client_count_ptr);
			// exit(0); // 错误时不一定需要退出整个服务器
		}
	}
	else if (triggermode == ET)
	{
		// 处理通信的文件描述符
				// 接收数据
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		// 循环读数据
		while (1)
		{
			int len = recv(curfd, buf, sizeof(buf), 0);
			if (len == 0)
			{
				// 非阻塞模式下和阻塞模式是一样的 => 判断对方是否断开连接
				printf("客户端断开了连接: fd=%d\n", curfd);
				// 将这个文件描述符从epoll模型中删除
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, curfd, NULL);
				close(curfd);
				// 从列表中移除
				remove_client_fd(curfd, client_fds, client_count_ptr);
				break;
			}
			else if (len > 0)
			{
				// 通信
				// 接收的数据打印到终端 (可选)
				// write(STDOUT_FILENO, buf, len);
				// 发送数据 (原回显逻辑)
				// send(curfd, buf, len, 0);
				// 调用广播函数
				// 注意：ET模式下需要循环读取，但广播通常只在完整消息接收后进行
				// 这里简单处理：每次读取到数据都广播，实际应用可能需要缓冲拼接完整消息
				printf("收到来自 fd=%d 的消息 (ET模式, %d bytes)，准备广播...\n", curfd, len);
				broadcast_message(epoll_fd, client_fds, client_count_ptr, curfd, buf, len);
			}
			else
			{
				// len == -1
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					printf("数据读完了 (EAGAIN/EWOULDBLOCK): fd=%d\n", curfd);
					break; // 数据已读完，退出循环
				}
				else {
					perror("recv ET");
					// 发生错误，移除客户端
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, curfd, NULL);
					close(curfd);
					remove_client_fd(curfd, client_fds, client_count_ptr);
					break; // 退出循环
				}
			}
		}
	}
}