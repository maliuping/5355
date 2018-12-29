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
#include "mtk_display_ddp_comp.h"
#include "mtk_display_macro.h"

#define DISP_OD_EN					0x0000
#define DISP_OD_INTEN				0x0008
#define DISP_OD_INTSTA				0x000c
#define DISP_OD_CFG					0x0020
#define DISP_OD_SIZE				0x0030
#define DISP_DITHER_5				0x0114
#define DISP_DITHER_7				0x011c
#define DISP_DITHER_15				0x013c
#define DISP_DITHER_16				0x0140

#define DISP_REG_UFO_START			0x0000
#define DISP_AAL_EN					0x0000
#define DISP_AAL_SIZE				0x0030
#define OD_RELAYMODE				BIT(0)
#define AAL_EN						BIT(0)

#define DISP_DITHERING				BIT(2)
#define DITHER_LSB_ERR_SHIFT_R(x)		(((x) & 0x7) << 28)
#define DITHER_OVFLW_BIT_R(x)			(((x) & 0x7) << 24)
#define DITHER_ADD_LSHIFT_R(x)			(((x) & 0x7) << 20)
#define DITHER_ADD_RSHIFT_R(x)			(((x) & 0x7) << 16)
#define DITHER_NEW_BIT_MODE				BIT(0)
#define DITHER_LSB_ERR_SHIFT_B(x)		(((x) & 0x7) << 28)
#define DITHER_OVFLW_BIT_B(x)			(((x) & 0x7) << 24)
#define DITHER_ADD_LSHIFT_B(x)			(((x) & 0x7) << 20)
#define DITHER_ADD_RSHIFT_B(x)			(((x) & 0x7) << 16)
#define DITHER_LSB_ERR_SHIFT_G(x)		(((x) & 0x7) << 12)
#define DITHER_OVFLW_BIT_G(x)			(((x) & 0x7) << 8)
#define DITHER_ADD_LSHIFT_G(x)			(((x) & 0x7) << 4)
#define DITHER_ADD_RSHIFT_G(x)			(((x) & 0x7) << 0)

/*  disp color */
#define DISP_COLOR_START_MT2712		0x0c00
#define DISP_COLOR_CFG_MAIN			0x0400
#define DISP_COLOR_START_MT2701		0x0f00
#define DISP_COLOR_START_MT8173		0x0c00
#define DISP_COLOR_START(module)	((module).color_offset)
#define DISP_COLOR_WIDTH(reg)		(DISP_COLOR_START(reg) + 0x50UL)
#define DISP_COLOR_HEIGHT(reg)		(DISP_COLOR_START(reg) + 0x54UL)

#define COLOR_BYPASS_ALL			BIT(7)
#define COLOR_SEQ_SEL				BIT(13)

/* disp rdma */
#define SIZE_8K							8192
#define DISP_REG_RDMA_INT_ENABLE		0x0000
#define DISP_REG_RDMA_INT_STATUS		0x0004
#define RDMA_TARGET_LINE_INT			BIT(5)
#define RDMA_FIFO_UNDERFLOW_INT			BIT(4)
#define RDMA_EOF_ABNORMAL_INT			BIT(3)
#define RDMA_FRAME_END_INT				BIT(2)
#define RDMA_FRAME_START_INT			BIT(1)
#define RDMA_REG_UPDATE_INT				BIT(0)
#define DISP_REG_RDMA_GLOBAL_CON		0x0010
#define RDMA_ENGINE_EN					BIT(0)
#define RDMA_SOFT_RESET					BIT(4)
#define RDMA_MODE_MEMORY				BIT(1)
#define DISP_REG_RDMA_SIZE_CON_0		0x0014
#define RDMA_MATRIX_ENABLE				BIT(17)
#define RDMA_MATRIX_INT_MTX_SEL			(7UL << 20)
#define DISP_REG_RDMA_SIZE_CON_1		0x0018
#define DISP_REG_RDMA_TARGET_LINE		0x001c
#define DISP_RDMA_MEM_CON				0x0024
#define MEM_MODE_INPUT_SWAP				BIT(8)
#define DISP_RDMA_MEM_SRC_PITCH			0x002c
#define DISP_RDMA_MEM_GMC_SETTING_0		0x0030
#define DISP_REG_RDMA_FIFO_CON			0x0040
#define RDMA_FIFO_UNDERFLOW_EN				BIT(31)
#define RDMA_FIFO_PSEUDO_SIZE(bytes)		(((bytes) / 16UL) << 16)
#define RDMA_OUTPUT_VALID_FIFO_THRESHOLD(bytes)		((bytes) / 16)
#define RDMA_FIFO_SIZE(module)			((module).fifo_size)
#define DISP_RDMA_MEM_START_ADDR		0x0f00

#define MATRIX_INT_MTX_SEL_DEFAULT		0xb00000
#define RDMA_MEM_GMC					0x40402020

