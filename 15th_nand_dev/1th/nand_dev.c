/*�ο�s3c_nand.c at91_nand.c
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

static struct nand_chip *my_nand;
static struct mtd_info  *my_mtd;

static void nand_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1)
	{
	}
	
	if(chipnr == 0)
	{
		
	}
}
static void nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	if (ctrl & NAND_CLE)
	{
		/*������NFCMMD = cmd*/
	}
	else
	{
		/*����ַNFCDATA = cmd*/
	}
}

static int nand_device_ready(struct mtd_info *mtd)
{
	/*����NFSTAT��0λ*/
	return ;
}
static int nand_init(void)
{
	/*1.����һ��nand_chip�ṹ��*/
	my_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
	/*2.����nand_chip
	�ܷ���ַ�������ݣ������ݣ���״̬�Ⱥ���*/
	my_nand->select_chip = nand_select_chip;
	my_nand->cmd_ctrl  = nand_cmd_ctrl;
	my_nand->IO_ADDR_R =
	my_nand->IO_ADDR_W = 
	my_nand->dev_ready = nand_device_ready;
	/*3.Ӳ����ص�����*/
	/*4.ʹ�ã�nand_scan ɨ��ʶ�� nandflash*/
	my_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	my_mtd->priv  = my_nand;
	my_mtd->owner = THIS_MODULE;
	
	nand_scan(my_mtd, 1);
	/*5.add_mtd_partitions*/
	
	return 0;
}

static void nand_exit(void)
{
	
}

module_init(nand_init);
module_exit(nand_exit);

MODULE_LICENSE("GPL);