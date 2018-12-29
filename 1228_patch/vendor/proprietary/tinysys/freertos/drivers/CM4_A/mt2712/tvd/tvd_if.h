/*
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

#ifndef TVD_DRIVER_H
#define TVD_DRIVER_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "video_core.h"

#define TVD_LOG_DISABLE 1 /* should be 1 when merge code */
#define TVD_ADV_SETTING 0
#define TVD_SELF_TEST 0 /* should be 0 when merge code */

#define TVD_AUTO 0U
#define TVD_NTSC 1U
#define TVD_PAL 2U
#define TVD_SIGNAL_MODE TVD_NTSC

#define TVD_DISCARD_FRAME_COUNT 10

struct TVD_CONFIG {
	uint32_t color_format;
};

video_handle tvd_open(void);
int32_t tvd_close(video_handle handle);
int32_t tvd_config(video_handle handle, video_config_type type, void *config);
int32_t tvd_commit(
	video_handle handle, video_buffer_type type, video_buffer *buffer);
int32_t tvd_register_handler(
	void *handle, void (*buffer_done_handler)(void *usr_data, 
		video_buffer_type type, video_buffer *buffer), void *usr_data);
int32_t tvd_init(void);
int32_t tvd_uninit(void);

#endif
