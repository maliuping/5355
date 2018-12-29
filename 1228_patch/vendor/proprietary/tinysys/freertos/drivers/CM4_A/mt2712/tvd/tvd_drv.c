/*
 * tvd_drv.c - mtk tvd driver
 *
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

#include "bit_op.h"
#include "interrupt.h"
#include "mt_systmr.h"
#include "scp_ipi.h"
#include "tvd_if.h"
#include "tvd_private.h"
#include "tvd_cvbs_reg.h"
#include "tvd_wch_reg.h"
#include "tvd_reg.h"

static int32_t tvd_get_next_buf(struct TVD_DRV *drv)
{
	BaseType_t ret;
	video_buffer *buffer = NULL;

	ret = xQueueReceive(drv->queue_handle, &buffer, 0);
	if (ret != pdPASS) {
		/* TVD_LOG_DBG("fail to receive buf with queue[%p], ret %ld",
			drv->queue_handle, ret); */
		return TVD_RET_OK;
	} else {
		TVD_LOG_DBG("ok to receive buf with queue[%p], ret %ld",
			drv->queue_handle, ret);
	}

	if (buffer->image != NULL) {
		drv->buf.wch_next_buf.store = buffer;
		drv->buf.wch_next_buf.y =
			(unsigned long)(buffer->image);
		drv->buf.wch_next_buf.c =
			drv->buf.wch_next_buf.y +
			drv->wch.width * drv->wch.height;
		TVD_LOG_DBG("ok to get buf stamp[%lld] yc[0x%lx, 0x%lx]",
			drv->buf.wch_next_buf.store->timestamp,
			drv->buf.wch_next_buf.y,
			drv->buf.wch_next_buf.c);
	} else {
		TVD_LOG_ERR("fail to get buf");
	}

	return TVD_RET_OK;
}

static int32_t tvd_cb(struct TVD_DRV *drv)
{
	if ((drv->buf.wch_pre_buf.store->image != NULL) &&
			(drv->cb_func != NULL)) {
		TVD_LOG_DBG("cb buf stamp[%lld] addr[%p] func[%p] data[%p]",
			drv->buf.wch_pre_buf.store->timestamp,
			drv->buf.wch_pre_buf.store->image,
			drv->cb_func,
			drv->cb_data);
		drv->cb_func(
			drv->cb_data,
			VIDEO_BUFFER_OUTPUT,
			drv->buf.wch_pre_buf.store);
		drv->buf.wch_pre_buf.store = NULL;
	}

	return TVD_RET_OK;
}

static void wch_block_mode_enable(struct TVD_DRV *drv)
{
	void *base = drv->reg.wch_base;

	TVD_LOG_FUNC_START;;

	if (drv->wch.write_dram_method == BLOCK) {
		if (drv->wch.block_format == FORMAT_16_32) {
			TVD_CLR_BIT(VDOIN_ENABLE, BG_LINE_ADDR_EN);
			TVD_CLR_BIT(VDOIN_ENABLE, BIT(15));
			TVD_SET_FIELD(VDOIN_ENABLE, (0x3U << 23), (0x0U << 23));
			TVD_SET_FIELD(VDOIN_BLOCK_MODE, (0x1FFFU << 0), 720U);
		} else {
			TVD_CLR_BIT(VDOIN_ENABLE, BG_LINE_ADDR_EN);
			TVD_SET_BIT(VDOIN_ENABLE, BIT(15));
			TVD_SET_FIELD(VDOIN_ENABLE, (0x3U << 23), (0x2U << 23));
			TVD_SET_FIELD(VDOIN_BLOCK_MODE, (0x1FFFU << 0), 768U);
		}
	} else {
		TVD_SET_BIT(VDOIN_ENABLE, BG_LINE_ADDR_EN);
	}
}

static void wch_data_fmt(struct TVD_DRV *drv)
{
	void *base = drv->reg.wch_base;
	uint32_t tmp;

	TVD_LOG_FUNC_START;;

	tmp = TVD_READ32(VDOIN_DW_NEED) & DW_NEED_Y_LINE;
	if (drv->wch.data_format == FORMAT_420) {
		TVD_CLR_BIT(VDOIN_ENABLE, BG_422_MODE);
		tmp = (tmp >> 1) << 16;
		TVD_SET_FIELD(VDOIN_DW_NEED, DW_NEED_C_LINE, tmp);
	} else {
		//422 only used scanline
		TVD_SET_BIT(VDOIN_ENABLE, BG_LINE_ADDR_EN);
		TVD_SET_BIT(VDOIN_ENABLE, BG_422_MODE);
		tmp = tmp << 16;
		TVD_SET_FIELD(VDOIN_DW_NEED, DW_NEED_C_LINE, tmp);
	}
}

static void wch_img_status_clear(struct TVD_DRV *drv)
{
	void *base = drv->reg.dispsys_config_base;

	TVD_SET_BIT(WCH_STATUS_CLEAR_ENABLE, BIT(0));
}

