#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>


/*
    eg.
    input : ./open my.txt
    argc = 2
    argv[0] = "./open"
    argv[1] = my.txt
*/

int main(int argc, char **argv)
{
    //打印用法
    if(argc != 2)
    {
        printf("Usage : %s <file>\n", argv[0]);
        return -1;
    }
    //以可读可写(O_RDWR)的方式打开
    int fd =open(argv[1], O_RDWR);

    //打开失败
    if(fd<0)
    {
        printf("open file failed %s\n",argv[1]);
        printf("errno = %d\n",errno);
        printf("err : %s\n",strerror(errno));
        perror("open");
    }

    while (1)
    {
        //休眠10秒
        sleep(10);
    }
    close(fd);
    return 0;
}