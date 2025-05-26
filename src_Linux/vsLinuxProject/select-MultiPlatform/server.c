//#include "select_server.h"
#include "select_server_thread.h"
#include <stdio.h>
int main(void)
{
	select_pthread_init(8080, COMM_TCP, 5);
	printf("Server started.\n");
	server_pthread_start();

	// 等待用户输入以退出
	printf("Press Enter to exit...\n");
	getchar();

	server_pthread_close();

	return 0;
}