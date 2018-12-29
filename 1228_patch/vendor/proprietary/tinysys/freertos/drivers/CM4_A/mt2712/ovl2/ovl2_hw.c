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


#include "ovl2_util.h"
#include "ovl2_hw.h"

/* OVL2 */
#define OVL_MAX_WIDTH                   (8191u)
#define OVL_MAX_HEIGHT                  (4095u)
#define OVL_LAYER_NUM                   (4u)

#define OVL_LAYER_OFF_SITE                  (0x20UL)

#define DISPSYS_OVL0_BASE                   (0UL)

#define DISP_REG_OVL_STA                    (0x000UL)
#define DISP_REG_OVL_INTEN                  (0x004UL)
#define DISP_REG_OVL_INTSTA                 (0x008UL)
#define DISP_REG_OVL_EN                     (0x00CUL)
#define DISP_REG_OVL_TRIG                   (0x010UL)
#define DISP_REG_OVL_RST                    (0x014UL)
#define DISP_REG_OVL_ROI_SIZE               (0x020UL)
#define DISP_REG_OVL_DATAPATH_CON           (0x024UL)
#define DISP_REG_OVL_ROI_BGCLR              (0x028UL)
#define DISP_REG_OVL_SRC_CON                (0x02CUL)
#define DISP_REG_OVL_L0_CON                 (0x030UL)
#define DISP_REG_OVL_L0_SRCKEY              (0x034UL)
#define DISP_REG_OVL_L0_SRC_SIZE            (0x038UL)
#define DISP_REG_OVL_L0_OFFSET              (0x03CUL)
#define DISP_REG_OVL_L0_ADDR                (0xf40UL)
#define DISP_REG_OVL_L0_PITCH               (0x044UL)
#define DISP_REG_OVL_L1_CON                 (0x050UL)
#define DISP_REG_OVL_L1_SRCKEY              (0x054UL)
#define DISP_REG_OVL_L1_SRC_SIZE            (0x058UL)
#define DISP_REG_OVL_L1_OFFSET              (0x05CUL)
#define DISP_REG_OVL_L1_ADDR                (0xf60UL)
#define DISP_REG_OVL_L1_PITCH               (0x064UL)
#define DISP_REG_OVL_L2_CON                 (0x070UL)
#define DISP_REG_OVL_L2_SRCKEY              (0x074UL)
#define DISP_REG_OVL_L2_SRC_SIZE            (0x078UL)
#define DISP_REG_OVL_L2_OFFSET              (0x07CUL)
#define DISP_REG_OVL_L2_ADDR                (0xf80UL)
#define DISP_REG_OVL_L2_PITCH               (0x084UL)
#define DISP_REG_OVL_L3_CON                 (0x090UL)
#define DISP_REG_OVL_L3_SRCKEY              (0x094UL)
#define DISP_REG_OVL_L3_SRC_SIZE            (0x098UL)
#define DISP_REG_OVL_L3_OFFSET              (0x09CUL)
#define DISP_REG_OVL_L3_ADDR                (0xfA0UL)
#define DISP_REG_OVL_L3_PITCH               (0x0A4UL)
#define DISP_REG_OVL_RDMA0_CTRL             (0x0C0UL)
#define DISP_REG_OVL_RDMA0_FIFO_CTRL        (0x0D0UL)
#define DISP_REG_OVL_RDMA1_CTRL             (0x0E0UL)
#define DISP_REG_OVL_RDMA1_FIFO_CTRL        (0x0F0UL)
#define DISP_REG_OVL_RDMA2_CTRL             (0x100UL)
#define DISP_REG_OVL_RDMA2_FIFO_CTRL        (0x110UL)
#define DISP_REG_OVL_RDMA3_CTRL             (0x120UL)
#define DISP_REG_OVL_RDMA3_FIFO_CTRL        (0x130UL)
#define DISP_REG_OVL_DEBUG_MON_SEL          (0x1D4UL)
#define DISP_REG_OVL_RDMA0_MEM_GMC_S2       (0x1E0UL)
#define DISP_REG_OVL_RDMA1_MEM_GMC_S2       (0x1E4UL)
#define DISP_REG_OVL_RDMA2_MEM_GMC_S2       (0x1E8UL)
#define DISP_REG_OVL_RDMA3_MEM_GMC_S2       (0x1ECUL)
#define DISP_REG_OVL_DUMMY_REG              (0x200UL)
#define DISP_REG_OVL_FLOW_CTRL_DBG          (0x240UL)
#define DISP_REG_OVL_ADDCON_DBG	            (0x244UL)
#define DISP_REG_OVL_RDMA0_DBG              (0x24CUL)
#define DISP_REG_OVL_RDMA1_DBG              (0x250UL)
#define DISP_REG_OVL_RDMA2_DBG              (0x254UL)
#define DISP_REG_OVL_RDMA3_DBG              (0x258UL)


