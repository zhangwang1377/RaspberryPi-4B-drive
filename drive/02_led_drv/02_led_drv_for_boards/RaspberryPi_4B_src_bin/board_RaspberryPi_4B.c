#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <asm/io.h>
#include "led_opr.h"

/*
*  查看树莓派GPIO地址映射基地址 cat /proc/iomem
*  fe200000-fe2000b3 : fe200000.gpio    , fe200000为GPIO基地址，
*
*  led gpio21 1-亮,0-灭  
*  gpio21 GPFSEL2(0x08) 5:3  000 = input 001 = output
*
*  0x08 GPFSEL2 GPIO Function Select 2
*  0x1C GPSET0 GPIO Pin Output Set 0     (0-31)
*  0x20 GPSET1 GPIO Pin Output Set 1     (32-57)
*  0x28 GPCLR0 GPIO Pin Output Clear 0   (0-31)
*  0x2C GPCLR1 GPIO Pin Output Clear 1   (32-57)
*/
#define BCM2711_GPIO_BASE 0xfe200000

static volatile unsigned int *GPFSEL2;
static volatile unsigned int *GPSET0;
static volatile unsigned int *GPCLR0;


static int board_demo_led_init(int which) /* 初始化LED, which-哪个LED */
{
    unsigned int val;

    //printk("%s %s line %d, led %d\n", __FILE__, __FUNCTION__, __LINE__, which);
    if (which == 0)
    {
        if (!GPFSEL2)
        {
            GPFSEL2 = ioremap(BCM2711_GPIO_BASE + 0x08, 4);
            GPSET0 = ioremap(BCM2711_GPIO_BASE + 0x1C, 4);
            GPCLR0 = ioremap(BCM2711_GPIO_BASE + 0x28, 4);
        }

        val = *GPFSEL2;
        val &= ~(0x111 << 3);
        val |= (0x001 << 3);
        *GPFSEL2 = val;
        
    }

    return 0;
}
static int board_demo_led_ctl(int which, char status) /* 控制LED, which-哪个LED, status:1-亮,0-灭 */
{
    //printk("%s %s line %d, led %d, %s\n", __FILE__, __FUNCTION__, __LINE__, which, status ? "on" : "off");
    if (which == 0)
    {
        if (status)
        {
            *GPSET0 |= (0x1 << 21);
        }
        else
        {
            *GPCLR0 |= (0x1 << 21);
        }
    }

    return 0;
}

static struct led_operations board_demo_led_opr = {
    .num = 1,
    .init = board_demo_led_init,
    .ctl = board_demo_led_ctl,
};

struct led_operations *get_board_led_opr(void)
{
    return &board_demo_led_opr;
}