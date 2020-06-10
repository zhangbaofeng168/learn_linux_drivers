#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/videodev2.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <asm/atomic.h>
#include <asm/unaligned.h>

#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <media/videobuf-core.h>

#include <linux/clk.h>
#include <asm/io.h>

#define OV9650_INIT_REGS_SIZE (sizeof(ov96540_setting_30fps_VGA_640_480)/sizeof(ov96540_setting_30fps_VGA_640_480[0]))

typedef struct cmos_ov9650_i2c_value {
	unsigned char regaddr;
	unsigned char value;
}ov9650_t;
/* init: 640x480,30fps的,YUV422输出格式 */
ov9650_t ov9650_setting_30fps_VGA_640_480[] =
{
	{0x12, 0x80},
	{0x47, 0x02},
	{0x17, 0x27},
	{0x04, 0x40},
	{0x1B, 0x81},
	{0x29, 0x17},
	{0x5F, 0x03},
	{0x3A, 0x09},
	{0x33, 0x44},
	{0x68, 0x1A},

	{0x14, 0x38},
	{0x5F, 0x04},
	{0x64, 0x00},
	{0x67, 0x90},
	{0x27, 0x80},
	{0x45, 0x41},
	{0x4B, 0x40},
	{0x36, 0x2f},
	{0x11, 0x01},
	{0x36, 0x3f},
	{0x0c, 0x12},

	{0x12, 0x00},
	{0x17, 0x25},
	{0x18, 0xa0},
	{0x1a, 0xf0},
	{0x31, 0xa0},
	{0x32, 0xf0},

	{0x85, 0x08},
	{0x86, 0x02},
	{0x87, 0x01},
	{0xd5, 0x10},
	{0x0d, 0x34},
	{0x19, 0x03},
	{0x2b, 0xf8},
	{0x2c, 0x01},

	{0x53, 0x00},
	{0x89, 0x30},
	{0x8d, 0x30},
	{0x8f, 0x85},
	{0x93, 0x30},
	{0x95, 0x85},
	{0x99, 0x30},
	{0x9b, 0x85},

	{0xac, 0x6E},
	{0xbe, 0xff},
	{0xbf, 0x00},
	{0x38, 0x14},
	{0xe9, 0x00},
	{0x3D, 0x08},
	{0x3E, 0x80},
	{0x3F, 0x40},
	{0x40, 0x7F},
	{0x41, 0x6A},
	{0x42, 0x29},
	{0x49, 0x64},
	{0x4A, 0xA1},
	{0x4E, 0x13},
	{0x4D, 0x50},
	{0x44, 0x58},
	{0x4C, 0x1A},
	{0x4E, 0x14},
	{0x38, 0x11},
	{0x84, 0x70}
};
static unsigned long *GPE0CON;/*0xE02000E0*/
static unsigned long *GPE1CON; /*0xE0200100*/
static unsigned long *GPE0PUD;
static unsigned long *GPE1PUD;

// CAMIF
static unsigned long *CISRCFMT0;
static unsigned long *CIWDOFST0;
static unsigned long *CIGCTRL0;
static unsigned long *CIOYSA10;
static unsigned long *CIOYSA20;
static unsigned long *CIOYSA30;
static unsigned long *CIOYSA40;
static unsigned long *CITRGFMT0;
static unsigned long *CIOCTRL0;
static unsigned long *CISCPRERATIO0;
static unsigned long *CISCPREDST0;
static unsigned long *CISCCTRL0;
static unsigned long *CITAREA0;
static unsigned long *CIIMGCPT0;


static struct i2c_client *cmos_ov9650_client;

/* A2 参考 uvc_v4l2_do_ioctl */
static int cmos_ov9650_vidioc_querycap(struct file *file, void  *priv,
					struct v4l2_capability *cap)
{
	return 0;
}

/* A3 列举支持哪种格式
 * 参考: uvc_fmts 数组
 */
static int cmos_ov9650_vidioc_enum_fmt_vid_cap(struct file *file, void  *priv,
					struct v4l2_fmtdesc *f)
{
	return 0;
}

/* A4 返回当前所使用的格式 */
static int cmos_ov9650_vidioc_g_fmt_vid_cap(struct file *file, void *priv,
					struct v4l2_format *f)
{
	return 0;
}

/* A5 测试驱动程序是否支持某种格式, 强制设置该格式 
 * 参考: uvc_v4l2_try_format
 *       myvivi_vidioc_try_fmt_vid_cap
 */
static int cmos_ov9650_vidioc_try_fmt_vid_cap(struct file *file, void *priv,
			struct v4l2_format *f)
{
	return 0;
}

