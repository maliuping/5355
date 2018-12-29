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
#include "task.h"
#include <stdlib.h>
#include <string.h>
#include <tinysys_config.h>
#include "fastrvc_setting.h"
#include "fastrvc/module.h"

typedef struct disp_device {
	char		*name;
	void 		*handle;
	disp_config 	config;

	uint32_t	feed_buffer;
	video_buffer 	*last_buffer;
	video_buffer 	*current_buffer;
	video_buffer 	*cache_buffer;
#ifdef CFG_FASTLOGO_SUPPORT
	video_buffer	logo_buffer;
	uint32_t	logo_flag;
	uint32_t	logo_frame_count;
#endif
} disp_device;

#ifdef CFG_FASTLOGO_SUPPORT
#define LOGO_FLAG_START		0
#define LOGO_FLAG_STOP		1
#define LOGO_FLAG_STOPPED	2
uint32_t logo_should_stop(module *mod)
{
	disp_device *dev = (disp_device*)module_driver_data(mod);
	dev->logo_frame_count++;
	return ((dev->logo_flag != LOGO_FLAG_START) || (dev->logo_frame_count > 120));
}
#endif

void disp_buffer_done(void *usr_data, video_buffer_type type, video_buffer *buffer)
{
	disp_device *dev;
	module *mod = (module*)usr_data;

	if (!usr_data || !buffer)
		return;
	dev = (disp_device*)module_driver_data(mod);

	if (type == dev->config.input_format.type) {
#ifdef CFG_FASTLOGO_SUPPORT
		if (dev->logo_flag != LOGO_FLAG_STOPPED) {
			if (!logo_should_stop(mod)) {
				video_core_commit(dev->handle, dev->config.input_format.type, &(dev->logo_buffer));
				return;
			} else {
				video_buffer null_buffer = {0};
				if (dev->logo_flag == LOGO_FLAG_START) {
					/*
					* detect stop flag by REE, disbale layer by commit null buffer
					* otherwise, just let camera buffer flush out logo buffer
					*/
					video_core_commit(dev->handle, dev->config.input_format.type, &null_buffer);
				}

				dev->feed_buffer = 1;
				dev->logo_flag = LOGO_FLAG_STOPPED;
				LOGD("%s: logo stopped\n", dev->name);
				return;
			}
		}
#endif
		if (dev->last_buffer)
			module_prev_push_buffer(mod, VIDEO_BUFFER_OUTPUT, dev->last_buffer);

		dev->last_buffer = dev->current_buffer;
		dev->current_buffer = buffer;
		dev->feed_buffer = 1;
	} else {
		LOGE("%s: unknown buffer type %d\n", dev->name, type);
	}
}

int32_t module_disp_config(module *mod, uint32_t type, void *config)
{
	(void)type;

	disp_device *dev = (disp_device*)module_driver_data(mod);
	memcpy(&(dev->config), config, sizeof(dev->config));

	if (!dev->config.dev_name) {
		LOGE("dev_name can't be null\n");
		return -1;
	}

	dev->name = dev->config.dev_name;
	module_set_name(mod, dev->name);
	return 0;
}

int32_t module_disp_init(module *mod)
{
#ifdef CFG_FASTLOGO_SUPPORT
	disp_device *dev = (disp_device*)module_driver_data(mod);
	dev->logo_buffer.image = VA_TO_IOVA(FASTLOGO_ADDRESS);
	dev->logo_flag = LOGO_FLAG_START;

#ifdef FASTRVC_MODULE_DEBUG
	uint8_t *logo = (uint8_t*)FASTLOGO_ADDRESS;
	LOGD("dump logo start\n");
	for (int i = 0; i < 10; i ++) {
		LOGD("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
			*logo, *(logo+1), *(logo+2), *(logo+3),
			*(logo+4), *(logo+5), *(logo+6), *(logo+7),
			*(logo+8), *(logo+9), *(logo+10), *(logo+11),
			*(logo+12), *(logo+13), *(logo+14), *(logo+15));
		logo += 16;
	}
	LOGD("dump logo end\n");
#endif
#endif
	return 0;
}

void module_disp_deinit(module *mod)
{
	(void)mod;
}

