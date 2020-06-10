#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <plat/adc.h>
#include <plat/regs-adc.h>
#include <plat/ts.h>

static struct input_dev *lcd_ts_input;

static int lcd_ts_init(void)
{
	int ret;
	/*1.分配一个input*/
	lcd_ts_input = input_allocate_device();
	/*2.设置*/
	/*2.1能产生哪些事件*/
	set_bit(EV_KEY,lcd_ts_input->evbit);
	set_bit(EV_ABS,lcd_ts_input->evbit);
	/*2.2产生事件里的那些事件*/
	set_bit(BTN_TOUCH,lcd_ts_input->keybit);
	/*2.3参数的设置*/
	input_set_abs_params(lcd_ts_input, ABS_X, 0, 0x3FF, 0, 0);
	input_set_abs_params(lcd_ts_input, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(lcd_ts_input, ABS_PRESSURE, 0, 1, 0, 0);
	/*3.注册*/
	ret = input_register_device(lcd_ts_input);
	
	/*4.硬件相关的操作*/
	
	return 0;
}

static void lcd_ts_exit(void)
{
	
}

module_init(lcd_ts_init);
module_exit(lcd_ts_exit);
MODULE_LICENSE("GPL");
