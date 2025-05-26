#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>


/*
    eg.
    input : ./write my.txt str1 str2
    argc = 2
    argv[0] = "./write"
    argv[1] = my.txt
*/

int main(int argc, char **argv)
{
    //打印用法
    if(argc <3)
    {
        printf("Usage : %s <file> <str1> <str2> ...\n", argv[0]);
        return -1;
    }
    //*O_RDWR:可读可写
    //*O_CREAT:表示如果文件不存在，则创建它。
    //当与O_CREAT一起使用时，必须提供第三个参数，它是一个八进制数，表示新创建文件的权限。
    //但是当文件存在时，此参数将被忽略，不可修改其权限
    //*O_TRUNC:如果文件存在且为常规文件，则将其截断为长度为0
    int fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0777);

    //打开失败
    if(fd<0)
    {
        printf("open file failed %s\n",argv[1]);
        printf("errno = %d\n",errno);
        printf("err : %s\n",strerror(errno));
        perror("open");
    }

    for(int i=2;i<argc;i++)
    {
        // write 函数的返回值是实际写入的字节数。
        //如果写入成功，返回值将是传入的缓冲区中的字节长度；
        //如果发生错误，返回值会是 -1，
        //*               str      len
        int len=write(fd,argv[i],strlen(argv[i]));
        //写入的值和输入的值不相等
        if(len!=strlen(argv[i]))
        {
            perror("write");
            break;
        }
        write(fd,"\n",1);
    }


    close(fd);
    return 0;
}