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

#include "interrupt.h"
#include "event_groups.h"
#include "ovl2.h"
#include "wdma2_hw.h"
#include "ovl2_hw.h"

#define DISP_OVL1_MOUT_EN        0x44U
#define DISP_WDMA1_SEL_IN        0x9CU

#define DISP_MUTEX_GAP       0x20U
#define DISP_MUTEX_EN        0x20U
#define DISP_MUTEX_MOD       0x2CU
#define DISP_MUTEX_SOF       0x30U
#define DISP_MUTEX_MOD2      0x34U

#define OVL2_EV_CLOSE_STEP_1 (1 << 0)
#define OVL2_EV_CLOSE_STEP_2 (1 << 1)

enum OVL2_THREAD_ST {
	THR_NONE = 0,
	THR_IDLE,
	THR_WAITING,
	THR_RELEASE,
	THR_MAX
};

struct ovl_reg {
	unsigned long ovl_base;
	unsigned long color_base;
	unsigned long gamma_base;
	unsigned long wdma_base;
	unsigned long mmsys_base;
	unsigned long mutex_base;
};

struct ovl_layer {
	uint32_t enable;
	video_format ovl_fmt;
	video_rect ovl_crop;
	video_rect ovl_area;
	QueueHandle_t queue;
	SemaphoreHandle_t sem_q;
	struct ovl_inst *inst;
	video_buffer *buf;
	enum video_buffer_type buf_type;
};

struct ovl_inst {
	void (*cb_func)
		(void *cb_data, video_buffer_type type, video_buffer *buf);
	void *cb_data;
};

struct ovl_drv {
	struct ovl_layer input[OVL_LAYER_COUNT];
	struct ovl_layer output;

	struct ovl_reg reg;
	uint32_t wdma2_irq;
	TaskHandle_t thread_handle;
	SemaphoreHandle_t sem_irq;
	EventGroupHandle_t event;
	struct MTK_WDMA2_HW_PARAM wdma_hw;
	struct MTK_OVL2_HW_PARAM_ALL ovl_hw;
	enum OVL2_THREAD_ST thr_st;
};

static struct ovl_drv *s_drv = NULL;

#if !OVL2_LOG_DISABLE
static char *layer_type_to_name(enum video_buffer_type buf_type)
{
	#define LAYER_NAME_LENGTH 20
	static char str[LAYER_NAME_LENGTH];

	memset(str, 0, LAYER_NAME_LENGTH);

	switch (buf_type) {
	case OVL_BUFFER_INPUT_0:
		strcpy(str, "input[0]");
		break;
	case OVL_BUFFER_INPUT_1:
		strcpy(str, "input[1]");
		break;
	case OVL_BUFFER_INPUT_2:
		strcpy(str, "input[2]");
		break;
	case OVL_BUFFER_INPUT_3:
		strcpy(str, "input[3]");
		break;
	case OVL_BUFFER_OUTPUT:
		strcpy(str, "output");
		break;
	default:
		strcpy(str, "unknown");
		break;
	}

	return str;
}
#endif

void thread_st_change(struct ovl_drv *drv, enum OVL2_THREAD_ST st)
{
	OVL2_LOG_D("thread st change, [%d]->[%d]", drv->thr_st, st);
	drv->thr_st = st;
}

static void hw_wdma2_irq_handler(int irq, void *data)
{
	struct ovl_drv *drv = (struct ovl_drv *)data;
	portBASE_TYPE irqWoken = pdFALSE;

	OVL2_LOG_FUNC_START;

	/*PRINTF_D("**********WDMA interrupt status[%x]***************",
	DISP_REG_GET(drv->reg.wdma_base + DISP_REG_WDMA_INTSTA));*/

	mtk_wdma2_hw_irq_clear(drv->reg.wdma_base);

	if (xSemaphoreGiveFromISR(drv->sem_irq, &irqWoken) != pdTRUE) {
		OVL2_LOG_E("fail to give wdma irq\n");
	} else {
		OVL2_LOG_D("ok to give wdma irq\n");
	}
}

int hw_wdma2_wait_irq(struct ovl_drv *drv)
{
	if (xSemaphoreTake(drv->sem_irq, pdMS_TO_TICKS(WDMA_WAIT_COUNT)) == pdPASS) {
		OVL2_LOG_D("[wdma] ok to wait irq");
	} else {
		OVL2_LOG_E("[wdma] fail to wait irq");
	}

	return 0;
}

int hw_wdma2_irq_handler_register(struct ovl_drv *drv)
{
	OVL2_LOG_FUNC_START;

	request_irq(drv->wdma2_irq, hw_wdma2_irq_handler,
		IRQ_TYPE_LEVEL_LOW, "wdma2", drv);
	OVL2_LOG_D("[wdma] request wdma irq[%ld]", drv->wdma2_irq);

	return 0;
}

