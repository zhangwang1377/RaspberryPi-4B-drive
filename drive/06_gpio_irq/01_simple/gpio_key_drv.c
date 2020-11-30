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
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/gpio/driver.h>

static struct gpio_desc *key_gpio;
static int irq;

static irqreturn_t gpio_key_isr(int irq, void *gpio)
{
	struct gpio_desc *gpio_key = gpio;
	int val;
	val = gpiod_get_value(gpio_key);
	

	printk("key %d %d\n", desc_to_gpio(gpio_key), val);
	
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

    err = request_irq(irq, gpio_key_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "pi_gpio_key", key_gpio);

    return 0;
}
static int gpio_key_remove(struct platform_device *pdev)
{
    free_irq(irq, key_gpio);
    gpiod_put(key_gpio);
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

