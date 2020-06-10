/*******************
*�ο��ں�
*xd.c
*
********************/

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/mutex.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>

static struct gendisk *block_disk;
static struct request_queue *block_queue;
static DEFINE_SPINLOCK(xd_lock);

static int major;
#define RAMBLOCK_SIZE 1024*1024
static unsigned char *rambloak_buf;
static int ramblock_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	/* ����=heads*cylinders*sectors*512 */
	geo->heads     = 2;
	geo->cylinders = 32;
	geo->sectors   = RAMBLOCK_SIZE/2/32/512;
	return 0;
}
static const struct block_device_operations block_fops = {
	.owner	= THIS_MODULE,
	//.locked_ioctl	= xd_ioctl,
	.getgeo = ramblock_getgeo,
};
static void do_block_request(struct request_queue * q)
{
	//static int cnt = 0;
	struct request *req;
	//printk("do_ramblock_request %d\n",++cnt);
	req = blk_fetch_request(q);
	while(req)
	{
		/*1.Դ/Ŀ��*/
		unsigned long offset = blk_rq_pos(req) << 9;
		/*2.Ŀ��*/
		//req->buffer
		/*3.����*/
		unsigned long len  = blk_rq_cur_bytes(req);
		
		if (rq_data_dir(req) == READ)
			memcpy(req->buffer,rambloak_buf + offset, len);
		else
			memcpy(rambloak_buf + offset, req->buffer, len);
			
		if (!__blk_end_request_cur(req, 0))
			req = blk_fetch_request(q);
	} 
}
static int block_init(void)
{
	/*1.����һ��gendisk�ṹ��*/
	block_disk = alloc_disk(16);
	/*2.������*/
	/*2.1����/���ö���*/
	block_queue = blk_init_queue(do_block_request, &xd_lock);
	block_disk->queue = block_queue;
	/*2.2������������*/
	major = register_blkdev(0,"ramblock");
	block_disk->major = major;
	block_disk->first_minor = 0;
	
	sprintf(block_disk->disk_name, "ramblock");	
	block_disk->fops = &block_fops;
	set_capacity(block_disk,RAMBLOCK_SIZE/512);
	
	/*3.�����ڴ�*/
	rambloak_buf = kzalloc(RAMBLOCK_SIZE,GFP_KERNEL);
	
	/*4.ע��*/
	add_disk(block_disk);
	
	return 0;
}

static void block_exit(void)
{
	unregister_blkdev(major,"ramblock");
	del_gendisk(block_disk);
	put_disk(block_disk);
	blk_cleanup_queue(block_queue);
	kfree(rambloak_buf);
}

module_init(block_init);
module_exit(block_exit);

MODULE_LICENSE("GPL");