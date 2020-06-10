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


struct cdev key_dev;
dev_t devno;

static struct class *keydrv_class;
static struct class_device	*key_class_dev;
static unsigned char key_val;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);// 生成一个等待队列头

struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};

struct pin_desc pins_desc[8] = {
	{S5PV210_GPH0(0), 0x00},
	{S5PV210_GPH0(1), 0x01},
	{S5PV210_GPH0(2), 0x02},
	{S5PV210_GPH0(3), 0x03},
	
	{S5PV210_GPH0(4), 0x04},
	{S5PV210_GPH0(5), 0x05},
	
	{S5PV210_GPH2(6), 0x06},
	{S5PV210_GPH2(7), 0x07},
};
/* 中断事件标志, 中断服务程序将它置1，key_read将它清0 */
static volatile int ev_press = 0;
static struct fasync_struct *button_async;

//static atomic_t canopen = ATOMIC_INIT(1);     //定义原子变量并初始化为1

static DECLARE_MUTEX(button_lock);     //定义互斥锁

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	
	pinval = gpio_get_value(pindesc->pin);

	if (pinval)
	{
		/* 松开 */
		key_val = 0x80 | pindesc->key_val;
	}
	else
	{
		/* 按下 */
		key_val = pindesc->key_val;
	}

    	ev_press = 1;                  /* 表示中断发生了 */
    	wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
	kill_fasync (&button_async, SIGIO, POLL_IN);
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

int  button_open(struct inode *devno, struct file *filp)
{
	int ret;
#if 0	
	if (!atomic_dec_and_test(&canopen))
	{
		atomic_inc(&canopen);
		return -EBUSY;
	}
#endif		

	if (filp->f_flags & O_NONBLOCK)
	{
		if (down_trylock(&button_lock))
			return -EBUSY;
	}
	else
	{
		/* 获取信号量 */
		down(&button_lock);
	}
	
 	ret = request_irq(IRQ_EINT(0),buttons_irq, IRQF_TRIGGER_FALLING|IRQ_TYPE_EDGE_RISING, "key1", &pins_desc[0]);
 	if(ret){printk(" request_irq fail!\n");}
 	ret = request_irq(IRQ_EINT(1),buttons_irq, IRQF_TRIGGER_FALLING|IRQ_TYPE_EDGE_RISING, "key2", &pins_desc[1]);
 	if(ret){printk(" request_irq fail!\n");}
 	ret = request_irq(IRQ_EINT(2),buttons_irq, IRQF_TRIGGER_FALLING|IRQ_TYPE_EDGE_RISING, "key3", &pins_desc[2]);
 	if(ret){printk(" request_irq fail!\n");}
 	ret = request_irq(IRQ_EINT(3),buttons_irq, IRQF_TRIGGER_FALLING|IRQ_TYPE_EDGE_RISING, "key4", &pins_desc[3]);
 	if(ret){printk(" request_irq fail!\n");}
 	ret = request_irq(IRQ_EINT(4),buttons_irq, IRQF_TRIGGER_FALLING|IRQ_TYPE_EDGE_RISING, "key5", &pins_desc[4]);
 	if(ret){printk(" request_irq fail!\n");}
 	ret = request_irq(IRQ_EINT(5),buttons_irq, IRQF_TRIGGER_FALLING|IRQ_TYPE_EDGE_RISING, "key6", &pins_desc[5]);
 	if(ret){printk(" request_irq fail!\n");}
 	ret = request_irq(IRQ_EINT(22),buttons_irq, IRQF_TRIGGER_FALLING|IRQ_TYPE_EDGE_RISING, "key7", &pins_desc[6]);
 	if(ret){printk(" request_irq fail!\n");}
 	ret = request_irq(IRQ_EINT(23),buttons_irq, IRQF_TRIGGER_FALLING|IRQ_TYPE_EDGE_RISING, "key8", &pins_desc[7]);
 	if(ret){printk(" request_irq fail!\n");}
 	return 0;
}

ssize_t  button_read(struct file *filp, char __user *buf, size_t size, loff_t *opp)
{
	int ret;
	if (filp->f_flags & O_NONBLOCK)//非阻塞
	{
		if (!ev_press)
			return -EAGAIN;
	}
	else
	{
		/* 如果没有按键动作, 休眠 */
		wait_event_interruptible(button_waitq, ev_press);
	}
	/* 如果有按键动作, 返回键值 */
	ret=copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	return 1;
}
int button_close(struct inode *devno, struct file *filp)
{
	//atomic_inc(&canopen);
	up(&button_lock);
	free_irq(IRQ_EINT(0), &pins_desc[0]);
	free_irq(IRQ_EINT(1), &pins_desc[1]);
	free_irq(IRQ_EINT(2), &pins_desc[2]);
	free_irq(IRQ_EINT(3), &pins_desc[3]);
	free_irq(IRQ_EINT(4), &pins_desc[4]);
	free_irq(IRQ_EINT(5), &pins_desc[5]);
	free_irq(IRQ_EINT(22), &pins_desc[6]);
	free_irq(IRQ_EINT(23), &pins_desc[7]);
	return 0;
}
unsigned int button_poll(struct file *filp, struct poll_table_struct *wait)
{
	unsigned int mask = 0;
	poll_wait(filp,&button_waitq,wait);//把当前进程放入队列
	
	if(ev_press)
		mask = POLLIN | POLLRDNORM;//如果有时间则唤醒进程，否则mask=0
	
	return mask; 
}

static int button_fasync (int fd, struct file *filp, int on)
{
	printk("driver: button_drv_fasync\n");
	return fasync_helper(fd, filp, on, &button_async);
}
static const struct file_operations keydev_fops = {
	.owner		=  THIS_MODULE,
	.read		=  button_read,
	.open		=  button_open,
	.release	=  button_close,
	.poll		=  button_poll,
	.fasync	        =  button_fasync,
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
	cdev_del(&key_dev);
	unregister_chrdev_region(devno, 2);
	
	device_del(key_class_dev);
	class_destroy(keydrv_class);
}

MODULE_LICENSE("GPL");
module_init(key_irq_init);
module_exit(key_irq_exit);