int32_t module_disp_start(module *mod)
{
	int32_t ret;
	disp_device *dev = (disp_device*)module_driver_data(mod);

	if (dev->handle == NULL) {
		dev->handle = video_core_open(dev->name);
		if (!dev->handle) {
			LOGE("%s: device open fail\n", dev->config.dev_name ? dev->config.dev_name : "null");
			return -1;
		}
		video_core_register_handler(dev->handle, disp_buffer_done, mod);
	} /*else if (dev->logo_flag == LOGO_FLAG_START)*/
#ifdef CFG_FASTLOGO_SUPPORT
	else if (dev->logo_flag != LOGO_FLAG_STOPPED) {
			/*
			* set flag and wait for logo stop
			*/
			dev->logo_flag = LOGO_FLAG_STOP;
			while (dev->logo_flag != LOGO_FLAG_STOPPED)
				vTaskDelay(pdMS_TO_TICKS(1));
	}

	if (dev->logo_flag == LOGO_FLAG_START) {
		ret = video_core_config(dev->handle, VIDEO_CONFIG_FORMAT, &(dev->config.logo_format));
		if (ret != 0) {
			LOGE("%s: config input_format error\n", dev->name);
			goto err;
		}
	} else
#endif
	/*else*/ {
		ret = video_core_config(dev->handle, VIDEO_CONFIG_FORMAT, &(dev->config.input_format));
		if (ret != 0) {
			LOGE("%s: config input_format error\n", dev->name);
			goto err;
		}
	}

	ret = video_core_config(dev->handle, VIDEO_CONFIG_AREA, &(dev->config.input_rect));
	if (ret != 0) {
		LOGE("%s: config input_rect error\n", dev->name);
		goto err;
	}

	dev->feed_buffer = 1;

#ifdef CFG_FASTLOGO_SUPPORT
	if (dev->logo_flag == LOGO_FLAG_START) {
		dev->feed_buffer = 0;
		return video_core_commit(dev->handle, dev->config.input_format.type, &(dev->logo_buffer));
	}
#endif

	return 0;
err:
	video_core_close(dev->handle);
	dev->handle = NULL;
	return ret;
}

int32_t module_disp_stop(module *mod)
{
	int32_t ret;
	video_buffer null_buffer = {0};
	disp_device *dev = (disp_device*)module_driver_data(mod);

#ifdef CFG_FASTLOGO_SUPPORT
	if (dev->logo_flag != LOGO_FLAG_STOPPED) {
		/*
		* set flag and wait for logo stop
		*/
		dev->logo_flag = LOGO_FLAG_STOP;

		while (dev->logo_flag != LOGO_FLAG_STOPPED)
			vTaskDelay(pdMS_TO_TICKS(1));
	}
#endif
	/*
	* disable layer before close
	*/
	video_core_commit(dev->handle, dev->config.input_format.type, &null_buffer);

	ret = video_core_close(dev->handle);
	dev->handle = NULL;
	return ret;
}

int32_t module_disp_commit(module *mod, video_buffer_type type, video_buffer *buffer)
{
	disp_device *dev = (disp_device*)module_driver_data(mod);
	if (!buffer)
		return -1;

	if (type != VIDEO_BUFFER_INPUT) {
		LOGE("%s: error buffer type %d\n", dev->name, type);
		return -1;
	}

	if (!dev->feed_buffer) {
		LOGW("%s: can't feed new buffer, drop frame\n", dev->name);
		module_prev_push_buffer(mod, VIDEO_BUFFER_OUTPUT, buffer);
	} else {
		dev->feed_buffer = 0;
		return video_core_commit(dev->handle, dev->config.input_format.type, buffer);
	}
	return 0;
}


static module_driver module_disp = {
	.config = module_disp_config,
	.init = module_disp_init,
	.deinit = module_disp_deinit,
	.start = module_disp_start,
	.stop = module_disp_stop,
	.commit = module_disp_commit,
};

int32_t disp_module_create(module *mod)
{
	disp_device *dev = (disp_device*)malloc(sizeof(disp_device));
	if (!dev) {
		LOGE("malloc fail\n");
		return -1;
	}

	memset(dev, sizeof(*dev), 0);
	/*
	* drvier_data free by module_destroy
	*/
	module_register_driver(mod, &module_disp, dev);
	return 0;
}