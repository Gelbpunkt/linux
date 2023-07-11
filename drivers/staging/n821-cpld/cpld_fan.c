/*
 * cpld-fan.c - fan part of the CPLD for Niagara 821
 */

#include <linux/hwmon.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/property.h>
#include <linux/mfd/cisco-n821-cpld.h>
#include <linux/mfd/syscon.h>


struct n821_cpld_fan_data {
  struct regmap *regmap;
  u32 offset;
};

static umode_t n821_cpld_fan_is_visible(const void *data,
    enum hwmon_sensor_types type,
    u32 attr, int channel)
{
  return 0444;
}

static int n821_cpld_fan_read(struct device *dev,
    enum hwmon_sensor_types type, u32 attr,
    int channel, long *input)
{
  struct n821_cpld_fan_data *fan = dev_get_drvdata(dev);
  unsigned int value;
  int ret;
  switch (attr) {
    case hwmon_fan_input:
      ret = regmap_read(fan->regmap, fan->offset + channel, &value);
      if (ret)
        return ret;
      value *= 60;
      break;
    default:
      return -EOPNOTSUPP;
  }
  *input = value;
  return 0;
}

static const struct hwmon_channel_info *n821_cpld_fan_info[] = {
  HWMON_CHANNEL_INFO(fan, HWMON_F_INPUT, HWMON_F_INPUT),
  NULL
};

static const struct hwmon_ops n821_cpld_fan_ops = {
  .is_visible = n821_cpld_fan_is_visible,
  .read = n821_cpld_fan_read,
};

static const struct hwmon_chip_info n821_cpld_fan_chip_info = {
  .ops = &n821_cpld_fan_ops,
  .info = n821_cpld_fan_info,
};

static int n821_cpld_fan_probe(struct platform_device *pdev)
{
  struct device *dev = &pdev->dev;
  struct n821_cpld_fan_data *fan;
  struct device *hwmon_dev;

  fan = devm_kzalloc(dev, sizeof(*fan), GFP_KERNEL);
  if (!fan)
    return -ENOMEM;

  fan->regmap = syscon_node_to_regmap(dev->of_node->parent);
  if (IS_ERR(fan->regmap)) {
    dev_err(dev, "failed to lookup regmap\n");
    return PTR_ERR(fan->regmap);
  }

  fan->offset = N821_CPLD_REG_FAN0_SPEED;

  hwmon_dev = devm_hwmon_device_register_with_info(&pdev->dev,
      "n821_fan", fan, &n821_cpld_fan_chip_info, NULL);
  if (IS_ERR(hwmon_dev))
    dev_err(&pdev->dev, "failed to register as hwmon device");
  dev_info(dev, "probed");
  return PTR_ERR_OR_ZERO(hwmon_dev);
}

static int n821_cpld_fan_remove(struct platform_device *pdev)
{
  return 0;
}

static struct of_device_id n821_cpld_fan_match_table[] = {
  { .compatible = "n821,cpld-fan" },
  { }
};

static struct platform_driver n821_cpld_fan_driver = {
        .driver = {
                .name = "n821-cpld-fan",
                .of_match_table = of_match_ptr(n821_cpld_fan_match_table),
        },
        .probe = n821_cpld_fan_probe,
        .remove = n821_cpld_fan_remove,
};
module_platform_driver(n821_cpld_fan_driver);

MODULE_DEVICE_TABLE(of, n821_cpld_fan_match_table);
MODULE_DESCRIPTION("Niagara 821 CPLD fan driver");
MODULE_AUTHOR("Christian Svensson <blue@cmd.nu>");
MODULE_LICENSE("GPL");
