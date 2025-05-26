//! 编译命令：  gcc -o client client_main.c client.c -lpthread
//! 执行命令：  ./client <server_ip> <server_port>

#include "client.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	//if (argc != 3)
	//{
	//	fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
	//	return -1;
	//}

	//const char* server_ip = argv[1];
	//int server_port = atoi(argv[2]);
	const char* server_ip = "192.168.225.131";
	int server_port = 8080;

	struct server_info info;

	if (client_init(&info, server_ip, server_port) != 0)
	{
		return -1;
	}

	client_start(&info);

	// 等待用户输入以退出
	printf("Press Enter to exit...\n");
	getchar();

	client_close(&info);

	return 0;
}