static void wch_img_status_enable(struct TVD_DRV *drv)
{
	void *base = drv->reg.dispsys_config_base;

	TVD_CLR_BIT(WCH_STATUS_CLEAR_ENABLE, BIT(0));
}

static uint32_t wch_check_field_status(struct TVD_DRV *drv)
{
	void *base = drv->reg.wch_base;
	uint32_t field_flag;

	field_flag = (TVD_READ32(VDOIN_DEBUG_PORT_4) & BIT(28)) >> 28;
	return field_flag;
}

static void wch_ntsc_setting(struct TVD_DRV *drv)
{
	void *base = drv->reg.wch_base;

	TVD_LOG_FUNC_START;

	TVD_WRITE32(VDOIN_MODE, 0x48A00400U);
	TVD_WRITE32(VDOIN_LINE, 0xF60810EFU);
	TVD_WRITE32(VDOIN_DW_NEED, 0x007700EFU);
	TVD_WRITE32(VDOIN_HPKCNT, 0x00000000U);
	TVD_WRITE32(VDOIN_HBLACK, 0x00EF0040U);
	TVD_WRITE32(VDOIN_CTRL, 0x00000024U);
	TVD_WRITE32(VDOIN_VPKCNT, 0x00040004U);
	TVD_WRITE32(VDOIN_WRAPPER_3D, 0x089808CAU);
	TVD_WRITE32(VDOIN_WRAPPER_3D_VSYNC, 0x00000000U);
	TVD_WRITE32(VDOIN_SWRST, 0x00140005U);
	TVD_WRITE32(VDOIN_HCNT, 0x02D00338U);
	TVD_WRITE32(VDOIN_HCNT_SETTING, 0x002C002CU);
	TVD_WRITE32(VDOIN_VSCALE, 0x002D54E3U);
	TVD_WRITE32(VDOIN_HSCALE, 0x00000000U);
	TVD_WRITE32(VDOIN_REQOUT, 0x31800000U);
}

static void wch_pal_setting(struct TVD_DRV *drv)
{
	void *base = drv->reg.wch_base;

	TVD_LOG_FUNC_START;

	TVD_WRITE32(VDOIN_MODE, 0x48A00400U);
	TVD_WRITE32(VDOIN_LINE, 0xF608111fU);
	TVD_WRITE32(VDOIN_DW_NEED, 0x008f011fU);
	TVD_WRITE32(VDOIN_HPKCNT, 0x00000000U);
	TVD_WRITE32(VDOIN_HBLACK, 0x00EF0040U);
	TVD_WRITE32(VDOIN_CTRL, 0x00000024U);
	TVD_WRITE32(VDOIN_VPKCNT, 0x00040004U);
	TVD_WRITE32(VDOIN_WRAPPER_3D, 0x089808CAU);
	TVD_WRITE32(VDOIN_WRAPPER_3D_VSYNC, 0x00000000U);
	TVD_WRITE32(VDOIN_SWRST, 0x00140005U);
	TVD_WRITE32(VDOIN_HCNT, 0x02D00359U);
	TVD_WRITE32(VDOIN_HCNT_SETTING, 0x002C002CU);
	TVD_WRITE32(VDOIN_VSCALE, 0x002D54E3U);
	TVD_WRITE32(VDOIN_HSCALE, 0x00000000U);
	TVD_WRITE32(VDOIN_REQOUT, 0x31800000U);
}

static void wch_iommu_disable(struct TVD_DRV *drv)
{
	void *base = drv->reg.dram2axi_bridge_base;

	TVD_LOG_FUNC_START;

	TVD_CLR_BIT(WCH_IOMMU_ENABLE, BIT(23));
	TVD_CLR_BIT(WCH_IOMMU_ENABLE, BIT(7));
}

static void wch_shield_reg_enable(struct TVD_DRV *drv)
{
	void *base = drv->reg.wch_base;

	TVD_LOG_FUNC_START;

	TVD_SET_FIELD(VDOIN_REQ_OUT, SHIELD_REG, BIT(28));
}

static void wch_interrupt_method_choose(struct TVD_DRV *drv,
				int int_method)
{
	void *base = drv->reg.wch_base;

	TVD_LOG_FUNC_START;

	if (int_method == 0) {
		//tvd vsync interrupt
		TVD_CLR_BIT(VDOIN_CTRL, INTERRUPT_CONTROL);
		TVD_CLR_BIT(VDOIN_VSCALE, BIT(7));
		TVD_CLR_BIT(VDOIN_VSCALE, BIT(5));
		TVD_SET_BIT(VDOIN_VSCALE, BIT(14));
		TVD_CLR_BIT(VDOIN_VSCALE, BIT(6));
	} else if (int_method == 1) {
		//tvd field interrupt
		TVD_CLR_BIT(VDOIN_CTRL, INTERRUPT_CONTROL);
		TVD_CLR_BIT(VDOIN_VSCALE,  BIT(7));
		TVD_CLR_BIT(VDOIN_VSCALE,  BIT(5));
		TVD_CLR_BIT(VDOIN_VSCALE,  BIT(14));
		TVD_SET_BIT(VDOIN_VSCALE,  BIT(6));
	} else {
		//wch field interrupt
		TVD_SET_BIT(VDOIN_CTRL, INTERRUPT_CONTROL);
		TVD_SET_BIT(VDOIN_VSCALE,  BIT(5));
		TVD_CLR_BIT(VDOIN_VSCALE,  BIT(6));
		TVD_CLR_BIT(VDOIN_VSCALE,  BIT(7));
		TVD_CLR_BIT(VDOIN_VSCALE,  BIT(14));
	}
}