#define DATAPATH_CON_FLD_LAYER_SMI_ID_EN    REG_FLD(1, 0)

#define SRC_CON_FLD_L3_EN                   REG_FLD(1, 3)
#define SRC_CON_FLD_L2_EN                   REG_FLD(1, 2)
#define SRC_CON_FLD_L1_EN                   REG_FLD(1, 1)
#define SRC_CON_FLD_L0_EN                   REG_FLD(1, 0)

#define L_CON_FLD_SKEN                      REG_FLD(1, 30)
#define L_CON_FLD_LARC                      REG_FLD(2, 28)
#define L_CON_FLD_BTSW                      REG_FLD(1, 24)
#define L_CON_FLD_MTX                       REG_FLD(4, 16)
#define L_CON_FLD_CFMT                      REG_FLD(4, 12)
#define L_CON_FLD_AEN                       REG_FLD(1, 8)
#define L_CON_FLD_APHA                      REG_FLD(8, 0)

#define L_PITCH_FLD_LSP                     REG_FLD(16, 0)
#define L_PITCH_FLD_SUR_ALFA                REG_FLD(16, 16)

#define ADDCON_DBG_FLD_L3_WIN_HIT           REG_FLD(1, 31)
#define ADDCON_DBG_FLD_L2_WIN_HIT           REG_FLD(1, 30)
#define ADDCON_DBG_FLD_ROI_Y                REG_FLD(13, 16)
#define ADDCON_DBG_FLD_L1_WIN_HIT           REG_FLD(1, 15)
#define ADDCON_DBG_FLD_L0_WIN_HIT           REG_FLD(1, 14)
#define ADDCON_DBG_FLD_ROI_X                REG_FLD(13, 0)

static uint32_t mtk_ovl2_hw_input_fmt_byte_swap(enum OVL_INPUT_FORMAT fmt)
{
	uint32_t input_swap = 0;

	switch (fmt) {
	case OVL_INPUT_FORMAT_BGR565:
	case OVL_INPUT_FORMAT_RGB888:
	case OVL_INPUT_FORMAT_RGBA8888:
	case OVL_INPUT_FORMAT_ARGB8888:
	case OVL_INPUT_FORMAT_VYUY:
	case OVL_INPUT_FORMAT_YVYU:
		input_swap = 1;
		break;
	case OVL_INPUT_FORMAT_RGB565:
	case OVL_INPUT_FORMAT_BGR888:
	case OVL_INPUT_FORMAT_BGRA8888:
	case OVL_INPUT_FORMAT_ABGR8888:
	case OVL_INPUT_FORMAT_UYVY:
	case OVL_INPUT_FORMAT_YUYV:
		input_swap = 0;
		break;
	default:
		OVL2_LOG_E("unsupport fmt[%d] map to swap", fmt);
		input_swap = 0;
		break;
	}

	return input_swap;
}

