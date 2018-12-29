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

#include <FreeRTOS.h>
#include <mt_reg_base.h>
#include "mtk_display_drv.h"
#include "mtk_display_ddp_comp.h"
#include "mtk_display_macro.h"

#ifdef CFG_DSP_IPC_SUPPORT
#include "scp_ipi.h"
#include "scp_uid_queue.h"
#endif

#define CM4_DISP_GCE_OVL0_REG_BASE			(0x1400c000)
#define CM4_DISP_GCE_COLOR0_REG_BASE		(0x14013000)
#define CM4_DISP_GCE_AAL_REG_BASE			(0x14015000)
#define CM4_DISP_GCE_OD_REG_BASE			(0x14023000)
#define CM4_DISP_GCE_RDMA0_REG_BASE			(0x1400e000)
#define CM4_DISP_GCE_WDMA0_REG_BASE			(0x14011000)
#define CM4_DISP_GCE_PWM0_REG_BASE			(0x1401e000)
#define CM4_DISP_GCE_MTTEX_REG_BASE			(0x14020000)

static const uint32_t mt2712_comp_reg_base[DDP_COMPONENT_ID_MAX] = {
	[DDP_COMPONENT_OVL0] 	= CM4_DISP_GCE_OVL0_REG_BASE,
	[DDP_COMPONENT_COLOR0]	= CM4_DISP_GCE_COLOR0_REG_BASE,
	[DDP_COMPONENT_AAL]		= CM4_DISP_GCE_AAL_REG_BASE,
	[DDP_COMPONENT_OD]		= CM4_DISP_GCE_OD_REG_BASE,
	[DDP_COMPONENT_RDMA0] 	= CM4_DISP_GCE_RDMA0_REG_BASE,
	[DDP_COMPONENT_WDMA0] 	= CM4_DISP_GCE_WDMA0_REG_BASE,
	[DDP_COMPONENT_PWM0] 	= CM4_DISP_GCE_PWM0_REG_BASE,
	[DDP_COMPONENT_MUTEX] 	= CM4_DISP_GCE_MTTEX_REG_BASE,
};

extern uint32_t mtk_bpp(uint32_t fmt);

static int32_t mtk_semaphore_create(mtk_display_ctx *ctx)
{
	ctx->que_semaphore = xSemaphoreCreateBinary();
	if (ctx->que_semaphore == NULL)
	{
		DISP_LOGE("mtk_semaphore_create failed.\n");
		return -pNOMEM;
	}

	xSemaphoreGive(ctx->que_semaphore);
	return 0;
}

static void mtk_semaphore_destroy(mtk_display_ctx *ctx)
{
	vSemaphoreDelete(ctx->que_semaphore);
}

static int32_t mtk_semaphore_take(mtk_display_ctx *ctx, bool ISR)
{
	if(ctx->que_semaphore == NULL)
	{
		DISP_LOGE("que_semaphore null.\n");
		return -pINVA;
	}

	if(ISR)
	{
		if(xSemaphoreTakeFromISR(ctx->que_semaphore, NULL) != pdPASS)
	 	{
			DISP_LOGE("SemaphoreTake failed.\n");
			return -pBUSY;
	 	}
	}
	else
	{
		if(xSemaphoreTake(ctx->que_semaphore, portMAX_DELAY) != pdPASS)
	 	{
			DISP_LOGE("SemaphoreTake failed.\n");
			return -pBUSY;
	 	}
	}

	return 0;
}

static int32_t mtk_semaphore_give(mtk_display_ctx *ctx, bool ISR)
{
	//BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if(ctx->que_semaphore == NULL)
	{
		DISP_LOGE("que_semaphore null.\n");
		return -pINVA;
	}

	if(ISR)
	{
		if(xSemaphoreGiveFromISR(ctx->que_semaphore, NULL) != pdPASS)
	 	{
			DISP_LOGE("SemaphoreGive failed.\n");
			return -pBUSY;
	 	}
	}
	else
	{
	 	if(xSemaphoreGive(ctx->que_semaphore) != pdPASS)
	 	{
			DISP_LOGE("SemaphoreGive failed.\n");
			return -pBUSY;
	 	}
	}
	return pPASS;
}

