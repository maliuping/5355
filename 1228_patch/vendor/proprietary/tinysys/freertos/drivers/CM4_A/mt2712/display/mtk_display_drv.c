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
#include <FreeRTOSConfig.h>
#include <task.h>
#include "mtk_display_drv.h"
#include "mtk_display_macro.h"
#ifdef CFG_DSP_IPC_SUPPORT
#include "scp_ipi.h"
#include "scp_uid_queue.h"
#endif

#define STACK_SIZE 400  // words
#define DISPLAY_TASK_PRIIO 0
#define USER_INVALID	0xf

static mtk_display_drv display_drv;

/* insert config.db later,  currently hardcode */
static const mtk_display_ddp_comp_id mt2712_mtk_ddp[] = {
	DDP_COMPONENT_OVL0,
	DDP_COMPONENT_COLOR0,
	DDP_COMPONENT_AAL,
	DDP_COMPONENT_OD,
	DDP_COMPONENT_RDMA0,
	DDP_COMPONENT_DPI0,
	DDP_COMPONENT_LVDS0,
	//DDP_COMPONENT_WDMA0,
	DDP_COMPONENT_PWM0,
};

static display_handle device_user_list[USER_INVALID];

static mtk_display_ctx* mtk_get_display_ctx()
{
	return display_drv.disp_ctx;
}

static bool mtk_ctx_check_enable()
{
	return mtk_get_display_ctx()->enabled;
}

static void mtk_ctx_enable()
{
	mtk_get_display_ctx()->enabled = true;
}

static void mtk_ctx_disable()
{
	mtk_get_display_ctx()->enabled = false;
}

static void mtk_display_ctx_reference()
{
	display_drv.refCount++;
}

static void mtk_display_ctx_unreference()
{
	if (display_drv.refCount > 0)
		display_drv.refCount--;
}
static uint32_t mtk_read_refcount()
{
	return display_drv.refCount;
}

static int32_t mtk_display_show(mtk_display_ctx *ctx)
{
	/* commit layer setting to cmdq */
	mtk_display_ctx_commit(ctx);

	return 0;
}

static int32_t mtk_display_frame_done(mtk_display_ctx *ctx)
{
	/* update buf_que, notification to fastrvc or |and  AP */
	mtk_display_ctx_done(ctx);

	return 0;
}

static void mtk_display_task_handle(void *data)
{
	mtk_display_ctx *ctx = (mtk_display_ctx *)data;

	DISP_LOGI("enter\n");

	while(1)
	{
		while(!mtk_check_block_state(ctx))
		{
			mtk_display_frame_done(ctx);

			mtk_display_show(ctx);
		}

		vTaskDelay(pdMS_TO_TICKS(1));
	}
}
static int32_t mtk_display_task_create()
{
	DISP_LOGI("enter\n");

	if(xTaskCreate(mtk_display_task_handle,
					"Display Task",
					STACK_SIZE,
					(void *) display_drv.disp_ctx,
					DISPLAY_TASK_PRIIO,
					&display_drv.task_handle) != pdPASS)
	{
		DISP_LOGE("mtk_display_task_create failed.\n");
		return -pFAIL;
	}

	return pPASS;
}

static void mtk_display_task_destroy()
{
	vTaskDelete(display_drv.task_handle);
}

static int32_t mtk_display_drv_init()
{
	int32_t ret, i;

	DISP_LOGI("enter\n");

	display_drv.refCount = 0;

	for (i = 0; i < USER_INVALID; i++)
		device_user_list[i] = NULL;

	/* create display ctx */
    ret = mtk_display_ctx_create(&display_drv, mt2712_mtk_ddp, ARRAY_SIZE(mt2712_mtk_ddp));
	if (ret < 0)
	{
		DISP_LOGE("display: mtk_display_ctx_create failed.\n");
		return -pFAIL;
	}

	/* create display task */
	ret = mtk_display_task_create();

	return ret;
}

