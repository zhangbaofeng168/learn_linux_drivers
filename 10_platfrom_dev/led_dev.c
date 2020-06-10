#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/uaccess.h>

#include <asm/irq.h>
#include <mach/hardware.h>
#include <linux/uaccess.h>

static struct resource led_resource[] = {
	[0] = {
        .start = 0xE0200060,
        .end   = 0xE0200060 + 8 -1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = 3,
        .end   = 3,
        .flags = IORESOURCE_IRQ,
    }
};

static void led_release()
{
	
}
static struct platform_device gpio_device = {
	.name = "led_test",
	 .id       = -1,
    	.num_resources    = ARRAY_SIZE(led_resource),
	.dev = {
		.release = &led_release,
		},
	.resource = &led_resource,
};
static int gpio_device_init(void)
{
	platform_device_register(&gpio_device);
	return 0;
}

static void gpio_device_exit(void)
{
	platform_device_unregister(&gpio_device);
}
module_init(gpio_device_init);
module_exit(gpio_device_exit);

MODULE_LICENSE("GPL");
