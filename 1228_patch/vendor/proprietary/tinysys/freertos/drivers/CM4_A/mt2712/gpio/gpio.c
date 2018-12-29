/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#include <stdio.h>
#include <platform_mtk.h>
#include <interrupt.h>
#include <driver_api.h>
#include <utils.h>
#include "FreeRTOS.h"
#include "task.h"
#include "scp_regs.h"
#include "gpio.h"
#include "gpio_in.h"

#define ARRAY_SIZE(a)       (sizeof(a) / sizeof(a[0]))

#define MAX_GPIO_REG_BITS      (uint16_t)16
#define MAX_GPIO_MODE_PER_REG  (uint16_t)5
#define GPIO_MODE_BITS         (uint16_t)3

static struct gpio_regs *mt2721_gpio = (struct gpio_regs *)GPIO_BASE;

#define write16(a, v)	drv_write_reg16(a, v)
#define read16(a)	drv_reg16(a)

#define clrsetbits_le16(addr, clear, set) \
	write16((addr), ((read16((addr)) & ~((uint16_t)(clear))) | (uint16_t)(set)))

static int32_t gpio_cust_set_dir_chip(uint16_t pin, enum GPIO_DIR dir)
{
	uint32_t pos;
	uint16_t bit = 0;
	int32_t ret = 0;

	if (pin >= MAX_GPIO_PIN) {
		ret = -1;
	} else if (dir >= GPIO_DIR_MAX) {
		ret = -1;
	} else {
		pos = (uint32_t)(pin / MAX_GPIO_REG_BITS);
		bit = pin % MAX_GPIO_REG_BITS;

		if (dir == GPIO_DIR_IN) {
			write16(&mt2721_gpio->dir[pos].rst,
				(uint16_t)(1U << bit));
		} else {
			write16(&mt2721_gpio->dir[pos].set,
				(uint16_t)(1U << bit));
		}
		ret = 0;
	}
	return ret;
}

static uint32_t gpio_cust_get_dir_chip(uint16_t pin)
{
	uint32_t pos;
	uint16_t bit;
	uint32_t reg;
	uint32_t ret;

	if (pin >= MAX_GPIO_PIN) {
		ret = ~(0UL);
	} else {
		pos = (uint32_t)(pin / MAX_GPIO_REG_BITS);
		bit = pin % MAX_GPIO_REG_BITS;
		reg = read16(&mt2721_gpio->dir[pos].val);
		ret = ((reg & BIT(bit)) != 0UL) ? 1UL : 0UL;
	}
	return ret;
}

static int32_t gpio_cust_set_pull_enable_chip(uint16_t pin,
					      enum GPIO_PULL_EN enable)
{
	uint32_t pos;
	uint16_t bit;
	int32_t ret = 0;

	if (pin >= MAX_GPIO_PIN) {
		ret = -1;
	} else if (enable >= GPIO_PULL_EN_MAX) {
		ret = -1;
	} else {
		pos = (uint32_t)(pin / MAX_GPIO_REG_BITS);
		bit = pin % MAX_GPIO_REG_BITS;

		if (enable == GPIO_PULL_DISABLE) {
			write16(&mt2721_gpio->pullen[pos].rst,
				(uint16_t)(1U << bit));
		} else {
			write16(&mt2721_gpio->pullen[pos].set,
				(uint16_t)(1U << bit));
		}
		ret = 0;
	}
	return ret;
}

static uint32_t gpio_cust_get_pull_enable_chip(uint16_t pin)
{
	uint32_t pos;
	uint16_t bit;
	uint32_t reg = 0;
	uint32_t ret = 0;

	if (pin >= MAX_GPIO_PIN) {
		ret = ~(0UL);
	} else {
		pos = (uint32_t)(pin / MAX_GPIO_REG_BITS);
		bit = pin % MAX_GPIO_REG_BITS;

		reg = read16(&mt2721_gpio->pullen[pos].val);
		ret = ((reg & BIT(bit)) != 0UL) ? 1UL : 0UL;
	}
	return ret;
}

static int32_t gpio_cust_set_pull_select_chip(uint16_t pin,
					      enum GPIO_PULL select)
{
	uint32_t pos;
	uint16_t bit;
	int32_t ret = 0;

