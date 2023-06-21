# SPDX-License-Identifier: GPL-2.0

ccflags-$(CONFIG_DEBUG_N821_CPLD) += -DDEBUG

obj-$(CONFIG_N821_CPLD)      += n821-cpld.o
obj-$(CONFIG_N821_CPLD_FAN)  += n821-cpld-fan.o
obj-$(CONFIG_N821_CPLD_GPIO) += n821-cpld-gpio.o
obj-$(CONFIG_N821_CPLD_I2C)  += n821-cpld-i2c.o
obj-$(CONFIG_N821_CPLD_I2C)  += mdio-i2c-gen.o
obj-$(CONFIG_N821_CPLD_PSU)  += n821-cpld-psu.o

n821-cpld-objs      := cpld_core.o
n821-cpld-i2c-objs  := cpld_i2c.o
n821-cpld-fan-objs  := cpld_fan.o
n821-cpld-psu-objs  := cpld_psu.o
n821-cpld-gpio-objs := cpld_gpio.o
