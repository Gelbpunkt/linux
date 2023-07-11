/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Declarations for the CPLD used in the Cisco vEdge 1000 board Niagara 821
 *
 * Author: Christian Svensson <blue@cmd.nu>
 */

#ifndef __MFD_CISCO_N821_CPLD__
#define __MFD_CISCO_N821_CPLD__

#include <linux/regmap.h>

#define N821_CPLD_INT_IRQ_I2C       BIT(3)
#define N821_CPLD_INT_IRQ_RESET_BTN BIT(6)
#define N821_CPLD_INT_IRQ_OVERTEMP  BIT(7)

#define N821_CPLD_REG_REVISION    0x2
#define N821_CPLD_REG_PWR_CTRL    0x3
#define N821_CPLD_REG_USB3_RST    0x4
#define N821_CPLD_REG_PWR_STATUS  0x6
#define N821_CPLD_REG_FAN0_SPEED  0x7
#define N821_CPLD_REG_FAN1_SPEED  0x8
#define N821_CPLD_REG_SFP_PRSNT   0x0B
#define N821_CPLD_REG_IRQ_STATUS  0x10
#define N821_CPLD_REG_IRQ_MASK    0x11
#define N821_CPLD_REG_PWR_TEST    0x20

#define N821_CPLD_I2C_REG_CTRL    0x2

enum cisco_n821_cpld_irqs {
  N821_CPLD_IRQ_I2C,
  N821_CPLD_IRQ_RESET_BTN,
  N821_CPLD_IRQ_OVERTEMP,
};

#endif
