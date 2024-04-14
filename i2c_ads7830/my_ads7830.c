

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <linux/printk.h>

struct my_wrapper {
  struct i2c_client *client;
};

static u8 getData(struct i2c_client *client) {
  u8 ch = 0;
  uint8_t command = (0xff);
  command = (command) & (ch << 4);
  command = command | 0x80 | 0x04;

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

static void poll(struct input_dev *input) {
  u8 recv[6];
  int zpressed, cpressed, bx, by;

  /* Retrieve the physical i2c device */
  struct my_wrapper *mydev = input_get_drvdata(input);
  struct i2c_client *client = mydev->client;

  /* Get the state of the device registers */
  int ret = getData(client);
  if (ret < 0)
    return;

  zpressed = (recv[5] & BIT(0)) ? 0 : 1;
  cpressed = (recv[5] & BIT(1)) ? 0 : 1;
  bx = recv[0];
  by = recv[1];

  input_report_abs(input, ABS_VOLUME, ret);

  input_sync(input);
}

static int ads7830_probe(struct i2c_client *client) {
  pr_info("ads7830_probe hello\n");

  struct input_dev *dev;

  dev = devm_input_allocate_device(&client->dev);
  if (!dev)
    return -ENOMEM;

  struct my_wrapper *mydev;

  mydev = devm_kzalloc(&client->dev, sizeof(*mydev), GFP_KERNEL);
  if (!mydev)
    /* No message necessary here, already issued by allocation functions */
    return -ENOMEM;

  mydev->client = client;

  input_set_drvdata(dev, mydev);
  dev->name = "Vol control";
  dev->id.bustype = BUS_I2C;

  set_bit(EV_ABS, dev->evbit);
  set_bit(ABS_VOLUME, dev->absbit);
  input_set_abs_params(dev, ABS_VOLUME, 0, 255, 4, 8);

  int ret = input_setup_polling(dev, poll);
  if (ret) {
    dev_err(&client->dev, "Failed to set polling function (%d)\n", ret);
    return ret;
  }

  input_set_poll_interval(dev, 50);

  /* Register the dev device when everything is ready */
  ret = input_register_device(dev);
  if (ret) {
    dev_err(&client->dev, "Cannot register input device (%d)\n", ret);
    return ret;
  }
  pr_info("ads7830_probe exit\n");

  return 0;
}

static void ads7830_remove(struct i2c_client *client) {
  pr_info("ads7830_remove hello\n");
  pr_info("ads7830_remove exit\n");
}

/* Specification of supported Device Tree devices */
static const struct of_device_id adc_dt_match[] = {
    {.compatible = "cust,my_ads7830"},
    {},
};
MODULE_DEVICE_TABLE(of, adc_dt_match);

/* Driver declaration */
static struct i2c_driver ads7830_cust_driver = {
    .driver =
        {
            .name = "my_ads7830",
            .of_match_table = adc_dt_match,
        },
    .probe_new = ads7830_probe,
    .remove = ads7830_remove,
};

module_i2c_driver(ads7830_cust_driver);

MODULE_LICENSE("GPL");