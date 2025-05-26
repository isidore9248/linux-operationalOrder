#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

//*noblock---是否以阻塞的方式进行查询
// 默认阻塞模式
/* ./xxx /dev/input/event0  [noblock]  */
int main(int argc, char **argv)
{
    int fd;
    if (argc == 3 && !strcmp(argv[2], "noblock"))
    {
        fd = open(argv[1], O_RDWR | O_NONBLOCK);
    }
    else
    {
        fd = open(argv[1], O_RDWR);
    }
    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    struct input_event ev;
    while (true)
    {
        //*使用 read 函数从输入设备中读取事件。
        //*read 函数需要三个参数：
        //*第一个参数是文件描述符（fd），
        //*第二个参数是 event 结构体，
        //*第三个参数是 event 结构体的大小。
        ssize_t bytesRead = read(fd, &ev, sizeof(struct input_event));

        //?以阻塞态打开设备
        if (bytesRead == sizeof(struct input_event))
        {
            printf("block\n");
            printf("Event type: %d, Code: %d, Value: %d\n", ev.type, ev.code, ev.value);
        }
        //?以非阻塞打开
        else
        {
            printf("no-block:");
            printf("read input information:%d\n", bytesRead);
        }

        //! 如果以非阻塞态读取设备输入信息时
        //! 如果成功读取到设备信息,则会立刻停止输出
    }
    close(fd);
    return 0;
}