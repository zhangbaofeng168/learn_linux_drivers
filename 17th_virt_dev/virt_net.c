

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
#include <linux/ip.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>

static struct net_device *my_dev;

static void emulator_rx_packet(struct sk_buff *skb, struct net_device *dev)
{
	/* 参考LDD3 */
	unsigned char *type;
	struct iphdr *ih;
	__be32 *saddr, *daddr, tmp;
	unsigned char	tmp_dev_addr[ETH_ALEN];
	struct ethhdr *ethhdr;
	
	struct sk_buff *rx_skb;
		
	/* 从硬件读出/保存数据*/
	/* 对调"源/目的"的mac地址 */
	ethhdr = (struct ethhdr *)skb->data;
	memcpy(tmp_dev_addr, ethhdr->h_dest, ETH_ALEN);
	memcpy(ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);
	memcpy(ethhdr->h_source, tmp_dev_addr, ETH_ALEN);

	/* 对调"源/目的"的ip地址 */    
	ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
	saddr = &ih->saddr;
	daddr = &ih->daddr;

	tmp = *saddr;
	*saddr = *daddr;
	*daddr = tmp;
	
	//((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */
	//((u8 *)daddr)[2] ^= 1;
	type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
	//printk("tx package type = %02x\n", *type);
      /*修改类型, 原来0x8表示ping*/
	*type = 0; /* 0表示reply */
	
	ih->check = 0;		   /* and rebuild the checksum (ip needs it) */
	ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
	
	/* 构造一个sk_buff*/
	rx_skb = dev_alloc_skb(skb->len + 2);
	skb_reserve(rx_skb, 2); /* align IP on 16B boundary */	
	memcpy(skb_put(rx_skb, skb->len), skb->data, skb->len);

	/* Write metadata, and then pass to the receive level */
	rx_skb->dev = dev;
	rx_skb->protocol = eth_type_trans(rx_skb, dev);
	rx_skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += skb->len;

	// 提交sk_buff
	netif_rx(rx_skb);
	
	
}

static int virt_net_send_packet(struct sk_buff *skb, struct net_device *dev)
{
	static int cnt = 0;
	printk("vent cnt = %d\n",++cnt);
	/*对于真实的网卡，把skb里的数据通过网卡发出去*/
	
	netif_stop_queue(dev);/*停止该网卡的队列*/

	emulator_rx_packet(skb,dev);
	dev_kfree_skb (skb); /*释放skb*/
	netif_wake_queue(dev);/*唤醒进程*/
	
	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;

	
	return 0;
}

static const struct net_device_ops vnet_ops = {
	.ndo_start_xmit 	= virt_net_send_packet,

};

static int virt_net_init(void)
{
	//分配一个net_device 结构体
	my_dev = alloc_netdev(0,"vnet%d",ether_setup);
	//设置
	my_dev->netdev_ops	= &vnet_ops;
	my_dev->dev_addr[0] = 0x08;
	my_dev->dev_addr[1] = 0x89;
	my_dev->dev_addr[2] = 0x89;
	my_dev->dev_addr[3] = 0x89;
	my_dev->dev_addr[4] = 0x89;
	my_dev->dev_addr[5] = 0x11;
	
	//注册
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