static void wch_set_addr(struct TVD_DRV *drv)
{
	void *base = drv->reg.wch_base;

	memcpy(&drv->buf.wch_pre_buf, &drv->buf.wch_curr_buf,
		sizeof(struct TVD_BUF_INFO));

	if (drv->buf.wch_next_buf.y != 0) {
		memcpy(&drv->buf.wch_curr_buf, &drv->buf.wch_next_buf,
			sizeof(struct TVD_BUF_INFO));
		memset(&drv->buf.wch_next_buf, 0, sizeof(struct TVD_BUF_INFO));
	} else {
		memcpy(&drv->buf.wch_curr_buf, &drv->buf.wch_priv_buf,
			sizeof(struct TVD_BUF_INFO));
	}

	if ((drv->buf.wch_curr_buf.y != drv->buf.wch_pre_buf.y) ||
			(drv->buf.wch_curr_buf.c != drv->buf.wch_pre_buf.c)) {
		TVD_WRITE32(VDOIN_Y0ADDR, drv->buf.wch_curr_buf.y >> 2);
		TVD_WRITE32(VDOIN_C0ADDR, drv->buf.wch_curr_buf.c >> 2);

		TVD_LOG_DBG("%s[%d] 0x%lx 0x%lx",
			__func__, __LINE__,
			drv->buf.wch_curr_buf.y, drv->buf.wch_curr_buf.c);
	}
}

static void wch_special_setting(struct TVD_DRV *drv)
{
	void *base = drv->reg.wch_base;
	uint32_t tvd_mode;

	TVD_LOG_FUNC_START;

	tvd_mode = drv->tvd.mode_switch_ctr.current_mode;
	if (tvd_mode == AV_PAL_N ||
	    tvd_mode == AV_PAL ||
	    tvd_mode == AV_SECAM)
		wch_pal_setting(drv);
	else
		wch_ntsc_setting(drv);

	TVD_SET_BIT(VDOIN_ENABLE, BG_EN_FLD_INV);
	TVD_SET_BIT(VDOIN_ENABLE, BIT(31));
	TVD_SET_BIT(VDOIN_ENABLE, BIT(10));
	TVD_SET_BIT(VDOIN_ENABLE, BIT(4));
	TVD_SET_BIT(VDOIN_ENABLE, BIT(2));
	TVD_SET_BIT(VDOIN_ENABLE, BIT(2));
	TVD_SET_BIT(VDOIN_REQ_OUT, BIT(26));
	TVD_SET_BIT(VDOIN_REQ_OUT, BIT(27));
	TVD_SET_BIT(VDOIN_REQ_OUT, BIT(30));
	TVD_SET_BIT(VDOIN_REQ_OUT, BIT(31));
	wch_block_mode_enable(drv);
	wch_data_fmt(drv);
	wch_interrupt_method_choose(drv, 1);
}

static void wch_init(struct TVD_DRV *drv)
{
	void *base = drv->reg.wch_base;

	TVD_LOG_FUNC_START;

	drv->wch.first_field = 0;
	drv->wch.discard_frame = 0;
	wch_iommu_disable(drv);
	wch_ntsc_setting(drv);
	wch_block_mode_enable(drv);
	wch_data_fmt(drv);
	wch_interrupt_method_choose(drv, 1);
	wch_special_setting(drv);
	TVD_SET_FIELD(VDOIN_SWRST, WCH_SW_RST, (0x3U << 30));
	TVD_SET_FIELD(VDOIN_SWRST, WCH_SW_RST, (0x0U << 30));
}

static void wch_enable(struct TVD_DRV *drv)
{
	void *base = drv->reg.wch_base;

	TVD_LOG_FUNC_START;

	TVD_SET_BIT(VDOIN_ENABLE, BG_EN_VDOIN_ON);
}

static void wch_disable(struct TVD_DRV *drv)
{
	void *base = drv->reg.wch_base;

	TVD_LOG_FUNC_START;

	TVD_CLR_BIT(VDOIN_ENABLE, BG_EN_VDOIN_ON);
}

static bool tdc_enable_chk(struct TVD_DRV *drv)
{
	void *base = drv->reg.tdc_base;
	uint32_t tmp;

	TVD_LOG_FUNC_START;

	tmp = bHwTvdMode();

	if (fgHwTvdSVID_TDC() != 0U)
		return false;

	if (tmp != AV_NTSC && tmp != AV_PAL &&
	    tmp != AV_PAL_M && tmp != AV_PAL_N) {
		TVD_LOG_ERR("tvd mode(%ld) can't enable 3D", tmp);
		return false;
	}

	return true;
}

