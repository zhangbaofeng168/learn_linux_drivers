#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/device.h>


#define LEDCON 0xE0200060
#define LEDDAT 0xE0200064
unsigned int *led_config; 
unsigned int *led_data;

struct cdev led_dev;
static struct class *myfirstdrv_class;
static struct class_device	*myfirstdrv_class_dev;
dev_t devn;

int led_open(struct inode *node, struct file *filp)
{
	led_config = ioremap(LEDCON,4);
	writel(0x00011000,led_config);
	
	led_data = ioremap(LEDDAT,4);	
	return 0;
	
}
int led_close(struct inode *node, struct file *filp)
{
	return 0;
}

ssize_t led_write(struct file *filp, const char __user *buff, size_t size, loff_t *ppos)
{
	int val;

	copy_from_user(&val, buff, size);
	
	if(val == 1)
	{
		writel(0x18,led_data);
	}
	else 
	{
		writel(0x00,led_data);
	}
	return size;
}
static const struct file_operations ledfops = {
	.owner  =   THIS_MODULE,
	.open = led_open,
	.release = led_close,
	.write = led_write,
};

int leddev_init(void)
{
	cdev_init(&led_dev, &ledfops);
	
	alloc_chrdev_region(&devn, 0, 2,"leddev");//动态分配设备号
	cdev_add(&led_dev, devn, 2);
	
	myfirstdrv_class = class_create(THIS_MODULE, "myfirstdrv");

	myfirstdrv_class_dev = device_create(myfirstdrv_class, NULL, devn, NULL, "leddev0");

	return 0;
}

void leddev_exit(void)
{
	cdev_del(&led_dev);
	unregister_chrdev_region(devn, 2);
	iounmap(led_config);
	iounmap(led_data);
	
	device_del(myfirstdrv_class_dev);
	class_destroy(myfirstdrv_class);
}


MODULE_LICENSE("GPL");

module_init(leddev_init);
module_exit(leddev_exit);