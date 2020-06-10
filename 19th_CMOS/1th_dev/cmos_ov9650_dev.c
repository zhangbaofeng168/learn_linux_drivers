#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/slab.h>

static struct i2c_board_info cmos_ov9650_info = {	
	I2C_BOARD_INFO("cmos_ov9650", 0x30),
};
static struct i2c_client *cmos_ov9650_client;

static int cmos_ov9650_dev_init(void)
{
	struct i2c_adapter *i2c_adap;

	i2c_adap = i2c_get_adapter(0);
	
	cmos_ov9650_client = i2c_new_device(i2c_adap, &cmos_ov9650_info);
	
	i2c_put_adapter(i2c_adap);
	return 0;
}

static void cmos_ov9650_dev_exit(void)
{
	i2c_unregister_device(cmos_ov9650_client);
}

module_init(cmos_ov9650_dev_init);
module_exit(cmos_ov9650_dev_exit);
MODULE_LICENSE("GPL");