static int hw_mmsys_path_connect(struct ovl_drv *drv)
{
	int32_t ret = 0;
	uint32_t addr;
	uint32_t val;

#if MTK_OVL2_SUPPORT_PQ
	addr = drv->reg.mmsys_base + DISP_OVL1_MOUT_EN;
	val = DISP_REG_GET(addr);
	val &= ~(1u << 17);
	val |= (1u << 16);
	ret += DISP_REG_SET(addr, val);

	addr = drv->reg.mmsys_base + DISP_WDMA1_SEL_IN;
	val = DISP_REG_GET(addr);
	val &= ~(1u << 16);
	ret += DISP_REG_SET(addr, val);
#else
	addr = drv->reg.mmsys_base + DISP_OVL1_MOUT_EN;
	val = DISP_REG_GET(addr);
	val &= ~(1u << 16);
	val |= (1u << 17);
	ret += DISP_REG_SET(addr, val);

	addr = drv->reg.mmsys_base + DISP_WDMA1_SEL_IN;
	val = DISP_REG_GET(addr);
	val |= (1u << 16);
	ret += DISP_REG_SET(addr, val);
#endif

	return ret;
}

static int hw_mmsys_path_disconnect(struct ovl_drv *drv)
{
	int32_t ret = 0;
	uint32_t addr;
	uint32_t val;

	addr = drv->reg.mmsys_base + DISP_OVL1_MOUT_EN;
	val = DISP_REG_GET(addr);
	val &= ~(1u << 16);
	val &= ~(1u << 17);
	ret += DISP_REG_SET(addr, val);

	addr = drv->reg.mmsys_base + DISP_WDMA1_SEL_IN;
	val = DISP_REG_GET(addr);
	val &= ~(1u << 16);
	ret += DISP_REG_SET(addr, val);

	return ret;
}

static int hw_mutex_enable(struct ovl_drv *drv)
{
	int32_t ret = 0;
	uint32_t addr;
	uint32_t val;

#if MTK_OVL2_SUPPORT_PQ
	addr = (drv->reg.mutex_base +
		DISP_MUTEX_MOD + DISP_MUTEX_GAP * DISP_MUTEX_IDX);
	val = DISP_REG_GET(addr);
	val |= (unsigned int)((1u << 21) | (1u << 30) | (1u << 31));
	ret += DISP_REG_SET(addr, val);

	addr = (drv->reg.mutex_base +
		DISP_MUTEX_MOD2 + DISP_MUTEX_GAP * DISP_MUTEX_IDX);
	val = DISP_REG_GET(addr);
	val |= (1u << 0);
	ret += DISP_REG_SET(addr, val);
#else
	addr = (drv->reg.mutex_base +
		DISP_MUTEX_MOD + DISP_MUTEX_GAP * DISP_MUTEX_IDX);
	val = DISP_REG_GET(addr);
	val |= (unsigned int)((1u << 30) | (1u << 31));
	ret += DISP_REG_SET(addr, val);
#endif

	addr = (drv->reg.mutex_base +
		DISP_MUTEX_SOF + DISP_MUTEX_GAP * DISP_MUTEX_IDX);
	val = DISP_REG_GET(addr);
	val &= (~(7u << 0));
	ret += DISP_REG_SET(addr, val);

	addr = (drv->reg.mutex_base +
		DISP_MUTEX_EN + DISP_MUTEX_GAP * DISP_MUTEX_IDX);
	val = DISP_REG_GET(addr);
	val |= (1u << 0);
	ret += DISP_REG_SET(addr, val);

	return ret;
}

static int hw_mutex_disable(struct ovl_drv *drv)
{
	int32_t ret = 0;
	uint32_t addr;
	uint32_t val;

#if MTK_OVL2_SUPPORT_PQ
	addr = (drv->reg.mutex_base + DISP_MUTEX_MOD + DISP_MUTEX_GAP * DISP_MUTEX_IDX);
	val = DISP_REG_GET(addr);
	val &= ~((1u << 21) | (1u << 30) | (1u << 31));
	ret += DISP_REG_SET(addr, val);

	addr = (drv->reg.mutex_base + DISP_MUTEX_MOD2 + DISP_MUTEX_GAP * DISP_MUTEX_IDX);
	val = DISP_REG_GET(addr);
	val &= ~(1u << 0);
	ret += DISP_REG_SET(addr, val);
#else
	addr = (drv->reg.mutex_base + DISP_MUTEX_MOD + DISP_MUTEX_GAP * DISP_MUTEX_IDX);
	val = DISP_REG_GET(addr);
	val &= ~((1u << 30) | (1u << 31));
	ret += DISP_REG_SET(addr, val);
#endif

	addr = (drv->reg.mutex_base + DISP_MUTEX_SOF + DISP_MUTEX_GAP * DISP_MUTEX_IDX);
	val = DISP_REG_GET(addr);
	val &= (~(7u << 0));
	ret += DISP_REG_SET(addr, val);

	addr = (drv->reg.mutex_base + DISP_MUTEX_EN + DISP_MUTEX_GAP * DISP_MUTEX_IDX);
	val = DISP_REG_GET(addr);
	val &= ~(1u << 0);
	ret += DISP_REG_SET(addr, val);

	return ret;
}

