/*
 * cpld.c - Interface Masters CPLD module for Niagara 821
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/mfd/core.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>

#include "cpld.h"

struct n821_cpld_data {
  struct device *dev;
  struct regmap_irq_chip_data *irqdata;
};

static int n821_cpld_irq_pre(void * __unused)
{
  pr_info("n821_cpld_irq_pre called");
  return 0;
}

static const struct regmap_irq n821_cpld_irq_map[] = {
  REGMAP_IRQ_REG(N821_CPLD_IRQ_I2C, 0, N821_CPLD_INT_IRQ_I2C),
  REGMAP_IRQ_REG(N821_CPLD_IRQ_RESET_BTN, 0, N821_CPLD_INT_IRQ_RESET_BTN),
  REGMAP_IRQ_REG(N821_CPLD_IRQ_OVERTEMP, 0, N821_CPLD_INT_IRQ_OVERTEMP),
};

static const struct regmap_irq_chip n821_cpld_irq_chip = {
  .name            = "n821-cpld-ic",
  .status_base     = N821_CPLD_REG_IRQ_STATUS,
  .mask_base       = N821_CPLD_REG_IRQ_MASK,
  .ack_base        = N821_CPLD_REG_IRQ_STATUS,
  .mask_invert     = true,
  .num_regs        = 1,
  .init_ack_masked = 1,
  .irqs            = n821_cpld_irq_map,
  .num_irqs        = ARRAY_SIZE(n821_cpld_irq_map),
  .handle_pre_irq  = n821_cpld_irq_pre,
};

static ssize_t
cpld_rev_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  struct regmap *regmap = syscon_node_to_regmap(dev->of_node);
  int rev = 0;
  int ret = regmap_read(regmap, N821_CPLD_REG_REVISION, &rev);
  if (ret < 0) {
    return ret;
  }
  return snprintf(buf, PAGE_SIZE, "%d\n", rev & 0xf);
}

static DEVICE_ATTR_RO(cpld_rev);

static struct attribute *n821_cpld_attrs[] = {
  &dev_attr_cpld_rev.attr,
  NULL
};

static struct attribute_group n821_cpld_group = {
      .attrs = n821_cpld_attrs,
};

static void n821_cpld_enable_usb3(struct device *dev)
{
  // Ensure USB3 is enabled
  struct regmap *regmap = syscon_node_to_regmap(dev->of_node);
  regmap_write(regmap, N821_CPLD_REG_USB3_RST, 0);
}

static int n821_cpld_probe(struct platform_device *pdev)
{
  struct device *dev = &pdev->dev;
  struct regmap *regmap;
  struct n821_cpld_data *drvdata;
  int irq;
  int ret;

  regmap = syscon_node_to_regmap(dev->of_node);
  if (IS_ERR(regmap)) {
    dev_info(dev, "cannot get regmap, try again later\n");
    return -EPROBE_DEFER;
  }

  irq = irq_of_parse_and_map(dev->of_node, 0);
  if (!irq) {
    dev_err(dev, "no IRQ found\n");
    return -ENODEV;
  }

  drvdata = devm_kzalloc(dev, sizeof(struct n821_cpld_data), GFP_KERNEL);
  if (drvdata == NULL) {
    return -ENOMEM;
  }
  drvdata->dev = dev;
  drvdata->irqdata = NULL;
  dev_set_drvdata(dev, drvdata);

  ret = devm_regmap_add_irq_chip_fwnode(dev, dev_fwnode(dev), regmap, irq, IRQF_ONESHOT, 0, &n821_cpld_irq_chip, &drvdata->irqdata);
  if (ret) {
    dev_err(dev, "failed to add IRQ chip: %d\n", ret);
    return ret;
  }

  dev_info(dev, "probed");

  ret = devm_of_platform_populate(dev);
  if (ret) {
    dev_err(dev, "populate platform failed: %d\n", ret);
    return ret;
  }

  n821_cpld_enable_usb3(dev);
  return devm_device_add_group(dev, &n821_cpld_group);
}

static int n821_cpld_remove(struct platform_device *pdev)
{
  return 0;
}


static struct of_device_id n821_cpld_match_table[] = {
  { .compatible = "n821,cpld" },
  { }
};


static struct platform_driver n821_cpld_platform_driver = {
  .probe = n821_cpld_probe,
  .remove = n821_cpld_remove,
  .driver = {
    .name = "n821-cpld",
    .owner = THIS_MODULE,
    .of_match_table = of_match_ptr(n821_cpld_match_table),
  },
};

module_platform_driver(n821_cpld_platform_driver);

MODULE_DEVICE_TABLE(of, n821_cpld_match_table);
MODULE_DESCRIPTION("Niagara 821 CPLD driver");
MODULE_AUTHOR("Christian Svensson <blue@cmd.nu>");
MODULE_LICENSE("GPL");
