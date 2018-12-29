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
#include "mtk_display_ddp_comp.h"
#include "mtk_display_ddp.h"
#include "mtk_display_macro.h"

#define DISP_REG_OVL_INTEN				0x0004
#define OVL_FME_CPL_INT					BIT(1)
#define DISP_REG_OVL_INTSTA				0x0008
#define DISP_REG_OVL_EN					0x000c
#define DISP_REG_OVL_RST				0x0014
#define DISP_REG_OVL_ROI_SIZE			0x0020
#define DISP_REG_OVL_ROI_BGCLR			0x0028
#define DISP_REG_OVL_SRC_CON			0x002c
#define DISP_REG_OVL_CON(n)				(0x0030 + 0x20 * (n))
#define DISP_REG_OVL_SRC_SIZE(n)		(0x0038 + 0x20 * (n))
#define DISP_REG_OVL_OFFSET(n)			(0x003c + 0x20 * (n))
#define DISP_REG_OVL_PITCH(n)			(0x0044 + 0x20 * (n))
#define DISP_REG_OVL_RDMA_CTRL(n)		(0x00c0 + 0x20 * (n))
#define DISP_REG_OVL_RDMA_GMC(n)		(0x00c8 + 0x20 * (n))
#define DISP_REG_OVL_ADDR_MT2701		0x0040
#define DISP_REG_OVL_ADDR_MT8173		0x0f40
#define DISP_REG_OVL_ADDR(module, n)	((module).addr + 0x20 * (n))

#define	OVL_RDMA_MEM_GMC		0x40402020
#define OVL_CON_BYTE_SWAP		BIT(24)
#define OVL_CON_MTX_YUV_TO_RGB	(6UL << 16)
#define OVL_CON_CLRFMT_RGB		(1UL << 12)
#define OVL_CON_CLRFMT_RGBA8888	(2 << 12)
#define OVL_CON_CLRFMT_ARGB8888	(3 << 12)
#define OVL_CON_CLRFMT_RGB565(module)	(((module).fmt_rgb565_is_0 \
					== true) ? 0UL : OVL_CON_CLRFMT_RGB)
#define OVL_CON_CLRFMT_RGB888(module)	(((module).fmt_rgb565_is_0 \
					== true) ? OVL_CON_CLRFMT_RGB : 0UL)
#define OVL_CON_CLRFMT_UYVY(module)	((module).fmt_uyvy)
#define OVL_CON_CLRFMT_YUYV(module)	((module).fmt_yuyv)
#define	OVL_CON_AEN			BIT(8)
#define	OVL_CON_ALPHA		0xff

#define OVL_CON_MTX_JPEG_TO_RGB			0x0
#define OVL_CON_MTX_BT601_TO_RGB		(0x6 << 16)
#define OVL_CON_MTX_BT709_TO_RGB		(0x7 << 16)
#define OVL_CON_MTX_FULL_BT709_TO_RGB	(0xf << 16)
#define DISP_REG_OVL_ADDR_MT2712		0x0f40

#define fourcc_code(a, b, c, d) ((uint32_t)(a) | ((uint32_t)(b) << 8) | \
				  ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#define DISPLAY_FORMAT_RGB565	fourcc_code('R', 'G', '1', '6') /* [15:0] R:G:B 5:6:5 little endian */
#define DISPLAY_FORMAT_BGR565	fourcc_code('B', 'G', '1', '6') /* [15:0] B:G:R 5:6:5 little endian */

 /* 24 bpp RGB */
#define DISPLAY_FORMAT_RGB888	fourcc_code('R', 'G', '2', '4') /* [23:0] R:G:B little endian */
#define DISPLAY_FORMAT_BGR888	fourcc_code('B', 'G', '2', '4') /* [23:0] B:G:R little endian */

 /* 32 bpp RGB */
#define DISPLAY_FORMAT_XRGB8888	fourcc_code('X', 'R', '2', '4') /* [31:0] x:R:G:B 8:8:8:8 little endian */
#define DISPLAY_FORMAT_XBGR8888	fourcc_code('X', 'B', '2', '4') /* [31:0] x:B:G:R 8:8:8:8 little endian */
#define DISPLAY_FORMAT_RGBX8888	fourcc_code('R', 'X', '2', '4') /* [31:0] R:G:B:x 8:8:8:8 little endian */
#define DISPLAY_FORMAT_BGRX8888	fourcc_code('B', 'X', '2', '4') /* [31:0] B:G:R:x 8:8:8:8 little endian */
#define DISPLAY_FORMAT_ARGB8888	fourcc_code('A', 'R', '2', '4') /* [31:0] A:R:G:B 8:8:8:8 little endian */
#define DISPLAY_FORMAT_ABGR8888	fourcc_code('A', 'B', '2', '4') /* [31:0] A:B:G:R 8:8:8:8 little endian */
#define DISPLAY_FORMAT_RGBA8888	fourcc_code('R', 'A', '2', '4') /* [31:0] R:G:B:A 8:8:8:8 little endian */
#define DISPLAY_FORMAT_BGRA8888	fourcc_code('B', 'A', '2', '4') /* [31:0] B:G:R:A 8:8:8:8 little endian */

