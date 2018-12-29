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
#include <stdlib.h>
#include <string.h>
#include <task.h>
#include <tinysys_config.h>
#include "fastrvc/module.h"
#include "fastrvc_setting.h"
#include "gpio.h"

#define FASTRVC_STACK_SIZE 400
#define FASTRVC_TASK_PRIORITY 0

void reverse_gear_init();
uint32_t reverse_gear_on();

#if defined(CFG_VIRTUAL_CAMERA_SUPPORT) || defined(CFG_TVD_SUPPORT)
static base_config camera_cfg = {
#ifdef CFG_TVD_SUPPORT
	.dev_name	= TVD_DEV_NAME,
#else
	.dev_name	= VIRTUAL_CAMERA_DEV_NAME,
#endif
	.buffer_count 	= 3,
	.buffer_size	= CAMERA_IMAGE_SIZE,
	.input_format	= {0},
	.output_format	= {
		.type 	= VIDEO_BUFFER_OUTPUT,
		.fourcc	= CAMERA_PIX_FMT,
		.width	= CAMERA_RESOLUTION_WIDTH,
		.height	= CAMERA_RESOLUTION_HEIGHT,
	},
	.input_rect	= {0},
	.output_rect	= {0},
};
#endif

#if defined(CFG_DISPLAY_SUPPORT) || defined(CFG_VIRTUAL_DISPLAY_SUPPORT)
static disp_config disp_cfg = {
#ifdef CFG_DISPLAY_SUPPORT
	.dev_name	= DISPLAY_DEV_NAME,
#else
	.dev_name	= VIRTUAL_DISPLAY_DEV_NAME,
#endif
	.input_format	= {
		.type 	= OVL_BUFFER_INPUT_3,
		.fourcc	= DISPLAY_OVL_FMT,
		.width	= DISPLAY_RESOLUTION_WIDTH,
		.height	= DISPLAY_RESOLUTION_HEIGHT,
	},
	.input_rect	= {
		.type	= OVL_BUFFER_INPUT_3,
		.left	= 0,
		.top	= 0,
		.width	= DISPLAY_RESOLUTION_WIDTH,
		.height	= DISPLAY_RESOLUTION_HEIGHT,
	},
#ifdef CFG_FASTLOGO_SUPPORT
	.logo_format = {
		.type 	= OVL_BUFFER_INPUT_3,
		.fourcc	= FASTLOGO_PIX_FMT,
		.width	= DISPLAY_RESOLUTION_WIDTH,
		.height	= DISPLAY_RESOLUTION_HEIGHT,
	},
#endif
};
#endif

#if defined(CFG_MDP_SUPPORT) || defined(CFG_VIRTUAL_M2M_SUPPORT)
static base_config mdp_cfg = {
#ifdef CFG_MDP_SUPPORT
	.dev_name	= MDP_DEV_NAME,
#else
	.dev_name	= VIRTUAL_M2M_DEV_NAME,
#endif
	.buffer_count 	= 3,
	.buffer_size	= DISPLAY_IMAGE_SIZE,
	.input_format	= {
		.type 	= VIDEO_BUFFER_INPUT,
		.fourcc	= CAMERA_PIX_FMT,
		.width	= CAMERA_RESOLUTION_WIDTH,
		.height	= CAMERA_RESOLUTION_HEIGHT,
	},
	.output_format	= {
		.type 	= VIDEO_BUFFER_OUTPUT,
		.fourcc	= DISPLAY_PIX_FMT,
		.width	= DISPLAY_RESOLUTION_WIDTH,
		.height	= DISPLAY_RESOLUTION_HEIGHT,
	},
	.input_rect	= {
		.type	= VIDEO_BUFFER_INPUT,
		.left	= 0,
		.top	= 0,
		.width	= CAMERA_RESOLUTION_WIDTH,
		.height	= CAMERA_RESOLUTION_HEIGHT,
	},
	.output_rect	= {
		.type	= VIDEO_BUFFER_OUTPUT,
		.left	= 0,
		.top	= 0,
		.width	= DISPLAY_RESOLUTION_WIDTH,
		.height	= DISPLAY_RESOLUTION_HEIGHT,
	},
};
#endif

