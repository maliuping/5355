/*
 * tvd_if.c - mtk tvd driver interface
 *
 * Copyright (c) 2018 MediaTek Inc.
 * Author: Qing Li <qing.li@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


#include "tvd_if.h"
#include "tvd_private.h"

struct TVD_DRV *s_tvd_drv = NULL;

#if TVD_SELF_TEST
#define TVD_TEST_LOOP_COUNT 100
#define TVD_TEST_BUF_COUNT 5

enum TVD_TEST_BUF_ST {
	TVD_TEST_BUF_FREE,
	TVD_TEST_BUF_USE,
	TVD_TEST_BUF_MAX,
};

struct TVD_TEST {
	TaskHandle_t thread_handle;
	video_handle handle_test;
	struct TVD_CONFIG config;
	struct video_buffer buf_info[TVD_TEST_BUF_COUNT];
	enum TVD_TEST_BUF_ST buf_st[TVD_TEST_BUF_COUNT];
};

struct TVD_TEST *s_tvd_test = NULL;

static void tvd_thread_test(void *args);
#endif

video_handle tvd_open(void)
{
	video_handle h;
	struct TVD_DRV *drv = s_tvd_drv;

	TVD_LOG_FUNC_START;

	h = (void *)1;

	tvd_irq_req(drv);

	TVD_LOG_FUNC_END;

	return h;
}

int32_t tvd_close(video_handle handle)
{
	struct TVD_DRV *drv = s_tvd_drv;

	TVD_LOG_FUNC_START;

	tvd_irq_unreq(drv);

	TVD_LOG_FUNC_END;

	return TVD_RET_OK;
}

int32_t tvd_config(video_handle handle, video_config_type type, void *config)
{
	struct TVD_DRV *drv = s_tvd_drv;
	struct TVD_CONFIG *tvd_cfg = (struct TVD_CONFIG *)config;

	TVD_LOG_FUNC_START;

	if (type != VIDEO_CONFIG_FORMAT) {
		TVD_LOG_ERR("invalid config type %d", type);
		return TVD_RET_PARAM_ERR;
	}

	/*if (tvd_cfg->color_format != ) {
		TVD_LOG_ERR("invalid color fmt type 0x%x", tvd_cfg->color_format);
		return TVD_RET_PARAM_ERR;
	} else */{
		drv->color_format = tvd_cfg->color_format;
		TVD_LOG_DBG("set color fmt type 0x%lx", tvd_cfg->color_format);
	}

	TVD_LOG_FUNC_END;

	return TVD_RET_OK;
}

int32_t tvd_commit(
	video_handle handle, video_buffer_type type, video_buffer *buffer)
{
	BaseType_t ret;
	struct TVD_DRV *drv = s_tvd_drv;

	TVD_LOG_FUNC_START;

	if (type != VIDEO_BUFFER_OUTPUT) {
		TVD_LOG_ERR("invalid commit buffer type %d", type);
		return TVD_RET_PARAM_ERR;
	}

	ret = xQueueSendToBack(drv->queue_handle, &buffer, 0);
	if (ret != pdPASS) {
		TVD_LOG_ERR("fail to send queue with queue[%p], ret %ld",
			drv->queue_handle, ret);
		return -1;
	} else {
		TVD_LOG_DBG("ok to send queue with queue[%p], ret %ld, image[%p] stamp[%lld]",
			drv->queue_handle, ret,
			buffer->image, buffer->timestamp);
	}

	TVD_LOG_FUNC_END;

	return 0;
}

int32_t tvd_register_handler(
	void *handle, void (*buffer_done_handler)(void *usr_data,
		video_buffer_type type, video_buffer *buffer), void *usr_data)
{
	struct TVD_DRV *drv = s_tvd_drv;

	TVD_LOG_FUNC_START;

	drv->cb_func = buffer_done_handler;
	drv->cb_data = usr_data;
	TVD_LOG_DBG("ok to reg cb, func[%p] data[%p]",
		drv->cb_func, drv->cb_data);

	TVD_LOG_FUNC_END;

	return 0;
}

int32_t tvd_init(void)
{
	TVD_LOG_FUNC_START;

	s_tvd_drv = (struct TVD_DRV *)malloc(sizeof(struct TVD_DRV));
	if (s_tvd_drv == NULL) {
		TVD_LOG_ERR("fail to malloc drv");
		return -1;
	} else {
		TVD_LOG_DBG("ok to malloc drv = %p", s_tvd_drv);
	}
	memset(s_tvd_drv, 0, sizeof(struct TVD_DRV));

	if (xTaskCreate(tvd_thread,
			"tvd",
			400,
			(void*)s_tvd_drv,
			0,
			&(s_tvd_drv->thread_handle)
			) != pdPASS) {
		TVD_LOG_ERR("fail to create thread");
		return TVD_RET_FAIL;
	}

	#if TVD_SELF_TEST
	s_tvd_test = (struct TVD_TEST *)malloc(sizeof(struct TVD_TEST));
	if (s_tvd_test == NULL) {
		TVD_LOG_ERR("fail to malloc test");
		return -1;
	} else {
		TVD_LOG_DBG("ok to malloc test = %p", s_tvd_test);
	}
	memset(s_tvd_test, 0, sizeof(struct TVD_TEST));

	if (xTaskCreate(tvd_thread_test,
			"tvd_test",
			400,
			(void*)s_tvd_test,
			0,
			&(s_tvd_test->thread_handle)
			) != pdPASS) {
		TVD_LOG_ERR("fail to create thread for test");
		return TVD_RET_FAIL;
	}
	#endif

	TVD_LOG_FUNC_END;

	return 0;
}