static enum OVL_INPUT_FORMAT hw_ovl2_input_fmt_convert(uint32_t color)
{
	enum OVL_INPUT_FORMAT ovl_fmt = OVL_INPUT_FORMAT_UNKNOWN;

	OVL2_LOG_FUNC_START;

	switch (color) {
	case PIX_FMT_ARGB32:
		ovl_fmt = OVL_INPUT_FORMAT_BGRA8888;
		break;
	case PIX_FMT_ABGR32:
		ovl_fmt = OVL_INPUT_FORMAT_ARGB8888;
		break;
	case PIX_FMT_UYVY:
		ovl_fmt = OVL_INPUT_FORMAT_UYVY;
		break;
	case PIX_FMT_YUYV:
		ovl_fmt = OVL_INPUT_FORMAT_YUYV;
		break;
	default:
		OVL2_LOG_E("unsupport color[0x%lx] map to fmt", color);
		ovl_fmt = OVL_INPUT_FORMAT_BGRA8888;
		break;
	}

	return ovl_fmt;
}

static int32_t hw_ovl2_prepare_hw(struct ovl_drv *drv)
{
	int32_t ret = 0;
	int32_t i = 0;

	OVL2_LOG_FUNC_START;

	for (i = 0; i < OVL_LAYER_COUNT; i++) {
		if (drv->ovl_hw.ovl_config[i].layer_en != 0U)
			break;
	}

	ret = mtk_ovl2_hw_set(drv->reg.ovl_base, &drv->ovl_hw);

	OVL2_LOG_FUNC_END;

	return ret;
}

static int32_t hw_ovl2_unprepare_hw(struct ovl_drv *drv)
{
	int32_t ret = 0;

	OVL2_LOG_FUNC_START;

	ret = mtk_ovl2_hw_unset(drv->reg.ovl_base);

	OVL2_LOG_FUNC_END;

	return ret;
}

static void hw_ovl2_param_store(
		struct ovl_drv *drv,
		struct video_buffer *ovl_buf,
		int32_t idx)
{
	struct MTK_OVL2_HW_PARAM_ALL *hw = &drv->ovl_hw;
	struct MTK_OVL2_HW_PARAM_LAYER *layer = NULL;

	OVL2_LOG_FUNC_START;

	hw->dst_w = drv->input[idx].ovl_area.width;
	hw->dst_h = drv->input[idx].ovl_area.height;
	layer = &(hw->ovl_config[idx]);
	layer->layer_en = 1;
	layer->source = OVL_LAYER_SOURCE_MEM;
	layer->fmt = hw_ovl2_input_fmt_convert(
			drv->input[idx].ovl_fmt.fourcc);
	layer->addr = (unsigned long)ovl_buf->image;
	layer->src_x = (unsigned int)drv->input[idx].ovl_crop.left;
	layer->src_y = (unsigned int)drv->input[idx].ovl_crop.top;

	switch (layer->fmt) {
	case OVL_INPUT_FORMAT_ARGB8888:
	case OVL_INPUT_FORMAT_BGRA8888:
		layer->src_pitch = drv->input[idx].ovl_fmt.width * 4;
		break;
	case OVL_INPUT_FORMAT_YUYV:
	case OVL_INPUT_FORMAT_UYVY:
		layer->src_pitch = drv->input[idx].ovl_fmt.width * 2;
		break;
	default:
		OVL2_LOG_E("unsupport color[0x%x] map to fmt", layer->fmt);
		break;
	}

	layer->dst_x = (unsigned int)drv->input[idx].ovl_area.left;
	layer->dst_y = (unsigned int)drv->input[idx].ovl_area.top;
	layer->dst_w = drv->input[idx].ovl_area.width;
	layer->dst_h = drv->input[idx].ovl_area.height;
	layer->keyEn = 0;
	layer->key = 0xFF020100U;
	layer->aen = 1;
	layer->alpha = 0xFF;
	layer->sur_aen = 0;
	layer->src_alpha = 0;
	layer->dst_alpha = 0;
	layer->yuv_range = 0;

	OVL2_LOG_D("dst wh[%ld, %ld]",
		hw->dst_w,
		hw->dst_h);
	OVL2_LOG_D("layer[%p] en[%ld] src[%d] fmt[%d] addr[0x%llx]",
		layer,
		layer->layer_en,
		layer->source,
		layer->fmt,
		layer->addr);
	OVL2_LOG_D("src xy[%ld, %ld] pitch[%ld]",
		layer->src_x,
		layer->src_y,
		layer->src_pitch);
	OVL2_LOG_D("dst xy[%ld, %ld] wh[%ld, %ld]",
		layer->dst_x,
		layer->dst_y,
		layer->dst_w,
		layer->dst_h);
	OVL2_LOG_D("key[%ld, %ld]",
		layer->keyEn,
		layer->key);
	OVL2_LOG_D("alpha[%ld, %d, %ld, %ld, %ld]",
		layer->aen,
		layer->alpha,
		layer->sur_aen,
		layer->src_alpha,
		layer->dst_alpha);
	OVL2_LOG_D("yuv_range[%ld]",
		layer->yuv_range);

	OVL2_LOG_FUNC_END;
}