static void tdc_off(struct TVD_DRV *drv)
{
	void *base = drv->reg.tdc_base;

	TVD_LOG_FUNC_START;

	TVD_SET_BIT(COMB_CTRL_03, FSKBACK);
	TVD_CLR_BIT(COMB_CTRL_02, EN3D);
}

static void tdc_on(struct TVD_DRV *drv)
{
	void *base = drv->reg.tdc_base;

	TVD_LOG_FUNC_START;

	if (tdc_enable_chk(drv)) {
		TVD_CLR_BIT(COMB_CTRL_03, FSKBACK);
		TVD_SET_BIT(COMB_CTRL_02, EN3D);
	} else {
		tdc_off(drv);
	}
}

static bool tdc_set_dram_base(struct TVD_DRV *drv,
			      unsigned long comb_memory_address)
{
	void *base = drv->reg.tdc_base;
	unsigned long addr;

	TVD_LOG_FUNC_START;

	addr = comb_memory_address >> 4;
	if ((addr & 0xF0000000U) != 0U ||
	    (addr & 0xFFFFFFFU) == 0U) {
		TVD_LOG_ERR("tdc addr is wrong, disable 3D");
		return false;
	}
	if ((addr & BIT(27)) != 0U)
		TVD_SET_FIELD(REG_SYS_0A, (0xFU << 28), (0xFU << 28));
	else
		TVD_SET_FIELD(REG_SYS_0A, (0xFU << 28), (0x0U << 28));

	if ((addr & BIT(26)) != 0U)
		TVD_SET_BIT(COMB2D_0D, CHANNEL_B_SEL);
	else
		TVD_CLR_BIT(COMB2D_0D, CHANNEL_B_SEL);

	if ((addr & BIT(25)) != 0U)
		TVD_SET_BIT(COMB2D_0D, DRAMBASEADR_MSB);
	else
		TVD_CLR_BIT(COMB2D_0D, DRAMBASEADR_MSB);

	addr = addr & DRAMBASEADR;
	TVD_SET_FIELD(COMB_CTRL_01, DRAMBASEADR, addr);
	return true;
}

static void tvd_3d_comb_setting(struct TVD_DRV *drv, enum TVD_MODE_E tvd_mode)
{
	bool ret;

	TVD_LOG_FUNC_START;

	tvd_3d_setting(drv);
	if (drv->tvd.support_3d != 0) {
		ret = tdc_set_dram_base(drv, drv->buf.tdc_buf);
		if (ret)
			tdc_on(drv);
		else
			tdc_off(drv);
	} else {
		tdc_off(drv);
	}
}

void tvd_polling_status(struct TVD_DRV *drv, int *arg)
{
	void *base = drv->reg.tvd_base;
	unsigned int tvd_status_lock;

	TVD_LOG_FUNC_START;

	tvd_status_lock = StatusLock();
	if (tvd_status_lock == 0x1U)
		*arg = 0x1;
	else
		*arg = 0x0;
}

static void tvd_special_setting(struct TVD_DRV *drv)
{
	void *base = drv->reg.tvd_base;

	TVD_LOG_FUNC_START;

	TVD_SET_BIT(REG_VSRC_07, RG_VSRC_INV_AIDX);
	TVD_SET_BIT(REG_CDET_00, MODE000);
	TVD_CLR_BIT(REG_DFE_0E, VPRES_SEL);
	TVD_WRITE32(REG_SYS_06, 0xFF001218U);
	TVD_WRITE32(REG_SYS_07, 0x2506FF0AU);
	TVD_WRITE32(REG_SYS_08, 0x18F5FFEBU);
	TVD_WRITE32(REG_SYS_09, 0x800FFF00U);
	TVD_CLR_BIT(REG_DEF_24, BIT(26));
	TVD_SET_BIT(REG_DEF_24, BIT(27));
	TVD_CLR_BIT(REG_SYS_09, BIT(31));
	TVD_CLR_BIT(REG_DFE_08, BIT(31));
	TVD_CLR_BIT(REG_DFE_09, BIT(15));
	TVD_SET_BIT(REG_DFE_09, BIT(31));
	TVD_SET_FIELD(REG_DFE_0A, (0x3FFU << 10), (0x100U << 10));
	TVD_CLR_BIT(CTG_07, BIT(10));
	TVD_SET_FIELD(REG_DFE_0B, (0x3FU << 17), (0xBU << 17));
}

static void tvd_manual_mode(struct TVD_DRV *drv)
{
	void *base = drv->reg.tvd_base;

	TVD_LOG_FUNC_START;

	TVD_SET_BIT(REG_CDET_00, TVD_MMODE);
	TVD_SET_FIELD(REG_CDET_00, 0x7U, drv->tvd.mode_switch_ctr.current_mode);
}

