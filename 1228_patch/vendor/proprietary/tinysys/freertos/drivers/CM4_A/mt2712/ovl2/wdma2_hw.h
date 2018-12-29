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
#ifndef WDMA2_HW_H
#define WDMA2_HW_H

#include <FreeRTOS.h>

#define YUV2RGB_601_16_16  0U
#define YUV2RGB_601_16_0   1U
#define YUV2RGB_601_0_0    2U
#define YUV2RGB_709_16_16  3U
#define YUV2RGB_709_16_0   4U
#define YUV2RGB_709_0_0    5U
#define RGB2YUV_601        6U
#define RGB2YUV_601_XVYCC  7U
#define RGB2YUV_709        8U
#define RGB2YUV_709_XVYCC  9U
#define TABLE_NO           10U

enum WDMA_COLOR_SPACE {
	WDMA_COLOR_SPACE_RGB = 0,
	WDMA_COLOR_SPACE_YUV,
};

enum MTK_WDMA_HW_FORMAT {
	MTK_WDMA_HW_FORMAT_RGB565 = 0x00,
	MTK_WDMA_HW_FORMAT_RGB888 = 0x01,
	MTK_WDMA_HW_FORMAT_RGBA8888 = 0x02,
	MTK_WDMA_HW_FORMAT_ARGB8888 = 0x03,
	MTK_WDMA_HW_FORMAT_UYVY = 0x04,
	MTK_WDMA_HW_FORMAT_YUYV = 0x05,
	MTK_WDMA_HW_FORMAT_NV21 = 0x06,
	MTK_WDMA_HW_FORMAT_YV12 = 0x07,
	MTK_WDMA_HW_FORMAT_BGR565 = 0x08,
	MTK_WDMA_HW_FORMAT_BGR888 = 0x09,
	MTK_WDMA_HW_FORMAT_BGRA8888 = 0x0a,
	MTK_WDMA_HW_FORMAT_ABGR8888 = 0x0b,
	MTK_WDMA_HW_FORMAT_VYUY = 0x0c,
	MTK_WDMA_HW_FORMAT_YVYU = 0x0d,
	MTK_WDMA_HW_FORMAT_YONLY = 0x0e,
	MTK_WDMA_HW_FORMAT_NV12 = 0x0f,
	MTK_WDMA_HW_FORMAT_IYUV = 0x10,
	MTK_WDMA_HW_FORMAT_UNKNOWN = 0x100,
};


struct MTK_WDMA2_HW_PARAM {
	enum MTK_WDMA_HW_FORMAT in_format;
	enum MTK_WDMA_HW_FORMAT out_format;
	uint8_t alpha;
	uint32_t use_specified_alpha;
	uint32_t src_width;
	uint32_t src_height;
	uint32_t clip_x;
	uint32_t clip_y;
	uint32_t clip_width;
	uint32_t clip_height;
	uint64_t addr_plane;

	uint32_t wdma2_irq;
};

int32_t mtk_wdma2_hw_set(unsigned long reg_base, struct MTK_WDMA2_HW_PARAM *pParam);
int32_t mtk_wdma2_hw_unset(unsigned long reg_base);
void mtk_wdma2_hw_irq_clear(unsigned long reg_base);
int mtk_wdma2_wait_irq();
#endif
