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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tinysys_config.h>
#include "video_core.h"
#include "mt_printf.h"

#ifdef CFG_SYSTMR_SUPPORT
#include "mt_systmr.h"
#endif

typedef struct video_device {
	char 		*name;
	device_driver 	*driver;
	video_handle 	handle;
} video_device;

/*
* device driver declaration
*/
#ifdef CFG_VIRTUAL_CAMERA_SUPPORT
#include "sample_virtual_camera.h"
static device_driver virtual_camera_driver = {
	.open = virtual_camera_open,
	.close = virtual_camera_close,
	.config = virtual_camera_config,
	.commit = virtual_camera_commit,
	.register_handler = virtual_camera_register_handler,
};
#endif

#ifdef CFG_VIRTUAL_DISPLAY_SUPPORT
#include "sample_virtual_display.h"
static device_driver virtual_display_driver = {
	.open = virtual_display_open,
	.close = virtual_display_close,
	.config = virtual_display_config,
	.commit = virtual_display_commit,
	.register_handler = virtual_display_register_handler,
};
#endif

#ifdef CFG_VIRTUAL_M2M_SUPPORT
#include "sample_virtual_m2m.h"
static device_driver virtual_m2m_driver = {
	.open = virtual_m2m_open,
	.close = virtual_m2m_close,
	.config = virtual_m2m_config,
	.commit = virtual_m2m_commit,
	.register_handler = virtual_m2m_register_handler,
};
#endif

#ifdef CFG_DISPLAY_SUPPORT
#include "mtk_display_drv.h"
static device_driver display_driver = {
	.open = display_core_open,
	.close = display_core_close,
	.config = display_core_config,
	.commit = display_core_commit,
	.register_handler = display_core_register_handler,
};
#endif

#ifdef CFG_MDP_SUPPORT
#include "mdp.h"
static device_driver mdp_driver = {
	.open = mdp_open,
	.close = mdp_close,
	.config = mdp_config,
	.commit = mdp_commit,
	.register_handler = mdp_register_handler,
};
#endif

#ifdef CFG_OVERLAY2_SUPPORT
#include "ovl2.h"
static device_driver ovl2_driver = {
	.open = ovl2_open,
	.close = ovl2_close,
	.config = ovl2_config,
	.commit = ovl2_commit,
	.register_handler = ovl2_register_handler,
};
#endif

#ifdef CFG_TVD_SUPPORT
#include "tvd_if.h"
static device_driver tvd_driver = {
	.open = tvd_open,
	.close = tvd_close,
	.config = tvd_config,
	.commit = tvd_commit,
	.register_handler = tvd_register_handler,
};
#endif

#ifdef CFG_MTK_NR_SUPPORT
#include "mtk_nr_main.h"
static device_driver nr_driver = {
	.open = mtk_nr_open,
	.close = mtk_nr_close,
	.config = mtk_nr_config,
	.commit = mtk_nr_commit,
	.register_handler = mtk_nr_register_handler,
};
#endif


/*
* device list
*/
static video_device device_list[] = {

#ifdef CFG_DISPLAY_SUPPORT
	{
		.name = DISPLAY_DEV_NAME,
		.driver = &display_driver,
	},
#endif
#ifdef CFG_MDP_SUPPORT
	{
		.name = MDP_DEV_NAME,
		.driver = &mdp_driver,
	},
#endif
#ifdef CFG_MTK_NR_SUPPORT
	{
		.name = NR_DEV_NAME,
		.driver = &nr_driver,
	},
#endif
#ifdef CFG_TVD_SUPPORT
	{
		.name = TVD_DEV_NAME,
		.driver = &tvd_driver,
	},
#endif
#ifdef CFG_OVERLAY2_SUPPORT
	{
		.name = OVL2_DEV_NAME,
		.driver = &ovl2_driver,
	},
#endif
#ifdef CFG_VIRTUAL_CAMERA_SUPPORT
	{
		.name = VIRTUAL_CAMERA_DEV_NAME,
		.driver = &virtual_camera_driver,
	},
#endif
#ifdef CFG_VIRTUAL_DISPLAY_SUPPORT
	{
		.name = VIRTUAL_DISPLAY_DEV_NAME,
		.driver = &virtual_display_driver,
	},
#endif
#ifdef CFG_VIRTUAL_M2M_SUPPORT
	{
		.name = VIRTUAL_M2M_DEV_NAME,
		.driver = &virtual_m2m_driver,
	},
#endif

	{0},
};

