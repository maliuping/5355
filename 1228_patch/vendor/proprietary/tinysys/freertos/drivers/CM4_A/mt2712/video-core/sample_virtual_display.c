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

/*
* Author: Huangfei Xiao <huangfei.xiao@mediatek.com>
*/

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdlib.h>
#include <string.h>
#include "video_core.h"
#include "sample_virtual_display.h"

#ifdef CFG_SYSTMR_SUPPORT
#include "mt_systmr.h"
#endif

typedef struct display_device {
	void (*buffer_done_callback)(void *, video_buffer_type, video_buffer *);
	void *callback_data;

	uint32_t stop_flag;
	TaskHandle_t thread_handle;
	QueueHandle_t queue_handle;
} display_device;

typedef struct display_info {
	video_buffer *buffer;
	video_buffer_type type;
} display_info;

static void virtual_display_thread(void *args)
{
	display_device *dev = (display_device*)args;
	video_buffer *buffer;
	display_info info;

	while (dev->stop_flag != 1) {
		vTaskDelay(pdMS_TO_TICKS(17));
		if (xQueueReceive(dev->queue_handle, &info, 0) == pdPASS) {
			buffer = info.buffer;
#ifdef CFG_SYSTMR_SUPPORT
			buffer->latency = get_boot_time_us() - buffer->timestamp;
			PRINTF_D("virtual_display: latency(%lldus)\n", buffer->latency);
#endif
			dev->buffer_done_callback(dev->callback_data, info.type, info.buffer);
		}
	}
	dev->stop_flag = 2;
	vTaskDelete(NULL);
}

video_handle virtual_display_open()
{
	display_device *dev;

	dev = (display_device*)malloc(sizeof(*dev));
	memset(dev, 0, sizeof(*dev));

	dev->queue_handle = xQueueCreate(5, sizeof(display_info));
	if (!dev->queue_handle) {
		PRINTF_E("virtual_display: xQueueCreate error\n");
		goto error_1;
	}

	dev->stop_flag = 0;
	if ( xTaskCreate(
		virtual_display_thread,
		"Vdisp",
		400,
		(void*)dev,
		0,
		&(dev->thread_handle)
		) != pdPASS) {
		PRINTF_E("virtual_display: xTaskCreate error\n");
		goto error_2;
	}

	return (video_handle)dev;
error_2:
	vQueueDelete(dev->queue_handle);
	dev->queue_handle = NULL;
error_1:
	free(dev);
	return NULL;
}

int32_t virtual_display_close(video_handle handle)
{
	display_device *dev = (display_device*)handle;

	dev->stop_flag = 1;
	while (dev->stop_flag != 2)
		vTaskDelay(pdMS_TO_TICKS(1));
	//vTaskDelete(dev->thread_handle);

	if (dev->queue_handle)
		vQueueDelete(dev->queue_handle);

	memset(dev, 0, sizeof(*dev));
	free(handle);
	return 0;
}

int32_t virtual_display_config(video_handle handle, video_config_type type, void *config)
{
	(void)handle;
	(void)type;
	(void)config;

	return 0;
}

int32_t virtual_display_commit(video_handle handle, video_buffer_type type, video_buffer *buffer)
{
	display_device *dev = (display_device*)handle;
	display_info info;

	info.buffer = buffer;
	info.type = type;

	if (xQueueSendToBack(dev->queue_handle, &info, 0) != pdPASS) {
		PRINTF_E("virtual_display: %s fail\n", __func__);
		return -1;
	}
	return 0;
}

int32_t virtual_display_register_handler(void *handle,
					void (*buffer_done_handler)(void *usr_data, video_buffer_type type, video_buffer *buffer),
					void *usr_data)
{
	display_device *dev = (display_device*)handle;

	dev->buffer_done_callback = buffer_done_handler;
	dev->callback_data = usr_data;
	return 0;
}