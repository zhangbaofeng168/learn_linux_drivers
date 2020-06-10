#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

static int myusb_mouse_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	printk("usbmouse driver has install!\n");
	printk("bcdUSB = %x\n",dev->descriptor.bcdUSB);
	printk("VID = 0x%x\n",dev->descriptor.idVendor);
	printk("PID = 0x%x\n",dev->descriptor.idProduct);
	return 0;
}

static void myusb_mouse_disconnect(struct usb_interface *intf)
{
	printk("usbmouse disconnect!\n");
}

static struct usb_device_id myusb_mouse_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	{ }	/* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, myusb_mouse_id_table);
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