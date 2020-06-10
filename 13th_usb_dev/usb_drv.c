#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

static char *usb_buf;
static dma_addr_t usb_buf_phys;
static int len ;
static struct urb *uk_urb;
static struct input_dev *usb_input_dev;

static struct usb_device_id myusb_mouse_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	{ }	/* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, myusb_mouse_id_table);

static void usbmouse_irq(struct urb *urb)
{
	static unsigned char pre_val;

	/*int i;
	static int cnt = 0;
	printk("data cnt %d: ",++cnt);
	for(i = 0; i < len; i++)
	{
		printk("%d",usb_buf[i]);
	}
	printk("\n");*/

	/*
	*data[0] bit0 左键 
	*        bit1 右键
	*	 bit2 中键
	*/
	
	if((pre_val & (1<<0))!=(usb_buf[1] & (1<<0)))
	{
		input_event(usb_input_dev, EV_KEY, KEY_L,(usb_buf[1] & (1<<0))?1:0);
		input_sync(usb_input_dev);
	}
	
	if((pre_val & (1<<1))!=(usb_buf[1] & (1<<1)))
	{
		input_event(usb_input_dev, EV_KEY, KEY_S,(usb_buf[1] & (1<<1))?1:0);
		input_sync(usb_input_dev);
	}
	
	if((pre_val & (1<<2))!=(usb_buf[1] & (1<<2)))
	{
		input_event(usb_input_dev, EV_KEY, KEY_ENTER,(usb_buf[1] & (1<<2))?1:0);
		input_sync(usb_input_dev);
	}
	pre_val = usb_buf[1];
	/*重新提交urb*/
	usb_submit_urb(urb, GFP_KERNEL);
	
}
static int myusb_mouse_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	int pipe;
	
	interface = intf->cur_altsetting;
	endpoint = &interface->endpoint[0].desc;
	/*1.分配input_dev结构体*/
	usb_input_dev = input_allocate_device();
	/*设置*/
	/* 2.1 能产生哪类事件 */
	usb_input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	
	/* 2.2 能产生这类操作里的哪些事件: L,S,ENTER */
	set_bit(KEY_L, usb_input_dev->keybit);
	set_bit(KEY_S, usb_input_dev->keybit);
	set_bit(KEY_ENTER, usb_input_dev->keybit);
	//usb_input_dev->keybit[0] = BIT_MASK(KEY_L) | BIT_MASK(KEY_S) | BIT_MASK(KEY_ENTER);
	/*3.注册*/
	input_register_device(usb_input_dev);
	/*4.硬件相关*/
	/*数据传输3要素*/
	
	/*1.源: USB设备的某个端点*/
	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
	
	/*2.长度：*/
	len = endpoint->wMaxPacketSize;
	
	/*3.目的：*/
	usb_buf = usb_alloc_coherent(dev, len, GFP_ATOMIC, &usb_buf_phys);
	/**使用三要素**/
	uk_urb = usb_alloc_urb(0,GFP_KERNEL);
	usb_fill_int_urb(uk_urb, dev, pipe, usb_buf, len, usbmouse_irq, NULL, endpoint->bInterval);
	uk_urb->transfer_dma    = usb_buf_phys;
	uk_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	
	/**使用urb*/
	usb_submit_urb(uk_urb,GFP_KERNEL);
	
	return 0;
}

static void myusb_mouse_disconnect(struct usb_interface *intf)
{
	/*printk("usbmouse disconnect!\n");*/
	struct usb_device *dev = interface_to_usbdev(intf);
	usb_kill_urb(uk_urb);
	usb_free_urb(uk_urb);
	usb_free_coherent(dev, len, usb_buf, usb_buf_phys);
	input_unregister_device(usb_input_dev);
	input_free_device(usb_input_dev);
}


static struct usb_driver myusb_mouse_driver = {
	.name		= "myusbmouse",
	.probe		= myusb_mouse_probe,
	.disconnect	= myusb_mouse_disconnect,
	.id_table	= myusb_mouse_id_table,
};

static int usb_drv_init(void)
{
	usb_register(&myusb_mouse_driver);
	return 0;
}

static void usb_drv_exit(void)
{
	usb_deregister(&myusb_mouse_driver);
}

module_init(usb_drv_init);
module_exit(usb_drv_exit);

MODULE_LICENSE("GPL");