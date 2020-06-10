
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
#include <mach/regs-gpio.h>


static volatile unsigned int *gpf0con; 
static volatile unsigned int *gpf1con;
static volatile unsigned int *gpf2con;
static volatile unsigned int *gpf3con;
static volatile unsigned int *gpd0con;
static volatile unsigned int *gpd0dat;


static volatile unsigned int *display_control;

static volatile unsigned int *vidcon0;
static volatile unsigned int *vidcon1;
static volatile unsigned int *vidcon2;
static volatile unsigned int *vidcon3;

static volatile unsigned int *vidtcon0;
static volatile unsigned int *vidtcon1;
static volatile unsigned int *vidtcon2;
static volatile unsigned int *vidtcon3;

static volatile unsigned int *wincon0;
static volatile unsigned int *vidosd0a;
static volatile unsigned int *vidosd0b;
static volatile unsigned int *vidosd0c;
static volatile unsigned int *vidw00add0b0;
static volatile unsigned int *vidw00add1b0;
static volatile unsigned int *shodowcon;

#define VBPD (23) //垂直同步信号后肩Vsync back-porch
#define VFPD (22) //垂直同步信号前肩Vsync front-porch
#define VSPW (2) //垂直同步信号脉宽Vsync pulse width

#define HBPD (46) //水平同步信号后肩Hsync back-porch
#define HFPD (210) //水平同步信号前肩Hsync front-porch
#define HSPW (2) //水平同步信号脉宽Hsync pulse width

#define WIDTH 800
#define HEIGHT  480
#define BPP		24

static unsigned int bytes_per_line;
static unsigned int  frame_buffer_size;
//static unsigned int  frame_buffer_addr = 0x28000000;

/* from pxafb.c */
static unsigned long pseudo_palette[16];

