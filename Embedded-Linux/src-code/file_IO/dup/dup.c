/*
int ret = dup(fd) :  在文件总线中找到一个最小数值，返回给ret  
//*得到的新的ret与fd指向的文件描述符相同

dup2(fd2, fd1)    
//*用fd2代替fd1，fd2被关闭
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd1=open(argv[1],O_RDWR | O_CREAT | O_TRUNC);
    int fd2=dup(fd1);   //
    printf("fd1=%d\n",fd1);
    printf("fd2=%d\n",fd2);

    dup2(fd2,1);
    printf("hello world\n");

}
