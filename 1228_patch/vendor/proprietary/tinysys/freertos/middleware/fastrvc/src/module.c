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

#include <stdlib.h>
#include <string.h>
#include "fastrvc/module.h"

module* module_create(int32_t (*create_callback)(module *))
{
	module *mod;
	if (!create_callback)
		return NULL;

	mod = (module*)malloc(sizeof(*mod));
	if (!mod) {
		LOGE("malloc fail\n");
		return NULL;
	}
	memset(mod, 0, sizeof(*mod));

	mod->mutex = xSemaphoreCreateMutex();
	if (mod->mutex == NULL) {
		LOGE("xSemaphoreCreateMuxtex error\n");
		free(mod);
		return NULL;
	}

	if ((create_callback(mod) != 0) || (mod->driver == NULL)) {
		free(mod);
		return NULL;
	}

	return mod;
}

/*
* module_dup()
*
*	duplicate a module, the new module share the same resource with old module
*	so be carefull, these modules can't be used concurrenctly, especially in multi thread
*
*	new module is limited to do module_init/module_deinit/module_config
*
* usecase:
*	use the same module to create a new link_path
*
*/
module* module_dup(module *mod)
{
	module *new_mod;

#ifdef FASTRVC_MODULE_DEBUG
	if (!mod || !mod->driver || !mod->driver_data) {
		LOGE("invalid module\n");
		return NULL;
	}
#endif
	new_mod = (module*)malloc(sizeof(*mod));
	if (!new_mod) {
		LOGE("malloc fail\n");
		return NULL;
	}
	memcpy(new_mod, mod, sizeof(*mod));

	new_mod->prev = new_mod->next = NULL;
	new_mod->dup_flag = 1;
	return new_mod;
}

void module_destroy(module *mod)
{
	if (mod->driver_data)
		free(mod->driver_data);
	memset(mod, 0, sizeof(*mod));
	free(mod);
}

void module_register_driver(module *mod, module_driver *driver, void *driver_data)
{
	mod->driver = driver;
	mod->driver_data = driver_data;
}

int32_t module_config(module *mod, uint32_t type, void *config)
{
#ifdef FASTRVC_MODULE_DEBUG
	if (!mod || !mod->driver || !mod->driver->config) {
		LOGE("invalid module\n");
		return -1;
	}
#endif
	if (mod->dup_flag)
		return 0;

	return mod->driver->config(mod, type, config);
}

int32_t module_init(module *mod)
{
	int32_t err = -1;
#ifdef FASTRVC_MODULE_DEBUG
	if (!mod || !mod->driver || !mod->driver->init) {
		LOGE("invalid module\n");
		return -1;
	}
#endif
	if (mod->dup_flag)
		return 0;

	xSemaphoreTake(mod->mutex, portMAX_DELAY);
	err = mod->driver->init(mod);
	if (err == 0)
		mod->status = MODULE_STATUS_INIT;
	xSemaphoreGive(mod->mutex);

	return err;
}

void module_deinit(module *mod)
{
#ifdef FASTRVC_MODULE_DEBUG
	if (!mod || !mod->driver || !mod->driver->deinit) {
		LOGE("invalid module\n");
		return;
	}
#endif
	if (mod->dup_flag)
		return;

	xSemaphoreTake(mod->mutex, portMAX_DELAY);
	mod->driver->deinit(mod);
	mod->status = MODULE_STATUS_NONE;
	xSemaphoreGive(mod->mutex);
}

int32_t module_start(module *mod)
{
	int32_t err = -1;
#ifdef FASTRVC_MODULE_DEBUG
	if (!mod || !mod->driver || !mod->driver->start) {
		LOGE("invalid module\n");
		return -1;
	}
#endif
	xSemaphoreTake(mod->mutex, portMAX_DELAY);
	err = mod->driver->start(mod);
	if (err == 0)
		mod->status = MODULE_STATUS_RUN;
	xSemaphoreGive(mod->mutex);

	return err;
}

int32_t module_stop(module *mod)
{
	int32_t err = -1;
#ifdef FASTRVC_MODULE_DEBUG
	if (!mod || !mod->driver || !mod->driver->stop) {
		LOGE("invalid module\n");
		return -1;
	}
#endif
	xSemaphoreTake(mod->mutex, portMAX_DELAY);
	err = mod->driver->stop(mod);
	if (err == 0)
		mod->status = MODULE_STATUS_STOP;
	xSemaphoreGive(mod->mutex);

	return err;
}

int32_t module_push_buffer(module *mod, video_buffer_type type, video_buffer *buffer)
{
	int32_t err = -1;
#ifdef FASTRVC_MODULE_DEBUG
	if (!mod || !mod->driver || !mod->driver->commit) {
		LOGE("invalid module\n");
		return -1;
	}
#endif
	if (pdPASS != xSemaphoreTake(mod->mutex, pdMS_TO_TICKS(100))) {
		LOGE("%s: get lock fail\n", mod->name);
		return -1;
	}

	if (mod->status == MODULE_STATUS_RUN) {
		err = mod->driver->commit(mod, type, buffer);
	} else if (mod->status == MODULE_STATUS_STOP) {
		/*
		* it's ok in stop flow
		*/
		err = 0;
		LOGW("%s: module is not running\n", mod->name);
	}
	xSemaphoreGive(mod->mutex);

	return err;
}