static void tvd_mode_detect(struct TVD_DRV *drv)
{
	void *base = drv->reg.tvd_base;
	uint32_t curr_tvd_mode;

	TVD_LOG_FUNC_START;

	if (drv->tvd.mode_switch_ctr.mode_is_fix != 0) {
		curr_tvd_mode = drv->tvd.mode_switch_ctr.current_mode;
		tvd_manual_mode(drv);
		TVD_LOG_ERR("[tvd] manual mode %ld, reg mode %ld",
			curr_tvd_mode, bHwTvdMode());
	} else {
		curr_tvd_mode = bHwTvdMode();
		drv->tvd.mode_switch_ctr.current_mode = curr_tvd_mode;
	}

	if (drv->tvd.support_3d == 1 &&
	    drv->tvd.mode != AV_SECAM &&
	    drv->tvd.mode != AV_PAL_60 &&
	    drv->tvd.mode != AV_NTSC443)
		tvd_3d_comb_setting(drv, curr_tvd_mode);
	else
		tvd_2d_setting(drv);
	//change_tvd_std(tvd, curr_tvd_mode);
	tvd_special_setting(drv);
	wch_special_setting(drv);
}

static void tvd_config_param_with_mode(struct TVD_DRV *drv)
{
	uint32_t mode;
	void *base = drv->reg.tvd_base;

	mode = bHwTvdMode();

	switch (mode) {
	case AV_PAL_N:
	case AV_PAL:
	case AV_PAL_M:
	case AV_PAL_60:
		drv->wch.width = 720;
		drv->wch.height = 576;
		break;
	case AV_NTSC:
	case AV_NTSC443:
		drv->wch.width = 720;
		drv->wch.height = 480;
		break;
	case AV_SECAM:
	case AV_UNSTABLE:
	default:
		drv->wch.width = 720;
		drv->wch.height = 576;
		break;
	}

	TVD_LOG_DBG("config wh[%ld, %ld] with mode[%ld]",
		drv->wch.width, drv->wch.height, mode);
}

static void tvd_irq_enable(struct TVD_DRV *drv)
{
	void *base = drv->reg.tvd_base;

	TVD_LOG_FUNC_START;

	TVD_CLR_BIT(TVD_INTR_EN, INTR_MODE_TVD);
	TVD_CLR_BIT(TVD_INTR_EN, INTR_VPRES_TVD);
}

static void tvd_irq_clear(struct TVD_DRV *drv, uint32_t irqstatus)
{
	void *base = drv->reg.tvd_base;

	TVD_WRITE32(TVD_INTR_STA, irqstatus);
}

static void tvd_img_status_clear(struct TVD_DRV *drv)
{
	void *base = drv->reg.dispsys_config_base;

	TVD_SET_BIT(TVD_STATUS_CLEAR_ENABLE, BIT(6));
}

static void tvd_img_status_enable(struct TVD_DRV *drv)
{
	void *base = drv->reg.dispsys_config_base;

	TVD_CLR_BIT(TVD_STATUS_CLEAR_ENABLE, BIT(6));
}

static void vsync_irq_enable(struct TVD_DRV *drv)
{
	void *base = drv->reg.tvd_base;

	TVD_LOG_FUNC_START;

	TVD_CLR_BIT(TVD_INTR_EN, INTR_VSYNC_TVD);
}

static void vsync_irq_disable(struct TVD_DRV *drv)
{
	void *base = drv->reg.tvd_base;

	TVD_LOG_FUNC_START;

	TVD_SET_BIT(TVD_INTR_EN, INTR_VSYNC_TVD);
}

static void tvd_sw_init(struct TVD_DRV *drv)
{
	TVD_LOG_FUNC_START;

	drv->tvd.mode_switch_ctr.tvd_vpres_flag = 1;
	drv->tvd.mode_switch_ctr.tvd_vpres_count = 60;
	drv->tvd.mode_switch_ctr.tvd_mode_switch_flag = 0;
	drv->tvd.mode_switch_ctr.tvd_vsync_count = 0;
}

static void tvd_hw_reset(struct TVD_DRV *drv)
{
	void *base = drv->reg.dispsys_config_base;

	TVD_LOG_FUNC_START;

	TVD_CLR_BIT(TVD_HW_RESET, BIT(13));
	TVD_CLR_BIT(TVD_HW_RESET, BIT(10));
	TVD_SET_BIT(TVD_HW_RESET, BIT(13));
	TVD_SET_BIT(TVD_HW_RESET, BIT(10));
}

static void tvd_sw_reset(struct TVD_DRV *drv)
{
	void *base = drv->reg.tvd_base;

	TVD_LOG_FUNC_START;

	TVD_SET_BIT(REG_SYS_00, BIT(2));
	TVD_CLR_BIT(REG_SYS_00, BIT(2));
}

static void tvd_hw_init(struct TVD_DRV *drv)
{
	TVD_LOG_FUNC_START;

	tvd_sw_init(drv);
	tvd_hw_reset(drv);
	tvd_sw_reset(drv);
	wch_init(drv);
	tvd_mode_detect(drv);
	tvd_irq_enable(drv);

	TVD_LOG_FUNC_END;
}

