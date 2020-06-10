#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

struct cdev mdev;
dev_t devn;

int dev1_regs[5];
int dev2_regs[5];


int mem_open(struct inode *node, struct file *filp)
{
	int num = MINOR(node->i_rdev);
	if(num == 0)
		filp->private_data = dev1_regs;
	if(num == 1)
		filp->private_data = dev2_regs;
		
	return 0;
	
}
int mem_close(struct inode *node, struct file *filp)
{
	return 0;
}
loff_t mem_lseek (struct file *filp, loff_t offset, int whence)
{
	loff_t new_pos = 0;
	switch(whence){
		case SEEK_SET:
			new_pos = 0 + offset;
			break;
		case SEEK_CUR:
			new_pos = filp->f_pos + offset;
			break;
		case SEEK_END:
			new_pos = 5*sizeof(int) + offset;
			break;
	}
	filp->f_pos = new_pos;
	return new_pos;
}
ssize_t mem_read(struct file *filp, char __user *buff, size_t size, loff_t *ppos)
{
	int *reg_base = filp->private_data;
	
	copy_to_user(buff,reg_base+(*ppos),size);
	
	filp->f_pos += size;
	
	return size;
	
}
ssize_t mem_write(struct file *filp, const char __user *buff, size_t size, loff_t *ppos)
{
	int *reg_base = filp->private_data;
	
	copy_from_user(reg_base+(*ppos),buff,size);
	
	filp->f_pos += size;
	
	return size;
}
static const struct file_operations memfops = {
	.open = mem_open,
	.read = mem_read,
	.llseek = mem_lseek,
	.release = mem_close,
	.write = mem_write,
};

int memdev_init(void)
{
	cdev_init(&mdev, &memfops);
	
	alloc_chrdev_region(&devn, 0, 2,"memdev");//动态分配设备号
	cdev_add(&mdev, devn, 2);
	return 0;
}

void memdev_exit(void)
{
	cdev_del(&mdev);
	unregister_chrdev_region(devn, 2);
}


MODULE_LICENSE("GPL");

module_init(memdev_init);
module_exit(memdev_exit);