void* video_core_open(char *name)
{
	video_device *dev, *handle;
	if (!name)
		return NULL;

#ifdef VIDEO_CORE_DEBUG
	PRINTF_E("video-core: open(%s)\n", name);
#endif

	for (dev = device_list; dev->name != NULL; dev++) {
		if (strcmp(name, dev->name) == 0) {
			if (!dev->driver->open)
				return NULL;

			handle = (video_device*)malloc(sizeof(*handle));
			if (!handle)
				return NULL;

			handle->handle = dev->driver->open();
			if (!handle->handle) {
				free(handle);
				return NULL;
			}

			handle->name = dev->name;
			handle->driver = dev->driver;
			return handle;
		}
	}

	return NULL;
}

int32_t video_core_close(void *handle)
{
	int32_t ret;
	video_device *dev = (video_device*)handle;

#ifdef VIDEO_CORE_DEBUG
	if (!dev || !dev->driver || !dev->driver->close)
		return -1;
	PRINTF_E("video-core: close(%s)\n", dev->name);
#endif

	ret = dev->driver->close(dev->handle);
	free(handle);
	return ret;
}

int32_t video_core_config(void *handle, video_config_type type, void *config)
{
	video_device *dev = (video_device*)handle;

#ifdef VIDEO_CORE_DEBUG
	if (!dev || !dev->driver || !dev->driver->config)
		return -1;

	video_format *fmt;
	video_rect *rect;

	switch (type) {
	case VIDEO_CONFIG_FORMAT:
		fmt = (video_format*)config;
		PRINTF_E("video-core: config(%s) VIDEO_CONFIG_FORMAT\n", dev->name);
		PRINTF_E("            type(%d) fourcc(%ld) width(%ld) height(%ld)\n",
				fmt->type, fmt->fourcc, fmt->width, fmt->height);
		break;
	case VIDEO_CONFIG_CROP:
	case VIDEO_CONFIG_AREA:
		rect = (video_rect*)config;
		PRINTF_E("video-core: config(%s) VIDEO_CONFIG_CROP/VIDEO_CONFIG_AREA\n", dev->name);
		PRINTF_E("            type(%d) left(%ld) top(%ld) width(%ld) height(%ld)\n",
				rect->type, rect->left, rect->top, rect->width, rect->height);
		break;
	default:
		PRINTF_E("video-core: video_config_type(%d) not support\n", type);
	}
#endif
	return dev->driver->config(dev->handle, type, config);
}

int32_t video_core_commit(void *handle, video_buffer_type type, video_buffer *buffer)
{
	video_device *dev = (video_device*)handle;

#ifdef VIDEO_CORE_DEBUG
	if (!dev || !dev->driver || !dev->driver->commit)
		return -1;
#endif

#ifdef CFG_SYSTMR_SUPPORT
	buffer->latency = get_boot_time_us();
#endif

#ifdef VIDEO_CORE_DEBUG
	PRINTF_E("video-core: commit(%s) at %lldus\n", dev->name, buffer->latency);
	PRINTF_E("            type(%d) buffer(%p) image(%p) timestamp(%lldus)\n",
			type, buffer, buffer->image, buffer->timestamp);
#endif
	return dev->driver->commit(dev->handle, type, buffer);
}

int32_t video_core_register_handler(void *handle, buffer_done_handler handler, void *usr_data)
{
	video_device *dev = (video_device*)handle;

#ifdef VIDEO_CORE_DEBUG
	if (!dev || !dev->driver || !dev->driver->register_handler)
		return -1;

	PRINTF_E("video-core: register(%s)\n", dev->name);
	PRINTF_E("            handler(%p) usr_data(%p)\n", handler, usr_data);
#endif
	return dev->driver->register_handler(dev->handle, handler, usr_data);
}