static enum MTK_WDMA_HW_FORMAT hw_wdma2_fmt_convert(uint32_t fmt)
{
	OVL2_LOG_FUNC_START;
	enum MTK_WDMA_HW_FORMAT wdma_fmt = MTK_WDMA_HW_FORMAT_UNKNOWN;

	switch (fmt) {
	case PIX_FMT_ARGB32:
		wdma_fmt = MTK_WDMA_HW_FORMAT_ARGB8888;
		break;
	case PIX_FMT_ABGR32:
		wdma_fmt = MTK_WDMA_HW_FORMAT_ABGR8888;
		break;
	/*case eYUY2:*/
	case PIX_FMT_YUYV:
		wdma_fmt = MTK_WDMA_HW_FORMAT_YUYV;
		break;
	case PIX_FMT_UYVY:
		wdma_fmt = MTK_WDMA_HW_FORMAT_UYVY;
		break;
	default:
		OVL2_LOG_E("[wdma] %s[%d] unsupport wdma output fmt=0x%lx",
			__func__, __LINE__, fmt);
		break;
	}

	return wdma_fmt;
}

static void hw_wdma2_param_store(
		struct ovl_drv *drv,
		struct video_buffer *ovl_buf)
{
	OVL2_LOG_FUNC_START;

	drv->wdma_hw.in_format = MTK_WDMA_HW_FORMAT_RGB888;
	OVL2_LOG_D("[wdma] out_fmt[0x%lx]", drv->output.ovl_fmt.fourcc);
	drv->wdma_hw.out_format =
		hw_wdma2_fmt_convert(drv->output.ovl_fmt.fourcc);
	drv->wdma_hw.src_width = drv->output.ovl_fmt.width;
	drv->wdma_hw.src_height = drv->output.ovl_fmt.height;
	drv->wdma_hw.clip_x = (unsigned int)drv->output.ovl_crop.left;
	drv->wdma_hw.clip_y = (unsigned int)drv->output.ovl_crop.top;
	drv->wdma_hw.clip_width = drv->output.ovl_crop.width;
	drv->wdma_hw.clip_height = drv->output.ovl_crop.height;
	drv->wdma_hw.addr_plane = (unsigned long)ovl_buf->image;
	drv->wdma_hw.use_specified_alpha = 0;
	drv->wdma_hw.alpha = 0;

	OVL2_LOG_D("[wdma] store hw param :");
	OVL2_LOG_D("[wdma] fmt[%d, %d]",
		drv->wdma_hw.in_format,
		drv->wdma_hw.out_format);
	OVL2_LOG_D("[wdma] wh[%ld, %ld]",
		drv->wdma_hw.src_width,
		drv->wdma_hw.src_height);
	OVL2_LOG_D("[wdma] clip[%ld, %ld, %ld, %ld]",
		drv->wdma_hw.clip_x,
		drv->wdma_hw.clip_y,
		drv->wdma_hw.clip_width,
		drv->wdma_hw.clip_height);
	OVL2_LOG_D("[wdma] addr[0x%llx]",
		drv->wdma_hw.addr_plane);
	OVL2_LOG_D("[wdma] alpha[%ld, %d]",
		drv->wdma_hw.use_specified_alpha,
		drv->wdma_hw.alpha);

	OVL2_LOG_FUNC_END;
}

static int32_t hw_wdma2_prepare_hw(struct ovl_drv *drv)
{
	int32_t ret = 0;

	OVL2_LOG_FUNC_START;

	ret = mtk_wdma2_hw_set(drv->reg.wdma_base, &drv->wdma_hw);

#if MTK_OVL2_SUPPORT_PQ
	mtk_color2_hw_init();

	mtk_color2_window_set();

	mtk_color2_bypass_set();

	ret = mtk_color2_hw_set();

	mtk_gamma_hw_set();
#endif

	ret = hw_mmsys_path_connect(drv);
	ret = hw_mutex_enable(drv);

	OVL2_LOG_FUNC_END;

	return ret;
}

static int32_t hw_wdma2_unprepare_hw(struct ovl_drv *drv)
{
	int32_t ret = 0;

	OVL2_LOG_FUNC_START;

	ret = hw_mmsys_path_disconnect(drv);
	ret = hw_mutex_disable(drv);

	ret = mtk_wdma2_hw_unset(drv->reg.wdma_base);

	OVL2_LOG_FUNC_END;

	return ret;
}

