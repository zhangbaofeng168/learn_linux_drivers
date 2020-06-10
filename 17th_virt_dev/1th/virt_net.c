#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/gfp.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>

static struct net_device *my_dev;

static int virt_net_init(void)
{
	//����һ��net_device �ṹ��
	my_dev = alloc_netdev(0,"vnet%d",ether_setup);
	//����
	//ע��
	register_netdev(my_dev);
	return 0;
}

static void virt_net_exit(void)
{
	unregister_netdev(my_dev);
	free_netdev(my_dev);
}

module_init(virt_net_init);
module_exit(virt_net_exit);

MODULE_LICENSE("GPL");