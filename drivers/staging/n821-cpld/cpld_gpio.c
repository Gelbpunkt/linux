/*
 * cpld-gpio.c - SFP GPIO part of the CPLD for Niagara 821
 */


#include <linux/gpio/driver.h>
#include <linux/gpio/regmap.h>
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


const char *n821_cpld_gpio_names[] = {
  /* 0  */ "mod-present Ge0/6",
  /* 1  */ "mod-present Ge0/7",
  /* 2  */ "mod-present Ge0/4",
  /* 3  */ "mod-present Ge0/5",
  /* 4  */ "mod-present Ge0/2",
  /* 5  */ "mod-present Ge0/3",
  /* 6  */ "mod-present Ge0/0",
  /* 7  */ "mod-present Ge0/1",
  /* 8  */ "tx-disable Ge0/6",
  /* 9  */ "tx-disable Ge0/7",
  /* 10 */ "tx-disable Ge0/4",
  /* 11 */ "tx-disable Ge0/5",
  /* 12 */ "tx-disable Ge0/2",
  /* 13 */ "tx-disable Ge0/3",
  /* 14 */ "tx-disable Ge0/0",
  /* 15 */ "tx-disable Ge0/1",
  /* 16 */ "los Ge0/6",
  /* 17 */ "los Ge0/7",
  /* 18 */ "los Ge0/4",
  /* 19 */ "los Ge0/5",
  /* 20 */ "los Ge0/2",
  /* 21 */ "los Ge0/3",
  /* 22 */ "los Ge0/0",
  /* 23 */ "los Ge0/1",
  /* 24 */ "tx-fault Ge0/6",
  /* 25 */ "tx-fault Ge0/7",
  /* 26 */ "tx-fault Ge0/4",
  /* 27 */ "tx-fault Ge0/5",
  /* 28 */ "tx-fault Ge0/2",
  /* 29 */ "tx-fault Ge0/3",
  /* 30 */ "tx-fault Ge0/0",
  /* 31 */ "tx-fault Ge0/1",
};

int n821_cpld_gpio_dir[] = {
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_OUT,
  GPIO_LINE_DIRECTION_OUT,
  GPIO_LINE_DIRECTION_OUT,
  GPIO_LINE_DIRECTION_OUT,
  GPIO_LINE_DIRECTION_OUT,
  GPIO_LINE_DIRECTION_OUT,
  GPIO_LINE_DIRECTION_OUT,
  GPIO_LINE_DIRECTION_OUT,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN,
  GPIO_LINE_DIRECTION_IN
};

static int n821_cpld_gpio_xlate(struct gpio_regmap *gpio, unsigned int base,
    unsigned int offset, unsigned int *reg,
    unsigned int *mask)
{
  unsigned int line = offset % 8;
  unsigned int stride = offset / 8;
  int dir = n821_cpld_gpio_dir[offset];

  if (base != 0) {
    *reg = base + stride;
    *mask = BIT(line);
    return 0;
  }
  // In/out directions are hard-coded for our GPIO controller
  // Map these read/writes to read-only buffers (in our case, the ID reg)
  // For N821 the ID reg is 0x7C, so we can use 0x10 as the mask for DIR_IN
  *reg = 0;
  if (dir == GPIO_LINE_DIRECTION_OUT) {
    *mask = 0x10;
  } else {
    *mask = 0;
  }
  return 0;
}

static int n821_cpld_gpio_probe(struct platform_device *pdev)
{
  struct device *dev = &pdev->dev;
  struct gpio_regmap_config config = {};
  struct regmap *regmap;
  int ret;

  regmap = syscon_node_to_regmap(dev->of_node->parent);
  if (IS_ERR(regmap)) {
    dev_err(dev, "failed to lookup regmap\n");
    return PTR_ERR(regmap);
  }

  config.regmap = regmap;
  config.parent = dev;
  config.names = n821_cpld_gpio_names;
  config.ngpio = ARRAY_SIZE(n821_cpld_gpio_names);
  config.ngpio_per_reg = 8;
  config.reg_dat_base = N821_CPLD_REG_SFP_PRSNT;
  config.reg_set_base = N821_CPLD_REG_SFP_PRSNT;
  config.reg_dir_out_base = GPIO_REGMAP_ADDR_ZERO;
  config.reg_mask_xlate = n821_cpld_gpio_xlate;

  ret = PTR_ERR_OR_ZERO(devm_gpio_regmap_register(dev, &config));
  dev_info(dev, "probed");
  return ret;
}

static int n821_cpld_gpio_remove(struct platform_device *pdev)
{
  return 0;
}

static struct of_device_id n821_cpld_gpio_match_table[] = {
  { .compatible = "n821,cpld-gpio" },
  { }
};

static struct platform_driver n821_cpld_gpio_driver = {
  .driver = {
    .name = "n821-cpld-gpio",
    .of_match_table = of_match_ptr(n821_cpld_gpio_match_table),
  },
  .probe = n821_cpld_gpio_probe,
  .remove = n821_cpld_gpio_remove,
};
module_platform_driver(n821_cpld_gpio_driver);

MODULE_DEVICE_TABLE(of, n821_cpld_gpio_match_table);
MODULE_DESCRIPTION("Niagara 821 CPLD GPIO driver");
MODULE_AUTHOR("Christian Svensson <blue@cmd.nu>");
MODULE_LICENSE("GPL");