static int32_t mtk_buffer_que_init(mtk_display_ctx *disp_ctx)
{
	uint32_t i;
	for (i = 0; i < BUF_QUE_SIZE; i++)
	{
		disp_ctx->buffer_que[i] = (display_buffer *)pvPortMalloc(sizeof(display_buffer));
		if (NULL == disp_ctx->buffer_que[i])
		{
			DISP_LOGE("buffer_que malloc failed.\n");
			return -pNOMEM;
		}

	    disp_ctx->buffer_que[i]->image = NULL;
		disp_ctx->buffer_que[i]->latency = 0;
		disp_ctx->buffer_que[i]->timestamp = 0;
	}

	return pPASS;
}

static void mtk_layer_config_init(mtk_display_ctx *disp_ctx)
{
	uint32_t i;
	for (i = 0; i < OVL_LAYER_NR; i++)
	{
		disp_ctx->disp_layer[i].enable = false;
		disp_ctx->disp_layer[i].alpha = 0xff;
		disp_ctx->disp_layer[i].color_matrix = 0;
	}
}

static void mtk_buffer_que_deinit(mtk_display_ctx *disp_ctx)
{
	uint32_t i;
	for (i = 0; i < BUF_QUE_SIZE; i++)
	{
		if (disp_ctx->buffer_que[i] != NULL)
			vPortFree(disp_ctx->buffer_que[i]);
	}
}

static int32_t mtk_display_enable_vblank(mtk_display_ctx *ctx)
{
	mtk_display_ddp_comp *comp = ctx->ddp_comp[0];

	mtk_display_ddp_comp_enable_vblank(comp, NULL);

	return pPASS;
}

static void mtk_display_disable_vblank(mtk_display_ctx *ctx)
{
	mtk_display_ddp_comp *comp = ctx->ddp_comp[0];

	mtk_display_ddp_comp_disable_vblank(comp, NULL);
}


static int32_t mtk_display_cmdq_client_init(mtk_display_ctx *ctx)
{
	uint32_t i;

	ctx->cmdq_client = cmdq_msg_create(GCE_CM4_DRM, false);
	ctx->cmdq_event = CMDQ_EVENT_MUTEX0_STREAM_EOF;

	for (i = 0; i < BUF_QUE_SIZE; i++)
	{
		cmdq_pkt_create(&(ctx->cmdq_handle[i]));
		ctx->cmdq_cb_data[i] = (mtk_cmdq_cb_data *)pvPortMalloc(sizeof(mtk_cmdq_cb_data));
		if (NULL == ctx->cmdq_cb_data[i])
		{
			DISP_LOGE("cmdq_cb_data malloc failed.\n");
			return -pNOMEM;
		}

	    ctx->cmdq_cb_data[i]->display_ctx = ctx;
		ctx->cmdq_cb_data[i]->layer_commit_flag = 0;
		ctx->cmdq_cb_data[i]->busy = false;
	}

	DISP_LOGI("cmdq enabled\n");
	return pPASS;

}

static void mtk_display_cmdq_client_deinit(mtk_display_ctx *ctx)
{
	uint32_t i;
	for (i = 0; i < BUF_QUE_SIZE; i++)
	{
		if (ctx->cmdq_cb_data[i] != NULL)
		{
			vPortFree(ctx->cmdq_cb_data[i]);
			ctx->cmdq_cb_data[i] = NULL;
			cmdq_pkt_destroy(ctx->cmdq_handle[i]);
		}
	}

	DISP_LOGI("cmdq disabled\n");
}