static void queue_clear(QueueHandle_t queue)
{
	video_buffer *buf;

	while(1) {
		if (xQueueReceive(queue, &buf, 0) != pdPASS){
			break;
		}
	}

	OVL2_LOG_D("ok to clear queue[%p]", queue);
}

static void param_init(struct ovl_drv *drv)
{
	uint32_t layer_idx = 0;
	struct ovl_layer *layer = NULL;
	memset(drv, 0, sizeof(struct ovl_drv));

	drv->thr_st = THR_NONE;
	drv->reg.ovl_base = CM4_DISP_OVL2_REG_BASE;
	drv->reg.color_base = CM4_DISP_COLOR2_REG_BASE;
	drv->reg.gamma_base = CM4_DISP_GAMMA_REG_BASE;
	drv->reg.wdma_base = CM4_DISP_WDMA2_REG_BASE;
	drv->reg.mmsys_base = CM4_MMSYS_CONFIG_REGS_BASE;
	drv->reg.mutex_base = CM4_DISP_MUTEX_BASE;
	drv->wdma2_irq = DISP_WDMA2_IRQ_BIT;

	drv->sem_irq = xSemaphoreCreateBinary();
	if (drv->sem_irq == NULL) {
		OVL2_LOG_E("fail to create sem_irq");
	} else {
		OVL2_LOG_D("ok to create sem_irq[%p]", drv->sem_irq);
	}

	drv->event = xEventGroupCreate();
	if (drv->event == NULL) {
		OVL2_LOG_E("fail to create event");
	} else {
		OVL2_LOG_D("ok to create event[%p]", drv->event);
	}

	hw_wdma2_irq_handler_register(drv);

	for (layer_idx = 0; layer_idx < OVL_LAYER_COUNT; layer_idx++) {
		layer = &drv->input[layer_idx];
		layer->queue = xQueueCreate(
			BUF_QUEUE_SIZE, sizeof(struct video_buffer *));
		if (layer->queue == NULL) {
			OVL2_LOG_E("fail to create input[%ld] queue",
				layer_idx);
		} else {
			OVL2_LOG_D("ok to create input[%ld] queue[%p]",
				layer_idx, layer->queue);
		}
		layer->sem_q = xSemaphoreCreateBinary();
	}

	layer = &drv->output;
	layer->queue = xQueueCreate(
		BUF_QUEUE_SIZE, sizeof(struct video_buffer *));
	if (layer->queue == NULL) {
		OVL2_LOG_E("fail to create output queue");
	} else {
		OVL2_LOG_D("ok to create output queue[%p]", layer->queue);
	}
	layer->sem_q = xSemaphoreCreateBinary();
}

static void param_uninit(struct ovl_drv *drv)
{
	uint32_t layer_idx = 0;
	struct ovl_layer *layer = NULL;

	for (layer_idx = 0; layer_idx < OVL_LAYER_COUNT; layer_idx++) {
		layer = &drv->input[layer_idx];

		if (layer->queue != NULL) {
			vQueueDelete(layer->queue);
			layer->queue = NULL;
			OVL2_LOG_D("ok to del input[%ld] queue[%p]",
				layer_idx, layer->queue);
		}

		if (layer->sem_q != NULL) {
			vSemaphoreDelete(layer->sem_q);
			layer->sem_q = NULL;
			OVL2_LOG_D("ok to del input[%ld] sem[%p]",
				layer_idx, layer->sem_q);
		}
	}

	layer = &drv->output;
	if (layer->queue != NULL) {
		vQueueDelete(layer->queue);
		layer->queue = NULL;
		OVL2_LOG_D("ok to del output queue[%p]", layer->queue);
	}

	if (layer->sem_q != NULL) {
		vSemaphoreDelete(layer->sem_q);
		layer->sem_q = NULL;
		OVL2_LOG_D("ok to del output sem[%p]", layer->sem_q);
	}

	if (drv->event != NULL) {
		vEventGroupDelete(drv->event);
		OVL2_LOG_D("ok to del event[%p]", drv->event);
	}

	if (drv->sem_irq != NULL) {
		vSemaphoreDelete(drv->sem_irq);
		OVL2_LOG_D("ok to del sem_irq[%p]", drv->sem_irq);
	}
}

static void work_cb_output(struct ovl_drv *drv)
{
	struct ovl_layer *output = NULL;

	output = &drv->output;

	if (output->buf == NULL) {
		OVL2_LOG_D("NULL buf for cb output");
		return;
	}

	if (output->inst == NULL) {
		OVL2_LOG_E("NULL point inst for cb output");
		return;
	}

	if (output->inst->cb_func == NULL) {
		OVL2_LOG_E("NULL point cb_func for cb output");
		return;
	}

	output->inst->cb_func(
		output->inst->cb_data, OVL_BUFFER_OUTPUT, output->buf);

	OVL2_LOG_D("ok to cb output buf[%p, %lld]",
		output->buf->image, output->buf->timestamp);

	output->buf = NULL;
}

