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

struct pin_desc{
	int pin;
	int key_val;
	char *name;
	int eint;
};
static struct pin_desc *irq_pd;
static struct input_dev *buttons_dev;
static struct timer_list buttons_timer;//声明定时器结构体
struct pin_desc pins_desc[]={

	{ 
		.pin = S5PV210_GPH0(0),
		.key_val = KEY_L,      
		.name = "s0",
		.eint = IRQ_EINT(0),
	},
	{ 
		.pin = S5PV210_GPH0(1),
		.key_val = KEY_S,      
		.name = "s1",
		.eint = IRQ_EINT(1),
	},
	{ 
		.pin = S5PV210_GPH0(2),
		.key_val = KEY_ENTER,      
		.name = "s2",
		.eint = IRQ_EINT(2),
	},
	{ 
		.pin = S5PV210_GPH0(3),
		.key_val = KEY_LEFTSHIFT,      
		.name = "s3",
		.eint = IRQ_EINT(3),
	},
};

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	
	/* 10ms后启动定时器 */
	irq_pd = (struct pin_desc *)dev_id;
	mod_timer(&buttons_timer, jiffies+HZ/100);//设定定时器的值
	
    	return IRQ_RETVAL(IRQ_HANDLED);
}

static void buttons_timer_function(unsigned long data)
{
	struct pin_desc * pindesc = irq_pd;
	int val;
	val = gpio_get_value(pindesc->pin);
	if(val)
	{
		/* 松开 : 最后一个参数: 0-松开, 1-按下 */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);
		input_sync(buttons_dev);
	}
	else
	{
		/* 按下 */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);
		input_sync(buttons_dev);
	}
}



int input_keys_init(void)
{
	int i;
	int ret;
	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;
	add_timer(&buttons_timer); 
	/* 1. 分配一个input_dev结构体 */
	buttons_dev = input_allocate_device();;

	/* 2. 设置 */
	/* 2.1 能产生哪类事件 */
	set_bit(EV_KEY, buttons_dev->evbit);
	set_bit(EV_REP, buttons_dev->evbit);
	
	/* 2.2 能产生这类操作里的哪些事件: L,S,ENTER,LEFTSHIT */
	set_bit(KEY_L, buttons_dev->keybit);
	set_bit(KEY_S, buttons_dev->keybit);
	set_bit(KEY_ENTER, buttons_dev->keybit);
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);
	/* 3. 注册 */
	ret = input_register_device(buttons_dev);

	for(i=0;i<4;i++)
	{
		ret = request_irq(pins_desc[i].eint,buttons_irq, IRQF_TRIGGER_FALLING|IRQ_TYPE_EDGE_RISING, pins_desc[i].name, &pins_desc[i]);
	}
	return 0;
}

void input_keys_exit(void)
{
	int i;
	for(i=0;i<4;i++)
	{
		free_irq(pins_desc[i].eint, &pins_desc[i]);
	}
	
	del_timer(&buttons_timer);
	input_unregister_device(buttons_dev);
	input_free_device(buttons_dev);	
}
module_init(input_keys_init);
module_exit(input_keys_exit);

MODULE_LICENSE("GPL");