static void tvd_hw_uninit(struct TVD_DRV *drv)
{
	TVD_LOG_FUNC_START;

	tdc_off(drv);

	TVD_LOG_FUNC_END;
}

static void tvd_irq_handler(int irq, void *priv)
{
	struct TVD_DRV *drv = (struct TVD_DRV *)priv;
	void *base = drv->reg.tvd_base;
	static int vsync_cont;
	uint32_t irqenable;
	uint32_t irqstatus;
	uint32_t tmp;

	irqenable = (~(TVD_READ32(TVD_INTR_EN))) &
	    (INTR_MODE_TVD | INTR_VPRES_TVD | INTR_VSYNC_TVD);
	irqstatus = TVD_READ32(TVD_INTR_STA);
	tvd_irq_clear(drv, irqstatus);

	TVD_LOG_IRQ("%s cnt[%d] irq[0x%lx] MODE[%d] VPRES[%d] VSYNC[%d]",
		__func__,
		vsync_cont,
		irqstatus,
		((irqstatus & INTR_MODE_TVD) == INTR_MODE_TVD),
		((irqstatus & INTR_VPRES_TVD) == INTR_VPRES_TVD),
		((irqstatus & INTR_VSYNC_TVD) == INTR_VSYNC_TVD));

	irqstatus &= irqenable;
	tvd_img_status_clear(drv);
	if ((irqstatus & INTR_MODE_TVD) == INTR_MODE_TVD) {
		drv->tvd.mode_switch_ctr.tvd_mode_switch_flag = 1;
		tvd_mode_detect(drv);
	}

	if ((irqstatus & INTR_VPRES_TVD) == INTR_VPRES_TVD) {
		drv->tvd.mode_switch_ctr.tvd_vsync_count = 0;
	}

	if ((irqstatus & INTR_VSYNC_TVD) == INTR_VSYNC_TVD) {
		vsync_cont++;
		tmp = TVD_READ32(TG_STA_00) & LOCK_STATUS;
		if (tmp == LOCK_STATUS) {
			TVD_LOG_DBG("ok to lock signal, [%d, 0x%lx, 0x%lx]",
				vsync_cont, irqstatus, tmp);
			drv->tvd.tvd_signal_st.h_lock = 1;
			drv->tvd.tvd_signal_st.v_lock = 1;
			drv->tvd.tvd_signal_st.line_lock = 1;
			drv->tvd.tvd_signal_st.vpress_on = 1;
			vsync_irq_disable(drv);
			vsync_cont = 0;
			tvd_config_param_with_mode(drv);
		} else {
			if (vsync_cont > 32) {
				TVD_LOG_DBG("fail to lock signal, [%d, 0x%lx, 0x%lx]",
					vsync_cont, irqstatus, tmp);
				vsync_irq_disable(drv);
				vsync_cont = 0;
				tvd_config_param_with_mode(drv);
			}
			drv->tvd.tvd_signal_st.h_lock = 0;
			drv->tvd.tvd_signal_st.v_lock = 0;
			drv->tvd.tvd_signal_st.line_lock = 0;
			drv->tvd.tvd_signal_st.vpress_on = 0;
		}
	}

	tvd_img_status_enable(drv);

	return;
}

static void wch_irq_handler(int irq, void *priv)
{
	struct TVD_DRV *drv = (struct TVD_DRV *)priv;
	uint32_t field_flag;
	int first_field;

	first_field = drv->wch.first_field;
	wch_img_status_clear(drv);

	if (drv->wch.discard_frame < TVD_DISCARD_FRAME_COUNT) {
		drv->wch.discard_frame++;
		wch_img_status_enable(drv);
		return;
	} else if (drv->wch.discard_frame == TVD_DISCARD_FRAME_COUNT) {
		TVD_LOG_IRQ("wch discard %ld frame", drv->wch.discard_frame);
	}

	field_flag = wch_check_field_status(drv);

	TVD_LOG_IRQ("%s fst_fld[%ld] discard[%ld] fld_flg[%ld]",
		__func__,
		drv->wch.first_field, drv->wch.discard_frame, field_flag);

	if (field_flag == 0U) {
		if (first_field == 0) {
			wch_img_status_enable(drv);
			return;
		}
	}

	if (field_flag == 0U) {
		tvd_thread_st_change(drv, THREAD_CB_BUF);
	} else {
		if (first_field == 1)
			wch_set_addr(drv);
	}

	drv->wch.first_field = 1;
	wch_img_status_enable(drv);

	return;
}

