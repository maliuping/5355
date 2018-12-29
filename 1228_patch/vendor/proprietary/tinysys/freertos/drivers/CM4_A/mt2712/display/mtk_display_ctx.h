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

#ifndef MTK_DISPLAY_CTX_H
#define MTK_DISPLAY_CTX_H

#include <FreeRTOS.h>
#include <semphr.h>
#include "mtk_display_ddp_comp.h"
#include "mtk_display_ddp.h"
#include "mtk_display_macro.h"
#include "mtk_display_drv.h"
#include "video_core.h"
#include "panel.h"

#define OVL_LAYER0		0
#define OVL_LAYER1		1
#define OVL_LAYER2		2
#define OVL_LAYER3		3
#define OVL_LAYER_NR	4
#define BUF_QUE_SIZE	4

typedef video_buffer display_buffer;

typedef void (*buffer_done_handler)(void *usr_data, video_buffer_type type, video_buffer *buffer);

typedef struct mtk_display_cb_data{
	buffer_done_handler cb;
	void				*data;
}mtk_display_cb_data;

struct mtk_display_ctx;
typedef struct mtk_cmdq_cb_data{
	bool						busy;
	uint32_t					layer_commit_flag;
	struct mtk_display_ctx	 	*display_ctx;
}mtk_cmdq_cb_data;

struct mtk_display_drv;
typedef struct mtk_display_ctx{
	struct mtk_display_drv  *display_drv;
	bool						enabled;
	mtk_cmdq_cb_data			*cmdq_cb_data[BUF_QUE_SIZE];;
	struct cmdq_client			*cmdq_client;
	enum cmdq_event		 		cmdq_event;
	struct cmdq_pkt  			*cmdq_handle[BUF_QUE_SIZE];;
	SemaphoreHandle_t 			que_semaphore;
	uint32_t    				buffer_update_flag; //semaphore
	uint32_t    				buffer_commit_flag; //semaphore
	uint32_t    				buffer_done_flag;   //semaphore
	display_buffer  			*buffer_rvc;
	display_buffer  			*buffer_que[BUF_QUE_SIZE]; //semaphore
	struct display_mode_info	disp_mode;
	mtk_display_layer_config	disp_layer[OVL_LAYER_NR];
	uint32_t                	ddp_comp_nr;
	mtk_display_ddp_comp 		**ddp_comp;
	mtk_display_mutex			*mutex;
	mtk_display_cb_data			*disp_cb_data;
}mtk_display_ctx;

int32_t mtk_display_ctx_create(struct mtk_display_drv *disp_drv, const mtk_display_ddp_comp_id *path, uint32_t path_len);
void mtk_display_ctx_destroy(mtk_display_ctx *ctx);
int32_t mtk_display_ddp_hw_init(mtk_display_ctx* ctx);
void mtk_display_ddp_hw_fini(mtk_display_ctx* ctx);
void mtk_display_set_mode(mtk_display_ctx* ctx, void* mode);
void mtk_display_set_layer_config(mtk_display_ctx* ctx, uint32_t index, void* layer);
void mtk_display_ctx_commit(mtk_display_ctx* ctx);
void mtk_display_ctx_done(mtk_display_ctx *ctx);
bool mtk_check_block_state(mtk_display_ctx *ctx);
void mtk_display_buffer_update(mtk_display_ctx* ctx, uint32_t index, display_buffer *buffer);
void mtk_display_set_layer_rvc_config(mtk_display_ctx *ctx, video_config_type type, void *config);
void mtk_display_ipc_buffer_update(mtk_display_ctx* ctx, uint32_t index, uint32_t addr);
void mtk_display_ipc_config_update(mtk_display_ctx* ctx, uint32_t index, mtk_display_layer_config *config);
void mtk_display_ipc_layer_update(mtk_display_ctx* ctx, mtk_display_ipc_layer_data* data);
#endif  /* MTK_DISPLAY_CTX_H */

