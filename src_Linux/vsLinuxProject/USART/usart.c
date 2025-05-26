#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <pthread.h>

void* send_thread(void* arg);
void* recv_thread(void* arg);

// 设置数据位数
void set_bits(struct termios* newtio, int nBits)
{
	switch (nBits)
	{
	case 7:
		newtio->c_cflag |= CS7;
		break;
	case 8:
		newtio->c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data bits: %d\n", nBits);
		// 设置默认值
		newtio->c_cflag |= CS8;
		break;
	}
}

// 设置奇偶校验
void set_event(struct termios* newtio, char nEvent)
{
	switch (nEvent)
	{
	case 'O': // 奇校验
		newtio->c_cflag |= PARENB;
		newtio->c_cflag |= PARODD;
		newtio->c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E': // 偶校验
		newtio->c_iflag |= (INPCK | ISTRIP);
		newtio->c_cflag |= PARENB;
		newtio->c_cflag &= ~PARODD;
		break;
	case 'N': // 无校验
		newtio->c_cflag &= ~PARENB;
		break;
	default:
		fprintf(stderr, "Unsupported parity event: %c\n", nEvent);
		// 设置默认值
		newtio->c_cflag &= ~PARENB;
		break;
	}
}

// 设置波特率
void set_baud(struct termios* newtio, int nBaud)
{
	switch (nBaud)
	{
	case 2400:
		cfsetispeed(newtio, B2400);
		cfsetospeed(newtio, B2400);
		break;
	case 4800:
		cfsetispeed(newtio, B4800);
		cfsetospeed(newtio, B4800);
		break;
	case 9600:
		cfsetispeed(newtio, B9600);
		cfsetospeed(newtio, B9600);
		break;
	case 115200:
		cfsetispeed(newtio, B115200);
		cfsetospeed(newtio, B115200);
		break;
	default:
		fprintf(stderr, "Unsupported baud rate: %d\n", nBaud);
		// 设置默认值
		cfsetispeed(newtio, B9600);
		cfsetospeed(newtio, B9600);
		break;
	}
}

// 设置停止位
void set_stop(struct termios* newtio, int nStop)
{
	if (nStop == 1)
		newtio->c_cflag &= ~CSTOPB;
	else if (nStop == 2)
		newtio->c_cflag |= CSTOPB;
	else
	{
		fprintf(stderr, "Unsupported stop bits: %d\n", nStop);
		// 设置默认值 1位停止位
		newtio->c_cflag &= ~CSTOPB;
	}
}

/// <summary>设置串口参数</summary>
/// <param name="fd">串口文件描述符</param>
/// <param name="nBaud">波特率</param>
/// <param name="nBits">数据位</param>
/// <param name="nEvent">奇偶校验</param>
/// <param name="nStop">停止位</param>
/// <returns>成功返回0，失败返回-1</returns>
/// <example>set_opt(fd,115200,8,'N',1)
int set_serial(int fd, int nBaud, int nBits, char nEvent, int nStop)
{
	struct termios newtio, oldtio;

	if (tcgetattr(fd, &oldtio) != 0) { perror("SetupSerial 1"); return -1; }

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	newtio.c_oflag &= ~OPOST;   /*Output*/

	set_bits(&newtio, nBits);
	set_event(&newtio, nEvent);
	set_baud(&newtio, nBaud);
	set_stop(&newtio, nStop);

	// 设置等待时间和最小接受字符，没读到足够数据就不返回
	newtio.c_cc[VMIN] = 1;

	//等待第1个数据的时间
	//为0:	如果没有数据到达，程序将会一直等待，直到至少有一个字节的数据到达为止。
	//设置为非零值，例如x:	读取操作会等待最多 x*10 秒，如果在这段时间内没有数据到达，读取操作将会返回。
	newtio.c_cc[VTIME] = 0;

	//确保清除任何残留的输入数据，避免旧数据干扰新的通信。
	tcflush(fd, TCIFLUSH);
	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) { perror("com set error"); return -1; }
	//printf("set done!\n");
	return 0;
}

/// <summary> </summary>		打开串口设备文件
/// <param name="com"></param>	这是一个字符串，表示要打开的设备文件的路径，例如 /dev/ttySAC1。
/// <returns></returns>			fd设备文件描述符
int open_serial(char* com)
{
	//O_RDWR:	表示以读写模式打开文件
	//O_NOCTTY:	表示不要将串口设备作为控制终端。
	//O_NDELAY:	以非阻塞模式打开串口设备
	int fd = open(com, O_RDWR | O_NOCTTY /*| O_NDELAY*/);
	if (-1 == fd) { return(-1); }

	/* 设置串口为阻塞状态*/
	if (fcntl(fd, F_SETFL, 0) < 0) { printf("fcntl failed!\n"); return -1; }

	return fd;
}

/*
 * ./serial_send_recv <dev>
 */
int main(int argc, char** argv)
{
	//if (argc != 2) { printf("Usage: \n"); printf("%s </dev/ttySAC1 or other>\n", argv[0]); return -1; }

	char* com = "/dev/ttyS1";
	int fd = open_serial(com);
	if (fd < 0) { printf("open %s err!\n", com); return -1; }

	int ret_set = set_serial(fd, 115200, 8, 'N', 1);
	if (ret_set) { printf("set port err!\n"); return -1; }

	pthread_t send_tid, recv_tid;
	pthread_create(&send_tid, NULL, send_thread, (void*)&fd);
	pthread_create(&recv_tid, NULL, recv_thread, (void*)&fd);
	pthread_join(send_tid, NULL);
	pthread_join(recv_tid, NULL);

	return 0;
}

#define BUFFER_SIZE 1024
void* send_thread(void* arg)
{
	char* send_buf;
	for (;;)
	{
		printf("enter your send message\n");
		fgets(send_buf, BUFFER_SIZE, stdin);
		send_buf[strlen(send_buf) - 1] = '\n';
		send_buf[strlen(send_buf) - 1] = '\0';
		write((*(int*)arg), send_buf, sizeof(send_buf));
	}
}

void* recv_thread(void* arg)
{
	for (;;)
	{
		char recv_buf[BUFFER_SIZE];
		int iRet = read((*(int*)arg), recv_buf, sizeof(recv_buf));
		if (iRet > 0) { printf("get: %s\n", recv_buf); 		printf("enter your send message\n"); }
		else { printf("can not get data\n"); }
	}
}