#define MEM_MODE_INPUT_FORMAT_RGB565		0x0U
#define MEM_MODE_INPUT_FORMAT_RGB888		(0x001U << 4)
#define MEM_MODE_INPUT_FORMAT_RGBA8888		(0x002U << 4)
#define MEM_MODE_INPUT_FORMAT_ARGB8888		(0x003U << 4)
#define MEM_MODE_INPUT_FORMAT_UYVY			(0x004U << 4)
#define MEM_MODE_INPUT_FORMAT_YUYV			(0x005U << 4)
#define RDMA_DUMMY_BUFFER_SIZE(h, v)		((h) * (v) * 4)
#define RDMA_DUMMY_BUFFER_PITCH(h)			((h) * 4)

typedef struct mtk_display_ddp_comp_match {
  mtk_display_ddp_comp_id index;
  const mtk_display_ddp_comp_funcs *funcs;
}mtk_display_ddp_comp_match;

typedef struct mtk_display_color_data {
	uint32_t color_offset;
}mtk_display_color_data;

typedef struct mtk_display_rdma_data {
	uint32_t fifo_size;
}mtk_display_rdma_data;

extern struct mtk_display_ddp_comp_funcs mtk_disp_ovl_funcs;

static const struct mtk_display_color_data mt2712_color_driver_data = {
	.color_offset = DISP_COLOR_START_MT2712,
};


static const struct mtk_display_rdma_data mt2712_rdma_driver_data = {
	.fifo_size = SIZE_8K,
};

static void mtk_color_config(mtk_display_ddp_comp *comp, uint32_t w,
			     uint32_t h, uint32_t vrefresh,
			     uint32_t bpc, void *handle)
{

#ifdef CONFIG_MTK_DISPLAY_COLOR
/*
	if (mtk_color_if_bypass() == 0)
		mtk_color_init(comp, w, h);
	else
		mtk_ddp_write(comp, COLOR_BYPASS_ALL | COLOR_SEQ_SEL,
				DISP_COLOR_CFG_MAIN, handle);
				*/
#endif

	mtk_ddp_write(comp, w, DISP_COLOR_WIDTH(mt2712_color_driver_data), handle);
	mtk_ddp_write(comp, h, DISP_COLOR_HEIGHT(mt2712_color_driver_data), handle);

}

static void mtk_color_start(mtk_display_ddp_comp *comp, void *handle)
{
#ifndef CONFIG_MTK_DISPLAY_COLOR
	mtk_ddp_write(comp, COLOR_BYPASS_ALL | COLOR_SEQ_SEL,
		      DISP_COLOR_CFG_MAIN, handle);
#endif

	mtk_ddp_write(comp, 0x1, DISP_COLOR_START(mt2712_color_driver_data), handle);
}

static void mtk_color_stop(mtk_display_ddp_comp *comp, void *handle)
{
}

static void mtk_rdma_start(mtk_display_ddp_comp *comp, void *handle)
{
	mtk_ddp_write_mask(comp, MATRIX_INT_MTX_SEL_DEFAULT,
			   DISP_REG_RDMA_SIZE_CON_0, 0xff0000, handle);

	mtk_ddp_write_mask(comp, RDMA_ENGINE_EN,
			   DISP_REG_RDMA_GLOBAL_CON, RDMA_ENGINE_EN, handle);
}

static void mtk_rdma_stop(mtk_display_ddp_comp *comp, void *handle)
{
	mtk_ddp_write(comp, RDMA_SOFT_RESET, DISP_REG_RDMA_GLOBAL_CON, handle);
	mtk_ddp_write(comp, 0x0, DISP_REG_RDMA_GLOBAL_CON, handle);
}

static void mtk_rdma_config(mtk_display_ddp_comp *comp, uint32_t width,
			    uint32_t height, uint32_t vrefresh,
			    uint32_t bpc, void *handle)
{
	uint32_t threshold;
	uint32_t reg;

	mtk_ddp_write_mask(comp, width,
			   DISP_REG_RDMA_SIZE_CON_0, 0x1fff, handle);
	mtk_ddp_write_mask(comp, height,
			   DISP_REG_RDMA_SIZE_CON_1, 0xfffff, handle);

	/*
	 * Enable FIFO underflow since DSI and DPI can't be blocked.
	 * Keep the FIFO pseudo size reset default of 8 KiB. Set the
	 * output threshold to 6 microseconds with 7/6 overhead to
	 * account for blanking, and with a pixel depth of 4 bytes:
	 */
	threshold = width * height * vrefresh * 4 * 7 / 1000000;
	reg = RDMA_FIFO_UNDERFLOW_EN |
	      RDMA_FIFO_PSEUDO_SIZE(RDMA_FIFO_SIZE(mt2712_rdma_driver_data)) |
	      RDMA_OUTPUT_VALID_FIFO_THRESHOLD(threshold);
	mtk_ddp_write(comp, reg, DISP_REG_RDMA_FIFO_CON, handle);
}

