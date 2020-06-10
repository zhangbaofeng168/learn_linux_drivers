#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/gpio.h>
#include <linux/cdev.h>


struct cdev key_dev;
dev_t devno;

static struct class *keydrv_class;
static struct class_device	*key_class_dev;
static unsigned char key_val;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

struct pin_desc{
	unsigned int irq;
	unsigned int pin;
	unsigned int key_val;
	unsigned char *name;
};

struct pin_desc pins_desc[8] = {
	{IRQ_EINT(0),S5PV210_GPH0(0), 0x00,"key0"},
	{IRQ_EINT(1),S5PV210_GPH0(1), 0x01,"key1"},
	{IRQ_EINT(2),S5PV210_GPH0(2), 0x02,"key2"},
	{IRQ_EINT(3),S5PV210_GPH0(3), 0x03,"key3"},
	
	{IRQ_EINT(4),S5PV210_GPH0(4), 0x04,"key4"},
	{IRQ_EINT(5),S5PV210_GPH0(5), 0x05,"key5"},
	
	{IRQ_EINT(22),S5PV210_GPH2(6), 0x06,"key6"},
	{IRQ_EINT(23),S5PV210_GPH2(7), 0x07,"key7"},
};
// 中断事件标志, 中断服务程序将它置1，key_read将它清0 

static volatile int ev_press = 0;

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	
	pinval = gpio_get_value(pindesc->pin);

	if (pinval)
	{
		//松开 
		key_val = 0x80 | pindesc->key_val;
	}
	else
	{
		//按下 
		key_val = pindesc->key_val;
	}

    	ev_press = 1;                  // 表示中断发生了 
    	wake_up_interruptible(&button_waitq);   // 唤醒休眠的进程 

	
	return IRQ_RETVAL(IRQ_HANDLED);
}

int  button_open(struct inode *devno, struct file *filp)
{
	int ret;
	int i;
	for(i = 0; i < 8; i++)
	{
 		ret = request_irq(pins_desc[i].irq,buttons_irq, IRQF_TRIGGER_FALLING|IRQ_TYPE_EDGE_RISING, pins_desc[i].name, &pins_desc[i]);
 		if(ret)
 		{
 			printk(" %s request_irq fail!\n",pins_desc[i].name);
 		}
	}
 	return 0;
}

ssize_t  button_read(struct file *filp, char __user *buf, size_t size, loff_t *opp)
{
	int ret;
	wait_event_interruptible(button_waitq, ev_press);
	// 如果有按键动作, 返回键值 
	ret=copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	return 1;
}
int button_close(struct inode *devno, struct file *filp)
{
	for(i = 0; i < 8; i++)
	{
		free_irq(pins_desc[i].irq, &pins_desc[i]);
	}
	return 0;
}

static const struct file_operations keydev_fops = {
	.owner		= THIS_MODULE,
	.read		=  button_read,
	.open		=  button_open,
	.release	=  button_close,
};
int key_irq_init(void)
{
	cdev_init(&key_dev, &keydev_fops);
	alloc_chrdev_region(&devno, 0, 2,"butdrv");//动态分配设备号
	cdev_add(&key_dev, devno, 2);
	
	keydrv_class = class_create(THIS_MODULE, "button");
	key_class_dev = device_create(keydrv_class, NULL, devno, NULL, "buttons");
	
	return 0;	
}

void key_irq_exit(void)
{
	device_del(key_class_dev);
	class_destroy(keydrv_class);
	
	cdev_del(&key_dev);
	unregister_chrdev_region(devno, 2);
}

MODULE_LICENSE("GPL");
module_init(key_irq_init);
module_exit(key_irq_exit);

/*
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/gpio.h>
#include <linux/cdev.h>
#include <linux/poll.h>

#include <linux/platform_device.h>
#include <linux/input.h>


static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
static volatile int ev_press = 0;
struct cdev *button_dev;
static dev_t devno;
static unsigned char key_value;
struct pin_desc{
	int pin;
	int val;
	char *name;
	int eint;
};
struct pin_desc pins_desc[]={

	{ 
		.pin = S5PV210_GPH0(0),
		.val = 0x0,      
		.name = "s0",
		.eint = IRQ_EINT(0),
	},
	{ 
		.pin = S5PV210_GPH0(1),
		.val = 0x1,      
		.name = "s1",
		.eint = IRQ_EINT(1),
	},
	{ 
		.pin = S5PV210_GPH0(2),
		.val = 0x2,      
		.name = "s2",
		.eint = IRQ_EINT(2),
	},
	{ 
		.pin = S5PV210_GPH0(3),
		.val = 0x3,      
		.name = "s3",
		.eint = IRQ_EINT(3),
	},
};

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc * pin_temp = (struct pin_desc *)dev_id;
	int val;
	val = gpio_get_value(pin_temp->pin);
	if(val)
	{
		key_value = 0x80 | pin_temp->val;
	}
	else
	{
		key_value = pin_temp->val;
	}
	ev_press = 1;                  //表示中断发生了 
    	wake_up_interruptible(&button_waitq);   // 唤醒休眠的进程 
    	return IRQ_RETVAL(IRQ_HANDLED);
}
int button_open(struct inode *devno, struct file *filp)
{
	int i;
	for(i=0;i<4;i++)
	{
		request_irq(pins_desc[i].eint,buttons_irq, IRQF_TRIGGER_FALLING|IRQ_TYPE_EDGE_RISING, pins_desc[i].name, &pins_desc[i]);
	}
	return 0;
}

ssize_t button_read(struct file *filp, char __user *buf, size_t size, loff_t *ops)
{
	int ret;
	wait_event_interruptible(button_waitq, ev_press);
	//如果有按键动作, 返回键值
	ret=copy_to_user(buf, &key_value, 1);
	ev_press = 0;
	return 1;
}
struct file_operations button_fips = {
	.owner	= THIS_MODULE,
	.open   = button_open,
	.read   = button_read,

};
int input_keys_init(void)
{
	cdev_init(&button_dev,&button_fips);
	alloc_chrdev_region(&devno, 0, 2,"button");
	cdev_add(&button_dev,devno,2);
	
	return 0;
}

void input_keys_exit(void)
{
	int i;
	for(i=0;i<4;i++)
	{
		free_irq(pins_desc[i].eint, &pins_desc[i]);
	}
	unregister_chrdev_region(devno,2);
	cdev_del(&button_dev);
}
module_init(input_keys_init);
module_exit(input_keys_exit);

MODULE_LICENSE("GPL");*/
