

#include <linux/i2c.h>
#include <linux/printk.h>
#include <linux/delay.h>

static u8 getData(struct i2c_client *client, u8 ch=0)
{
    uint8_t command = (0xff);
    command = (command) & (ch << 4);
    command = command | 0x80 |0x04;

    int ret = i2c_master_send(client, &command, 1);
	if (ret < 0) {
		dev_err(&client->dev, "i2c send failed (%d)\n", ret);
		return ret;
	}
    udelay(1000);
    u8 data;
    ret = i2c_master_recv(client, &data, 1);
	if (ret < 0) {
		dev_err(&client->dev, "i2c recv failed (%d)\n", ret);
		return ret;
	}
    return data;
}

static int ads7830_probe(struct i2c_client *client) {
    pr_info("ads7830_probe hello\n");
    
    pr_info("Data recieved = %d\n", getData(client));
    pr_info("ads7830_probe exit\n");

  return 0;
}

static void ads7830_remove(struct i2c_client *client)
{
    pr_info("ads7830_remove hello\n");
    pr_info("ads7830_remove exit\n");
}

/* Specification of supported Device Tree devices */
static const struct of_device_id adc_dt_match[] = {
	{ .compatible = "cust,my_ads7830" },
	{ },
};
MODULE_DEVICE_TABLE(of, adc_dt_match);

/* Driver declaration */
static struct i2c_driver ads7830_cust_driver = {
	.driver = {
		.name = "my_ads7830",
		.of_match_table = adc_dt_match,
	},
	.probe_new = ads7830_probe,
	.remove = ads7830_remove,
};

module_i2c_driver(ads7830_cust_driver);

MODULE_LICENSE("GPL");