/* A6 参考 myvivi_vidioc_s_fmt_vid_cap */
static int cmos_ov9650_vidioc_s_fmt_vid_cap(struct file *file, void *priv,
					struct v4l2_format *f)
{
	return 0;
}

static int cmos_ov9650_vidioc_reqbufs(struct file *file, void *priv,
			  struct v4l2_requestbuffers *p)
{
	return 0;
}

/* A11 启动传输 
 * 参考: uvc_video_enable(video, 1):
 *           uvc_commit_video
 *           uvc_init_video
 */
static int cmos_ov9650_vidioc_streamon(struct file *file, void *priv, enum v4l2_buf_type i)
{
	return 0;
}

/* A17 停止 
 * 参考 : uvc_video_enable(video, 0)
 */
static int cmos_ov9650_vidioc_streamoff(struct file *file, void *priv, enum v4l2_buf_type t)
{
	return 0;
}

static const struct v4l2_ioctl_ops cmos_ov9650_ioctl_ops = {
        // 表示它是一个摄像头设备
        .vidioc_querycap      = cmos_ov9650_vidioc_querycap,

        /* 用于列举、获得、测试、设置摄像头的数据的格式 */
        .vidioc_enum_fmt_vid_cap  = cmos_ov9650_vidioc_enum_fmt_vid_cap,
        .vidioc_g_fmt_vid_cap     = cmos_ov9650_vidioc_g_fmt_vid_cap,
        .vidioc_try_fmt_vid_cap   = cmos_ov9650_vidioc_try_fmt_vid_cap,
        .vidioc_s_fmt_vid_cap     = cmos_ov9650_vidioc_s_fmt_vid_cap,
        
        /* 缓冲区操作: 申请/查询/放入队列/取出队列 */
        .vidioc_reqbufs       = cmos_ov9650_vidioc_reqbufs,
#if 0
        .vidioc_querybuf      = cmos_ov9650_vidioc_querybuf,
        .vidioc_qbuf          = cmos_ov9650_vidioc_qbuf,
        .vidioc_dqbuf         = cmos_ov9650_vidioc_dqbuf,
#endif      
        // 启动/停止
        .vidioc_streamon      = cmos_ov9650_vidioc_streamon,
        .vidioc_streamoff     = cmos_ov9650_vidioc_streamoff,   
};

static int cmos_ov9650_open(struct file *file)
{
	return 0;
}

static int cmos_ov9650_close(struct file *file)
{
	return 0;
}

/* 应用程序通过读的方式读取摄像头的数据 */
static ssize_t cmos_ov9650_read(struct file *filep, char __user *buf, size_t count, loff_t *pos)
{
	return 0;
}

static const struct v4l2_file_operations cmos_ov9650_fops = {
	 .owner	     = THIS_MODULE,
	 .open       = cmos_ov9650_open,
	 .release    = cmos_ov9650_close,
	 .unlocked_ioctl      = video_ioctl2, /* V4L2 ioctl handler */
	 .read       = cmos_ov9650_read,
};

static void cmos_ov9650_release(struct video_device *vdev)
{
	
}

/*1.分配结构体*/
static struct video_device cmos_ov9650_vdev={

        /* 2. 设置 */
        /* 2.1 */
        .release = cmos_ov9650_release,
        
        /* 2.2 */
        .fops    = &cmos_ov9650_fops,
        
        /* 2.3 */
        .ioctl_ops = &cmos_ov9650_ioctl_ops,
	
	/*2.4*/
	.name      = "cmos_ov9650",
       
};

static void cmos_ov9650_clk_cfg(void)
{
	struct clk *camif_clk;
	struct clk *camif_upll_clk;

	/* 使能CAMIF的时钟源 */
	camif_clk = clk_get(NULL, "camif");
	if(!camif_clk || IS_ERR(camif_clk))
	{
		printk(KERN_INFO "failed to get CAMIF clock source\n");
	}
	clk_enable(camif_clk);

	/* 使能并设置CAMCLK = 24MHz */
	camif_upll_clk = clk_get(NULL, "camif-upll");
	clk_set_rate(camif_upll_clk, 2400000);
	mdelay(100);
}


static void cmos_ov9650_reset(void)
{
	*CIGCTRL0 |= (1<<30);
	mdelay(30);
	*CIGCTRL0 &= ~(1<<30);
	mdelay(30);
	*CIGCTRL0 |= (1<<30);
	mdelay(30);
}

