/*
 * cpld-psu.c - PSU part of the CPLD for Niagara 821
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
#include <linux/mfd/syscon.h>

#include "cpld.h"


struct n821_cpld_psu_data {
  struct regmap *regmap;
  u32 offset;
};

static umode_t n821_cpld_psu_is_visible(const void *data,
    enum hwmon_sensor_types type,
    u32 attr, int channel)
{
  return 0444;
}

static int n821_cpld_psu_read(struct device *dev,
    enum hwmon_sensor_types type, u32 attr,
    int channel, long *input)
{
  struct n821_cpld_psu_data *psu = dev_get_drvdata(dev);
  unsigned int busbar;
  unsigned int fault;
  unsigned int value;
  int ret;

  ret = regmap_read(psu->regmap, psu->offset, &value);
  if (ret)
    return ret;

  if (channel == 0) {
    busbar = (value & 0x8) != 0;
    fault = (value & 0x2) != 0;
  } else if (channel == 1) {
    busbar = (value & 0x4) != 0;
    fault = (value & 0x1) != 0;
  } else {
    return -EINVAL;
  }

  switch (attr) {
    case hwmon_in_enable:
      *input = busbar;
      break;
    case hwmon_in_alarm:
      *input = fault;
      break;
    case hwmon_in_input:
      *input = (12 * 1000) * busbar;
      break;
    default:
      return -EOPNOTSUPP;
  }
  return 0;
}

static const struct hwmon_channel_info *n821_cpld_psu_info[] = {
  HWMON_CHANNEL_INFO(in, HWMON_I_ENABLE | HWMON_I_ALARM | HWMON_I_INPUT,
                         HWMON_I_ENABLE | HWMON_I_ALARM | HWMON_I_INPUT),
  NULL
};

static const struct hwmon_ops n821_cpld_psu_ops = {
  .is_visible = n821_cpld_psu_is_visible,
  .read = n821_cpld_psu_read,
};

static const struct hwmon_chip_info n821_cpld_psu_chip_info = {
  .ops = &n821_cpld_psu_ops,
  .info = n821_cpld_psu_info,
};

static int n821_cpld_psu_probe(struct platform_device *pdev)
{
  struct device *dev = &pdev->dev;
  struct n821_cpld_psu_data *psu;
  struct device *hwmon_dev;

  psu = devm_kzalloc(dev, sizeof(*psu), GFP_KERNEL);
  if (!psu)
    return -ENOMEM;

  psu->regmap = syscon_node_to_regmap(dev->of_node->parent);
  if (IS_ERR(psu->regmap)) {
    dev_err(dev, "failed to lookup regmap\n");
    return PTR_ERR(psu->regmap);
  }

  psu->offset = N821_CPLD_REG_PWR_TEST;
  hwmon_dev = devm_hwmon_device_register_with_info(&pdev->dev,
      "n821_psu", psu, &n821_cpld_psu_chip_info, NULL);
  if (IS_ERR(hwmon_dev))
    dev_err(&pdev->dev, "failed to register as hwmon device");
  dev_info(dev, "probed");
  return PTR_ERR_OR_ZERO(hwmon_dev);
}

static int n821_cpld_psu_remove(struct platform_device *pdev)
{
  return 0;
}

static struct of_device_id n821_cpld_psu_match_table[] = {
  { .compatible = "n821,cpld-psu" },
  { }
};

static struct platform_driver n821_cpld_psu_driver = {
        .driver = {
                .name = "n821-cpld-psu",
                .of_match_table = of_match_ptr(n821_cpld_psu_match_table),
        },
        .probe = n821_cpld_psu_probe,
        .remove = n821_cpld_psu_remove,
};
module_platform_driver(n821_cpld_psu_driver);

MODULE_DEVICE_TABLE(of, n821_cpld_psu_match_table);
MODULE_DESCRIPTION("Niagara 821 CPLD psu driver");
MODULE_AUTHOR("Christian Svensson <blue@cmd.nu>");
MODULE_LICENSE("GPL");
