/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
#include <driver_api.h>
#include <mt_reg_base.h>
#include "mtk_display_ddp.h"
#include "mtk_display_macro.h"

#define DISP_REG_CONFIG_DISP_OVL0_MOUT_EN	0x040
#define DISP_REG_CONFIG_DISP_OD_MOUT_EN		0x048
#define DISP_REG_CONFIG_DISP_GAMMA_MOUT_EN	0x04c
#define DISP_REG_CONFIG_DISP_UFOE_MOUT_EN	0x050
#define DISP_REG_CONFIG_DISP_COLOR0_SEL_IN	0x084
#define DISP_REG_CONFIG_DISP_WDMA0_SEL_IN	0x098
#define DISP_REG_CONFIG_DSIE_SEL_IN			0x0a4
#define DISP_REG_CONFIG_DSIO_SEL_IN			0x0a8
#define DISP_REG_CONFIG_DPI_SEL_IN			0x0ac
#define DISP_REG_CONFIG_DISP_RDMA0_MOUT_EN	0x0c4
#define DISP_REG_CONFIG_MMSYS_CG_CON0		0x100
#define DISP_REG_CONFIG_DISP_OVL_MOUT_EN	0x030
#define DISP_REG_CONFIG_OUT_SEL				0x04c
#define DISP_REG_CONFIG_DSI_SEL				0x050

#define DISP_REG_MUTEX_EN(n)	(0x20 + 0x20 * (n))
#define DISP_REG_MUTEX(n)		(0x24 + 0x20 * (n))
#define DISP_REG_MUTEX_RST(n)	(0x28 + 0x20 * (n))
#define DISP_REG_MUTEX_MOD(n)	(0x2c + 0x20 * (n))
#define DISP_REG_MUTEX_SOF(n)	(0x30 + 0x20 * (n))
#define DISP_REG_MUTEX_MOD2(n)	(0x34 + 0x20 * (n))

#define INT_MUTEX						BIT(1)
#define MT2712_MUTEX_MOD_DISP_OVL0		BIT(11)
#define MT2712_MUTEX_MOD_DISP_RDMA0		BIT(13)
#define MT2712_MUTEX_MOD_DISP_COLOR0	BIT(18)
#define MT2712_MUTEX_MOD_DISP_AAL		BIT(20)
#define MT2712_MUTEX_MOD_DISP_UFOE		BIT(22)
#define MT2712_MUTEX_MOD_DISP_PWM0		BIT(23)
#define MT2712_MUTEX_MOD_DISP_OD		BIT(25)

#define MUTEX_SOF_SINGLE_MODE		0
#define MUTEX_SOF_DPI0				3

#define OVL0_MOUT_EN_COLOR0		0x1
#define OD_MOUT_EN_RDMA0		0x1
#define OD_MOUT_EN_WDMA0		0x4
#define UFOE_MOUT_EN_DSI0		0x1
#define COLOR0_SEL_IN_OVL0		0x1
#define RDMA0_MOUT_DPI0			0x2
#define WDMA0_SEL_IN_OD0		0x0
#define OVL_MOUT_EN_RDMA		0x1
#define DSI_SEL_IN_BLS			0x0


static const uint32_t mt2712_mutex_mod[DDP_COMPONENT_ID_MAX] = {
	[DDP_COMPONENT_AAL] 	= MT2712_MUTEX_MOD_DISP_AAL,
	[DDP_COMPONENT_COLOR0] 	= MT2712_MUTEX_MOD_DISP_COLOR0,
	[DDP_COMPONENT_OD] 		= MT2712_MUTEX_MOD_DISP_OD,
	[DDP_COMPONENT_OVL0]	= MT2712_MUTEX_MOD_DISP_OVL0,
	[DDP_COMPONENT_PWM0] 	= MT2712_MUTEX_MOD_DISP_PWM0,
	[DDP_COMPONENT_RDMA0]	= MT2712_MUTEX_MOD_DISP_RDMA0,
};

static uint32_t mtk_ddp_mout_en(mtk_display_ddp_comp_id cur,
					 mtk_display_ddp_comp_id next,
					 uint32_t *addr)
{
	uint32_t value;

	if (cur == DDP_COMPONENT_OVL0 && next == DDP_COMPONENT_COLOR0) {
		*addr = DISP_REG_CONFIG_DISP_OVL0_MOUT_EN;
		value = OVL0_MOUT_EN_COLOR0;
	} else if (cur == DDP_COMPONENT_OVL0 && next == DDP_COMPONENT_RDMA0) {
		*addr = DISP_REG_CONFIG_DISP_OVL_MOUT_EN;
		value = OVL_MOUT_EN_RDMA;
	} else if (cur == DDP_COMPONENT_OD && next == DDP_COMPONENT_RDMA0) {
		*addr = DISP_REG_CONFIG_DISP_OD_MOUT_EN;
		value = OD_MOUT_EN_RDMA0;
	} else if (cur == DDP_COMPONENT_RDMA0 && next == DDP_COMPONENT_DPI0) {
		*addr = DISP_REG_CONFIG_DISP_RDMA0_MOUT_EN;
		value = RDMA0_MOUT_DPI0;
	} else if (cur == DDP_COMPONENT_OD && next == DDP_COMPONENT_WDMA0) {
		*addr = DISP_REG_CONFIG_DISP_OD_MOUT_EN;
		value = OD_MOUT_EN_WDMA0;
	} else {
		value = 0;
	}

	return value;
}

