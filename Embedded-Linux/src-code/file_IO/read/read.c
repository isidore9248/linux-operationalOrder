#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>


/*
    eg.
    input : ./read my.txt
    argc = 2
    argv[0] = "./read"
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
    
    //O_RDONLY: 只读
    int fd = open(argv[1], O_RDONLY);

    //打开失败
    if(fd<0)
    {
        printf("open file failed %s\n",argv[1]);
        printf("errno = %d\n",errno);
        printf("err : %s\n",strerror(errno));
        perror("open");
    }

    unsigned char buf[100];
    while(1)
    {
        int len=read(fd,buf,sizeof(buf)-1);
        //如果返回值为 -1，表示读取操作发生错误，
        //并且只有-1表示发生错误， errno 会被设置为相应的错误码。
        if(len<0)
        {
            perror("read");
            close(fd);
            return -1;
        }
        //如果返回值为 0，表示到达文件末尾。
        else if(len==0)
        {
            return 0;
        }
        //如果返回值大于 0，表示成功读取了相应数量的字节
        else
        {
            //添加字符串结束符
            buf[len]='\0';
            printf("%s",buf);
        }
    }

    close(fd);
    return 0;
}