static uint32_t mtk_ovl2_hw_input_fmt_bpp(enum OVL_INPUT_FORMAT fmt)
{
	uint32_t bpp = 0;

	switch (fmt) {
	case OVL_INPUT_FORMAT_BGR565:
	case OVL_INPUT_FORMAT_RGB565:
	case OVL_INPUT_FORMAT_VYUY:
	case OVL_INPUT_FORMAT_UYVY:
	case OVL_INPUT_FORMAT_YVYU:
	case OVL_INPUT_FORMAT_YUYV:
		bpp = 2;
		break;
	case OVL_INPUT_FORMAT_RGB888:
	case OVL_INPUT_FORMAT_BGR888:
		bpp = 3;
		break;
	case OVL_INPUT_FORMAT_RGBA8888:
	case OVL_INPUT_FORMAT_BGRA8888:
	case OVL_INPUT_FORMAT_ARGB8888:
	case OVL_INPUT_FORMAT_ABGR8888:
		bpp = 4;
		break;
	default:
		OVL2_LOG_E("unsupport fmt[%d] map to bpp", fmt);
		bpp = 2;
		break;
	}

	return bpp;
}

static uint32_t mtk_ovl2_hw_input_fmt_reg_value(enum OVL_INPUT_FORMAT fmt)
{
	uint32_t reg_value = 0;

	switch (fmt) {
	case OVL_INPUT_FORMAT_BGR565:
	case OVL_INPUT_FORMAT_RGB565:
		reg_value = 0x0;
		break;
	case OVL_INPUT_FORMAT_RGB888:
	case OVL_INPUT_FORMAT_BGR888:
		reg_value = 0x1;
		break;
	case OVL_INPUT_FORMAT_RGBA8888:
	case OVL_INPUT_FORMAT_BGRA8888:
		reg_value = 0x2;
		break;
	case OVL_INPUT_FORMAT_ARGB8888:
	case OVL_INPUT_FORMAT_ABGR8888:
		reg_value = 0x3;
		break;
	case OVL_INPUT_FORMAT_VYUY:
	case OVL_INPUT_FORMAT_UYVY:
		reg_value = 0x4;
		break;
	case OVL_INPUT_FORMAT_YVYU:
	case OVL_INPUT_FORMAT_YUYV:
		reg_value = 0x5;
		break;
	default:
		OVL2_LOG_E("unsupport fmt[%d] map to reg_value", fmt);
		reg_value = 0;
		break;
	}

	return reg_value;
}

static enum OVL_COLOR_SPACE mtk_ovl2_hw_input_fmt_color_space(
	enum OVL_INPUT_FORMAT fmt)
{
	enum OVL_COLOR_SPACE color_space = OVL_COLOR_SPACE_RGB;

	switch (fmt) {
	case OVL_INPUT_FORMAT_BGR565:
	case OVL_INPUT_FORMAT_RGB565:
	case OVL_INPUT_FORMAT_RGB888:
	case OVL_INPUT_FORMAT_BGR888:
	case OVL_INPUT_FORMAT_RGBA8888:
	case OVL_INPUT_FORMAT_BGRA8888:
	case OVL_INPUT_FORMAT_ARGB8888:
	case OVL_INPUT_FORMAT_ABGR8888:
		color_space = OVL_COLOR_SPACE_RGB;
		break;
	case OVL_INPUT_FORMAT_VYUY:
	case OVL_INPUT_FORMAT_UYVY:
	case OVL_INPUT_FORMAT_YVYU:
	case OVL_INPUT_FORMAT_YUYV:
		color_space = OVL_COLOR_SPACE_YUV;
		break;
	default:
		OVL2_LOG_E("unsupport fmt[%d] map to color_space", fmt);
		color_space = OVL_COLOR_SPACE_RGB;
		break;
	}

	return color_space;
}

static int32_t mtk_ovl2_hw_layer_switch(unsigned long reg_base,
	uint32_t layer, uint32_t en)
{
	int32_t ret = 0;

	OVL2_LOG_FUNC_START;
	OVL2_LOG_D("layer[%ld] enable[%ld]", layer, en);

	switch (layer) {
	case 0:
		DISP_REG_SET_FIELD(
			SRC_CON_FLD_L0_EN, reg_base + DISP_REG_OVL_SRC_CON, en);
		DISP_REG_SET(reg_base + DISP_REG_OVL_RDMA0_CTRL, en);
		break;
	case 1:
		DISP_REG_SET_FIELD(
			SRC_CON_FLD_L1_EN, reg_base + DISP_REG_OVL_SRC_CON, en);
		DISP_REG_SET(reg_base + DISP_REG_OVL_RDMA1_CTRL, en);
		break;
	case 2:
		DISP_REG_SET_FIELD(
			SRC_CON_FLD_L2_EN, reg_base + DISP_REG_OVL_SRC_CON, en);
		DISP_REG_SET(reg_base + DISP_REG_OVL_RDMA2_CTRL, en);
		break;
	case 3:
		DISP_REG_SET_FIELD(
			SRC_CON_FLD_L3_EN, reg_base + DISP_REG_OVL_SRC_CON, en);
		DISP_REG_SET(reg_base + DISP_REG_OVL_RDMA3_CTRL, en);
		break;
	default:
		OVL2_LOG_E("unsupport ovl layer[%ld]", layer);
		ret = -1;
		break;
	}

	OVL2_LOG_FUNC_END;

	return ret;
}

