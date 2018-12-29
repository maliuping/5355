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

#ifndef MTK_DISPLAY_MACRO_H
#define MTK_DISPLAY_MACRO_H

#include <FreeRTOS.h>
#include <mt_printf.h>
#include <bit_op.h>
#include <string.h>
#include <mtk-cmdq.h>
#include <driver_api.h>


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef bool
#define bool uint8_t
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define FULLBIT			0xffff

typedef int32_t DispType_t;
#define pPASS			( ( DispType_t ) 0 )
#define pFAIL			( ( DispType_t ) 1 )
#define pNOMEM			( ( DispType_t ) 2 )
#define pINVA			( ( DispType_t ) 3 )
#define pBUSY			( ( DispType_t ) 4 )

//#define DISP_DEBUG_LOG
#define DISP_LOG(fmt, args...)		PRINTF_E("[DISP][%s:%d]" fmt, __func__, __LINE__, ##args)

#ifdef DISP_DEBUG_LOG
#define DISP_LOGD(fmt, args...)		DISP_LOG(fmt, ##args)
#define DISP_LOGI(fmt, args...)		DISP_LOG(fmt, ##args)
#else
#define DISP_LOGD(fmt, args...)
#define DISP_LOGI(fmt, args...)
#endif

#define DISP_LOGW(fmt, args...)		DISP_LOG(fmt, ##args)
#define DISP_LOGE(fmt, args...)		DISP_LOG(fmt, ##args)

#define DIV_ROUND_UP(x,y) (((x) + ((y) - 1)) / (y))
#define IPC_LAYER_NR 	3UL


/* open for user */
typedef struct mtk_display_layer_config{
	uint8_t				enable;
	uint32_t			addr;
	uint32_t			pitch;
	uint32_t			format;
	uint32_t			x;
	uint32_t			y;
	uint32_t			width;
	uint32_t			height;
	uint32_t			color_matrix;
	uint32_t			alpha;
}mtk_display_layer_config;

/* AP-IPC-CM4 */

typedef struct mtk_display_ipc_open{
}mtk_display_ipc_open;

typedef struct mtk_display_ipc_close{
	uint8_t id;
}mtk_display_ipc_close;

typedef struct mtk_display_ipc_open_rsp{
	uint8_t id;
}mtk_display_ipc_open_rsp;

typedef struct mtk_display_ipc_layer_data{
	uint8_t	user_id;
	uint8_t layer_mask;
	mtk_display_layer_config config[IPC_LAYER_NR];
}mtk_display_ipc_layer_data;



#endif  /* MTK_DISPLAY_MACRO_H */


