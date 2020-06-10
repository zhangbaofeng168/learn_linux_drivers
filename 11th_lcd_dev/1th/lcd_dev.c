
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>

#include <asm/io.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <mach/regs-lcd.h>
#include <mach/regs-gpio.h>
#include <mach/fb.h>

static struct fb_info *info;
static struct fb_ops s5fb_ops = {
	.owner		= THIS_MODULE,
	//.fb_setcolreg	= s5fb_setcolreg,
	.fb_fillrect	= s5fb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= s5fb_imageblit,
};
static int lcd_init(void)
{
	/*1.分配一个fb_info*/
	info = framebuffer_alloc(0,NULL);
	
	/*2.设置*/
	//2.1设置固定的参数
	strcpy(info->fix.id,"mylcd");
	info->fix.smem_len = 800*480*24/8;
	info->fix.type = FB_TYPE_PACKED_PIXELS;
	info->fix.visual  = FB_VISUAL_TRUECOLOR;
	info->fix.line_length = 800 * 3;
	//2.2设置可变的参数 
	info->var.xres           = 800;
	info->var.yres           = 480;
	info->var.xres_virtual   = 800;
	info->var.yres_virtual   = 480;
	info->var.bits_per_pixel = 24;
	
	info->var.red.offset = 16;
	info->var.red.length = 8;
	
	info->var.green.offset = 8;
	info->var.green.length = 8;
	
	info->var.blue.offset = 0;
	info->var.blue.length = 8;
	
	info->var.activate = FB_ACTIVATE_NOW;
	
	//2.3设置可操作函数
	info->fb_ops           = &s5fb_ops;
	//2.4其他设置
	//info->pseudo_palette   = ;
	//info->screen_base      =  //显存的虚拟地址
	info->screen_size      =  800*480*24/8;
	
	/*3.硬件相关的设置*/
	
	//3.1设置相关的gpio用于lcd
	//3.2设置lcd控制器
	//3.3分配framebuffer显存,并把地址告诉lcd控制器
	//info->fix.smem_start       //显存的物理地址
	/*4.注册*/
	
	return 0;
}

static void lcd_exit(void)
{
	
}


module_init(lcd_init);
module_exit(lcd_exit);
MODULE_LICENSE("GPL");