static int32_t mtk_ovl2_hw_enable(unsigned long reg_base)
{
	OVL2_LOG_FUNC_START;
	DISP_REG_SET(reg_base + DISP_REG_OVL_EN, 0x01);
	DISP_REG_SET(reg_base + DISP_REG_OVL_INTEN, 0x1FFE);
	DISP_REG_SET_FIELD(
		DATAPATH_CON_FLD_LAYER_SMI_ID_EN,
		reg_base + DISP_REG_OVL_DATAPATH_CON,
		0x1);

	return 0;
}

static int32_t mtk_ovl2_hw_disable(unsigned long reg_base)
{
	uint32_t i;
	int32_t ret = 0;

	OVL2_LOG_FUNC_START;

	DISP_REG_SET(reg_base + DISP_REG_OVL_INTEN, 0x00);
	DISP_REG_SET(reg_base + DISP_REG_OVL_EN, 0x00);
	DISP_REG_SET(reg_base + DISP_REG_OVL_INTSTA, 0x00);

	for (i = 0U; i < 4U; i++)
		ret += mtk_ovl2_hw_layer_switch(reg_base, i, 0);

	return ret;
}

static int32_t mtk_ovl2_hw_reset(unsigned long reg_base)
{
	#define OVL_IDLE (0x3U)
	#define OVL_WAIT_TIME (1U)
	#define OVL_WAIT_COUNT (2000U)

	int32_t ret = 0;
	uint32_t delay_cnt = 0;
	uint32_t val = 0;

	OVL2_LOG_FUNC_START;

	DISP_REG_SET(reg_base + DISP_REG_OVL_RST, 0x1);
	DISP_REG_SET(reg_base + DISP_REG_OVL_RST, 0x0);

	while (1) {
		delay_cnt++;
		udelay(OVL_WAIT_TIME);

		val = DISP_REG_GET(reg_base + DISP_REG_OVL_FLOW_CTRL_DBG);
		if ((val & OVL_IDLE) != 0U) {
			OVL2_LOG_D("ok to reset, timeout[%ld, %d] val[0x%lx]",
				delay_cnt, OVL_WAIT_TIME, val);
			break;
		}

		if (delay_cnt > OVL_WAIT_COUNT) {
			OVL2_LOG_E("fail to reset, timeout[%ld, %d] val[0x%lx]",
				delay_cnt, OVL_WAIT_TIME, val);
			ret = -1;
			break;
		}
	}

	return ret;
}

static int32_t mtk_ovl2_hw_roi(
	unsigned long reg_base,
	uint32_t bg_w,
	uint32_t bg_h,
	uint32_t bg_color)
{
	OVL2_LOG_FUNC_START;

	if ((bg_w > OVL_MAX_WIDTH) || (bg_h > OVL_MAX_HEIGHT)) {
		OVL2_LOG_E("unsupport roi width_height[%ld, %ld] max[%d, %d]",
			bg_w, bg_h,
			OVL_MAX_WIDTH, OVL_MAX_HEIGHT);
		return -1;
	}

	OVL2_LOG_D("[ovl] roi width_height[%ld, %ld]\n", bg_w, bg_h);
	OVL2_LOG_D("[ovl] reg_base[%x]\n", CM4_DISP_WDMA2_REG_BASE);

	//DISP_REG_GET(reg_base + DISP_REG_OVL_ROI_SIZE);
	DISP_REG_SET(reg_base + DISP_REG_OVL_ROI_SIZE, bg_h << 16 | bg_w);
	OVL2_LOG_D("[ovl] %s[%d]", __func__, __LINE__);

	//DISP_REG_GET(reg_base + DISP_REG_OVL_ROI_BGCLR);
	DISP_REG_SET(reg_base + DISP_REG_OVL_ROI_BGCLR, bg_color);
	OVL2_LOG_D("[ovl] %s[%d] end", __func__, __LINE__);

	return 0;
}

