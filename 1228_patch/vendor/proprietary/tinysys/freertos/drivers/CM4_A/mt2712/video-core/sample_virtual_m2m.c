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
#include "sample_virtual_m2m.h"

#ifdef CFG_SYSTMR_SUPPORT
#include "mt_systmr.h"
#endif

typedef struct m2m_device {
	void (*buffer_done_callback)(void *, video_buffer_type, video_buffer *);
	void *callback_data;

	uint32_t stop_flag;
	TaskHandle_t thread_handle;
	QueueHandle_t queue_input;
	QueueHandle_t queue_output;
} m2m_device;

typedef struct m2m_info {
	video_buffer *buffer;
	video_buffer_type type;
} m2m_info;

static void virtual_m2m_thread(void *args)
{
	m2m_device *dev = (m2m_device*)args;
	video_buffer *input_buffer;
	video_buffer *output_buffer;
	m2m_info in_info;
	m2m_info out_info;

	while (dev->stop_flag != 1) {
		while (xQueueReceive(dev->queue_input, &in_info, pdMS_TO_TICKS(100)) != pdPASS) {
			if (dev->stop_flag == 1)
				goto out;
		}
		while (xQueueReceive(dev->queue_output, &out_info, pdMS_TO_TICKS(100)) != pdPASS) {
			if (dev->stop_flag == 1)
				goto out;
		}

		//processing image
		vTaskDelay(pdMS_TO_TICKS(8));
		//processing done

		if (dev->stop_flag == 1)
			break;

		input_buffer = in_info.buffer;
		output_buffer = out_info.buffer;

#ifdef CFG_SYSTMR_SUPPORT
		output_buffer->timestamp = input_buffer->timestamp;
		output_buffer->latency = get_boot_time_us() - input_buffer->latency;
#endif
		dev->buffer_done_callback(dev->callback_data, out_info.type, output_buffer);
		dev->buffer_done_callback(dev->callback_data, in_info.type, input_buffer);
	}

out:
	dev->stop_flag = 2;
	vTaskDelete(NULL);
}

video_handle virtual_m2m_open()
{
	m2m_device *dev;

	dev = (m2m_device*)malloc(sizeof(*dev));
	memset(dev, 0, sizeof(*dev));

	dev->queue_input = xQueueCreate(5, sizeof(m2m_info));
	if (!dev->queue_input) {
		PRINTF_E("virtual_m2m: xQueueCreate error\n");
		goto error_1;
	}

	dev->queue_output = xQueueCreate(5, sizeof(m2m_info));
	if (!dev->queue_output) {
		PRINTF_E("virtual_m2m: xQueueCreate error\n");
		goto error_2;
	}

	dev->stop_flag = 0;
	if ( xTaskCreate(
		virtual_m2m_thread,
		"Vm2m",
		400,
		(void*)dev,
		0,
		&(dev->thread_handle)
		) != pdPASS) {
		PRINTF_E("virtual_m2m: xTaskCreate error\n");
		goto error_3;
	}

	return (video_handle)dev;

error_3:
	vQueueDelete(dev->queue_output);
	dev->queue_output = NULL;
error_2:
	vQueueDelete(dev->queue_input);
	dev->queue_input = NULL;
error_1:
	free(dev);
	return NULL;
}

int32_t virtual_m2m_close(video_handle handle)
{
	m2m_device *dev = (m2m_device*)handle;

	dev->stop_flag = 1;
	while (dev->stop_flag != 2)
		vTaskDelay(pdMS_TO_TICKS(10));

	if (dev->queue_input)
		vQueueDelete(dev->queue_input);
	if (dev->queue_output)
		vQueueDelete(dev->queue_output);

	memset(dev, 0, sizeof(*dev));
	free(dev);
	return 0;
}

int32_t virtual_m2m_config(video_handle handle, video_config_type type, void *config)
{
	(void)handle;
	(void)type;
	(void)config;
	return 0;
}

int32_t virtual_m2m_commit(video_handle handle, video_buffer_type type, video_buffer *buffer)
{
	m2m_device *dev = (m2m_device*)handle;
	m2m_info info;

	info.buffer = buffer;
	info.type = type;

	switch (type) {
	case VIDEO_BUFFER_INPUT:
	case OVL_BUFFER_INPUT_0:
	case OVL_BUFFER_INPUT_1:
	case OVL_BUFFER_INPUT_2:
	case OVL_BUFFER_INPUT_3:
#ifdef CFG_SYSTMR_SUPPORT
		buffer->latency = get_boot_time_us();
#endif
		if (xQueueSendToBack(dev->queue_input, &info, 0) != pdPASS) {
			PRINTF_E("virtual_m2m: queue_input fail\n");
			return -1;
		}
		break;
	case VIDEO_BUFFER_OUTPUT:
	case OVL_BUFFER_OUTPUT:
		if (xQueueSendToBack(dev->queue_output, &info, 0) != pdPASS) {
			PRINTF_E("virtual_m2m: queue_output fail\n");
			return -1;
		}
		break;
	default:
		PRINTF_E("virtual_m2m: type(%d) not support\n", type);
	}

	return 0;
}

int32_t virtual_m2m_register_handler(void *handle,
					void (*buffer_done_handler)(void *usr_data, video_buffer_type type, video_buffer *buffer),
					void *usr_data)
{
	m2m_device *dev = (m2m_device*)handle;
	dev->buffer_done_callback = buffer_done_handler;
	dev->callback_data = usr_data;
	return 0;
}