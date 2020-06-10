#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>

struct cdev *led_cdev;
dev_t devno;
static volatile unsigned int *led_config;
static volatile unsigned int *led_dat;
static int pin;
static struct class *led_class;
static struct class_device	*led_class_dev;


static int led_open(struct inode *devno, struct file *filp)
{
	writel(0x00011000,led_config);
	return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t size, loff_t *opp)
{
	int val;
	int ret;
	ret = copy_from_user(&val, buf, size);
	if(val == 1)
	{
		*led_dat |= (1<<pin);
	}
	else 
	{
		*led_dat &= ~(1<<pin);
	}
	return size;
}
struct file_operations led_filp = {
	.open = led_open,
	.write = led_write,
	.owner = THIS_MODULE,
};

static int led_probe(struct platform_device *pdev)
{
	struct resource	*res;
	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	led_config = ioremap(res->start,res->end - res->start + 1);
	led_dat = led_config + 1;
	
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	pin = res->start;
	
	printk("led_probe\n");
	cdev_init(&led_cdev,&led_filp);
	alloc_chrdev_region(&devno,0, 2,"myleds");
	cdev_add(&led_cdev, devno,2);
	
	led_class = class_create(THIS_MODULE, "myled");

	led_class_dev = device_create(led_class, NULL, devno, NULL, "led"); /* /dev/led */
	
	
	return 0;
}

static int led_remove(struct platform_device *dev)
{
	cdev_del(&led_cdev);
	unregister_chrdev_region(devno,2);
	iounmap(led_config);
	device_del(led_class_dev);
	class_destroy(led_class);
	return 0;
}

static struct platform_driver gpio_driver = {
       .driver = 
       {
	 .name   = "led_test",
	},
	 .probe     = led_probe,
	 .remove    = led_remove,
};
static int gpio_driver_init(void)
{
	platform_driver_register(&gpio_driver);
	return 0;
}

static void gpio_driver_exit(void)
{
	platform_driver_unregister(&gpio_driver);
}
module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