int32_t mtk_display_ctx_create(mtk_display_drv *disp_drv, const mtk_display_ddp_comp_id *path, uint32_t path_len)
{
	mtk_display_ctx *disp_ctx;
	int32_t ret = pPASS;
	int32_t i;

	disp_ctx = (mtk_display_ctx *)pvPortMalloc(sizeof(mtk_display_ctx));
	if (NULL == disp_ctx)
	{
		DISP_LOGE("ctx malloc failed.\n");
		return -pNOMEM;
	}

	if(mtk_buffer_que_init(disp_ctx) != pPASS)
		goto err_que;

	disp_ctx->buffer_rvc = NULL;
	disp_ctx->buffer_update_flag = 0;
	disp_ctx->buffer_commit_flag = 0;
	disp_ctx->buffer_done_flag = 0;

	mtk_layer_config_init(disp_ctx);

	ret = mtk_semaphore_create(disp_ctx);
	if (ret < 0)
	{
		goto err_sem;
	}

	disp_ctx->ddp_comp_nr = path_len;
	disp_ctx->ddp_comp = (mtk_display_ddp_comp **)pvPortMalloc(path_len * sizeof(mtk_display_ddp_comp*));
	if (NULL == disp_ctx->ddp_comp) {
		DISP_LOGE("ddp_comp malloc failed.\n");
		ret = -pNOMEM;
		goto err_ddp_comp;
	}

	disp_ctx->mutex = (mtk_display_mutex *)pvPortMalloc(sizeof(mtk_display_mutex));
	if (NULL == disp_ctx->mutex) {
		DISP_LOGE("mutex malloc failed.\n");
		ret = -pNOMEM;
		goto err_mutex;
	}

	disp_ctx->mutex->id = 0;
	mtk_display_mutex_get(disp_ctx->mutex);

	lcm_if_getmode(&disp_ctx->disp_mode);

	if(mtk_display_cmdq_client_init(disp_ctx) != pPASS)
		goto err_cmdq;

	for (i = 0; i < disp_ctx->ddp_comp_nr; i++){
		mtk_display_ddp_comp_id comp_id = path[i];
		mtk_display_ddp_comp *comp = (mtk_display_ddp_comp *)pvPortMalloc(sizeof(mtk_display_ddp_comp));
        if (NULL == comp){
			DISP_LOGE("comp malloc failed.\n");
            ret = -pNOMEM;
			goto err_comp;
		}

        mtk_display_ddp_comp_init(comp, comp_id);
        comp->cmdq_base = cmdq_register_device(mt2712_comp_reg_base[comp_id]);
		disp_ctx->ddp_comp[i] = comp;
    }

	disp_drv->disp_ctx = disp_ctx;
	disp_ctx->display_drv = disp_drv;

	return pPASS;

err_comp:
	while (--i >= 0)
	{
		vPortFree(disp_ctx->ddp_comp[i]);
		disp_ctx->ddp_comp[i] = NULL;
	}
	vPortFree(disp_ctx->cmdq_cb_data);
err_cmdq:
	mtk_display_cmdq_client_deinit(disp_ctx);
	vPortFree(disp_ctx->mutex);
err_mutex:
	vPortFree(disp_ctx->ddp_comp);
err_ddp_comp:
	mtk_semaphore_destroy(disp_ctx);
err_sem:
err_que:
	mtk_buffer_que_deinit(disp_ctx);
	vPortFree(disp_ctx);

	return ret;
}

static void mtk_display_ddp_comp_deinit(mtk_display_ctx* disp_ctx)
{
	uint32_t i;
	for (i = 0; i < disp_ctx->ddp_comp_nr; i++) {
		if (disp_ctx->ddp_comp[i] != NULL) {
			vPortFree(disp_ctx->ddp_comp[i]);
		}
	}

	vPortFree(disp_ctx->ddp_comp);
}

void mtk_display_ctx_destroy(mtk_display_ctx* disp_ctx)
{
	mtk_buffer_que_deinit(disp_ctx);
	mtk_semaphore_destroy(disp_ctx);
    vPortFree(disp_ctx->mutex);
	mtk_display_cmdq_client_deinit(disp_ctx);
	mtk_display_ddp_comp_deinit(disp_ctx);
	vPortFree(disp_ctx);
}