/* packed YCbCr */
#define DISPLAY_FORMAT_YUYV		fourcc_code('Y', 'U', 'Y', 'V') /* [31:0] Cr0:Y1:Cb0:Y0 8:8:8:8 little endian */
#define DISPLAY_FORMAT_UYVY		fourcc_code('U', 'Y', 'V', 'Y') /* [31:0] Y1:Cr0:Y0:Cb0 8:8:8:8 little endian */


 typedef struct mtk_display_ovl_data {
	 uint32_t addr;
	 bool fmt_rgb565_is_0;
	 uint32_t fmt_uyvy;
	 uint32_t fmt_yuyv;
 }mtk_display_ovl_data;

 static const struct mtk_display_ovl_data mt2712_ovl_driver_data = {
	 .addr = DISP_REG_OVL_ADDR_MT2712,
	 .fmt_rgb565_is_0 = true,
	 .fmt_uyvy = 4U << 12,
	 .fmt_yuyv = 5U << 12,
 };

 static void mtk_ovl_start(mtk_display_ddp_comp *comp, void *handle)
 {
	 mtk_ddp_write_relaxed(comp, 0x1, DISP_REG_OVL_EN, handle);
 }

 static void mtk_ovl_stop(mtk_display_ddp_comp *comp, void *handle)
 {
	 mtk_ddp_write_relaxed(comp, 0x0, DISP_REG_OVL_EN, handle);
 }

 static void mtk_ovl_config(mtk_display_ddp_comp *comp, uint32_t w,
				uint32_t h, uint32_t vrefresh,
				uint32_t bpc, void *handle)
 {
	 if (w != 0 && h != 0)
		 mtk_ddp_write_relaxed(comp, h << 16 | w, DISP_REG_OVL_ROI_SIZE,
					   handle);
	 mtk_ddp_write_relaxed(comp, 0x0, DISP_REG_OVL_ROI_BGCLR, handle);
	 mtk_ddp_write(comp, 0x1, DISP_REG_OVL_RST, handle);
	 mtk_ddp_write(comp, 0x0, DISP_REG_OVL_RST, handle);
 }

 static void mtk_ovl_layer_on(mtk_display_ddp_comp *comp,
				  uint32_t idx, void *handle)
 {
	 mtk_ddp_write(comp, 0x1, DISP_REG_OVL_RDMA_CTRL(idx), handle);
	 mtk_ddp_write(comp, OVL_RDMA_MEM_GMC,
			   DISP_REG_OVL_RDMA_GMC(idx), handle);
	 mtk_ddp_write_mask(comp, BIT(idx),
				DISP_REG_OVL_SRC_CON, BIT(idx), handle);
 }

 static void mtk_ovl_layer_off(mtk_display_ddp_comp *comp, uint32_t idx,
				   void *handle)
 {
	 mtk_ddp_write_mask(comp, 0, DISP_REG_OVL_SRC_CON, BIT(idx), handle);
	 mtk_ddp_write(comp, 0, DISP_REG_OVL_RDMA_CTRL(idx), handle);
 }

 static void mtk_ovl_enable_vblank(struct mtk_display_ddp_comp *comp, void *handle)
{
	writel(0x0, CM4_DISP_OVL0_REG_BASE + DISP_REG_OVL_INTSTA);
	writel(OVL_FME_CPL_INT, CM4_DISP_OVL0_REG_BASE + DISP_REG_OVL_INTEN);
}

static void mtk_ovl_disable_vblank(struct mtk_display_ddp_comp *comp, void *handle)
{
	writel(0x0, CM4_DISP_OVL0_REG_BASE + DISP_REG_OVL_INTEN);
}

