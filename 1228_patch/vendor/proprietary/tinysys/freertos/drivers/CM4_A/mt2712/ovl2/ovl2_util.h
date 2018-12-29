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
#ifndef OVL2_UTIL_H
#define OVL2_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <FreeRTOSConfig.h>
#include "driver_api.h"
#include <mt_systmr.h>
#include <portmacro.h>
#include "mt2712.h"
#include "mt_reg_base.h"
#include "video_core.h"


#define OVL_LAYER_COUNT (4U)
#define OVL_INST_COUNT (2U)
#define BUF_QUEUE_SIZE (20U)
#define MTK_OVL2_SUPPORT_PQ (0U)
#define DISP_MUTEX_IDX (9U)
#define WDMA_WAIT_COUNT (500U)
#define OVL2_LOG_DISABLE 1 /* should be 1 when merge code */

#define OVL2_LOG_TAG "[ovl2]"
#if OVL2_LOG_DISABLE
#define OVL2_LOG_E(fmt, ...)
#define OVL2_LOG_D(fmt, ...)
#define OVL2_LOG_IRQ(fmt, ...)
#define OVL2_LOG_IF(fmt, ...)
#if OVL2_SELF_TEST
#define OVL2_LOG_TEST_OK(fmt, ...)
#define OVL2_LOG_TEST_ERR(fmt, ...)
#endif
#else
#define OVL2_LOG_E(fmt, ...) \
		printf("%s [E]"fmt"\n", OVL2_LOG_TAG, ##__VA_ARGS__);
#define OVL2_LOG_D(fmt, ...) \
		printf("%s [D]"fmt"\n", OVL2_LOG_TAG, ##__VA_ARGS__);
#define OVL2_LOG_IRQ(fmt, ...) \
		printf("%s [IRQ]"fmt"\n", OVL2_LOG_TAG, ##__VA_ARGS__);
#define OVL2_LOG_IF(fmt, ...) \
		printf("%s [IF]"fmt"\n", OVL2_LOG_TAG, ##__VA_ARGS__);
#endif
#define OVL2_LOG_FUNC_START OVL2_LOG_IF("%s start", __func__);
#define OVL2_LOG_FUNC_END OVL2_LOG_IF("%s end", __func__);

#define REG_FLD_WIDTH(field) \
	((uint32_t)((((uint32_t)(field)) >> 16) & 0xFFu))
#define REG_FLD_SHIFT(field) \
	((uint32_t)(((uint32_t)(field)) & 0xFFu))
#define REG_FLD_MASK(field) \
	((uint32_t)(((uint32_t) \
	(1u << REG_FLD_WIDTH(((uint32_t)(field)))) - 1u) << \
	REG_FLD_SHIFT(((uint32_t)(field)))))
#define REG_FLD_VAL(field, val) \
	((((uint32_t)(val)) << REG_FLD_SHIFT(((uint32_t)(field)))) & \
	REG_FLD_MASK(((uint32_t)(field))))
#define REG_FLD(width, shift) \
	((uint32_t)(((((uint32_t)(width)) & 0xFFu) << 16) | \
	(((uint32_t)(shift)) & 0xFFu)))

#define DISP_REG_SET(addr, val)  writel((val), (addr))

#define DISP_REG_GET(addr) readl((addr))


#define DISP_REG_SET_FIELD(field, addr, val) \
	(*((uint32_t *)(addr)) |= \
		(((uint32_t)(val)) << \
		REG_FLD_SHIFT(((uint32_t)(field)))) & \
		REG_FLD_MASK(((uint32_t)(field))))

#endif