void cvbs_power_on(struct TVD_DRV *drv)
{
	void *base = drv->reg.cvbs_base;

	TVD_LOG_FUNC_START;

	//cvbs pll un-gate
	TVD_SET_BIT(PA_PLL_CON0, RG_CLKSQ_EN);
	vTaskDelay(pdMS_TO_TICKS(3)); //usleep_range(2000, 3000);
	TVD_CLR_BIT(CVBS_CON6, RG_BGR_PI_PWD);
	TVD_SET_BIT(CVBSREFPLL_CON0, CVBSREFPLL_EN);
	udelay(80);
	TVD_SET_BIT(CVBSREFPLL_CON1, CVBSREFPLL_DIV26_EN);
	TVD_SET_BIT(CVBSPLL_CON0, CVBSPLL_EN);
	udelay(80);
	TVD_SET_BIT(CVBSPLL_CON1, CVBSPLL_DIV2_EN);
	udelay(20);
	//cvbs power on
	TVD_SET_FIELD(CVBS_CON0, RG_AISEL, BIT(24));
	TVD_CLR_BIT(CVBS_CON2, RG_IGEN_PWD);
	TVD_CLR_BIT(CVBS_CON1, RG_CVBS_PWD);
	TVD_CLR_BIT(CVBS_CON2, RG_PGABUFNA_PWD);
	TVD_CLR_BIT(CVBS_CON2, RG_CORE_PWD);
	TVD_CLR_BIT(CVBS_CON1, RG_CLAMP_PWD);
	TVD_CLR_BIT(CVBS_CON2, RG_SHIFTA_PWD);
	TVD_CLR_BIT(CVBS_CON2, RG_PROT_PWD);
	TVD_CLR_BIT(CVBS_CON2, RG_INMUX_PWD);
	TVD_CLR_BIT(CVBS_CON2, RG_CVBSADC1_IO_PWD);
	TVD_CLR_BIT(CVBS_CON3, RG_CVBSADC1_CORE_PWD);
	TVD_CLR_BIT(CVBS_CON3, RG_CVBSADC1_CK_PWD);
	TVD_SET_BIT(CVBS_CON3, RG_CVBSADC1_DC_EN);
	TVD_SET_FIELD(CVBS_CON2, RG_CVBSADC1_VREF_SEL, (0x2U << 0));
	TVD_SET_BIT(CVBS_CON0, RG_BLANK_EN);
	TVD_CLR_BIT(CVBS_CON0, RG_CHA_MIDDLE_EN);

	TVD_LOG_FUNC_END;
}

void cvbs_power_off(struct TVD_DRV *drv)
{
	void *base = drv->reg.cvbs_base;

	TVD_LOG_FUNC_START;

	tdc_off(drv);
	wch_disable(drv);
	TVD_SET_BIT(CVBS_CON2, RG_IGEN_PWD);
	TVD_SET_BIT(CVBS_CON1, RG_CVBS_PWD);
	TVD_SET_BIT(CVBS_CON2, RG_PGABUFNA_PWD);
	TVD_SET_BIT(CVBS_CON2, RG_CORE_PWD);
	TVD_SET_BIT(CVBS_CON1, RG_CLAMP_PWD);
	TVD_SET_BIT(CVBS_CON2, RG_SHIFTA_PWD);
	TVD_SET_BIT(CVBS_CON2, RG_PROT_PWD);
	TVD_SET_BIT(CVBS_CON2, RG_INMUX_PWD);
	TVD_SET_BIT(CVBS_CON2, RG_CVBSADC1_IO_PWD);
	TVD_SET_BIT(CVBS_CON3, RG_CVBSADC1_CORE_PWD);
	TVD_SET_BIT(CVBS_CON3, RG_CVBSADC1_CK_PWD);
	//cvbs pll gate
	TVD_CLR_BIT(CVBSPLL_CON0, CVBSPLL_EN);
	udelay(80);
	TVD_CLR_BIT(CVBSREFPLL_CON0, CVBSREFPLL_EN);

	TVD_LOG_FUNC_END;
}

static unsigned long tvd_alloc_phy(uint32_t size)
{
	DATA_ADDR tmp;
	unsigned long addr = 0;

	tmp = noncached_mem_alloc(size);
	addr = (unsigned long)(VA_TO_IOVA(tmp));

	return addr;
}

static int32_t tvd_enable(struct TVD_DRV *drv)
{
	TVD_LOG_FUNC_START;;

	/* power/clock enable */
	cvbs_power_on(drv);

	/* alloc tdc buffer */
	drv->buf.tdc_buf = tvd_alloc_phy(TVD_TDC_BUF_SIZE);
	if (drv->buf.tdc_buf == 0) {
		TVD_LOG_ERR("fail to alloc tdc_buf");
		return TVD_RET_FAIL;
	}
	TVD_LOG_DBG("ok to alloc tdc_buf[0x%lx]", drv->buf.tdc_buf);

	/* alloc private buffer for tvd writing */
	drv->buf.wch_priv_buf.y = tvd_alloc_phy(TVD_WCH_PRIV_BUF_SIZE);
	if (drv->buf.wch_priv_buf.y == 0)
	{
		TVD_LOG_ERR("fail to alloc priv_buf");
		return TVD_RET_FAIL;
	}
	drv->buf.wch_priv_buf.c = drv->buf.wch_priv_buf.y + TVD_Y_SIZE;
	drv->buf.wch_priv_buf.store = NULL;
	TVD_LOG_DBG("ok to alloc priv_buf[0x%lx, 0x%lx]",
		drv->buf.wch_priv_buf.y, drv->buf.wch_priv_buf.c);

	memcpy(&drv->buf.wch_next_buf, &drv->buf.wch_priv_buf,
		sizeof(struct TVD_BUF_INFO));

	/* hw init */
	tvd_hw_init(drv);
	vsync_irq_enable(drv);
	wch_set_addr(drv);
	wch_shield_reg_enable(drv);
	vTaskDelay(pdMS_TO_TICKS(100));
	wch_enable(drv);

	/* other */
	//drv->tvd.mode_switch_ctr.mode_is_fix = 1; //lq-check

	TVD_LOG_FUNC_END;

	return TVD_RET_OK;
}

