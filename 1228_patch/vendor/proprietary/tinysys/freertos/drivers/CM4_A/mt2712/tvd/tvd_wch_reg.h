/*
 * Copyright (c) 2018 MediaTek Inc.
 * Author: Qing Li <qing.li@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef TVD_WCH_REG_BASE_H
#define TVD_WCH_REG_BASE_H

#define VDOIN_ENABLE			(0x00)
#define BG_FRAME_CLR			BIT(26)
#define BG_422_MODE			BIT(25)
#define MTK_DATA_EN			BIT(21)
#define BG_LINE_ADDR_EN			BIT(14)
#define BG_EN_FLD_INV			BIT(12)
#define SD_2FS_INPUT			BIT(11)
#define BG_EN_EXT_EAV			BIT(6)
#define SRAM_EN_SEL			BIT(4)
#define BG_PROG_SEL			BIT(3)
#define BG_VDOFORMAT			BIT(2)
#define BG_EN_VDOIN_ON			BIT(0)

#define VDOIN_MODE			(0x04)
#define BG_THRESHOLD			(0x3U << 4)

#define VDOIN_Y0ADDR			(0x08)
#define BG_Y0_ADDR_DW			(0x3FFFFFFFU << 0)

#define VDOIN_LINE			(0x0C)
#define ACTIVE_LINE			(0xFFFU << 0)
#define BGVSYNC601DET			(0x7FU << 12)

#define VDOIN_C0ADDR			(0x10)
#define BG_C0_ADDR_DW			(0x3FFFFFFFU << 0)

#define VDOIN_DW_NEED			(0x14)
#define DW_NEED_C_LINE			(0xFFFU << 16)
#define DW_NEED_Y_LINE			(0xFFFU << 0)

#define VDOIN_HPKCNT			(0x18)
#define N_PIXEL				(0x7FFU << 0)

#define VDOIN_HBLACK			(0x1c)


#define VDOIN_CTRL			(0x20)
#define SD_480I_MIX			BIT(31)
#define SD_444_2FS			BIT(30)
#define INTERRUPT_CONTROL		BIT(26)
#define SD_4FS_OPT			BIT(23)
#define Hsync_INV			BIT(16)
#define Vsync_INV			BIT(15)
#define VDION_444_MODE			BIT(9)

#define VDOIN_VPKCNT			(0x24)
#define BOTTON_LINE			(0xFFFU << 16)
#define TOP_LINE			(0xFFFU)

#define VDOIN_WRAPPER_3D		(0x28)

#define VDOIN_WRAPPER_3D_VSYNC		(0x2c)


#define VDOIN_SWRST			(0x30)
#define WCH_SW_RST			(0x3U << 30)

#define VDOIN_HCNT			(0x34)
#define HACTCNT				(0x1FFFU << 16)
#define HCNT				(0x1FFFU << 0)

#define VDOIN_HCNT_SETTING		(0x38)
#define CHCNT				(0x3FFU << 16)
#define YHCNT				(0x3FFU << 0)

#define VDOIN_VSCALE			(0x3C)
#define HSIZE_DW			(0x3FFU << 16)

#define VDOIN_HSCALE			(0x40)

#define VDOIN_DEBUG_PORT_4		(0x50)


#define VDOIN_REQOUT			(0x7C)


#define VDOIN_REQ_OUT			(0x80)
#define SHIELD_REG			(0x3U << 28)

#define VDOIN_BLOCK_MODE		(0x88)
#define BlOCK_FMT			BIT(31)

#define WCH_STATUS_CLEAR_ENABLE		(0x3c)
#define TVD_STATUS_CLEAR_ENABLE		(0x04)
#define WCH_IOMMU_ENABLE		(0x0c)
#define TVD_HW_RESET			(0x130)

#endif
