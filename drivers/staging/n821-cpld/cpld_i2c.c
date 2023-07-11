/*
 * cpld-i2c.c - I2C part of the CPLD for Niagara 821
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <linux/mfd/cisco-n821-cpld.h>


struct n821_cpld_i2c_data {
  struct device *dev;
  void __iomem *base;
  struct regmap *regmap;
  struct i2c_adapter       adap;
  struct i2c_algo_bit_data algo;
};


static void n821_cpld_i2c_set_scl(struct i2c_adapter *adap, int state)
{
  // TODO
}

static int n821_cpld_i2c_get_scl(struct i2c_adapter *adap)
{
  // TODO
  return -ENOTSUPP;
}

static int n821_cpld_i2c_xfer_polling(struct i2c_adapter *adap,
    struct i2c_msg *msgs, int num)
{
  struct n821_cpld_i2c_data *i2c = i2c_get_adapdata(adap);
  int count;

  for (count = 0; count < num; count++, msgs++) {
    regmap_write(i2c->regmap, 0x4, i2c_8bit_addr_from_msg(msgs));
    if (msgs->len == 0) {
      regmap_write(i2c->regmap, 0x0, 0x81);
    } else if (msgs->flags & I2C_M_RD) {
      if (msgs->len == 1) {
        regmap_write(i2c->regmap, 0x0, 0xc1);
      } else if (msgs->len == 2) {
        regmap_write(i2c->regmap, 0x0, 0xf1);
      } else if (msgs->len == 3) {
        regmap_write(i2c->regmap, 0x0, 0xfd);
      } else {
        goto err_unsupp;
      }
    } else {
      if (msgs->len == 1) {
        regmap_write(i2c->regmap, 0x7, msgs->buf[0]);
        regmap_write(i2c->regmap, 0x0, 0x41);
      } else if (msgs->len == 2) {
        regmap_write(i2c->regmap, 0x6, msgs->buf[0]);
        regmap_write(i2c->regmap, 0x7, msgs->buf[1]);
        regmap_write(i2c->regmap, 0x0, 0x51);
      } else if (msgs->len == 3) {
        regmap_write(i2c->regmap, 0x5, msgs->buf[0]);
        regmap_write(i2c->regmap, 0x6, msgs->buf[1]);
        regmap_write(i2c->regmap, 0x7, msgs->buf[2]);
        regmap_write(i2c->regmap, 0x0, 0x55);
      } else {
        goto err_unsupp;
      }
    }

    {
      int sleep;
      int status = 0;
      for (sleep = 100; sleep > 0; sleep--) {
        regmap_read(i2c->regmap, 0x1, &status);
        status &= 0x3f;
        if ((status & 0x2) != 0) {
          break;
        }
        usleep_range(5, 10);
      }
      if (status & 0x8) {
        // NACK
        return -EIO;
      }
      if (sleep == 0) {
        pr_info("n821 xfer timed out, last status 0x%02x", status);
      }
    }

    if (msgs->len > 0 && (msgs->flags & I2C_M_RD)) {
      unsigned int val = 0;
      if (msgs->len == 1) {
        regmap_read(i2c->regmap, 0x7, &val);
        msgs->buf[0] = val & 0xff;
      } else if (msgs->len == 2) {
        regmap_read(i2c->regmap, 0x6, &val);
        msgs->buf[0] = val & 0xff;
        regmap_read(i2c->regmap, 0x7, &val);
        msgs->buf[1] = val & 0xff;
      } else if (msgs->len == 3) {
        regmap_read(i2c->regmap, 0x5, &val);
        msgs->buf[0] = val & 0xff;
        regmap_read(i2c->regmap, 0x6, &val);
        msgs->buf[1] = val & 0xff;
        regmap_read(i2c->regmap, 0x7, &val);
        msgs->buf[2] = val & 0xff;
      } else {
        goto err_unsupp;
      }
    }
  }
  return count;
err_unsupp:
  dev_err(
      i2c->dev, "i2c xfer: id %d; addr: 0x%02x; flags: 0x%02x; len: 0x%02x\n",
      count, msgs->addr, msgs->flags, msgs->len);
  return -ENOTSUPP;
}

static u32 n821_cpld_i2c_functionallity(struct i2c_adapter *adap)
{
  return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static struct i2c_algorithm n821_cpld_i2c_algorithm = {
  .master_xfer = n821_cpld_i2c_xfer_polling,
  .master_xfer_atomic = n821_cpld_i2c_xfer_polling,
  .functionality = n821_cpld_i2c_functionallity,
};

static struct i2c_bus_recovery_info n821_cpld_i2c_recovery_info = {
  .get_scl = n821_cpld_i2c_get_scl,
  .set_scl = n821_cpld_i2c_set_scl,
};

#if 0
static const struct i2c_adapter_quirks n821_cpld_i2c_quirks = {
};
#endif

static const struct i2c_adapter n821_cpld_i2c_adapter = {
  .owner = THIS_MODULE,
  .name = "n821-cpld-i2c",
  .algo = &n821_cpld_i2c_algorithm,
  .bus_recovery_info = &n821_cpld_i2c_recovery_info,
#if 0
  .quirks = &n821_cpld_i2c_quirks,
#endif
};


static bool n821_cpld_i2c_is_volatile_reg(struct device *dev, unsigned int reg)
{
  return true;
}

static const struct regmap_config n821_cpld_i2c_regmap_config = {
  .reg_bits = 8,
  .val_bits = 8,
  .max_register = 0x8,
  .volatile_reg = n821_cpld_i2c_is_volatile_reg,
};

#ifdef __I2C_IRQ__
static irqreturn_t n821_cpld_i2c_interrupt(int irq, void *priv) {
  pr_info("got I2C interrupt 0x%x!", irq);
  disable_irq(irq);
  return IRQ_HANDLED;
}
#endif

static int n821_cpld_i2c_probe(struct platform_device *pdev)
{
  struct device *dev = &pdev->dev;
  struct regmap *regmap;
  struct n821_cpld_i2c_data *drvdata;
  void __iomem *reg_base;
  int ret = 0;
  int irq;

  reg_base = devm_of_iomap(dev, dev->of_node, 0, NULL);
  if (IS_ERR(reg_base)) {
    dev_err(dev, "failed to lock memory region");
    return PTR_ERR(reg_base);
  }

  regmap = devm_regmap_init_mmio(dev, reg_base, &n821_cpld_i2c_regmap_config);
  if (IS_ERR(regmap)) {
    dev_err(dev, "failed to create regmap");
    return PTR_ERR(reg_base);
  }

  irq = irq_of_parse_and_map(dev->of_node, 0);
  if (!irq) {
    dev_err(dev, "no IRQ found\n");
    return -ENODEV;
  }

  drvdata = devm_kzalloc(dev, sizeof(struct n821_cpld_i2c_data), GFP_KERNEL);
  if (drvdata == NULL) {
    return -ENOMEM;
  }
  drvdata->dev = dev;
  drvdata->base = reg_base;
  drvdata->regmap = regmap;
  dev_set_drvdata(dev, drvdata);

  // Set 100 kHz speed on I2C
  regmap_write(regmap, N821_CPLD_I2C_REG_CTRL, 0x4c);

#ifdef __I2C_IRQ__
  ret = devm_request_threaded_irq(dev, irq, NULL, n821_cpld_i2c_interrupt, IRQF_TRIGGER_HIGH | IRQF_ONESHOT, "n821-cpld-i2c", dev);
  if (ret) {
    dev_err(dev, "failed to request IRQ handler: %d", ret);
    return ret;
  }
#endif

  drvdata->adap = n821_cpld_i2c_adapter;
  i2c_set_adapdata(&drvdata->adap, drvdata);
  drvdata->adap.dev.parent = dev;
  drvdata->adap.dev.of_node = dev->of_node;
  drvdata->adap.nr = pdev->id;
  ret = i2c_add_adapter(&drvdata->adap);
  if (ret < 0) {
    dev_err(dev, "failed to add I2C adapter: %d", ret);
    return ret;
  }

  dev_info(dev, "probed, base = %px", reg_base);
  return 0;
}

static int n821_cpld_i2c_remove(struct platform_device *pdev)
{
  struct n821_cpld_i2c_data *drvdata = platform_get_drvdata(pdev);
  i2c_del_adapter(&drvdata->adap);
  return 0;
}

static struct of_device_id n821_cpld_i2c_match_table[] = {
  { .compatible = "n821,cpld-i2c" },
  { }
};

static struct platform_driver n821_cpld_i2_driver = {
        .driver = {
                .name = "n821-cpld-i2c",
                .of_match_table = of_match_ptr(n821_cpld_i2c_match_table),
        },
        .probe = n821_cpld_i2c_probe,
        .remove = n821_cpld_i2c_remove,
};
module_platform_driver(n821_cpld_i2_driver);

MODULE_DEVICE_TABLE(of, n821_cpld_i2c_match_table);
MODULE_DESCRIPTION("Niagara 821 CPLD I2C driver");
MODULE_AUTHOR("Christian Svensson <blue@cmd.nu>");
MODULE_LICENSE("GPL");