static int32_t mtk_ovl2_hw_layer_config(
	unsigned long reg_base,
	uint64_t layer,
	enum OVL_LAYER_SOURCE source,
	enum OVL_INPUT_FORMAT fmt,
	uint64_t addr,
	uint32_t src_x,	/* ROI x offset */
	uint32_t src_y,	/* ROI y offset */
	uint32_t src_pitch,
	uint32_t dst_x,	/* ROI x offset */
	uint32_t dst_y,	/* ROI y offset */
	uint32_t dst_w,	/* ROT width */
	uint32_t dst_h,	/* ROI height */
	uint32_t key_en,
	uint32_t key,	/* color key */
	uint32_t aen,	/* alpha enable */
	unsigned char alpha,
	uint32_t sur_aen,
	uint32_t src_alpha,
	uint32_t dst_alpha,
	uint32_t yuv_range)
{
	int32_t ret = 0;
	uint32_t value = 0;
	uint32_t bpp = mtk_ovl2_hw_input_fmt_bpp(fmt);
	uint32_t input_swap = mtk_ovl2_hw_input_fmt_byte_swap(fmt);
	uint32_t input_fmt = mtk_ovl2_hw_input_fmt_reg_value(fmt);
	enum OVL_COLOR_SPACE space = mtk_ovl2_hw_input_fmt_color_space(fmt);
	/*0100 MTX_JPEG_TO_RGB (YUV FUll TO RGB) */
	uint32_t color_matrix = 0x4;
	uint64_t layer_off_site =
		reg_base + (layer * OVL_LAYER_OFF_SITE);

	OVL2_LOG_FUNC_START;

	if ((dst_w > OVL_MAX_WIDTH) || (dst_h > OVL_MAX_HEIGHT)) {
		OVL2_LOG_E("unsupport dst width_height[%ld, %ld] max[%d, %d]",
			dst_w, dst_h,
			OVL_MAX_WIDTH, OVL_MAX_HEIGHT);
		return -1;
	}

	if (addr == 0UL) {
		OVL2_LOG_E("unsupport address[0x%llx]", addr);
		return -1;
	}

	switch (yuv_range) {
	case 0:
		color_matrix = 0x4;	/* JPEG_TO_RGB limited -> full */
		break;
	case 1:
		color_matrix = 0x6;	/* BT601_TO_RGB limited -> full */
		break;
	case 2:
		color_matrix = 0x7;	/* BT709_TO_RGB limited -> full */
		break;
	default:
		color_matrix = 0x4;	/* JPEG_TO_RGB */
		break;
	}

	OVL2_LOG_D("layer[0x%llx], off[%ld, %ld], dst[%ld, %ld, %ld, %ld], pitch[%ld], \
			col_mtx[%ld], fmt[%d], addr[0x%llx], key[%ld, %ld], \
				alpha[%ld, %d, %ld, 0x%lx]",
		layer,
		src_x, src_y,
		dst_x, dst_y, dst_w, dst_h,
		src_pitch,
		color_matrix,
		fmt,
		addr,
		key_en, key,
		aen, alpha, sur_aen, (dst_alpha << 2 | src_alpha));

	DISP_REG_SET(DISP_REG_OVL_RDMA0_CTRL + layer_off_site, 0x1);
	value = (REG_FLD_VAL((L_CON_FLD_LARC), (0u)) |
		 REG_FLD_VAL((L_CON_FLD_CFMT), (input_fmt)) |
		 REG_FLD_VAL((L_CON_FLD_AEN), (aen)) |
		 REG_FLD_VAL((L_CON_FLD_APHA), (alpha)) |
		 REG_FLD_VAL((L_CON_FLD_SKEN), (key_en)) |
		 REG_FLD_VAL((L_CON_FLD_BTSW), (input_swap)));

	if (space == OVL_COLOR_SPACE_YUV)
		value = value | REG_FLD_VAL((L_CON_FLD_MTX), (color_matrix));

	DISP_REG_SET(
		DISP_REG_OVL_L0_CON + layer_off_site, value);
	DISP_REG_SET(
		DISP_REG_OVL_L0_ADDR + layer_off_site,
		(uint32_t)addr + src_x * bpp + src_y * src_pitch);
	DISP_REG_SET(
		DISP_REG_OVL_L0_SRCKEY + layer_off_site, key);
	DISP_REG_SET(
		DISP_REG_OVL_L0_SRC_SIZE + layer_off_site, dst_h << 16 | dst_w);
	DISP_REG_SET(
		DISP_REG_OVL_L0_OFFSET + layer_off_site, dst_y << 16 | dst_x);

	value = (
		((sur_aen & 0x1u) << 15) |
		((dst_alpha & 0x3u) << 6) | ((dst_alpha & 0x3u) << 4) |
		((src_alpha & 0x3u) << 2) | (src_alpha & 0x3u)
		);

	value = (
		REG_FLD_VAL((L_PITCH_FLD_SUR_ALFA), (value)) |
		REG_FLD_VAL((L_PITCH_FLD_LSP), (src_pitch))
		);

	DISP_REG_SET(DISP_REG_OVL_L0_PITCH + layer_off_site, value);

	return ret;
}