	if (pin >= MAX_GPIO_PIN) {
		ret = -1;
	} else if (select >= GPIO_PULL_MAX) {
		ret = -1;
	} else {
		pos = (uint32_t)(pin / MAX_GPIO_REG_BITS);
		bit = pin % MAX_GPIO_REG_BITS;

		if (select == GPIO_PULL_DOWN) {
			write16(&mt2721_gpio->pullsel[pos].rst,
				(uint16_t)(1U << bit));
		} else {
			write16(&mt2721_gpio->pullsel[pos].set,
				(uint16_t)(1U << bit));
		}
		ret = 0;
	}
	return ret;
}

static uint32_t gpio_cust_get_pull_select_chip(uint16_t pin)
{
	uint32_t pos;
	uint16_t bit;
	uint32_t reg;
	uint32_t ret = 0;

	if (pin >= MAX_GPIO_PIN) {
		ret = ~(0UL);
	} else {
		pos = (uint32_t)(pin / MAX_GPIO_REG_BITS);
		bit = pin % MAX_GPIO_REG_BITS;

		reg = read16(&mt2721_gpio->pullsel[pos].val);
		ret = (((reg & BIT(bit)) != 0UL) ? 1UL : 0UL);
	}
	return ret;
}

static int32_t gpio_cust_set_out_chip(uint16_t pin, enum GPIO_OUT output)
{
	uint32_t pos;
	uint16_t bit;
	int32_t ret;

	if (pin >= (uint16_t)144) {
		ret = -1;
	} else if (output >= GPIO_OUT_MAX) {
		ret = -1;
	} else {
		pos = (uint32_t)(pin / MAX_GPIO_REG_BITS);
		bit = pin % MAX_GPIO_REG_BITS;

		if (output == GPIO_OUT_ZERO) {
			write16(&mt2721_gpio->dout[pos].rst,
				(uint16_t)(1U << bit));
		} else {
			write16(&mt2721_gpio->dout[pos].set,
				(uint16_t)(1U << bit));
		}
		ret = 0;
	}
	return ret;
}

static uint32_t gpio_cust_get_out_chip(uint16_t pin)
{
	uint32_t pos;
	uint16_t bit;
	uint32_t reg;
	uint32_t ret;

	if (pin >= MAX_GPIO_PIN) {
		ret = ~(0UL);
	} else {
		pos = (uint32_t)(pin / MAX_GPIO_REG_BITS);
		bit = pin % MAX_GPIO_REG_BITS;

		reg = read16(&mt2721_gpio->din[pos].val);
		ret = ((reg & BIT(bit)) != 0UL) ? 1UL : 0UL;
	}
	return ret;
}

static uint32_t gpio_cust_get_in_chip(uint16_t pin)
{
	uint32_t pos;
	uint16_t bit;
	uint32_t reg;
	uint32_t ret;

	if (pin >= MAX_GPIO_PIN) {
		ret = ~(0UL);
	} else {
		pos = (uint32_t)(pin / MAX_GPIO_REG_BITS);
		bit = pin % MAX_GPIO_REG_BITS;
		reg = read16(&mt2721_gpio->din[pos].val);
		ret = ((reg & BIT(bit)) != 0UL) ? 1UL : 0UL;
	}
	return ret;
}

static int32_t gpio_cust_set_mode_chip(uint16_t pin, enum GPIO_MODE mode)
{
	uint32_t pos;
	uint16_t bit;
	uint16_t mask = ((uint16_t)1 << GPIO_MODE_BITS) - (uint16_t)1;
	int32_t ret = 0;

	if (pin >= MAX_GPIO_PIN) {
		ret = -1;
	} else if (mode >= GPIO_MODE_MAX) {
		ret = -1;
	} else {
		pos = (uint32_t)(pin / MAX_GPIO_MODE_PER_REG);
		bit = (pin % MAX_GPIO_MODE_PER_REG) * GPIO_MODE_BITS;

		clrsetbits_le16(&mt2721_gpio->mode[pos].val, mask << bit,
				((uint16_t)mode) << bit);
		ret = 0;
	}
	return ret;
}

static uint32_t gpio_cust_get_mode_chip(uint16_t pin)
{
	uint32_t pos;
	uint16_t bit;
	uint32_t reg;
	uint32_t mask = (1UL << GPIO_MODE_BITS) - 1U;
	uint32_t ret = 0;

	if (pin >= MAX_GPIO_PIN) {
		ret = ~(0UL);
	} else {
		pos = (uint32_t)(pin / MAX_GPIO_MODE_PER_REG);
		bit = pin % MAX_GPIO_MODE_PER_REG;
		reg = read16(&mt2721_gpio->mode[pos].val);
		ret = (reg >> (uint32_t)(GPIO_MODE_BITS*bit)) & mask;
	}
	return ret;
}