static void work_cb_input_single(struct ovl_layer *input)
{
	if (input->buf == NULL) {
		OVL2_LOG_D("NULL buf for cb input");
		return;
	}

	if (input->inst == NULL) {
		OVL2_LOG_E("NULL point inst for cb input");
		return;
	}

	if (input->inst->cb_func != NULL) {
		input->inst->cb_func(
			input->inst->cb_data,
			input->buf_type,
			input->buf);

		OVL2_LOG_D("ok to cb %s buf[%p, %lld]",
			layer_type_to_name(input->buf_type),
			input->buf->image,
			input->buf->timestamp);

		input->buf = NULL;
	}
}

static void work_cb_input_all(struct ovl_drv *drv)
{
	uint32_t layer_idx = 0;
	uint32_t orlop_buf_cb = 0;
	struct ovl_layer *input = NULL;

	for (layer_idx = 0; layer_idx < OVL_LAYER_COUNT; layer_idx++) {
		input = &drv->input[layer_idx];

		if (input->enable != 0) {
			if (orlop_buf_cb == 0) {
				work_cb_input_single(input);

				if (orlop_buf_cb == 0) {
					orlop_buf_cb = 1;

					OVL2_LOG_D("ok to ready input");
				}
			} else {
				if ((uxQueueMessagesWaiting(
							input->queue) > 1))
					work_cb_input_single(input);
			}
		}
	}
}

static void work_get_output_buf(struct ovl_drv *drv, uint32_t *output_buf_ready)
{
	struct ovl_layer *output = NULL;

	output = &drv->output;

	if ((output->enable == 0) || (output->queue == NULL)) {
		*output_buf_ready = 0;
		return;
	}

	if (output->buf != NULL) {
		work_cb_output(drv);
		return;
	}

	if (xQueueReceive(output->queue, &output->buf, pdMS_TO_TICKS(10))
			!= pdPASS){
		*output_buf_ready = 0;
		/* OVL2_LOG_D("fail to get output"); */
		return;
	}

	OVL2_LOG_D("ok to get output buf[%p, %lld]",
		output->buf->image, output->buf->timestamp);

	*output_buf_ready = 1;
	hw_wdma2_param_store(drv, output->buf);
}

static void work_get_input_buf(struct ovl_drv *drv, uint32_t *input_buf_ready)
{
	uint32_t layer_idx = 0;
	uint32_t orlop_get = 0;
	struct ovl_layer *input = NULL;

	for (layer_idx = 0; layer_idx < OVL_LAYER_COUNT; layer_idx++) {
		input = &drv->input[layer_idx];

		if (input->enable != 0) {
			if (input->buf != NULL) {
				OVL2_LOG_E("input[%ld] buf is still not cb",
					layer_idx);
				continue;
			}

			if (xQueueReceive(
					input->queue,
					&input->buf,
					((orlop_get == 0)?pdMS_TO_TICKS(10):0))
					!= pdPASS){
				if (orlop_get == 0) {
					*input_buf_ready = 0;
					/*OVL2_LOG_D("fail to get orlop input[%ld]",
						layer_idx);*/
					return;
				}
			}

			OVL2_LOG_D("ok to get input[%ld] buf[%p, %lld]",
				layer_idx,
				input->buf->image,
				input->buf->timestamp);

			if (orlop_get == 0) {
				orlop_get = 1;
				*input_buf_ready = 1;

				OVL2_LOG_D("ok to ready input");
			}

			hw_ovl2_param_store(
				drv, input->buf, layer_idx);
		}
	}
}

static void work_thread(void *handle)
{
	uint32_t input_buf_ready = 0;
	uint32_t output_buf_ready = 0;
	struct ovl_drv *drv = (struct ovl_drv *)handle;

	OVL2_LOG_FUNC_START;

	param_init(drv);

	while (drv->thr_st != THR_RELEASE) {

		if (drv->thr_st == THR_WAITING) {
			OVL2_LOG_D("[close] start to set EV_CLOSE_STEP_1");
			xEventGroupSetBits(drv->event, OVL2_EV_CLOSE_STEP_1);
			OVL2_LOG_D("[close] start to wait EV_CLOSE_STEP_2");
			xEventGroupWaitBits(
				drv->event,
				OVL2_EV_CLOSE_STEP_2,
				pdTRUE,
				pdFALSE,
				pdMS_TO_TICKS(5000));
			OVL2_LOG_D("[close] end to wait EV_CLOSE_STEP_2");
		}

		if (output_buf_ready == 0) {
			work_get_output_buf(drv, &output_buf_ready);

			if (output_buf_ready == 0)
				continue;
		}

		if (input_buf_ready == 0) {
			work_get_input_buf(drv, &input_buf_ready);

			if (input_buf_ready == 0)
				continue;
		}

		hw_ovl2_prepare_hw(drv);
		hw_wdma2_prepare_hw(drv);
		hw_wdma2_wait_irq(drv);
		hw_wdma2_unprepare_hw(drv);
		hw_ovl2_unprepare_hw(drv);

		work_cb_output(drv);
		work_cb_input_all(drv);

		output_buf_ready = 0;
		input_buf_ready = 0;
	}

	param_uninit(drv);

	thread_st_change(drv, THR_NONE);
	vTaskDelete(NULL);
}