static void mtk_display_drv_deinit()
{
	int32_t i;

	display_drv.refCount = 0;



	for (i = 0; (i < USER_INVALID) && (device_user_list[i] != NULL); i++)
	{
		vPortFree(device_user_list[i]);
		device_user_list[i] = NULL;
	}

    mtk_display_ctx_destroy(mtk_get_display_ctx());

	mtk_display_task_destroy();
}


#ifdef CFG_DSP_IPC_SUPPORT
static void display_ipc_open_handler(void *private, char *data)
{
	display_handle user_handle;
	queue_msg_t ipc_msg;
	mtk_display_ipc_open *open = (mtk_display_ipc_open *)data;
	mtk_display_ipc_open_rsp rsp;

	if (open == NULL)
	{
		DISP_LOGW("display: display_ipc_open_handler ipc arg invalid.\n");
		return;
	}

	user_handle = display_core_open();
	if(user_handle != NULL)
	{
		ipc_msg.msg_id = MTK_DISP_IPC_OPEN_RSP;
		ipc_msg.magic = IPI_MSG_MAGIC_NUMBER;
		rsp.id = user_handle->user;
		memcpy(ipc_msg.data, &rsp, sizeof(rsp));
		scp_uid_queue_send(&ipc_msg);
	}
}

static void display_ipc_close_handler(void *private, char *data)
{
	mtk_display_ipc_close *close = (mtk_display_ipc_close *)data;

	if (close == NULL)
	{
		DISP_LOGW("display: display_ipc_close_handler ipc arg invalid.\n");
		return;
	}

	display_core_close(device_user_list[close->id]);
}


static void display_ipc_commit_handler(void *private, char *data)
{
	mtk_display_ipc_layer_data *layer_data = (mtk_display_ipc_layer_data *)data;
	mtk_display_ctx *disp_ctx = (mtk_display_ctx *)private;

	if ((layer_data == NULL) || (device_user_list[layer_data->user_id] == NULL) ||(disp_ctx == NULL))
	{
		DISP_LOGW("display: ipc arg invalid.\n");
		return;
	}

	mtk_display_ipc_layer_update(disp_ctx, layer_data);

	return;
}
#endif

int32_t display_core_init()
{
	int32_t ret;

	DISP_LOGI("enter\n");

	/* init display drv */
	ret = mtk_display_drv_init();

#ifdef CFG_DSP_IPC_SUPPORT
	scp_uid_queue_register(MTK_DISP_IPC_LAYER_DATA_COMMIT, display_ipc_commit_handler, (void *)mtk_get_display_ctx());
	scp_uid_queue_register(MTK_DISP_IPC_OPEN, display_ipc_open_handler, NULL);
	scp_uid_queue_register(MTK_DISP_IPC_CLOSE, display_ipc_close_handler, NULL);
#endif

	DISP_LOGD("done\n");

	return ret;
}

void display_core_exit()
{
	DISP_LOGI("enter\n");

	mtk_display_drv_deinit();
}

static int32_t mtk_user_index_generate(uint8_t *index)
{
	int32_t i = 0;
	while(i < USER_INVALID)
	{
		if (device_user_list[i] == NULL)
		{
			*index = i;
			break;
		}

		i++;
	}

	if(i == USER_INVALID)
	{
		DISP_LOGE("display: mtk_user_index_generate failed\n");
		return -pFAIL;
	}

	return pPASS;
}

static uint32_t mtk_dispaly_get_layer_index(video_buffer_type type)
{
	int32_t index = OVL_LAYER3;

	switch(type)
	{
		case OVL_BUFFER_INPUT_0:
			index = OVL_LAYER0;
			break;
		case OVL_BUFFER_INPUT_1:
			index = OVL_LAYER1;
			break;
		case OVL_BUFFER_INPUT_2:
			index = OVL_LAYER2;
			break;
		case OVL_BUFFER_INPUT_3:
			index = OVL_LAYER3;
			break;
		default:
			DISP_LOGW("display: type not supported.\n");
			break;
	}

	return index;
}