uint32_t mtk_bpp(uint32_t fmt)
{
 	uint32_t bpp = 0;

	switch (fmt) {
	case DISPLAY_FORMAT_RGB565:
	case DISPLAY_FORMAT_BGR565:
	case DISPLAY_FORMAT_UYVY:
	case DISPLAY_FORMAT_YUYV:
		 bpp = 16;
		 break;

	case DISPLAY_FORMAT_BGR888:
	case DISPLAY_FORMAT_RGB888:
		 bpp = 24;
		 break;

	case DISPLAY_FORMAT_ARGB8888:
	case DISPLAY_FORMAT_XRGB8888:
	case DISPLAY_FORMAT_ABGR8888:
	case DISPLAY_FORMAT_XBGR8888:
	case DISPLAY_FORMAT_RGBA8888:
	case DISPLAY_FORMAT_RGBX8888:
	case DISPLAY_FORMAT_BGRA8888:
	case DISPLAY_FORMAT_BGRX8888:
		 bpp = 32;
		 break;
	default:
	 	DISP_LOGW("unsupported format 0x%08x\n",  fmt);
	 	break;
	}

 	return bpp;
}

 static uint32_t ovl_fmt_convert(uint32_t fmt)
 {
	 switch (fmt) {
	 default:
	 case DISPLAY_FORMAT_RGB565:
		 return OVL_CON_CLRFMT_RGB565(mt2712_ovl_driver_data);
	 case DISPLAY_FORMAT_BGR565:
		 return (uint32_t)OVL_CON_CLRFMT_RGB565(mt2712_ovl_driver_data) |
					  OVL_CON_BYTE_SWAP;
	 case DISPLAY_FORMAT_RGB888:
		 return OVL_CON_CLRFMT_RGB888(mt2712_ovl_driver_data);
	 case DISPLAY_FORMAT_BGR888:
		 return (uint32_t)OVL_CON_CLRFMT_RGB888(mt2712_ovl_driver_data) |
					  OVL_CON_BYTE_SWAP;
	 case DISPLAY_FORMAT_RGBX8888:
	 case DISPLAY_FORMAT_RGBA8888:
		 return OVL_CON_CLRFMT_ARGB8888;
	 case DISPLAY_FORMAT_BGRX8888:
	 case DISPLAY_FORMAT_BGRA8888:
		 return OVL_CON_CLRFMT_ARGB8888 | OVL_CON_BYTE_SWAP;
	 case DISPLAY_FORMAT_XRGB8888:
	 case DISPLAY_FORMAT_ARGB8888:
		 return OVL_CON_CLRFMT_RGBA8888;
	 case DISPLAY_FORMAT_XBGR8888:
	 case DISPLAY_FORMAT_ABGR8888:
		 return OVL_CON_CLRFMT_RGBA8888 | OVL_CON_BYTE_SWAP;
	 case DISPLAY_FORMAT_UYVY:
		 return OVL_CON_CLRFMT_UYVY(mt2712_ovl_driver_data) | OVL_CON_MTX_YUV_TO_RGB;
	 case DISPLAY_FORMAT_YUYV:
		 return OVL_CON_CLRFMT_YUYV(mt2712_ovl_driver_data) | OVL_CON_MTX_YUV_TO_RGB;
	 }
 }

 static int ovl_mtx_convet(uint32_t color_matrix)
 {
	 switch (color_matrix) {
	 case 0:
		 return OVL_CON_MTX_JPEG_TO_RGB;
	 case 1:
		 return OVL_CON_MTX_BT601_TO_RGB;
	 case 2:
		 return OVL_CON_MTX_BT709_TO_RGB;
	 case 3:
		 return OVL_CON_MTX_FULL_BT709_TO_RGB;
	 default:
		 //DRM_ERROR("invalid color_matrix parameter.\n");
		 return -1;
	 }
 }

 static void mtk_ovl_layer_config(mtk_display_ddp_comp *comp, uint32_t idx,
				  struct mtk_display_layer_config *pending,
				  void *handle)
 {
	 uint32_t addr = pending->addr;
	 uint32_t pitch = pending->pitch & 0xffff;
	 uint32_t fmt = pending->format;
	 uint32_t offset = (pending->y << 16) | pending->x;
	 uint32_t src_size = (pending->height << 16) | pending->width;
	 uint32_t con;
	 uint32_t color_mtx;

	 DISP_LOGD("cloud debug: enable: %u, addr: %p, h: %u, w: %u, pitch: %u, fmt: %u, alpha: %u, matrix: %u\n",
	 		pending->enable, addr, pending->height,pending->width, pitch, fmt, pending->alpha, pending->color_matrix);
	 if (!pending->enable)
		 mtk_ovl_layer_off(comp, idx, handle);

	 if (pending->height == 0u || pending->width == 0u)
		 return;

	 con = ovl_fmt_convert(fmt);
	 if ((fmt != DISPLAY_FORMAT_XRGB8888) && (fmt != DISPLAY_FORMAT_XBGR8888))
		 con |= (uint32_t)(OVL_CON_AEN | pending->alpha);

	 color_mtx = ovl_mtx_convet(pending->color_matrix);
	 if (color_mtx < 0)
		 return;

	 con |= color_mtx;

	 mtk_ddp_write_relaxed(comp, con, DISP_REG_OVL_CON(idx), handle);
	 mtk_ddp_write_relaxed(comp, pitch, DISP_REG_OVL_PITCH(idx), handle);
	 mtk_ddp_write_relaxed(comp, src_size,
				   DISP_REG_OVL_SRC_SIZE(idx), handle);
	 mtk_ddp_write_relaxed(comp, offset, DISP_REG_OVL_OFFSET(idx), handle);
	 mtk_ddp_write_relaxed(comp, addr, DISP_REG_OVL_ADDR(mt2712_ovl_driver_data, idx), handle);

	 if (pending->enable)
		 mtk_ovl_layer_on(comp, idx, handle);
 }

 const struct mtk_display_ddp_comp_funcs mtk_disp_ovl_funcs = {
	 .config = mtk_ovl_config,
	 .start = mtk_ovl_start,
	 .stop = mtk_ovl_stop,
	 .enable_vblank = mtk_ovl_enable_vblank,
	 .disable_vblank = mtk_ovl_disable_vblank,
	 .layer_on = mtk_ovl_layer_on,
	 .layer_off = mtk_ovl_layer_off,
	 .layer_config = mtk_ovl_layer_config,
 };