static void cmos_ov9650_init(void)
{
	unsigned int mid;
	int i;

	/* 读 */
	mid = i2c_smbus_read_byte_data(cmos_ov9650_client, 0x0a)<<8;
	mid |= i2c_smbus_read_byte_data(cmos_ov9650_client, 0x0b);
	printk("manufacture ID = 0x%4x\n", mid);

	/* 写 */
	/*for(i = 0; i < OV9650_INIT_REGS_SIZE; i++)
	{
		i2c_smbus_write_byte_data(cmos_ov9650_client, ov9650_setting_30fps_VGA_640_480[i].regaddr, ov9650_setting_30fps_VGA_640_480[i].value);
		mdelay(2);
	}*/
}

static int __devinit cmos_ov9650_probe(struct i2c_client *client,const struct i2c_device_id *id)			
{
	/* 2.3 硬件相关 */
	/* 2.3.1 映射相应的寄存器 */
	GPE0CON = ioremap(0xE02000E0,4);
	GPE0PUD = GPE0CON + 2;
	GPE1CON = ioremap(0xE0200100,4);
	GPE1PUD = GPE1CON + 2;
	
	CISRCFMT0 = ioremap(0xFB200000,4);   
	CIWDOFST0 = CISRCFMT0 + 1;    
	CIGCTRL0  = CISRCFMT0 + 2;     
	CIOYSA10  = ioremap(0xFB200018 ,4);     
	CIOYSA20  = CIOYSA10 + 1;     
	CIOYSA30  = CIOYSA10 + 2;     
	CIOYSA40  = CIOYSA10 + 3;     
	CITRGFMT0 = ioremap(0xFB200048 ,4);    
	CIOCTRL0  = CITRGFMT0 + 1;     
	CISCPRERATIO0 = CITRGFMT0 + 2;
	CISCPREDST0 = CITRGFMT0 + 3;  
	CISCCTRL0 = CITRGFMT0 + 4;    
	CITAREA0 = CITRGFMT0 + 5;     
	CIIMGCPT0 = ioremap(0xFB2000C0,4);    

	/*2.3.2设置gpio*/
	*GPE0CON = 0x22222222;/*用于摄像头模式*/
	*GPE0CON = 0x22222;
	*GPE0PUD = 0xaaaa;/*设置上拉*/
	*GPE1PUD = 0x02aa;
	/* 2.3.3 复位一下CAMIF控制器
	[ITU601 case: ITU601_656n “1” → SwRst “1” → SwRst “0”  for 
	first SFR setting] */	
	*CISRCFMT0 |= (1<<31);/* 传输方式为BT601 */	
	*CIGCTRL0 |= (1<<31);/* 复位CAMIF控制器 */
	mdelay(10);
	*CIGCTRL0 &= ~(1<<31);
	mdelay(10);
        
	/* 2.3.4 设置、使能时钟(使能HCLK、使能并设置CAMCLK = 24MHz) */
	cmos_ov9650_clk_cfg();
        
	/* 2.3.5 复位一下摄像头模块 */
	cmos_ov9650_reset();

	/* 2.3.6 通过IIC总线,初始化摄像头模块 */
	cmos_ov9650_client = client;
	cmos_ov9650_init();

	/* 2.3.7 注册中断 */
/*	if (request_irq(IRQ_S3C2440_CAM_C, cmos_ov7740_camif_irq_c, IRQF_DISABLED , "CAM_C", NULL))
		printk("%s:request_irq failed\n", __func__);

	if (request_irq(IRQ_S3C2440_CAM_P, cmos_ov7740_camif_irq_p, IRQF_DISABLED , "CAM_P", NULL))
		printk("%s:request_irq failed\n", __func__);*/
	
	 /* 3. 注册 */
        video_register_device(&cmos_ov9650_vdev, VFL_TYPE_GRABBER, -1);
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

static int __devexit cmos_ov9650_remove(struct i2c_client *client)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	video_unregister_device(&cmos_ov9650_vdev);
	return 0;
}

static const struct i2c_device_id cmos_ov9650_id_table[] = {
	{ "cmos_ov9650", 0 },
	{}
};

/* 1. 分配/设置i2c_driver */
static struct i2c_driver cmos_ov9650_driver = {
	.driver	= {
		.name	= "cmos_ov9650",
		.owner	= THIS_MODULE,
	},
	.probe		= cmos_ov9650_probe,
	.remove		= __devexit_p(cmos_ov9650_remove),
	.id_table	= cmos_ov9650_id_table,
};

static int cmos_ov9650_drv_init(void)
{
	/* 2. 注册i2c_driver */
	i2c_add_driver(&cmos_ov9650_driver);
	
	return 0;
}

static void cmos_ov9650_drv_exit(void)
{
	i2c_del_driver(&cmos_ov9650_driver);
}

module_init(cmos_ov9650_drv_init);
module_exit(cmos_ov9650_drv_exit);
MODULE_LICENSE("GPL");
