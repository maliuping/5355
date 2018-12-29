/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
 /* Copyright Statement:
  *
  * This software/firmware and related documentation ("MediaTek Software") are
  * protected under relevant copyright laws. The information contained herein
  * is confidential and proprietary to MediaTek Inc. and/or its licensors.
  * Without the prior written permission of MediaTek inc. and/or its licensors,
  * any reproduction, modification, use or disclosure of MediaTek Software,
  * and information contained herein, in whole or in part, shall be strictly prohibited.
  */
 /* MediaTek Inc. (C) 2015. All rights reserved.
  *
  * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
  * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
  * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
  * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
  * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
  * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
  * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
  * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
  * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
  * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
  * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
  * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
  * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
  * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
  * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
  * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
  * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
  * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
  */

#ifndef MTK_DISPLAY_DDP_COMP_H
#define MTK_DISPLAY_DDP_COMP_H

#include <FreeRTOS.h>
#include "mtk_display_macro.h"

#define MTK_MAX_BPC		10
#define MTK_MIN_BPC		3

typedef enum mtk_display_ddp_comp_type {
	MTK_DISP_OVL,
	MTK_DISP_RDMA,
	MTK_DISP_WDMA,
	MTK_DISP_COLOR,
	MTK_DISP_AAL,
	MTK_DSI,
	MTK_DPI,
	MTK_LVDS,
	MTK_DISP_PWM,
	MTK_DISP_MUTEX,
	MTK_DISP_OD,
	MTK_DDP_COMP_TYPE_MAX,
}mtk_display_ddp_comp_type;

typedef enum mtk_display_ddp_comp_id {
	DDP_COMPONENT_OVL0,
	DDP_COMPONENT_COLOR0,
	DDP_COMPONENT_AAL,
	DDP_COMPONENT_OD,
	DDP_COMPONENT_RDMA0,
	DDP_COMPONENT_DPI0,
	DDP_COMPONENT_LVDS0,
	DDP_COMPONENT_WDMA0,
	DDP_COMPONENT_PWM0,
	DDP_COMPONENT_MUTEX,
	DDP_COMPONENT_ID_MAX,
}mtk_display_ddp_comp_id;

struct mtk_display_ddp_comp;
struct mtk_display_layer_config;

typedef struct mtk_display_ddp_comp_funcs {
	void (*config)(struct mtk_display_ddp_comp *comp, uint32_t w,
		       uint32_t h, uint32_t vrefresh,
		       uint32_t bpc, void *handle);
	void (*start)(struct mtk_display_ddp_comp *comp, void *handle);
	void (*stop)(struct mtk_display_ddp_comp *comp, void *handle);
	void (*enable_vblank)(struct mtk_display_ddp_comp *comp, void *handle);
	void (*disable_vblank)(struct mtk_display_ddp_comp *comp, void *handle);
	void (*layer_on)(struct mtk_display_ddp_comp *comp, uint32_t idx,
			 void *handle);
	void (*layer_off)(struct mtk_display_ddp_comp *comp, uint32_t idx,
			  void *handle);
	void (*layer_config)(struct mtk_display_ddp_comp *comp, uint32_t idx,
			     struct mtk_display_layer_config *config,
			     void *handle);
}mtk_display_ddp_comp_funcs;

typedef struct mtk_display_ddp_comp {
	mtk_display_ddp_comp_id id;
	const mtk_display_ddp_comp_funcs *funcs;
	struct cmdq_base *cmdq_base;
}mtk_display_ddp_comp;

static inline void mtk_display_ddp_comp_config(struct mtk_display_ddp_comp *comp,
				       uint32_t w, uint32_t h,
				       uint32_t vrefresh, uint32_t bpc,
				       void *handle)
{
	if (comp->funcs && comp->funcs->config)
		comp->funcs->config(comp, w, h, vrefresh, bpc, handle);
}

static inline void mtk_display_ddp_comp_start(struct mtk_display_ddp_comp *comp,
				      void *handle)
{
	if (comp->funcs && comp->funcs->start)
		comp->funcs->start(comp, handle);
}

static inline void mtk_display_ddp_comp_stop(mtk_display_ddp_comp *comp,
				     void *handle)
{
	if (comp->funcs && comp->funcs->stop)
		comp->funcs->stop(comp, handle);
}

static inline void mtk_display_ddp_comp_enable_vblank(struct mtk_display_ddp_comp *comp,
					      void *handle)
{
	if (comp->funcs && comp->funcs->enable_vblank)
		comp->funcs->enable_vblank(comp, handle);
}

static inline void mtk_display_ddp_comp_disable_vblank(struct mtk_display_ddp_comp *comp,
					       void *handle)
{
	if (comp->funcs && comp->funcs->disable_vblank)
		comp->funcs->disable_vblank(comp, handle);
}

static inline void mtk_display_ddp_comp_layer_on(mtk_display_ddp_comp *comp,
					 uint32_t idx,
					 void *handle)
{
	if (comp->funcs && comp->funcs->layer_on)
		comp->funcs->layer_on(comp, idx, handle);
}

static inline void mtk_display_ddp_comp_layer_off(mtk_display_ddp_comp *comp,
					  uint32_t idx,
					  void *handle)
{
	if (comp->funcs && comp->funcs->layer_off)
		comp->funcs->layer_off(comp, idx, handle);
}

static inline void mtk_display_ddp_comp_layer_config(mtk_display_ddp_comp *comp,
					     uint32_t idx,
					     mtk_display_layer_config* config,
					     void *handle)
{
	if (comp->funcs && comp->funcs->layer_config)
		comp->funcs->layer_config(comp, idx, config, handle);
}

void mtk_display_ddp_comp_init(mtk_display_ddp_comp *comp, mtk_display_ddp_comp_id comp_id);
void mtk_ddp_write(mtk_display_ddp_comp *comp, uint32_t value,
		   uint32_t offset, void *handle);
void mtk_ddp_write_relaxed(mtk_display_ddp_comp *comp, uint32_t value,
			uint32_t offset, void *handle);
void mtk_ddp_write_mask(mtk_display_ddp_comp *comp, uint32_t value,
			uint32_t offset, uint32_t mask, void *handle);
#endif /* MTK_DISPLAY_DDP_COMP_H */