#if defined(CFG_OVERLAY2_SUPPORT)
static ovl_config ovl2_cfg = {
	.dev_name	= OVL2_DEV_NAME,
	.buffer_count 	= 3,
	.buffer_size	= DISPLAY_IMAGE_SIZE,
	.input_format	= {
		.type 	= OVL_BUFFER_INPUT_0,
		.fourcc	= DISPLAY_PIX_FMT,
		.width	= DISPLAY_RESOLUTION_WIDTH,
		.height	= DISPLAY_RESOLUTION_HEIGHT,
	},
	.output_format	= {
		.type 	= OVL_BUFFER_OUTPUT,
		.fourcc	= DISPLAY_PIX_FMT,
		.width	= DISPLAY_RESOLUTION_WIDTH,
		.height	= DISPLAY_RESOLUTION_HEIGHT,
	},
	.input_rect	= {
		.type	= OVL_BUFFER_INPUT_0,
		.left	= 0,
		.top	= 0,
		.width	= DISPLAY_RESOLUTION_WIDTH,
		.height	= DISPLAY_RESOLUTION_HEIGHT,
	},
	.output_rect	= {
		.type	= OVL_BUFFER_OUTPUT,
		.left	= 0,
		.top	= 0,
		.width	= DISPLAY_RESOLUTION_WIDTH,
		.height	= DISPLAY_RESOLUTION_HEIGHT,
	},
#ifdef CFG_FASTRVC_GUIDELINE_SUPPORT
	.guideline_format	= {
		.type 	= OVL_BUFFER_INPUT_1,
		.fourcc	= GUIDELINE_PIX_FMT,
		.width	= DISPLAY_RESOLUTION_WIDTH,
		.height	= DISPLAY_RESOLUTION_HEIGHT,
	},
	.guideline_rect	= {
		.type	= OVL_BUFFER_INPUT_1,
		.left	= 0,
		.top	= 0,
		.width	= DISPLAY_RESOLUTION_WIDTH,
		.height	= DISPLAY_RESOLUTION_HEIGHT,
	},
#endif
};
#endif

#if defined(CFG_MTK_NR_SUPPORT)
static base_config nr_cfg = {
	.dev_name	= NR_DEV_NAME,
	.buffer_count 	= 3,
	.buffer_size	= CAMERA_IMAGE_SIZE,
	.input_format	= {
		.type 	= VIDEO_BUFFER_INPUT,
		.fourcc	= CAMERA_PIX_FMT,
		.width	= CAMERA_RESOLUTION_WIDTH,
		.height	= CAMERA_RESOLUTION_HEIGHT,
	},
	.output_format	= {
		.type 	= VIDEO_BUFFER_OUTPUT,
		.fourcc	= CAMERA_PIX_FMT,
		.width	= CAMERA_RESOLUTION_WIDTH,
		.height	= CAMERA_RESOLUTION_HEIGHT,
	},
	.input_rect	= {0},
	.output_rect	= {0},
};
#endif