int32_t module_prev_push_buffer(module *mod, video_buffer_type type, video_buffer *buffer)
{
	int32_t err;

#ifdef FASTRVC_MODULE_DEBUG
	if (!mod->prev) {
		LOGE("can't find previous module\n");
		return -1;
	}
#endif
	err = module_push_buffer(mod->prev, type, buffer);
	if (err != 0)
		LOGE("%s: push buffer to previous module error %ld\n", mod->name, err);
	return err;
}

int32_t module_next_push_buffer(module *mod, video_buffer_type type, video_buffer *buffer)
{
	int32_t err;

#ifdef FASTRVC_MODULE_DEBUG
	if (!mod->next) {
		LOGE("can't find next module\n");
		return -1;
	}
#endif
	err = module_push_buffer(mod->next, type, buffer);
	if (err != 0)
		LOGE("%s: push buffer to next module error %ld\n", mod->name, err);
	return err;
}

link_path* link_add(link_path *ln, module *mod)
{
	if (ln == NULL) {
		ln = (link_path*)malloc(sizeof(*ln));
		if (!ln)
			return NULL;
		memset(ln, 0, sizeof(*ln));

		ln->mutex = xSemaphoreCreateMutex();
		if (ln->mutex == NULL) {
			LOGE("xSemaphoreCreateMuxtex error\n");
			free(ln);
			return NULL;
		}
	}

	xSemaphoreTake(ln->mutex, portMAX_DELAY);
	if (ln->head == NULL) {
		ln->head = mod;
		ln->tail = mod;
		mod->prev = mod->next = NULL;
	} else {
		ln->tail->next = mod;
		mod->prev = ln->tail;
		mod->next = NULL;
		ln->tail = mod;
	}
	xSemaphoreGive(ln->mutex);

	return ln;
}

link_path* link_remove(link_path *ln, module *mod)
{
	xSemaphoreTake(ln->mutex, portMAX_DELAY);
	if (mod == ln->head) {
		ln->head = mod->next;
		mod->next->prev = NULL;
	} else if (mod == ln->tail) {
		ln->tail = mod->prev;
		mod->prev->next = NULL;
	} else {
		mod->prev->next = mod->next;
		mod->next->prev = mod->prev;
	}
	xSemaphoreGive(ln->mutex);

	module_destroy(mod);
	return ln;
}

void link_destroy(link_path *ln)
{
	module *mod;

	xSemaphoreTake(ln->mutex, portMAX_DELAY);
	mod = ln->head;
	while (mod) {
		module_destroy(mod);
		mod = mod->next;
	}

	memset(ln, 0, sizeof(*ln));
	free(ln);
	xSemaphoreGive(ln->mutex);
}

int32_t link_init_modules(link_path *ln)
{
	module *mod;
	int32_t err;

	xSemaphoreTake(ln->mutex, portMAX_DELAY);
	mod = ln->head;
	while (mod) {
#ifdef FASTRVC_MODULE_DEBUG
		if (!mod->driver || !mod->driver->init) {
			xSemaphoreGive(ln->mutex);
			LOGE("%s: driver not implement\n", mod->name);
			return -1;
		}
#endif
		LOGD("%s: init begin\n", mod->name);
		err = module_init(mod);
		LOGD("%s: init end %ld\n",  mod->name, err);

		if (err != 0) {
			xSemaphoreGive(ln->mutex);
			return err;
		}
		mod = mod->next;
	}
	xSemaphoreGive(ln->mutex);

	return 0;
}

int32_t link_deinit_modules(link_path *ln)
{
	module *mod;

	xSemaphoreTake(ln->mutex, portMAX_DELAY);
	mod = ln->head;
	while (mod) {
#ifdef FASTRVC_MODULE_DEBUG
		if (!mod->driver || !mod->driver->deinit) {
			xSemaphoreGive(ln->mutex);
			LOGE("%s: driver not implement\n", mod->name);
			return -1;
		}
#endif
		LOGD("%s: deinit begin\n", mod->name);
		module_deinit(mod);
		LOGD("%s: deinit end\n",  mod->name);

		mod = mod->next;
	}

	xSemaphoreGive(ln->mutex);
	return 0;
}

int32_t link_start_modules(link_path *ln)
{
	module *mod;
	int32_t err;

	xSemaphoreTake(ln->mutex, portMAX_DELAY);
	mod = ln->tail;
	while (mod) {
#ifdef FASTRVC_MODULE_DEBUG
		if (!mod->driver || !mod->driver->start) {
			xSemaphoreGive(ln->mutex);
			LOGE("%s: driver not implement\n", mod->name);
			return -1;
		}
#endif
		LOGD("%s: start begin\n", mod->name);
		err = module_start(mod);
		LOGD("%s: start end %ld\n",  mod->name, err);

		if (err != 0) {
			xSemaphoreGive(ln->mutex);
			return err;
		}
		mod = mod->prev;
	}

	xSemaphoreGive(ln->mutex);
	return 0;
}

int32_t link_stop_modules(link_path *ln)
{
	module *mod;
	int32_t err;

	xSemaphoreTake(ln->mutex, portMAX_DELAY);
	mod = ln->head;
	while (mod) {
#ifdef FASTRVC_MODULE_DEBUG
		if (!mod->driver || !mod->driver->stop) {
			xSemaphoreGive(ln->mutex);
			LOGE("%s: driver not implement\n", mod->name);
			return -1;
		}
#endif
		LOGD("%s: stop begin\n", mod->name);
		err = module_stop(mod);
		LOGD("%s: stop end %ld\n",  mod->name, err);

		if (err != 0) {
			xSemaphoreGive(ln->mutex);
			return err;
		}
		mod = mod->next;
	}

	xSemaphoreGive(ln->mutex);
	return 0;
}