int32_t mtk_display_ddp_hw_init(mtk_display_ctx* ctx)
{
	uint32_t i;
	uint32_t width, height, vrefresh, bpc = MTK_MAX_BPC;
	struct cmdq_pkt *cmdq_handle;

	DISP_LOGI("enter\n");

	if(ctx == NULL)
	{
		DISP_LOGE("ctx empty!\n");
		return -pINVA;
	}

	width = ctx->disp_mode.hdisplay;
	height = ctx->disp_mode.vdisplay;
	vrefresh = ctx->disp_mode.vrefresh;
	bpc = ctx->disp_mode.bpc;
	DISP_LOGD("debug mode info:%u,%u,%u,%u\n", width, height, vrefresh, bpc);

	DISP_LOGI("mtk_display_ddp_path_setup\n");

	for (i = 0; i < ctx->ddp_comp_nr - 1; i++) {
		mtk_display_ddp_add_comp_to_path(CM4_MMSYS_CONFIG_REGS_BASE,
					 ctx->ddp_comp[i]->id,
					 ctx->ddp_comp[i + 1UL]->id);
		mtk_display_mutex_add_comp(ctx->mutex,
					ctx->ddp_comp[i]->id);
	}

	mtk_display_mutex_add_comp(ctx->mutex, ctx->ddp_comp[i]->id);
	mtk_display_mutex_enable(ctx->mutex);

	/* only enable vblank once */
	mtk_display_enable_vblank(ctx);

	(void)cmdq_pkt_create(&cmdq_handle);

	for (i = 0; i < ctx->ddp_comp_nr; i++) {
		mtk_display_ddp_comp *comp = ctx->ddp_comp[i];

		mtk_display_ddp_comp_config(comp, width, height, vrefresh, bpc,
					cmdq_handle);

		mtk_display_ddp_comp_start(comp, cmdq_handle);
	}

	/* Initially configure all planes */
	for (i = 0; i < OVL_LAYER_NR; i++) {
		ctx->disp_layer[i].enable = false;

		mtk_display_ddp_comp_layer_config(ctx->ddp_comp[0], i,
					  &(ctx->disp_layer[i]), cmdq_handle);
	}

	(void)cmdq_pkt_flush(ctx->cmdq_client,  cmdq_handle);
	cmdq_pkt_destroy(cmdq_handle);

	lcm_if_enable(&(ctx->disp_mode));
	lcm_if_set_backlight(0);

	DISP_LOGI("%s done\n", __func__);

	return pPASS;
}

void mtk_display_ddp_hw_fini(mtk_display_ctx* ctx)
{
	struct cmdq_pkt *cmdq_handle;
	int i;

	DISP_LOGI("%s\n", __func__);

	if(ctx == NULL)
	{
		DISP_LOGE("ctx empty!\n");
	}

	(void)cmdq_pkt_create(&cmdq_handle);
	(void)cmdq_pkt_clear_event(cmdq_handle, ctx->cmdq_event);
	(void)cmdq_pkt_wfe(cmdq_handle, ctx->cmdq_event);

	/* Initially configure all planes */
	for (i = 0; i < OVL_LAYER_NR; i++) {
		ctx->disp_layer[i].enable = false;
		ctx->disp_layer[i].alpha = 0xff;
		ctx->disp_layer[i].color_matrix = 0;

		mtk_display_ddp_comp_layer_config(ctx->ddp_comp[0], i,
					  &(ctx->disp_layer[i]), cmdq_handle);
	}

	for (i = 0; i < ctx->ddp_comp_nr; i++){
		mtk_display_ddp_comp_stop(ctx->ddp_comp[i],  cmdq_handle);
	}

	(void)cmdq_pkt_flush(ctx->cmdq_client, cmdq_handle);
	cmdq_pkt_destroy(cmdq_handle);

	lcm_if_disable(&(ctx->disp_mode));

	for (i = 0; i < ctx->ddp_comp_nr; i++)
		mtk_display_mutex_remove_comp(ctx->mutex,
					   ctx->ddp_comp[i]->id);

	mtk_display_mutex_disable(ctx->mutex);
	for (i = 0; i < ctx->ddp_comp_nr - 1; i++) {
		mtk_display_ddp_remove_comp_from_path(CM4_MMSYS_CONFIG_REGS_BASE,
					      ctx->ddp_comp[i]->id,
					      ctx->ddp_comp[i + 1]->id);
		mtk_display_mutex_remove_comp(ctx->mutex,
					   ctx->ddp_comp[i]->id);
	}

	mtk_display_mutex_remove_comp(ctx->mutex, ctx->ddp_comp[i]->id);

	mtk_display_disable_vblank(ctx);
}


