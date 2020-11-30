#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <signal.h>

static int fd;
static void sig_func(int sig)
{
    int val;
    read(fd, &val, 4);
    printf("get button : 0x%x\n", val);
}

/*
 * ./button_test /dev/pi_gpio_key
 *
 */

int main(int argc, char const *argv[])
{
    int val;
    struct pollfd fds[1];
    int timeout_ms = 5000;
    int ret;
    int flags;

    /* 1. 判断参数 */
    if (argc != 2)
    {
        printf("Usage: %s <dev>\n", argv[0]);
        return -1;
    }

    signal(SIGIO, sig_func);

    /* 2. 打开文件 */
    fd = open(argv[1], O_RDWR);
    if (fd == -1)
    {
        printf("can not open file %s\n", argv[1]);
        return -1;
    }

    fcntl(fd, F_SETOWN, getpid());
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | FASYNC);

    while (1)
    {
        printf("RaspberryPi-4B \n");
        sleep(5);
    }

    close(fd);

    return 0;
}