video_handle display_core_open()
{
	int32_t ret;
	uint8_t id = USER_INVALID;
	display_handle user_handle;

	if(mtk_user_index_generate(&id) != pPASS)
		return NULL;

	DISP_LOGI("user_id: %u\n", id);

	/* allocate new user handle */
	user_handle = (display_handle)pvPortMalloc(sizeof(display_user_handle));
	if (NULL == user_handle)
	{
		DISP_LOGE("user_handle malloc failed.\n");
		return NULL;
	}

	user_handle->disp_ctx = mtk_get_display_ctx();
	user_handle->user = id;
	device_user_list[id] = user_handle;

    /* enable ddp hw for first user */
	if (!mtk_ctx_check_enable())
	{
		ret = mtk_display_ddp_hw_init(user_handle->disp_ctx);
		if (ret < 0)
		{
			DISP_LOGE("mtk_display_ddp_hw_init failed: %d\n", (int)ret);
			vPortFree(user_handle);
			device_user_list[id] = NULL;
			return NULL;
		}
		mtk_ctx_enable();
	}

	mtk_display_ctx_reference();
	return (video_handle)user_handle;
}

int32_t display_core_close(video_handle handle)
{
	DISP_LOGI("display: display_core_close.\n");

	display_handle user_handle = (display_handle)handle;
	if(user_handle == NULL || user_handle != device_user_list[user_handle->user])
	{
		DISP_LOGE("display: user handle invalid\n");
		return -pINVA;
	}

	mtk_display_ctx_unreference();
	if (0 == mtk_read_refcount())
	{
		mtk_display_ddp_hw_fini(user_handle->disp_ctx);
		mtk_ctx_disable();
	}

	device_user_list[user_handle->user] = NULL;
	vPortFree(user_handle);

	return pPASS;
}

int32_t display_core_config(video_handle handle, video_config_type type, void *config)
{
	display_handle user_handle = (display_handle)handle;

	if(user_handle == NULL || user_handle != device_user_list[user_handle->user] || config == NULL)
	{
		DISP_LOGE("display: user handle invalid\n");
		return -pINVA;
	}

	DISP_LOGD("userId:%u, type:%u\n", user_handle->user, type);

	if (type > VIDEO_CONFIG_AREA)
	{
		DISP_LOGW("display: invalid config type.\n");
		return -pINVA;
	}

	mtk_display_set_layer_rvc_config(user_handle->disp_ctx, type, config);

	return pPASS;
}

int32_t display_core_commit(video_handle handle, video_buffer_type type, video_buffer *buffer)
{
	display_handle user_handle = (display_handle)handle;

	if(user_handle == NULL || user_handle != device_user_list[user_handle->user] || buffer == NULL)
	{
		DISP_LOGE("display: user handle invalid\n");
		return -pINVA;
	}

	DISP_LOGD("userId:%u, type:%u\n", user_handle->user, type);

	if (type < OVL_BUFFER_INPUT_0 || type > OVL_BUFFER_INPUT_3)
	{
		DISP_LOGW("display: invalid buffer type.\n");
		return -pINVA;
	}

	mtk_display_buffer_update(user_handle->disp_ctx, mtk_dispaly_get_layer_index(type), buffer);

	return pPASS;
}

int32_t display_core_register_handler(void *handle,
					void (*buffer_done_handler)(void *usr_data, video_buffer_type type, video_buffer *buffer),
					void *usr_data)
{
	display_handle user_handle = (display_handle)handle;

	if(user_handle == NULL || user_handle != device_user_list[user_handle->user])
	{
		DISP_LOGE("display: user handle invalid\n");
		return -pINVA;
	}

	mtk_display_ctx *disp_ctx = user_handle->disp_ctx;

	disp_ctx->disp_cb_data = (mtk_display_cb_data *)pvPortMalloc(sizeof(mtk_display_cb_data));
	if(disp_ctx->disp_cb_data == NULL)
	{
		DISP_LOGE("display: disp_cb_data malloc failed.\n");
		return -pNOMEM;
	}

	disp_ctx->disp_cb_data->cb = buffer_done_handler;
	disp_ctx->disp_cb_data->data = usr_data;

	return pPASS;
}