/*
* main thread
*/
static void fastrvc_main_thread(void *args)
{
	LOGT("fastrvc_main_thread start\n");
	link_path *ln = NULL;

#if defined(CFG_VIRTUAL_CAMERA_SUPPORT) || defined(CFG_TVD_SUPPORT)
	module *module_camera = module_create(base_module_create);
	if (!module_camera) {
		LOGE("module_camera create fail\n");
		goto error_init;
	}

	module_config(module_camera, 0, &camera_cfg);
	ln = link_add(ln, module_camera);
#endif

#if defined(CFG_MTK_NR_SUPPORT)
	module *module_nr = module_create(base_module_create);
	if (!module_nr) {
		LOGE("module_nr create fail\n");
		goto error_init;
	}

	module_config(module_nr, 0, &nr_cfg);
	ln = link_add(ln, module_nr);
#endif

#if defined(CFG_MDP_SUPPORT) || defined(CFG_VIRTUAL_M2M_SUPPORT)
	module *module_mdp = module_create(base_module_create);
	if (!module_mdp) {
		LOGE("module_mdp create fail\n");
		goto error_init;
	}

	module_config(module_mdp, 0, &mdp_cfg);
	ln = link_add(ln, module_mdp);
#endif

#if defined(CFG_OVERLAY2_SUPPORT)
	module *module_ovl2 = module_create(ovl_module_create);
	if (!module_ovl2) {
		LOGE("module_ovl2 create fail\n");
		goto error_init;
	}

	module_config(module_ovl2, 0, &ovl2_cfg);
	ln = link_add(ln, module_ovl2);
#endif

#if defined(CFG_DISPLAY_SUPPORT) || defined(CFG_VIRTUAL_DISPLAY_SUPPORT)
	module *module_disp = module_create(disp_module_create);
	if (!module_disp) {
		LOGE("module_disp create fail\n");
		goto error_init;
	}

	module_config(module_disp, 0, &disp_cfg);
	ln = link_add(ln, module_disp);
#endif

#ifndef CFG_FASTLOGO_ONLY
	uint32_t reverse_gear_status;
	reverse_gear_init();
	reverse_gear_status = 0;
#endif

	if (link_init_modules(ln) != 0) {
		LOGE("link_init_modules fail\n");
		goto error_init;
	}
	LOGT("fastrvc module init done\n");

#ifdef CFG_FASTLOGO_SUPPORT
	module_start(module_disp);
#endif

#ifdef CFG_FASTLOGO_ONLY
	/*
	* extern from module_disp.c
	*/
	extern uint32_t logo_should_stop(module *mod);
	/*
	* wait for logo exit
	*/
	while (!logo_should_stop(module_disp))
		vTaskDelay(pdMS_TO_TICKS(100));
	module_stop(module_disp);
#else
	/*
	* main loop, will not break
	*/
	while (1) {
		LOGD("fastrvc_main_thread is running\n");

		if (reverse_gear_on()) {
			if (reverse_gear_status == 0) {
				LOGD("reverse gear on\n");
				if (link_start_modules(ln) != 0)
					LOGE("link_start_modules fail\n");
				else
					reverse_gear_status = 1;
			}
		} else {
			if (reverse_gear_status == 1) {
				LOGD("reverse gear off\n");
				link_stop_modules(ln);
				reverse_gear_status = 0;
			}
		}

		vTaskDelay(pdMS_TO_TICKS(500));
	}
#endif //CFG_FASTLOGO_ONLY

	link_deinit_modules(ln);
error_init:
	if (ln)
		link_destroy(ln);
	LOGI("fastrvc_main_thread exit\n");
	vTaskDelete(NULL);
}

void fastrvc_main()
{
	LOGT("fastrvc_main start\n");

	TaskHandle_t handle;
	if ( xTaskCreate(
		fastrvc_main_thread,
		"rvc_main",
		FASTRVC_STACK_SIZE,
		NULL,
		FASTRVC_TASK_PRIORITY,
		&handle
		) != pdPASS)
	{
		LOGE("fastrvc_main_thread create fail\n");
	}
}

void reverse_gear_init()
{
	gpio_set_mode(REVERSE_GEAR_GPIO, GPIO_MODE_GPIO);
	gpio_set_dir(REVERSE_GEAR_GPIO, GPIO_DIR_IN);
}

uint32_t reverse_gear_on()
{
	return (gpio_get_in(REVERSE_GEAR_GPIO) == GPIO_OUT_ZERO);
}
