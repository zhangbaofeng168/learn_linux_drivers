#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/device.h>

#define  GPH0CON 0xE0200C00
#define  GPH0DAT 0xE0200C04

struct cdev key_dev;
dev_t devno;

static struct class *keydrv_class;
static struct class_device	*key_class_dev;

unsigned int *gph0con;
unsigned int *gph0dat;

int key_open(struct inode *devno, struct file *filp)
{
	gph0con = ioremap(GPH0CON,4);
	*gph0con &=0x0;
	gph0dat = ioremap(GPH0DAT,4);
	
	return 0;
}
ssize_t key_read(struct file *filp, char __user *buf, size_t size, loff_t *opp)
{
	unsigned char val[6];
	int regval;
	regval = *gph0dat;
	val[0] = (regval&0x1) ? 1 : 0;
	val[1] = (regval&0x2) ? 1 : 0;
	val[2] = (regval&0x4) ? 1 : 0;
	val[3] = (regval&0x8) ? 1 : 0;
	val[4] = (regval&0x10) ? 1 : 0;
	val[5] = (regval&0x20) ? 1 : 0;
	copy_to_user(buf, val, sizeof(val));
	
	return sizeof(val);
}
int key_close(struct inode *devno, struct file *filp)
{
	return 0;
}

static const struct file_operations keydev_fops = {
	.owner		= THIS_MODULE,
	.read		= key_read,
	.open		= key_open,
	.release	= key_close,
};
int key_init(void)
{
	cdev_init(&key_dev, &keydev_fops);
	alloc_chrdev_region(&devno, 0, 2,"keydev");//动态分配设备号
	cdev_add(&key_dev, devno, 2);
	
	keydrv_class = class_create(THIS_MODULE, "keydrv");

	key_class_dev = device_create(keydrv_class, NULL, devno, NULL, "keydev0");

	return 0;	
}

void key_exit(void)
{
	device_del(key_class_dev);
	class_destroy(keydrv_class);
	
	cdev_del(&key_dev);
	unregister_chrdev_region(devno, 2);
	
	iounmap(gph0con);
	iounmap(gph0dat);
}

MODULE_LICENSE("GPL");
module_init(key_init);
module_exit(key_exit);