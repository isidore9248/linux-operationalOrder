/*
! 使用O_CREAT创建文件并赋权限时，并不是与后面给的八进制数字完全相同
! 每个用户存在一个umask，通过umask命令查看，  
! 最终创建的文件获得的权限是 ~umask & Oct_Num
*/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>


/*
    eg.
    input : ./create my.txt
    argc = 2
    argv[0] = "./create"
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

    while (1)
    {
        //休眠10秒
        sleep(10);
    }
    close(fd);
    return 0;
}