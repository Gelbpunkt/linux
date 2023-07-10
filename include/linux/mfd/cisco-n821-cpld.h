// SPDX-License-Identifier: GPL-2.0
/*
 * cpld.c - Interface Masters CPLD module for Niagara 821
 * CPLD driver for Niagara 821 board used in Cisco vEdge 1000.
 *
 * Author: Christian Svensson <blue@cmd.nu>
 */

#ifndef __HEADER__CPLD__
#define __HEADER__CPLD__

#include <linux/regmap.h>

#define CISCO_N821_CPLD_INT_IRQ_I2C       BIT(3)
#define CISCO_N821_CPLD_INT_IRQ_RESET_BTN BIT(6)
#define CISCO_N821_CPLD_INT_IRQ_OVERTEMP  BIT(7)

#define CISCO_N821_CPLD_REG_REVISION    0x2
#define CISCO_N821_CPLD_REG_PWR_CTRL    0x3
#define CISCO_N821_CPLD_REG_USB3_RST    0x4
#define CISCO_N821_CPLD_REG_PWR_STATUS  0x6
#define CISCO_N821_CPLD_REG_FAN0_SPEED  0x7
#define CISCO_N821_CPLD_REG_FAN1_SPEED  0x8
#define CISCO_N821_CPLD_REG_SFP_PRSNT   0x0B
#define CISCO_N821_CPLD_REG_IRQ_STATUS  0x10
#define CISCO_N821_CPLD_REG_IRQ_MASK    0x11
#define CISCO_N821_CPLD_REG_PWR_TEST    0x20

#define CISCO_N821_CPLD_I2C_REG_CTRL    0x2

enum cisco_n821_cpld_irqs {
  CISCO_N821_CPLD_IRQ_I2C,
  CISCO_N821_CPLD_IRQ_RESET_BTN,
  CISCO_N821_CPLD_IRQ_OVERTEMP,
};

#endif
