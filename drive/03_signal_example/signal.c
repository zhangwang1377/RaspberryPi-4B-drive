#include <stdio.h>
#include <unistd.h>
#include <signal.h>
/*
$ gcc -o signal signal.c // 编译程序
$ ./signal & // 后台运行 
$ ps -A | grep signal // 查看进程 ID，假设是 9527 
$ kill -SIGIO 9527 // 给这个进程发信号
*/

void my_sig_fun(int signo)
{
    printf("get a signal : %d\n",signo);
}

int main(int argc, char const *argv[])
{
    int i = 0;

    signal(SIGIO, my_sig_fun);

    while(1)
    {
        printf("Heelo World %d! \n",i++);
        sleep(2);
    }

    return 0;
}