void mtk_display_set_layer_rvc_config(mtk_display_ctx *ctx, video_config_type type, void *config)
{
	if (type == VIDEO_CONFIG_FORMAT)
	{
		video_format *display_format = (video_format *)config;
		ctx->disp_layer[OVL_LAYER3].format  = display_format->fourcc;
		uint32_t bpp = mtk_bpp(display_format->fourcc);
		ctx->disp_layer[OVL_LAYER3].pitch = DIV_ROUND_UP(bpp*(display_format->width),8);
	}
	else if (type == VIDEO_CONFIG_AREA)
	{
		video_rect *display_rect = (video_rect *)config;
		ctx->disp_layer[OVL_LAYER3].x = display_rect->left;
		ctx->disp_layer[OVL_LAYER3].y = display_rect->top;
		ctx->disp_layer[OVL_LAYER3].height = display_rect->height;
		ctx->disp_layer[OVL_LAYER3].width = display_rect->width;
	}
}

static void mtk_display_buffer_enqueue(mtk_display_ctx* ctx, uint32_t index, display_buffer *buffer)
{
	DISP_LOGD("cloud debug: image:%u, index:%u\n", buffer->image, index);
	ctx->buffer_que[index]->image = buffer->image;
	ctx->buffer_que[index]->timestamp = buffer->timestamp;
	ctx->buffer_que[index]->latency= buffer->latency;
}

static void mtk_display_buffer_dequeue(mtk_display_ctx* ctx, uint32_t index)
{
	ctx->buffer_que[index]->image = NULL;
	ctx->buffer_que[index]->timestamp = 0;
	ctx->buffer_que[index]->latency= 0;
}

static void mtk_set_layer_update_state(mtk_display_ctx* ctx, uint32_t index)
{
	ctx->buffer_update_flag |= BIT(index);
}

void mtk_display_buffer_update(mtk_display_ctx* ctx, uint32_t index, display_buffer *buffer)
{
	if (index == OVL_LAYER3)
		ctx->buffer_rvc = (buffer->image != NULL) ? buffer : NULL;

	mtk_semaphore_take(ctx, false);
	if(((ctx->buffer_update_flag) & BIT(index)) > 0)
	{
		mtk_semaphore_give(ctx, false);
		DISP_LOGE("%s layer buffer conflict index: %u.\n", __func__, index);
		return;
	}

	mtk_display_buffer_enqueue(ctx, index, buffer);
	mtk_set_layer_update_state(ctx, index);
	mtk_semaphore_give(ctx, false);
}

#ifdef CFG_DSP_IPC_SUPPORT
static void mtk_display_ipc_buffer_enqueue(mtk_display_ctx* ctx, uint32_t index, uint32_t addr)
{
	ctx->buffer_que[index]->image = (void *)addr;
}

void mtk_display_ipc_config_update(mtk_display_ctx* ctx, uint32_t index, mtk_display_layer_config *config)
{
	//mtk_semaphore_take(ctx, true);
	if(((ctx->buffer_update_flag) & BIT(index)) > 0)
	{
		//mtk_semaphore_give(ctx, true);
		DISP_LOGE("layer config conflict.\n");
		return;
	}

	ctx->disp_layer[index]= *config;
	//mtk_semaphore_give(ctx, true);
}

void mtk_display_ipc_buffer_update(mtk_display_ctx* ctx, uint32_t index, uint32_t addr)
{
	//mtk_semaphore_take(ctx, true);
	if(((ctx->buffer_update_flag) & BIT(index)) > 0)
	{
		//mtk_semaphore_give(ctx, true);
		DISP_LOGE("%s layer buffer conflict index: %u.\n", __func__, index);
		return;
	}
	mtk_display_ipc_buffer_enqueue(ctx, index, addr);
	mtk_set_layer_update_state(ctx, index);
	//mtk_semaphore_give(ctx, true);
}