void mtk_dither_set(struct mtk_display_ddp_comp *comp, uint32_t bpc,
		    uint32_t CFG, void *handle)
{
	/* If bpc equal to 0, the dithering function didn't be enabled */
	if (bpc == 0)
		return;

	if (bpc >= MTK_MIN_BPC) {
		mtk_ddp_write(comp, 0, DISP_DITHER_5, handle);
		mtk_ddp_write(comp, 0, DISP_DITHER_7, handle);
		mtk_ddp_write(comp,
			      DITHER_LSB_ERR_SHIFT_R(MTK_MAX_BPC - bpc) |
			      DITHER_ADD_LSHIFT_R(MTK_MAX_BPC - bpc) |
			      DITHER_NEW_BIT_MODE,
			      DISP_DITHER_15, handle);
		mtk_ddp_write(comp,
			      DITHER_LSB_ERR_SHIFT_B(MTK_MAX_BPC - bpc) |
			      DITHER_ADD_LSHIFT_B(MTK_MAX_BPC - bpc) |
			      DITHER_LSB_ERR_SHIFT_G(MTK_MAX_BPC - bpc) |
			      DITHER_ADD_LSHIFT_G(MTK_MAX_BPC - bpc),
			      DISP_DITHER_16, handle);
		mtk_ddp_write(comp, DISP_DITHERING, CFG, handle);
	}
}
static void mtk_od_config(mtk_display_ddp_comp *comp, uint32_t w,
			  uint32_t h, uint32_t vrefresh,
			  uint32_t bpc, void *handle)
{
	mtk_ddp_write(comp, w << 16 | h, DISP_OD_SIZE, handle);
	mtk_ddp_write(comp, OD_RELAYMODE, DISP_OD_CFG, handle);
	mtk_dither_set(comp, bpc, DISP_OD_CFG, handle);
}

static void mtk_od_start(mtk_display_ddp_comp *comp, void *handle)
{
	mtk_ddp_write(comp, 1, DISP_OD_EN, handle);
}

static void mtk_aal_config(mtk_display_ddp_comp *comp, uint32_t w,
			   uint32_t h, uint32_t vrefresh,
			   uint32_t bpc, void *handle)
{
	mtk_ddp_write(comp, h << 16 | w, DISP_AAL_SIZE, handle);
}

static void mtk_aal_start(mtk_display_ddp_comp *comp, void *handle)
{
	mtk_ddp_write(comp, AAL_EN, DISP_AAL_EN, handle);
}

static void mtk_aal_stop(mtk_display_ddp_comp *comp, void *handle)
{
	mtk_ddp_write_relaxed(comp, 0x0, DISP_AAL_EN, handle);
}


static const struct mtk_display_ddp_comp_funcs ddp_aal = {
	.config = mtk_aal_config,
	.start = mtk_aal_start,
	.stop = mtk_aal_stop,
};

static const struct mtk_display_ddp_comp_funcs ddp_od = {
	.config = mtk_od_config,
	.start = mtk_od_start,
};

static const struct mtk_display_ddp_comp_funcs mtk_disp_color_funcs = {
	.config = mtk_color_config,
	.start = mtk_color_start,
	.stop = mtk_color_stop,
};

static const struct mtk_display_ddp_comp_funcs mtk_disp_rdma_funcs = {
	.config = mtk_rdma_config,
	.start = mtk_rdma_start,
	.stop = mtk_rdma_stop,
};

static const struct mtk_display_ddp_comp_match mtk_display_ddp_matches[DDP_COMPONENT_ID_MAX] = {
	{ DDP_COMPONENT_OVL0, 	&mtk_disp_ovl_funcs },
	{ DDP_COMPONENT_COLOR0, 	&mtk_disp_color_funcs },
	{ DDP_COMPONENT_AAL, 		&ddp_aal },
	{ DDP_COMPONENT_OD,    	&ddp_od },
	{ DDP_COMPONENT_RDMA0, 	&mtk_disp_rdma_funcs },
	{ DDP_COMPONENT_DPI0, 	NULL },
	{ DDP_COMPONENT_LVDS0,	NULL },
	{ DDP_COMPONENT_WDMA0,	NULL },
	{ DDP_COMPONENT_PWM0, 	NULL },
};

void mtk_ddp_write(mtk_display_ddp_comp *comp, uint32_t value,
		   uint32_t offset, void *handle)
{
	(void)cmdq_pkt_write((struct cmdq_pkt *)handle, value, comp->cmdq_base,
			     offset);
}

void mtk_ddp_write_relaxed(mtk_display_ddp_comp *comp, uint32_t value,
			   uint32_t offset, void *handle)
{
	(void)cmdq_pkt_write((struct cmdq_pkt *)handle, value, comp->cmdq_base,
			     offset);
}

void mtk_ddp_write_mask(mtk_display_ddp_comp *comp, uint32_t value,
			uint32_t offset, uint32_t mask, void *handle)
{
	(void)cmdq_pkt_write_mask((struct cmdq_pkt *)handle,
				  value, comp->cmdq_base, offset, mask);
}

void mtk_display_ddp_comp_init(mtk_display_ddp_comp *comp, mtk_display_ddp_comp_id comp_id)
{
	if (comp_id < 0 || comp_id >= DDP_COMPONENT_ID_MAX)
    {
        DISP_LOGW("comp_id invalid");
		return;
    }

	comp->id = comp_id;
	comp->funcs = mtk_display_ddp_matches[comp_id].funcs;
}

