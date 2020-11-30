#include <linux/module.h>
#include <linux/poll.h>

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
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <asm/current.h>



static struct gpio_desc *key_gpio;
static int irq;
struct timer_list key_timer;
struct tasklet_struct tasklet;
struct work_struct work;


static int major = 0;
static struct class *gpio_key_class;

static DECLARE_WAIT_QUEUE_HEAD(gpio_key_wait);

/* 环形缓冲区 */
#define BUF_LEN 128
static int g_keys[BUF_LEN];
static int r,w;

struct fasync_struct *button_fasync;

#define NEXT_POS(x)  ((x+1) % BUF_LEN)

static int is_key_buf_empty(void)
{
    return (r == w);
}

static int is_key_buf_full(void)
{
    return (r == NEXT_POS(w));
}

static void put_key(int key)
{
    if (!is_key_buf_full())
    {
        g_keys[w] = key;
        w = NEXT_POS(w);
    }  
}


static int get_key(void)
{
    int key = 0;
    if (!is_key_buf_empty())
    {
        key = g_keys[r];
        r = NEXT_POS(r);
    }
    return key;
}


static void key_timer_expire(struct timer_list  *time)
{
	int val;
    int key;

	val = gpiod_get_value(key_gpio);

	//printk("key %d %d\n", desc_to_gpio(key_gpio), val);
	
    key = (desc_to_gpio(key_gpio) << 8) | val;
    put_key(key);
    wake_up_interruptible(&gpio_key_wait);
    kill_fasync(&button_fasync, SIGIO, POLL_IN);
}

static void key_tasklet_func(unsigned long data)
{
    int val;

    val = gpiod_get_value(key_gpio);

    printk("key_tasklet_func key %d %d\n", desc_to_gpio(key_gpio), val);
}

static void key_work_func(struct work_struct *work)
{
    int val;

    val = gpiod_get_value(key_gpio);

    printk("key_work_func: the process is %s pid %d\n",current->comm, current->pid);	
	printk("key_work_func key %d %d\n", desc_to_gpio(key_gpio), val);
}

static ssize_t gpio_key_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    int err;
    int key;

    if (is_key_buf_empty() && (file->f_flags & O_NONBLOCK))
    {
        return -EAGAIN;
    }

    wait_event_interruptible(gpio_key_wait,!is_key_buf_empty());
    key = get_key();
    err = copy_to_user(buf, &key, 4);

    return 4;
}

static unsigned int gpio_key_drv_poll(struct file *fp, poll_table * wait)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    poll_wait(fp, &gpio_key_wait, wait);
    return is_key_buf_empty() ? 0 : POLLIN | POLLRDNORM; 
}

static int gpio_key_drv_fasync(int fd, struct file *file, int on)
{
    if (fasync_helper(fd, file, on, &button_fasync) >= 0)
        return 0;
    else
        return -EIO;    
}

static struct file_operations gpio_key_drv = {
    .owner = THIS_MODULE,
    .read = gpio_key_drv_read,
    .poll = gpio_key_drv_poll,
    .fasync = gpio_key_drv_fasync,
};

static irqreturn_t gpio_key_isr(int irq, void *gpio)
{
    //struct gpio_desc *gpio_key = gpio;
	//printk("gpio_key_isr key %d irq happened\n", desc_to_gpio(gpio_key));
    tasklet_schedule(&tasklet);
    mod_timer(&key_timer,jiffies + HZ/50);
    schedule_work(&work);

	return IRQ_HANDLED;
}


static int gpio_key_probe(struct platform_device *pdev)
{
    int err;

    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    key_gpio = gpiod_get(&pdev->dev, "key", 0);
    if (IS_ERR(key_gpio))
    {
        dev_err(&pdev->dev, "Failed to get GPIO for key\n");
		return PTR_ERR(key_gpio);
    }
    
    irq = gpiod_to_irq(key_gpio);

    timer_setup(&key_timer, key_timer_expire,0);
    key_timer.expires = ~0;
    add_timer(&key_timer);

    tasklet_init(&tasklet, key_tasklet_func, 0);
    INIT_WORK(&work, key_work_func);

    err = request_irq(irq, gpio_key_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "pi_gpio_key", key_gpio);

    major = register_chrdev(0,"pi_gpio_key", &gpio_key_drv);
    gpio_key_class = class_create(THIS_MODULE, "pi_gpio_key_class");
    if (IS_ERR(gpio_key_class))
    {
        printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
        unregister_chrdev(major, "pi_gpio_key");
        return PTR_ERR(gpio_key_class);
    }

    device_create(gpio_key_class, NULL, MKDEV(major, 0), NULL, "pi_gpio_key"); /* /dev/pi_gpio_key */
    
    return 0;
}
static int gpio_key_remove(struct platform_device *pdev)
{

    device_destroy(gpio_key_class, MKDEV(major, 0));
    class_destroy(gpio_key_class);
    unregister_chrdev(major, "pi_gpio_key");

    free_irq(irq, key_gpio);
    gpiod_put(key_gpio);
    del_timer(&key_timer);
    tasklet_kill(&tasklet);

    return 0;
}

static const struct of_device_id my_pi_keys[] = {
    { .compatible = "pi_4b,my_pi_led_ley" },
    { },
};

static struct platform_driver gpio_key_driver = {
    .probe = gpio_key_probe,
    .remove = gpio_key_remove,
    .driver = {
        .name = "pi_key",
        .of_match_table = my_pi_keys,
    },
};


static int __init gpio_key_init(void)
{
    int err;

    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    err = platform_driver_register(&gpio_key_driver);

    return err;    
}

static void __exit gpio_key_exit(void)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    platform_driver_unregister(&gpio_key_driver); 
}

module_init(gpio_key_init);
module_exit(gpio_key_exit);

MODULE_LICENSE("GPL");