void mtk_display_ipc_layer_update(mtk_display_ctx* ctx,mtk_display_ipc_layer_data *data)
{
	int32_t i;
	for (i = 0; i < IPC_LAYER_NR; i++)
	{
		if (((data->layer_mask) & BIT(i)) > 0)
		{
			mtk_display_ipc_config_update(ctx, i, &(data->config[i]));
			mtk_display_ipc_buffer_update(ctx, i, data->config[i].addr);
		}
	}
}
#endif

static bool mtk_check_layer_update_state(mtk_display_ctx* ctx, uint32_t index)
{
	if(((ctx->buffer_update_flag) & BIT(index)) == 0)
		return false;

	return true;
}

static bool mtk_check_layer_commit_state(mtk_display_ctx* ctx, uint32_t index)
{
	if (index == OVL_LAYER_NR)
	{
		if(ctx->buffer_commit_flag > 0)
			return true;
		return false;
	}

	if(((ctx->buffer_commit_flag) & BIT(index)) == 0)
		return false;

	return true;
}

static void mtk_reset_layer_commit_state(mtk_display_ctx* ctx, uint32_t index)
{
	uint32_t mark = FULLBIT ^ (BIT(index));
	ctx->buffer_commit_flag &= mark;
}

static void mtk_set_layer_commit_state(mtk_display_ctx* ctx, uint32_t index)
{
	ctx->buffer_commit_flag |= BIT(index);
}

static void mtk_reset_layer_update_state(mtk_display_ctx* ctx, uint32_t index)
{
	uint32_t mark = FULLBIT ^ (BIT(index));
	ctx->buffer_update_flag &= mark;
}

static void mtk_reset_layer_done_state(mtk_display_ctx* ctx, uint32_t index)
{
	uint32_t mark = FULLBIT ^ (BIT(index));
	ctx->buffer_done_flag &= mark;
}

static bool mtk_check_layer_done_state(mtk_display_ctx* ctx, uint32_t index)
{
	if(((ctx->buffer_done_flag) & BIT(index)) == 0)
		return false;

	return true;
}

static void mtk_update_layer_done_state(mtk_display_ctx* ctx, uint32_t flag)
{
	ctx->buffer_done_flag |= flag;
}

static void mtk_display_layer_update(mtk_display_ctx* ctx,
							uint32_t layer_index,
							void *handle)
{
	mtk_display_ddp_comp *comp = ctx->ddp_comp[0];
	ctx->disp_layer[layer_index].addr = (uint32_t)(ctx->buffer_que[layer_index]->image);

	ctx->disp_layer[layer_index].enable = true;
	if (ctx->disp_layer[layer_index].addr == 0)
		ctx->disp_layer[layer_index].enable = false;

	mtk_display_ddp_comp_layer_config(comp, layer_index, &ctx->disp_layer[layer_index],
				 	handle);
}

static void mtk_display_cmdq_cb(struct cmdq_cb_data data)
{
	mtk_cmdq_cb_data *cb_data = (mtk_cmdq_cb_data *)data.data;
	mtk_display_ctx *ctx = cb_data->display_ctx;
	DISP_LOGD("cloud debug flag: %u\n",cb_data->layer_commit_flag);

	//mtk_semaphore_take(ctx, true);
	mtk_update_layer_done_state(ctx, cb_data->layer_commit_flag);
	//mtk_semaphore_give(ctx, true);

	cb_data->busy = false;
	cb_data->layer_commit_flag = 0;
}

static int mtk_get_free_cmdq_handle(mtk_display_ctx* ctx)
{
	int32_t i;
	for (i = 0; i < BUF_QUE_SIZE; i++)
	{
		if ((ctx->cmdq_handle[i] != NULL) && (ctx->cmdq_cb_data[i] != NULL)
			&& !(ctx->cmdq_cb_data[i]->busy))
		{
			return i;
		}
	}

	return -1;
}

