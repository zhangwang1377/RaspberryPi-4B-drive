#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/signal.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/capi.h>
#include <linux/kernelcapi.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/moduleparam.h>

#include "button_drv.h"

/*
*  查看树莓派GPIO地址映射基地址 cat /proc/iomem
*  fe200000-fe2000b3 : fe200000.gpio    , fe200000为GPIO基地址，
*
*  button gpio20 1-up,0-down  
*  gpio20 GPFSEL2(0x08) 2:0  000 = input 001 = output
*
*  0x08 GPFSEL2 GPIO Function Select 2
*  0x34 GPLEV0 31:0 LEVn (n=0..31), 0 = GPIO pin n is low, 1 = GPIO pin n is high
*/
#define BCM2711_GPIO_BASE 0xfe200000

static volatile unsigned int *GPFSEL2;
static volatile unsigned int *GPLEV0;


static void board_rpi_4b_button_init_gpio(int which)
{
   unsigned int val;

    //printk("%s %s line %d, led %d\n", __FILE__, __FUNCTION__, __LINE__, which);
    if (which == 0)
    {
        if (!GPFSEL2)
        {
            GPFSEL2 = ioremap(BCM2711_GPIO_BASE + 0x08, 4);
            GPLEV0 = ioremap(BCM2711_GPIO_BASE + 0x34, 4);
        }

        val = *GPFSEL2;
        val &= ~(0x111 << 0);
        *GPFSEL2 = val;  //input
    }
}

static int board_rpi_4b_button_read_gpio(int which)
{
     //printk("%s %s line %d, led %d, %s\n", __FILE__, __FUNCTION__, __LINE__, which, status ? "on" : "off");
    if (which == 0)
    {
        return (*GPLEV0 & (1<<20)) ? 0 : 1; //返回按键状态，非物理电平
    }else
    {
        return 0;
    }
}

static struct button_operations my_button_ops ={
    .count = 1,
    .init = board_rpi_4b_button_init_gpio,
    .read = board_rpi_4b_button_read_gpio,
};

static int __init board_rpi_4b_button_init(void)
{
    register_button_operations(&my_button_ops);
    return 0;
}

static void __exit board_rpi_4b_button_exit(void)
{
    unregister_button_operations();
}


module_init(board_rpi_4b_button_init);
module_exit(board_rpi_4b_button_exit);

MODULE_LICENSE("GPL");