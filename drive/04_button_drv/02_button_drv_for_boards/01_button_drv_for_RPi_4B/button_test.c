
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/*
 * ./button_test /dev/pi_button0
 *
 */

int main(int argc, char const *argv[])
{
    int fd;
    char val;

     /* 1. 判断参数 */
    if (argc != 2)
    {
        printf("Usage: %s <dev>\n", argv[0]);
        return -1;
    }

    /* 2. 打开文件 */
    fd = open(argv[1], O_RDWR);
    if (fd == -1)
    {
        printf("can not open file %s\n", argv[1]);
        return -1;
    }

    /* 3. 读文件 */
    while (1)
    {
        read(fd, &val, 1);
        printf("\033[1A\033[K");  //覆盖当前行打印 
        printf("get button : %d\n", val);
        usleep(3000);
    }

    close(fd);
    
    return 0;
}