static int32_t tvd_disable(struct TVD_DRV *drv)
{
	TVD_LOG_FUNC_START;

	tvd_hw_uninit(drv);

	TVD_LOG_FUNC_END;

	return TVD_RET_OK;
}

static int32_t tvd_param_init(struct TVD_DRV *drv)
{
	TVD_LOG_FUNC_START;

	drv->buf.tdc_buf = 0;
	drv->buf.wch_curr_buf.y = 0;
	drv->buf.wch_curr_buf.c = 0;
	drv->buf.wch_pre_buf.y = 0;
	drv->buf.wch_pre_buf.c = 0;
	drv->buf.wch_priv_buf.y = 0;
	drv->buf.wch_priv_buf.c = 0;

	drv->reg.cvbs_base = (void *)CM4_CVBS_BASE;
	drv->reg.dispsys_config_base = (void *)CM4_DISPSYS_CONFIG_BASE;
	drv->reg.tvd_base = (void *)CM4_TVD_BASE;
	drv->reg.tdc_base = drv->reg.tvd_base;
	drv->reg.wch_base = (void *)CM4_WCH_VDI_BASE;
	drv->reg.dram2axi_bridge_base = (void *)CM4_DRAM2AXI_BRIDGE_BASE;

	drv->tvd.irq = VDOIN_IRQ_BIT;
	drv->tvd.mode = 0; //lq-check
	//drv->tvd.mode_switch_ctr = ; //lq-check
	drv->tvd.support_3d = 0; //lq-check
	drv->tvd.tvd_signal_st.vpress_on = 0;
	drv->tvd.tvd_signal_st.line_lock= 0;
	drv->tvd.tvd_signal_st.h_lock = 0;
	drv->tvd.tvd_signal_st.v_lock = 0;

	drv->wch.block_format = FORMAT_16_32;
	drv->wch.data_format = FORMAT_420;
	drv->wch.write_dram_method = BLOCK;
	drv->wch.discard_frame = 0;
	drv->wch.first_field = 0;
	drv->wch.irq = WR_CHANNEL_VDI_IRQ_BIT;
	drv->wch.wch_fmt_fourcc = PIX_FMT_MT21;

	drv->thr_st = THREAD_NONE;
	drv->queue_handle = NULL;
	drv->thread_handle = NULL;
	drv->cb_func = NULL;
	drv->cb_data = NULL;

	return TVD_RET_OK;
}

void tvd_thread_st_change(struct TVD_DRV *drv, enum TVD_THREAD_ST st)
{
	/* TVD_LOG_DBG("thread st change, [%d]->[%d]", drv->thr_st, st); */
	drv->thr_st = st;
}

void tvd_thread(void *args)
{
	struct TVD_DRV *drv = (struct TVD_DRV *)args;

	tvd_param_init(drv);

	drv->queue_handle = NULL;
	drv->queue_handle = xQueueCreate(10, sizeof(video_buffer*));
	if (!drv->queue_handle) {
		TVD_LOG_ERR("fail to create queue");
		return;
	} else {
		TVD_LOG_DBG("ok to create queue[%p]", drv->queue_handle);
	}

	tvd_enable(drv);

	while (drv->thr_st != THREAD_RELEASE) {
		vTaskDelay(pdMS_TO_TICKS(1));

		switch (drv->thr_st) {
		case THREAD_GET_BUF:
			tvd_get_next_buf(drv);
			if (drv->buf.wch_next_buf.y != 0) {
				tvd_thread_st_change(drv, THREAD_IDLE);
			}
			break;
		case THREAD_CB_BUF:
			tvd_cb(drv);
			tvd_thread_st_change(drv, THREAD_GET_BUF);
			break;
		default:
			break;
		}
	}

	tvd_disable(drv);

	tvd_thread_st_change(drv, THREAD_NONE);
	vTaskDelete(NULL);
}

void tvd_irq_req(struct TVD_DRV *drv)
{
	TVD_LOG_FUNC_START;

	request_irq(
		drv->tvd.irq, tvd_irq_handler, IRQ_TYPE_LEVEL_LOW, "tvd", drv);
	request_irq(
		drv->wch.irq, wch_irq_handler, IRQ_TYPE_LEVEL_LOW, "wch", drv);
}

void tvd_irq_unreq(struct TVD_DRV *drv)
{
	TVD_LOG_FUNC_START;

	free_irq(drv->tvd.irq);
	free_irq(drv->wch.irq);
}