static uint32_t mtk_ddp_sel_in(mtk_display_ddp_comp_id cur,
				mtk_display_ddp_comp_id next,
				uint32_t *addr)
{
	uint32_t value;

	if (cur == DDP_COMPONENT_OVL0 && next == DDP_COMPONENT_COLOR0) {
		*addr = DISP_REG_CONFIG_DISP_COLOR0_SEL_IN;
		value = COLOR0_SEL_IN_OVL0;
	} else if (cur == DDP_COMPONENT_OD && next == DDP_COMPONENT_WDMA0) {
		*addr = DISP_REG_CONFIG_DISP_WDMA0_SEL_IN;
		value = WDMA0_SEL_IN_OD0;
	} else {
		value = 0;
	}

	return value;
}

void mtk_display_ddp_add_comp_to_path(uint32_t config_regs,
	   mtk_display_ddp_comp_id cur,
	   mtk_display_ddp_comp_id next)
{
	uint32_t addr, value, reg;

	value = mtk_ddp_mout_en(cur, next, &addr);
	if (value) {
		reg = readl(config_regs + addr) | value;
		writel(reg, config_regs + addr);
	}

	//mtk_ddp_sout_sel(config_regs, cur, next);

	value = mtk_ddp_sel_in(cur, next, &addr);
	if (value) {
		reg = readl(config_regs + addr) | value;
		writel(reg, config_regs + addr);
	}
}

void mtk_display_ddp_remove_comp_from_path(uint32_t config_regs,
		mtk_display_ddp_comp_id cur,
		mtk_display_ddp_comp_id next)
{
	uint32_t addr, value, reg;

	value = mtk_ddp_mout_en(cur, next, &addr);
	if (value) {
		reg = readl(config_regs + addr) & ~value;
		writel(reg, config_regs + addr);
	}

	value = mtk_ddp_sel_in(cur, next, &addr);
	if (value) {
		reg = readl(config_regs + addr) & ~value;
		writel(reg, config_regs + addr);
	}
}

void mtk_display_mutex_get(mtk_display_mutex *mutex)
{
	mutex->claimed = true;
}

void mtk_display_mutex_put(mtk_display_mutex *mutex)
{
	mutex->claimed = false;
}

void mtk_display_mutex_add_comp(struct mtk_display_mutex *mutex,
				  mtk_display_ddp_comp_id id)
{
	uint32_t reg;

	switch (id) {
		case DDP_COMPONENT_DPI0:
			reg = MUTEX_SOF_DPI0;
			break;
		default:
			if (mt2712_mutex_mod[id] <= BIT(31)) {
				reg = readl(CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_MOD(mutex->id));
				DISP_LOGD("cloud debug: id:%u, reg:%p, value:%u\n", id, CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_MOD(mutex->id), reg);

				reg |= mt2712_mutex_mod[id];

				writel(reg, CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_MOD(mutex->id));
			} else {
				reg = readl(CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_MOD2(mutex->id));
				reg |= (mt2712_mutex_mod[id] & ~BIT(31));
				writel(reg, CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_MOD2(mutex->id));
			}
			return;
	}

	writel(reg, CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_SOF(mutex->id));
}

void mtk_display_mutex_remove_comp(mtk_display_mutex *mutex,
				 mtk_display_ddp_comp_id id)
{
	uint32_t reg;


	switch (id) {
		case DDP_COMPONENT_DPI0:
			writel(MUTEX_SOF_SINGLE_MODE,
					CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_SOF(mutex->id));
			break;
		default:
			if (mt2712_mutex_mod[id] <= BIT(31)) {
				reg = readl(CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_MOD(mutex->id));
				reg &= ~(mt2712_mutex_mod[id]);
				writel(reg, CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_MOD(mutex->id));
			} else {
				reg = readl(CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_MOD2(mutex->id));
				reg &= ~(mt2712_mutex_mod[id] & ~BIT(31));
				writel(reg, CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_MOD2(mutex->id));
			}
			break;
	}
}

void mtk_display_mutex_enable(mtk_display_mutex *mutex)
{
	writel(1, CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_EN(mutex->id));
}

void mtk_display_mutex_disable(mtk_display_mutex *mutex)
{
	writel(0, CM4_DISP_MUTEX_BASE + DISP_REG_MUTEX_EN(mutex->id));
}

