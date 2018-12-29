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

#ifndef FASTRVC_MODULE_H
#define FASTRVC_MODULE_H

#include "FreeRTOS.h"
#include <stdint.h>
#include <tinysys_config.h>
#include <semphr.h>
#include "video_core.h"
#include "mt_reg_base.h"

/*
* log wrapper
*/
//#define FASTRVC_MODULE_DEBUG

#define LOG(fmt, args...) 	PRINTF_E("[%s:%d]" fmt, __func__, __LINE__, ##args)
#define LOGE(fmt, args...)	LOG(fmt, ##args)

#ifdef FASTRVC_MODULE_DEBUG
#define LOGW(fmt, args...)	LOG(fmt, ##args)
#define LOGI(fmt, args...)	LOG(fmt, ##args)
#define LOGD(fmt, args...)	LOG(fmt, ##args)
#else
#define LOGW(fmt, args...)
#define LOGI(fmt, args...)
#define LOGD(fmt, args...)
#endif

#ifdef CFG_SYSTMR_SUPPORT
#include "mt_systmr.h"
#define LOGT(fmt, args...)	LOG("[%lld]" fmt, get_boot_time_us(), ##args)
#else
#define LOGT(fmt, args...)	LOG(fmt, ##args)
#endif

/*
* extern from scp_ipi.c
*/
extern void* noncached_mem_alloc(uint32_t size);
//extern noncached_mem_free(void *p);


/*
* module interface
* be carefull, these interface is NOT thread safe
*/
typedef enum module_status {
	MODULE_STATUS_NONE = 0,
	MODULE_STATUS_INIT,
	MODULE_STATUS_RUN,
	MODULE_STATUS_STOP,
} module_status;

typedef struct module {
	struct module *prev, *next;
	struct module_driver *driver;
	void 		*driver_data;
	char		*name;
	module_status 	status;
	SemaphoreHandle_t mutex;
	int32_t		dup_flag;
} module;

typedef struct module_driver {
	int32_t (*config)(module *, uint32_t, void *);
	int32_t (*init)(module *);
	void (*deinit)(module *);
	int32_t (*start)(module *);
	int32_t (*stop)(module *);
	int32_t (*commit)(module *, video_buffer_type, video_buffer *);
} module_driver;

module* module_create(int32_t (*create_callback)(module *));
module* module_dup(module *mod);
void module_destroy(module *mod);
void module_register_driver(module *mod, module_driver *driver, void *driver_data);
int32_t module_config(module *mod, uint32_t type, void *config);
int32_t module_start(module *mod);
int32_t module_stop(module *mod);
int32_t module_push_buffer(module *mod, video_buffer_type type, video_buffer *buffer);
int32_t module_prev_push_buffer(module *mod, video_buffer_type type, video_buffer *buffer);
int32_t module_next_push_buffer(module *mod, video_buffer_type type, video_buffer *buffer);

static inline void* module_driver_data(module *mod)
{
	return mod->driver_data;
}

static inline void module_set_name(module *mod, char *name)
{
	mod->name = name;
}

/*
* link interface
*/
typedef struct link_path {
	struct module 	*head;
	struct module 	*tail;
	SemaphoreHandle_t mutex;
} link_path;

link_path* link_add(link_path *ln, module *mod);
link_path* link_remove(link_path *ln, module *mod);
void link_destroy(link_path *ln);
int32_t link_init_modules(link_path *ln);
int32_t link_deinit_modules(link_path *ln);
int32_t link_start_modules(link_path *ln);
int32_t link_stop_modules(link_path *ln);

/*
* modules declaration
*/
typedef struct base_config {
	char		*dev_name;
	uint32_t	buffer_count;
	uint32_t	buffer_size;

	video_format	input_format;
	video_rect	input_rect;
	video_format	output_format;
	video_rect	output_rect;
} base_config;
int32_t base_module_create(module *mod);

typedef struct disp_config {
	char		*dev_name;
	video_format	input_format;
	video_rect	input_rect;
#ifdef CFG_FASTLOGO_SUPPORT
	video_format	logo_format;
#endif
} disp_config;
int32_t disp_module_create(module *mod);

typedef struct ovl_config {
	char		*dev_name;
	uint32_t	buffer_count;
	uint32_t	buffer_size;

	video_format	input_format;
	video_rect	input_rect;
	video_format	output_format;
	video_rect	output_rect;
#ifdef CFG_FASTRVC_GUIDELINE_SUPPORT
	video_format	guideline_format;
	video_rect	guideline_rect;
#endif
} ovl_config;
int32_t ovl_module_create(module *mod);

#endif //FASTRVC_MODULE_H