int32_t tvd_uninit(void)
{
	struct TVD_DRV *drv = s_tvd_drv;

	TVD_LOG_FUNC_START;

	tvd_thread_st_change(drv, THREAD_RELEASE);

	while (drv->thr_st != THREAD_NONE)
		vTaskDelay(pdMS_TO_TICKS(1));

	if (drv->queue_handle)
		vQueueDelete(drv->queue_handle);

	free(drv);

	TVD_LOG_FUNC_END;

	return 0;
}

#if TVD_SELF_TEST
#include "scp_ipi.h"

static void tvd_cb_test(
	void *usr_data, video_buffer_type type, video_buffer *buf_info)
{
	uint32_t idx = 0;
	struct TVD_TEST *test = (struct TVD_TEST *)usr_data;

	if (test == NULL) {
		TVD_LOG_TEST_ERR("fail to cb, NULL data");
		return;
	}

	if (buf_info == NULL) {
		TVD_LOG_TEST_ERR("fail to cb, NULL buf_info struct");
		return;
	}

	if (type != VIDEO_BUFFER_OUTPUT) {
		TVD_LOG_TEST_ERR("fail to cb, invalid type %d", type);
		return;
	}

	if (buf_info->image == NULL) {
		TVD_LOG_TEST_ERR("fail to cb, NULL addr");
		return;
	}

	for (idx = 0; idx < TVD_TEST_BUF_COUNT; idx++) {
		if (test->buf_info[idx].image == buf_info->image) {
			test->buf_st[idx] = TVD_TEST_BUF_FREE;
			break;
		}
	}

	if (idx == TVD_TEST_BUF_COUNT) {
		TVD_LOG_TEST_ERR("fail to cb, non match buf, addr[%p] stamp[%lld]",
			buf_info->image, buf_info->timestamp);
	} else {
		TVD_LOG_TEST_OK("ok to cb, addr[%p] stamp[%lld]",
			buf_info->image, buf_info->timestamp);
	}
}

static void tvd_thread_test(void *args)
{
	int32_t ret = 0;
	uint32_t idx = 0;
	uint32_t loop = TVD_TEST_LOOP_COUNT;
	struct TVD_TEST *test = (struct TVD_TEST *)args;

	test->config.color_format = PIX_FMT_MT21;

	for (idx = 0; idx < TVD_TEST_BUF_COUNT; idx++) {
		test->buf_info[idx].image =
			(VA_TO_IOVA(noncached_mem_alloc(TVD_TDC_BUF_SIZE)));
		test->buf_info[idx].timestamp = idx;
		test->buf_st[idx] = TVD_TEST_BUF_FREE;
	}

	while (1) {
		vTaskDelay(pdMS_TO_TICKS(1));

		test->handle_test = tvd_open();
		if (test->handle_test == NULL) {
			TVD_LOG_TEST_ERR("fail to open, ret %p",
				test->handle_test);
		} else {
			TVD_LOG_TEST_OK("ok to open, ret %p",
				test->handle_test);
		}

		ret = tvd_config(
			test->handle_test, VIDEO_CONFIG_FORMAT, &test->config);
		if (ret != 0) {
			TVD_LOG_TEST_ERR("fail to config, ret %ld", ret);
		} else {
			TVD_LOG_TEST_OK("ok to config, ret %ld", ret);
		}

		ret = tvd_register_handler(
			test->handle_test, tvd_cb_test, test);
		if (ret != 0) {
			TVD_LOG_TEST_ERR("fail to reg-cb, ret %ld", ret);
		} else {
			TVD_LOG_TEST_OK("ok to reg-cb, ret %ld", ret);
		}

		while (loop--) {
			struct video_buffer *buf_info = NULL;

			vTaskDelay(pdMS_TO_TICKS(16));

			for (idx = 0; idx < TVD_TEST_BUF_COUNT; idx++) {
				if (test->buf_st[idx] == TVD_TEST_BUF_FREE) {
					buf_info = &test->buf_info[idx];
					test->buf_st[idx] = TVD_TEST_BUF_USE;
					break;
				}
			}

			if (buf_info == NULL) {
				continue;
			}

			ret = tvd_commit(test->handle_test,
				VIDEO_BUFFER_OUTPUT, buf_info);
			if (ret != 0) {
				TVD_LOG_TEST_ERR(
					"fail to commit, addr[%p] stamp[0x%lld], ret %ld",
					buf_info->image,
					buf_info->timestamp,
					ret);
			} else {
				TVD_LOG_TEST_OK(
					"ok to commit, addr[%p] stamp[0x%lld], ret %ld",
					buf_info->image,
					buf_info->timestamp,
					ret);
			}
		}

		ret = tvd_close(test->handle_test);
		if (ret != 0) {
			TVD_LOG_TEST_ERR("fail to close, ret %ld", ret);
		} else {
			TVD_LOG_TEST_OK("ok to close, ret %ld", ret);
		}
	}

	vTaskDelete(NULL);
}
#endif