static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int s5_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info)
{
	unsigned int val;
	
	if (regno > 16)
		return 1;

	/* 用red,green,blue三原色构造出val */
	val  = chan_to_field(red,	&info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue,	&info->var.blue);
	
	//((u32 *)(info->pseudo_palette))[regno] = val;
	pseudo_palette[regno] = val;
	return 0;
}
static struct fb_info *info;
static struct fb_ops s5fb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= s5_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};
static int lcd_init(void)
{
	/*1.分配一个fb_info*/
	info = framebuffer_alloc(0,NULL);
	if(info == NULL){  
          printk(KERN_ERR "alloc framebuffer failed!\n");  
          return -ENOMEM;  
        }  
	/*2.设置*/
	//2.1设置固定的参数
	strcpy(info->fix.id,"mylcd");
	info->fix.smem_len = 800*480*32/8;
	info->fix.type = FB_TYPE_PACKED_PIXELS;
	info->fix.visual  = FB_VISUAL_TRUECOLOR;
	info->fix.line_length = 800 * 4;
	//2.2设置可变的参数 
	info->var.xres           = 800;
	info->var.yres           = 480;
	info->var.xres_virtual   = 800;
	info->var.yres_virtual   = 480;
	info->var.bits_per_pixel = 32;
	
	info->var.red.offset = 16;
	info->var.red.length = 8;
	
	info->var.green.offset = 8;
	info->var.green.length = 8;
	
	info->var.blue.offset = 0;
	info->var.blue.length = 8;
	
	info->var.activate = FB_ACTIVATE_NOW;
	
	//2.3设置可操作函数
	info->fbops           = &s5fb_ops;
	//2.4其他设置
	info->pseudo_palette   = pseudo_palette;  //调色板
	//
	info->screen_size      =  800*480*32/8;
	/*3.硬件相关的设置*/
	//3.1设置相关的gpio用于lcd
	gpf0con = ioremap(0xE0200120,4);
	gpf1con = ioremap(0xE0200140,4);
	gpf2con = ioremap(0xE0200160,4);
	gpf3con = ioremap(0xE0200180,4);
	gpd0con = ioremap(0xE02000A0,4);
	gpd0dat = ioremap(0xE02000A4,4);
	
	*gpf0con = 0x22222222;
	*gpf1con = 0x22222222;
	*gpf2con = 0x22222222;
	*gpf3con = 0x2222;
	*gpd0con  = (*gpd0con & ~0xF) | (0x1 << 0); //设置电源控制引脚为输出
	
	//3.2设置lcd控制器
	display_control = ioremap(0xE0107008,4);
	
	vidcon0 = ioremap(0xF8000000,16);
	vidcon1 = vidcon0 + 1;
	vidcon2 = vidcon1 + 1;
	vidcon3 = vidcon2 + 1;
	
	vidtcon0 = ioremap(0xF8000010,16);
	vidtcon1 = vidtcon0 + 1;
	vidtcon2 = vidtcon1 + 1;
	vidtcon3 = vidtcon2 + 1;
	wincon0 = ioremap(0xF8000020,4);
	
	vidosd0a = ioremap(0xF8000040,4);
	vidosd0b = ioremap(0xF8000044,4);
	vidosd0c = ioremap(0xF8000048,4);
	
	vidw00add0b0 = ioremap(0xF80000A0,4);
	vidw00add1b0 = ioremap(0xF80000D0,4);
	shodowcon    = ioremap(0xF8000034,4);
	*display_control = (2 << 0);
	bytes_per_line = WIDTH;
	if (BPP > 16)
		bytes_per_line *= 32;
	else
		bytes_per_line *= BPP;
	bytes_per_line /= 8;
	
	frame_buffer_size = bytes_per_line * HEIGHT;
	
	*vidcon0 =(5<<6)|(1<<4)|(0<<2);  //平行显示 VCLK =33.3MHZ  时钟源 HCLK bit0,1用来使能开关
	*vidcon1 =(0<<7)|(1<<6)|(1<<5)|(0 << 4);//Video data is fetched at VCLK rising edge 
	/* Specifies output data format control */
	*vidcon2  = 	(0b000 << 19) |			/* RGB interface output order(Even line, line #2,4,6,8):RGB */
				(0b000 << 16);			/* RGB interface output order(Odd line, line #1,3,5,7):RGB */
	
	*vidtcon0  =	(VBPD << 16) |			/* VBPD */
				(VFPD << 8) |			/* VFPD */
				(VSPW << 0);			/* VSPW */
	*vidtcon1 =	(HBPD << 16) |			/* HBPD */
				(HFPD << 8) |			/* HFPD */
				(HSPW << 0);			/* HSPW */
	
	*vidtcon2 =((HEIGHT - 1)<<11)|(WIDTH-1);;//800*480
	*vidtcon3 =	(0b1 << 31);			/* Enables VSYNC Signal Output */
	
	*wincon0  &= ~(0xf << 2);
	*wincon0  |=	(0 << 30) |				/* BUFSEL_H = 0 */
				(0 << 20) |				/* BUFSEL_L = 0, BUFSEL=0b00(Selects the Buffer set 0) */
				(1 << 15) |				/* the Word swap Enable */
				(0xB << 2);				/* Unpacked 24 bpp ( non-palletized R:8-G:8-B:8 ) */
	
	*vidosd0a= 0x0;
	*vidosd0b =((WIDTH - 1)<<11)|(HEIGHT-1);
	/* Specifies the Window Size:Height * Width (Number of Word) */
	*vidosd0c = frame_buffer_size / 4;
	
	//3.3分配framebuffer显存,并把地址告诉lcd控制器
		
	//info->fix.smem_start =  frame_buffer_addr;   //显存的物理地址 
	info->screen_base    = dma_alloc_writecombine(NULL, info->fix.smem_len, &info->fix.smem_start, GFP_KERNEL); //显存的虚拟地址
	
	*vidw00add0b0 =  info->fix.smem_start;
	*vidw00add1b0 =  *vidw00add0b0 + frame_buffer_size;
	
	*shodowcon = 0x1;	/* Enables Channel 0 */
	*gpd0dat |= 1;
	*vidcon0 |= 0x3;
	*wincon0 |= 1;
	/*4.注册*/
	register_framebuffer(info);
	
	
	return 0;
}

static void lcd_exit(void)
{
	unregister_framebuffer(info);
	dma_free_writecombine(NULL, info->fix.smem_len, info->screen_base, info->fix.smem_start);
	*shodowcon = 0x0;	/* Enables Channel 0 */
	*gpd0dat &= ~(0x1);
	*vidcon0 &= ~(0x3);
	*wincon0 &= ~(0x1);
	
	iounmap(gpf0con);
	iounmap(gpf1con);
	iounmap(gpf2con);
	iounmap(gpf3con);
	iounmap(gpd0con);
	iounmap(gpd0dat);
	
	iounmap(display_control);
	iounmap(vidcon0);
	iounmap(vidtcon0);
	iounmap(vidosd0a);
	iounmap(vidosd0b);
	iounmap(vidosd0c);
	iounmap(vidw00add0b0);
	iounmap(vidw00add1b0);
	iounmap(shodowcon);
	framebuffer_release(info);
}


module_init(lcd_init);
module_exit(lcd_exit);
MODULE_LICENSE("GPL");