static int32_t mtk_ovl2_hw_config(unsigned long reg_base,
	struct MTK_OVL2_HW_PARAM_ALL *pConfig)
{
	int32_t ret = 0;
	uint32_t i = 0;

	OVL2_LOG_FUNC_START;

	ret += mtk_ovl2_hw_roi(reg_base,
		 pConfig->dst_w, pConfig->dst_h, 0xFFFFFFFFU);

	for (i = 0; i < OVL_LAYER_NUM; i++) {
		if (pConfig->ovl_config[i].layer_en != 0U) {
			ret += mtk_ovl2_hw_layer_config(
				reg_base,
				i,
				pConfig->ovl_config[i].source,
				pConfig->ovl_config[i].fmt,
				pConfig->ovl_config[i].addr,
				pConfig->ovl_config[i].src_x,
				pConfig->ovl_config[i].src_y,
				pConfig->ovl_config[i].src_pitch,
				pConfig->ovl_config[i].dst_x,
				pConfig->ovl_config[i].dst_y,
				pConfig->ovl_config[i].dst_w,
				pConfig->ovl_config[i].dst_h,
				pConfig->ovl_config[i].keyEn,
				pConfig->ovl_config[i].key,
				pConfig->ovl_config[i].aen,
				pConfig->ovl_config[i].alpha,
				pConfig->ovl_config[i].sur_aen,
				pConfig->ovl_config[i].src_alpha,
				pConfig->ovl_config[i].dst_alpha,
				pConfig->ovl_config[i].yuv_range);

			ret += mtk_ovl2_hw_layer_switch(reg_base,
				i, pConfig->ovl_config[i].layer_en);
		}
	}

	return ret;
}

int32_t mtk_ovl2_hw_set(
		unsigned long reg_base, struct MTK_OVL2_HW_PARAM_ALL *pConfig)
{
	int32_t ret = 0;

	OVL2_LOG_FUNC_START;

	if (pConfig == NULL) {
		OVL2_LOG_E("invalid param[0x%p] in %s", pConfig, __func__);
		return -1;
	}

	ret = mtk_ovl2_hw_reset(reg_base);
	if (ret != 0)
		return ret;

	ret = mtk_ovl2_hw_config(reg_base, pConfig);
	if (ret != 0)
		return ret;

	ret = mtk_ovl2_hw_enable(reg_base);

	return ret;
}

int32_t mtk_ovl2_hw_unset(unsigned long reg_base)
{
	int32_t ret = 0;

	OVL2_LOG_FUNC_START;

	ret = mtk_ovl2_hw_disable(reg_base);

	return ret;
}