void mtk_display_ctx_commit(mtk_display_ctx* ctx)
{
	uint32_t i;
	mtk_cmdq_cb_data *cb_data;
	uint32_t commit_flag = 0;
	int32_t index = -1;

	index = mtk_get_free_cmdq_handle(ctx);
	if (index < 0)
	{
		DISP_LOGE("cmdq_chandle que no available.\n");
		return;
	}

	cb_data = ctx->cmdq_cb_data[index];
	ctx->cmdq_handle[index]->cmd_buf_size = 0;
	(void)cmdq_pkt_clear_event(ctx->cmdq_handle[index], ctx->cmdq_event);
	(void)cmdq_pkt_wfe(ctx->cmdq_handle[index], ctx->cmdq_event);

	mtk_semaphore_take(ctx, false);
	if (ctx->buffer_update_flag <= ctx->buffer_commit_flag)
	{
		mtk_semaphore_give(ctx, false);
		return;
	}

	for (i = 0; i < BUF_QUE_SIZE; i++)
	{
		//DISP_LOGD("cloud debug: image: %u, update state: %u.\n", ctx->buffer_que[i]->image, mtk_check_layer_update_state(ctx, i));
		if(mtk_check_layer_update_state(ctx, i) && !mtk_check_layer_commit_state(ctx, i))
		{
			mtk_display_layer_update(ctx, i, ctx->cmdq_handle[index]);
			mtk_set_layer_commit_state(ctx, i);
			commit_flag |= BIT(i);
		}
	}
	mtk_semaphore_give(ctx, false);

	if (commit_flag == 0)
	{
		DISP_LOGE("no commit show.\n");
		return;
	}

	cb_data->layer_commit_flag = commit_flag;
	cb_data->busy = true;

    (void)cmdq_pkt_flush_async(ctx->cmdq_client, ctx->cmdq_handle[index],
			mtk_display_cmdq_cb, cb_data);
}

void mtk_display_ctx_done(mtk_display_ctx *ctx)
{
	uint32_t i;
	bool ap_path = false;
	bool cm4_path = false;
#ifdef CFG_DSP_IPC_SUPPORT
	queue_msg_t ipc_msg;
#endif

	mtk_semaphore_take(ctx, false);
	if (ctx->buffer_done_flag == 0)
	{
		mtk_semaphore_give(ctx, false);
		return;
	}

	DISP_LOGD("buffer_done_flag: %x \n", ctx->buffer_done_flag);

	for (i = 0; i < BUF_QUE_SIZE; i++)
	{
		if(mtk_check_layer_commit_state(ctx, i) && mtk_check_layer_done_state(ctx, i))
		{
			if (i < OVL_LAYER3)
			{
				ap_path = true;
			}
			else
			{
				cm4_path = true;
			}

			mtk_display_buffer_dequeue(ctx, i);
			mtk_reset_layer_update_state(ctx, i);
			mtk_reset_layer_commit_state(ctx, i);
			mtk_reset_layer_done_state(ctx, i);
		}
	}
	mtk_semaphore_give(ctx, false);

	if(cm4_path)
	{
		DISP_LOGD("cloud debug: cm4_path buffer_rvc: %p\n", ctx->buffer_rvc);
		if((ctx->disp_cb_data) && (ctx->disp_cb_data->data))
			ctx->disp_cb_data->cb(ctx->disp_cb_data->data, OVL_BUFFER_INPUT_3, ctx->buffer_rvc);
		else
			DISP_LOGW("disp_cb_data empty!\n");
	}

	if(ap_path)
	{
#ifdef CFG_DSP_IPC_SUPPORT
		DISP_LOGD("cloud debug: AP_path\n");

		ipc_msg.msg_id = MTK_DISP_IPC_PAGE_FLIP_DONE;
		ipc_msg.magic = IPI_MSG_MAGIC_NUMBER;
		scp_uid_queue_send(&ipc_msg);

#endif
	}
}

bool mtk_check_block_state(mtk_display_ctx *ctx)
{
	if(!ctx->enabled)
	{
		return true;
	}

	mtk_semaphore_take(ctx, false);
	if(ctx->buffer_done_flag > 0)
	{
		mtk_semaphore_give(ctx, false);
		return false;
	}

	mtk_semaphore_give(ctx, false);

	return false;
}