video_handle ovl2_open(void)
{
	struct ovl_inst *inst = NULL;

	OVL2_LOG_FUNC_START;

	inst = (struct ovl_inst *)malloc(sizeof(struct ovl_inst));
	if (NULL == inst) {
		OVL2_LOG_E("fail to alloc inst size[%d]",
			sizeof(struct ovl_inst));

		return NULL;
	} else {
		memset(inst, 0, sizeof(struct ovl_inst));
		OVL2_LOG_D("ok to alloc inst[%p] size[%d]",
			inst, sizeof(struct ovl_inst));
	}

	return inst;
}

int32_t ovl2_config(video_handle handle, video_config_type type, void *config)
{
	struct ovl_drv *drv = s_drv;
	struct ovl_inst *inst;
	video_buffer_type buf_type;
	struct ovl_layer *layer = NULL;
	video_format *param_fmt = (video_format *)config;
	video_rect *param_crop = (video_rect *)config;
	video_rect *param_area = (video_rect *)config;

	OVL2_LOG_FUNC_START;

	if (handle == NULL || config == NULL) {
		OVL2_LOG_D("NULL point in config inst[%p] type[%d] config[%p]",
			handle, type, config);

		return -1;
	}

	inst = (struct ovl_inst *)handle;

	if (type == VIDEO_CONFIG_FORMAT) {
		buf_type = param_fmt->type;
	}else if (type == VIDEO_CONFIG_CROP) {
		buf_type = param_crop->type;
	} else if (type == VIDEO_CONFIG_AREA) {
		buf_type = param_area->type;
	} else {
		OVL2_LOG_E("unknown config_type in config inst[%p] type[%d] config[%p]",
			handle, type, config);
		return -1;
	}

	switch (buf_type) {
	case OVL_BUFFER_INPUT_0:
		layer = &drv->input[0];
		break;
	case OVL_BUFFER_INPUT_1:
		layer = &drv->input[1];
		break;
	case OVL_BUFFER_INPUT_2:
		layer = &drv->input[2];
		break;
	case OVL_BUFFER_INPUT_3:
		layer = &drv->input[3];
		break;
	case OVL_BUFFER_OUTPUT:
		layer = &drv->output;
		break;
	default:
		OVL2_LOG_E("unknown buf_type[%d] in config inst[%p] type[%d] config[%p]",
			buf_type, handle, type, config);
		return -1;
	}

	if (layer->enable == 0) {
		layer->enable = 1;
		layer->inst = inst;
		layer->buf_type = buf_type;
		OVL2_LOG_D("enable %s", layer_type_to_name(buf_type));
	} else if (layer->inst != inst){
		OVL2_LOG_E("fail to config using layer %s in config inst[%p] type[%d] config[%p]",
			layer_type_to_name(buf_type), handle, type, config);
		return -1;
	}

	if (type == VIDEO_CONFIG_FORMAT) {
		memcpy(&layer->ovl_fmt, param_fmt, sizeof(video_format));
		OVL2_LOG_D("config %s fmt[0x%lx] wh[%ld, %ld]",
			layer_type_to_name(buf_type),
			param_fmt->fourcc, param_fmt->width, param_fmt->height);
	}else if (type == VIDEO_CONFIG_CROP) {
		memcpy(&layer->ovl_crop, param_crop, sizeof(video_rect));
		OVL2_LOG_D("config %s crop xy[%ld, %ld] wh[%ld, %ld]",
			layer_type_to_name(buf_type),
			param_crop->left, param_crop->top,
			param_crop->width, param_crop->height);
	} else if (type == VIDEO_CONFIG_AREA) {
		memcpy(&layer->ovl_area, param_area, sizeof(video_rect));
		OVL2_LOG_D("config %s area xy[%ld, %ld] wh[%ld, %ld]",
			layer_type_to_name(buf_type),
			param_area->left, param_area->top,
			param_area->width, param_area->height);
	} else {
		OVL2_LOG_E("unknown config type!");
			return -1;
	}

	return 0;
}

