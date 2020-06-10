/*参考s3c_nand.c atmel_nand.c
*
*/

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/io.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <plat/regs-nand.h>
#include <plat/nand.h>


static volatile unsigned int *nfconf;
static volatile unsigned int *nfcont;
static volatile unsigned int *nfcmmd;
static volatile unsigned int *nfaddr;
static volatile unsigned int *nfdata;
static volatile unsigned int *nfstat;
     
static struct nand_chip *my_nand;
static struct mtd_info  *my_mtd;
static struct mtd_partition my_parts[]={
	[0] = {
		.name		= "bootloader",
		.offset		= (0),          /* for bootloader */
		.size		= (1024*SZ_1K),
		.mask_flags	= MTD_CAP_NANDFLASH,
	},
	[1] = {
		.name		= "Logo",
		.offset		= (2*SZ_1M),
		.size		= (3*SZ_1M),
	},
	[2] = {
		.name		= "kernel",
		.offset		= MTDPART_OFS_APPEND,
		.size		= (5*SZ_1M),
	},
	[3] = {
		.name		= "rootfs",
		.offset		= MTDPART_OFS_APPEND,
		.size		= MTDPART_SIZ_FULL,
	}
};
static void nand_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1)
	{
		/**取消选中*/
		*nfcont |= (1 << 1);
	}
	else
	{
		/**选中片选*/
		*nfcont &= ~(1 << 1);	
	}
}
static void nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	if (ctrl & NAND_CLE)
	{
		/*发命令NFCMMD = cmd*/
		*nfcmmd = cmd;
	}
	else
	{
		/*发地址NFCDATA = cmd*/
		*nfaddr = cmd;
	}
}

static int nand_device_ready(struct mtd_info *mtd)
{
	/*返回NFSTAT的0位*/
	return *nfstat & 0x1;
}
static int nand_init(void)
{
	struct clk *clk;
	/*1.分配一个nand_chip结构体*/
	my_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
	clk = clk_get(NULL,"nand");
	clk_enable(clk);
	nfconf = ioremap(0xB0E00000,20);
	nfcont = nfconf + 1;
	nfcmmd = nfcont + 1;
	nfaddr = nfcmmd + 1;
	nfdata = nfaddr + 1;
	nfstat = ioremap(0xB0E00028,4);
	/*2.设置nand_chip
	能发地址，发数据，读数据，读状态等函数*/
	my_nand->select_chip = nand_select_chip;
	my_nand->cmd_ctrl  = nand_cmd_ctrl;
	my_nand->IO_ADDR_R = nfdata;
	my_nand->IO_ADDR_W = nfdata;
	my_nand->dev_ready = nand_device_ready;
	my_nand->ecc.mode = NAND_ECC_SOFT;
	/*3.硬件相关的设置:根据nandflash手册设置时间参数
	TACLS >=0
	TWRPH0 = 12ns TWRPH1 = 5ns*/
	*nfconf |= (0x1 << 23)|(2<<8)|(1<<4)|(1<<1);
	*nfcont |= 0x1;
	
	/*4.使用：nand_scan 扫描识别 nandflash*/
	my_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	my_mtd->priv  = my_nand;
	my_mtd->owner = THIS_MODULE;
	
	nand_scan(my_mtd, 1);
	/*5.add_mtd_partitions*/
	add_mtd_partitions(my_mtd,my_parts,4);
	
	return 0;
}

static void nand_exit(void)
{
	del_mtd_partitions(my_mtd);
	kfree(my_nand);
	kfree(my_mtd);
	iounmap(nfconf);
	iounmap(nfstat);
}

module_init(nand_init);
module_exit(nand_exit);

MODULE_LICENSE("GPL");