static uint16_t mt_gpio_pin_decrypt(uint16_t pin)
{
	uint16_t gpio;

	gpio = pin & ~(0x80000000U);

	return gpio;
}

/*set GPIO function in fact*/
int32_t gpio_set_dir(uint16_t pin, enum GPIO_DIR dir)
{
	uint16_t gpio;

	gpio = mt_gpio_pin_decrypt(pin);

	return gpio_cust_set_dir_chip(gpio, dir);
}

uint32_t gpio_get_dir(uint16_t pin)
{
	uint16_t gpio;

	gpio = mt_gpio_pin_decrypt(pin);
	return gpio_cust_get_dir_chip(gpio);
}

int32_t gpio_set_pull_enable(uint16_t pin,
				enum GPIO_PULL_EN enable)
{
	uint16_t gpio;

	gpio = mt_gpio_pin_decrypt(pin);

	return gpio_cust_set_pull_enable_chip(gpio, enable);
}

uint32_t gpio_get_pull_enable(uint16_t pin)
{
	uint16_t gpio;

	gpio = mt_gpio_pin_decrypt(pin);

	return gpio_cust_get_pull_enable_chip(gpio);
}

int32_t gpio_set_pull_select(uint16_t pin,
				enum GPIO_PULL select)
{
	uint16_t gpio;

	gpio = mt_gpio_pin_decrypt(pin);

	return gpio_cust_set_pull_select_chip(gpio, select);
}

uint32_t gpio_get_pull_select(uint16_t pin)
{
	uint16_t gpio;

	gpio = mt_gpio_pin_decrypt(pin);

	return gpio_cust_get_pull_select_chip(gpio);
}
int32_t gpio_set_out(uint16_t pin, enum GPIO_OUT output)
{
	uint16_t gpio;

	gpio = mt_gpio_pin_decrypt(pin);

	return gpio_cust_set_out_chip(gpio, output);
}

uint32_t gpio_get_out(uint16_t pin)
{
	uint16_t gpio;

	gpio = mt_gpio_pin_decrypt(pin);

	return gpio_cust_get_out_chip(gpio);
}

uint32_t gpio_get_in(uint16_t pin)
{
	uint16_t gpio;

	gpio = mt_gpio_pin_decrypt(pin);

	return gpio_cust_get_in_chip(gpio);
}

int32_t gpio_set_mode(uint16_t pin, enum GPIO_MODE mode)
{
	uint16_t gpio;

	gpio = mt_gpio_pin_decrypt(pin);

	return gpio_cust_set_mode_chip(gpio, mode);
}

uint32_t gpio_get_mode(uint16_t pin)
{
	uint16_t gpio;

	gpio = mt_gpio_pin_decrypt(pin);

	return gpio_cust_get_mode_chip(gpio);
}

#define PRINT_GPIO_GRP_REG(reg)\
do {\
uint32_t i;\
for (i = 0; i < ARRAY_SIZE(mt2721_gpio->reg); i++) {\
PRINTF("0x%lx: %s[%ld]:  val: %lx set: %lx rst: %lx \r\n", \
	(uint32_t)(&mt2721_gpio->reg[i]), #reg, i,\
	(uint32_t)(&mt2721_gpio->reg[i].val),\
	(uint32_t)(&mt2721_gpio->reg[i].set),\
	(uint32_t)(&mt2721_gpio->reg[i].rst));\
} \
} while (0)

void mt_gpio_dump_addr(void)
{
	PRINT_GPIO_GRP_REG(dir);
	PRINT_GPIO_GRP_REG(dout);
	PRINT_GPIO_GRP_REG(din);
	PRINT_GPIO_GRP_REG(mode);
	PRINT_GPIO_GRP_REG(ies);
	PRINT_GPIO_GRP_REG(smt);
	PRINT_GPIO_GRP_REG(tdsel);
	PRINT_GPIO_GRP_REG(rdsel);
	PRINT_GPIO_GRP_REG(pullen);
	PRINT_GPIO_GRP_REG(pullsel);
	PRINT_GPIO_GRP_REG(drv_mode);
}