int32_t ovl2_commit(
	video_handle handle, video_buffer_type type, video_buffer *buffer)
{
	int ret = 0;
	struct ovl_drv *drv = s_drv;
	struct ovl_layer *layer = NULL;

	OVL2_LOG_FUNC_START;

	if (handle == NULL || buffer == NULL) {
		OVL2_LOG_D("NULL point in commit inst[%p] type[%d] buf[%p]",
			handle, type, buffer);

		return -1;
	}

	switch(type) {
	case OVL_BUFFER_INPUT_0 :
		layer = &drv->input[0];
		break;
	case OVL_BUFFER_INPUT_1 :
		layer = &drv->input[1];
		break;
	case OVL_BUFFER_INPUT_2 :
		layer = &drv->input[2];
		break;
	case OVL_BUFFER_INPUT_3 :
		layer = &drv->input[3];
		break;
	case OVL_BUFFER_OUTPUT :
		layer = &drv->output;
		break;
	default:
		OVL2_LOG_E("unknown buffer type");
		return -1;
	}

	if (layer->queue != NULL) {
		if(xQueueSendToBack(
				layer->queue, &buffer, 0) != pdPASS) {
			OVL2_LOG_E("fail to send %s buf",
				layer_type_to_name(type));
			ret = -1;
		} else
			OVL2_LOG_D("ok to send %s buf[%p, %lld] to queue[%p]",
				layer_type_to_name(type),
				buffer->image,
				buffer->timestamp,
				layer->queue);
	}

	return ret;
}

int32_t ovl2_register_handler(
	void *handle,
	void (*buffer_done_handler)(
		void *usr_data, video_buffer_type type, video_buffer *buffer),
	void *usr_data)
{
	struct ovl_inst *inst;

	OVL2_LOG_FUNC_START;

	if (handle == NULL || buffer_done_handler == NULL) {
		OVL2_LOG_D("NULL point in reg inst[%p] cb[%p, %p]",
			handle, buffer_done_handler, usr_data);

		return -1;
	}

	inst = (struct ovl_inst *)handle;

	inst->cb_func = buffer_done_handler;
	inst->cb_data = usr_data;

	OVL2_LOG_D("ok to reg inst[%p] cb[%p, %p]",
		inst, buffer_done_handler, usr_data);

	return 0;
}

int32_t ovl2_close(video_handle handle)
{
	int32_t layer_idx = 0;
	struct ovl_drv *drv = s_drv;
	struct ovl_inst *inst = NULL;
	struct ovl_layer *layer = NULL;

	OVL2_LOG_FUNC_START;

	if (handle == NULL) {
		OVL2_LOG_D("NULL point in close inst[%p]", handle);

		return -1;
	}

	OVL2_LOG_D("[close] start to set THR_WAITING");
	thread_st_change(drv, THR_WAITING);
	OVL2_LOG_D("[close] start to wait EV_CLOSE_STEP_1");
	xEventGroupWaitBits(
		drv->event,
		OVL2_EV_CLOSE_STEP_1,
		pdTRUE,
		pdFALSE,
		pdMS_TO_TICKS(5000));
	OVL2_LOG_D("[close] end to wait EV_CLOSE_STEP_1");

	inst = (struct ovl_inst *)handle;

	for (layer_idx = 0; layer_idx < OVL_INST_COUNT; layer_idx++) {
		layer = &drv->input[layer_idx];
		if (layer->inst == inst) {
			queue_clear(layer->queue);
			layer->enable = 0;
			layer->inst = NULL;
		}
	}

	layer = &drv->output;
	if (layer->inst == inst) {
		queue_clear(layer->queue);
		layer->enable = 0;
		layer->inst = NULL;
	}

	free(inst);

	OVL2_LOG_D("[close] start to set THR_IDLE");
	thread_st_change(drv, THR_IDLE);
	OVL2_LOG_D("[close] start to set EV_CLOSE_STEP_2");
	xEventGroupSetBits(drv->event, OVL2_EV_CLOSE_STEP_2);

	OVL2_LOG_D("ok to free inst[%p]", inst);

	return 0;
}

int32_t ovl2_init(void)
{
	int ret = 0;
	struct ovl_drv *drv = NULL;

	/* create ovl drv */
	drv = (struct ovl_drv *)malloc(sizeof(struct ovl_drv));
	if (NULL == drv) {
		OVL2_LOG_E("fail to alloc drv size[%d]",
			sizeof(struct ovl_drv));
		ret = -1;
	} else {
		OVL2_LOG_D("ok to alloc drv, addr[%p], size[%d]",
			drv, sizeof(struct ovl_drv));

		/* create ovl2 work thread */
		if (xTaskCreate(
				work_thread,
				"ovl2",
				512,
				(void *)drv,
				0,
				&(drv->thread_handle))
				!= pdPASS) {
			OVL2_LOG_E("fail to create thread");
			ret = -1;
		}
	}

	s_drv = drv;

	return ret;
}

int32_t ovl2_uninit(void)
{
	struct ovl_drv *drv = s_drv;

	OVL2_LOG_FUNC_START;

	thread_st_change(drv, THR_RELEASE);

	while (drv->thr_st != THR_NONE)
		vTaskDelay(pdMS_TO_TICKS(1));

	free(drv);

	OVL2_LOG_FUNC_END;

	return 0;
}

