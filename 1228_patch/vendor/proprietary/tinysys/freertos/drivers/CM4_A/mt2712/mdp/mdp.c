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
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdlib.h>
#include <string.h>
#include "video_core.h"
#include "mdp.h"
#include "mtk_mdp_type.h"
#include "mtk-cmdq.h"
#include "mtk-cmdq-control.h"

typedef struct mdp_device {
	void (*buffer_done_callback)(void *, video_buffer_type, video_buffer *);
	void *callback_data;
	struct mdp_config mdp_cfg_src;
	struct mdp_config mdp_cfg_dst;

	uint32_t stop_flag;
	struct cmdq_client *client;
	struct cmdq_pkt *pkt;
	struct mdp_buffer src_buffer;/*used for frvc_process*/
	struct mdp_buffer dst_buffer;/*used for frvc_process*/
	TaskHandle_t thread_handle;
	QueueHandle_t queue_handle_in;
	QueueHandle_t queue_handle_out;
} mdp_device;

static void cmdqRecWrite(struct cmdq_pkt *h, u32 r, u32 v, u32 m)
{
	int ret;
	u32 offset;
	struct cmdq_base b;

	b.base = r & 0xffff0000u;
	b.subsys = (int)(u32)((r >> 16u) & 0x3u) + 1; /*cmdq_subsys_base_to_id*/
	offset = r & 0xffffu;

	ret = cmdq_pkt_write_mask(h, v, &b, offset, m);
	WARN_ON(ret);

}

static void cmdqRecPoll(struct cmdq_pkt *h, u32 r, u32 v, u32 m)
{
	int ret;
	u32 offset;
	struct cmdq_base b;

	b.base = r & 0xffff0000u;
	b.subsys = (int)(u32)((r >> 16u) & 0x3u) + 1; /*cmdq_subsys_base_to_id*/
	offset = r & 0xffffu;

	ret = cmdq_pkt_poll_mask(h, v, &b, offset, m);
	WARN_ON(ret);
}

static void cmdqRecWait(struct cmdq_pkt *h, enum cmdq_event e)
{
	int ret;

	ret = cmdq_pkt_wfe(h, e);
	WARN_ON(ret);
}

static void cmdqRecClear(struct cmdq_pkt *h, enum cmdq_event e)
{
	int ret;

	ret = cmdq_pkt_clear_event(h, e);
	WARN_ON(ret);
}

static void yuyv_1280x720_yuyv_1920x1080(struct cmdq_pkt *handle,
		struct mdp_buffer *input, struct mdp_buffer *output)
{
	cmdqRecWrite(handle, 0x14002008u, 0x00000001u, 0x00000001u);
	cmdqRecPoll(handle, 0x14002408u, 0x00000100u, 0x00000100u);
	cmdqRecWrite(handle, 0x14002008u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14005000u, 0x00010000u, 0x00010000u);
	cmdqRecWrite(handle, 0x14005000u, 0x00000000u, 0x00010000u);
	cmdqRecWrite(handle, 0x14005000u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x1400a100u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x1400a110u, 0x00000002u, 0x00000002u);
	cmdqRecWrite(handle, 0x1400a200u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a204u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a208u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a20cu, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a210u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a214u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a218u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a21cu, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a220u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a224u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a228u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a22cu, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a230u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a234u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a238u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a23cu, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a240u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a244u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14008010u, 0x00000001u, 0x00000001u);
	cmdqRecPoll(handle, 0x14008014u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14008010u, 0x00000000u, 0x00000001u);
	cmdqRecPoll(handle, 0x14008014u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14002270u, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14002028u, 0x00010070u, 0x00030070u);
	cmdqRecWrite(handle, 0x14002248u, 0x08080808u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002258u, 0x04040404u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002268u, 0x04040404u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002030u, 0x00080605u, 0x03c8fe0fu);
	cmdqRecWrite(handle, 0x14002f00u, (u32)input->addr_mva[0],
			0xffffffffu);
	cmdqRecWrite(handle, 0x14002f08u, (u32)input->addr_mva[1],
			0xffffffffu);
	cmdqRecWrite(handle, 0x14002f10u, (u32)input->addr_mva[2],
			0xffffffffu);
	cmdqRecWrite(handle, 0x14002100u, 0xfcfc2000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002108u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002110u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002060u, 0x00000a00u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14002090u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14002200u, 0x00000000u, 0x0f110000u);
	cmdqRecWrite(handle, 0x140020c8u, 0x00000000u, 0x0000ffffu);
	cmdqRecWrite(handle, 0x140020e0u, 0x00001200u, 0x0000ffffu);
	cmdqRecWrite(handle, 0x14002240u, 0x07000024u, 0x0700007fu);
	cmdqRecWrite(handle, 0x14002250u, 0x03000013u, 0x0700003fu);
	cmdqRecWrite(handle, 0x14002260u, 0x03000013u, 0x0700001fu);
	cmdqRecWrite(handle, 0x14005014u, 0x00005550u, 0x007fffffu);
	cmdqRecWrite(handle, 0x14005018u, 0x0000554bu, 0x007fffffu);
	cmdqRecWrite(handle, 0x14005004u, 0x037b0013u, 0x07ff1df3u);
	cmdqRecWrite(handle, 0x1400a0bcu, 0x00200000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0c0u, 0x00600040u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0c4u, 0x00a00080u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0c8u, 0x00e000c0u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0ccu, 0x01200100u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0d0u, 0x01600140u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0d4u, 0x01a00180u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0d8u, 0x01e001c0u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0dcu, 0x00000200u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0e4u, 0x00000000u, 0x0000007fu);
	cmdqRecWrite(handle, 0x1400a0e0u, 0x00800000u, 0x00ff0000u);
	cmdqRecWrite(handle, 0x1400a000u, 0x10000000u, 0x10000000u);
	cmdqRecWrite(handle, 0x1400a0bcu, 0x00200000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0c0u, 0x00600040u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0c4u, 0x00a00080u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0c8u, 0x00e000c0u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0ccu, 0x01200100u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0d0u, 0x01600140u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0d4u, 0x01a00180u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0d8u, 0x01e001c0u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0dcu, 0x00000200u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0e4u, 0x00000000u, 0x0000007fu);
	cmdqRecWrite(handle, 0x1400a0e0u, 0x00800000u, 0x00ff0000u);
	cmdqRecWrite(handle, 0x1400a000u, 0x10000000u, 0x10000000u);
	cmdqRecWrite(handle, 0x1400a000u, 0x80102010u, 0xefffffffu);
	cmdqRecWrite(handle, 0x1400a004u, 0x04020208u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a008u, 0x20300300u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a00cu, 0x8802020au, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a014u, 0x00183048u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a018u, 0x58687078u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a01cu, 0x807b7670u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a020u, 0x6b66605bu, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a024u, 0x56104000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a040u, 0x8c02730cu, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a044u, 0x7f1a1550u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a048u, 0x15410c20u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a04cu, 0xc4020618u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a050u, 0x60200ad0u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a054u, 0x0ac54c20u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a058u, 0x8c011420u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a05cu, 0xe03c1010u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a060u, 0x08010c20u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a110u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14008f00u, (u32)output->addr_mva[0],
		0xffffffffu);
	cmdqRecWrite(handle, 0x14008f04u, (u32)output->addr_mva[1],
		0xffffffffu);
	cmdqRecWrite(handle, 0x14008f08u, (u32)output->addr_mva[2],
		0xffffffffu);
	cmdqRecWrite(handle, 0x14008000u, 0x00005008u, 0xf131510fu);
	cmdqRecWrite(handle, 0x14008030u, 0x00000780u, 0x0000ffffu);
	cmdqRecWrite(handle, 0x1400803cu, 0x000003c0u, 0x0000ffffu);
	cmdqRecWrite(handle, 0x1400806cu, 0x000003c0u, 0x0000ffffu);
	cmdqRecWrite(handle, 0x14008004u, 0x07000000u, 0x1f000000u);
	cmdqRecWrite(handle, 0x14008084u, 0x00000000u, 0x000000f3u);
	cmdqRecWrite(handle, 0x14008054u, 0xff000000u, 0xff000000u);
	cmdqRecWrite(handle, 0x14008070u, 0x80000000u, 0x80000000u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000252u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f0u, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000400u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000001u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000002u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000001u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x1400802cu, 0x00000000u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008038u, 0x00000000u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008068u, 0x00000000u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008078u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008024u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008020u, 0x00000000u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008008u, 0x01e02000u, 0x1fff7f00u);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x1400a120u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a124u, 0x00000000u, 0x00ff00ffu);
	cmdqRecWrite(handle, 0x1400a128u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a064u, 0x01df0000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a068u, 0x04370000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400500cu, 0x02d00143u, 0xdfff1fffu);
	cmdqRecWrite(handle, 0x1400501cu, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005020u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005024u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005028u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x1400502cu, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005030u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005034u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005038u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005010u, 0x843801e0u, 0x9fff1fffu);
	cmdqRecWrite(handle, 0x14002000u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14002020u, 0x00001000u, 0x00001110u);
	cmdqRecWrite(handle, 0x140020c0u, 0x00000008u, 0x0000007fu);
	cmdqRecWrite(handle, 0x14002088u, 0x00288010u, 0x1ffff3ffu);
	cmdqRecWrite(handle, 0x140020d0u, 0x0160002cu, 0x3fff1fffu);
	cmdqRecWrite(handle, 0x140020d8u, 0x00000008u, 0x0000007fu);
	cmdqRecWrite(handle, 0x140020b8u, 0x00000000u, 0x1ffff3ffu);
	cmdqRecWrite(handle, 0x140020e8u, 0x00c00018u, 0x3fff1fffu);
	cmdqRecWrite(handle, 0x14002118u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002120u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002128u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002070u, 0x02d00144u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002078u, 0x02d00143u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002080u, 0x00000000u, 0x003f001fu);
	cmdqRecPoll(handle, 0x14002408u, 0x00000100u, 0x00000100u);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x140200e0u, 0x00000001u, 0x00000001u);
	cmdqRecPoll(handle, 0x140200e0u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000000u, 0x03ffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecWrite(handle, 0x14002000u, 0x00000000u, 0x00000001u);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000000u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000252u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f0u, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000400u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000001u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000002u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000001u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x1400802cu, 0x000001e0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008038u, 0x000000f0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008068u, 0x000000f0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008078u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008024u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008020u, 0x00000000u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008008u, 0x01e02000u, 0x1fff7f00u);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x1400a120u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a124u, 0x00000000u, 0x00ff00ffu);
	cmdqRecWrite(handle, 0x1400a128u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a064u, 0x01df0000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a068u, 0x04370000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400500cu, 0x02d00146u, 0xdfff1fffu);
	cmdqRecWrite(handle, 0x1400501cu, 0x00000002u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005020u, 0x00007600u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005024u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005028u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x1400502cu, 0x00000002u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005030u, 0x00007600u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005034u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005038u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005010u, 0x843801e0u, 0x9fff1fffu);
	cmdqRecWrite(handle, 0x14002000u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14002020u, 0x00001000u, 0x00001110u);
	cmdqRecWrite(handle, 0x140020c0u, 0x00000008u, 0x0000007fu);
	cmdqRecWrite(handle, 0x14002088u, 0x00290010u, 0x1ffff3ffu);
	cmdqRecWrite(handle, 0x140020d0u, 0x0160002cu, 0x3fff1fffu);
	cmdqRecWrite(handle, 0x140020d8u, 0x00000008u, 0x0000007fu);
	cmdqRecWrite(handle, 0x140020b8u, 0x00000000u, 0x1ffff3ffu);
	cmdqRecWrite(handle, 0x140020e8u, 0x00c00018u, 0x3fff1fffu);
	cmdqRecWrite(handle, 0x14002118u, 0x00000278u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002120u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002128u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002070u, 0x02d00148u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002078u, 0x02d00146u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002080u, 0x00000001u, 0x003f001fu);
	cmdqRecPoll(handle, 0x14002408u, 0x00000100u, 0x00000100u);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x140200e0u, 0x00000001u, 0x00000001u);
	cmdqRecPoll(handle, 0x140200e0u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000000u, 0x03ffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecWrite(handle, 0x14002000u, 0x00000000u, 0x00000001u);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000000u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000252u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f0u, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000400u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000001u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000002u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000001u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x1400802cu, 0x000003c0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008038u, 0x000001e0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008068u, 0x000001e0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008078u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008024u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008020u, 0x00000000u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008008u, 0x01e02000u, 0x1fff7f00u);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x1400a120u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a124u, 0x00000000u, 0x00ff00ffu);
	cmdqRecWrite(handle, 0x1400a128u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a064u, 0x01df0000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a068u, 0x04370000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400500cu, 0x02d00146u, 0xdfff1fffu);
	cmdqRecWrite(handle, 0x1400501cu, 0x00000002u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005020u, 0x00006c00u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005024u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005028u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x1400502cu, 0x00000002u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005030u, 0x00006c00u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005034u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005038u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005010u, 0x843801e0u, 0x9fff1fffu);
	cmdqRecWrite(handle, 0x14002000u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14002020u, 0x00001000u, 0x00001110u);
	cmdqRecWrite(handle, 0x140020c0u, 0x00000008u, 0x0000007fu);
	cmdqRecWrite(handle, 0x14002088u, 0x00290010u, 0x1ffff3ffu);
	cmdqRecWrite(handle, 0x140020d0u, 0x0160002cu, 0x3fff1fffu);
	cmdqRecWrite(handle, 0x140020d8u, 0x00000008u, 0x0000007fu);
	cmdqRecWrite(handle, 0x140020b8u, 0x00000000u, 0x1ffff3ffu);
	cmdqRecWrite(handle, 0x140020e8u, 0x00c00018u, 0x3fff1fffu);
	cmdqRecWrite(handle, 0x14002118u, 0x000004f8u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002120u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002128u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002070u, 0x02d00148u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002078u, 0x02d00146u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002080u, 0x00000001u, 0x003f001fu);
	cmdqRecPoll(handle, 0x14002408u, 0x00000100u, 0x00000100u);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x140200e0u, 0x00000001u, 0x00000001u);
	cmdqRecPoll(handle, 0x140200e0u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000000u, 0x03ffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecWrite(handle, 0x14002000u, 0x00000000u, 0x00000001u);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000000u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000252u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f0u, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000400u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000001u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000002u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000001u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x1400802cu, 0x000005a0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008038u, 0x000002d0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008068u, 0x000002d0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008078u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008024u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008020u, 0x00000000u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008008u, 0x01e02000u, 0x1fff7f00u);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x1400a120u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a124u, 0x00000000u, 0x00ff00ffu);
	cmdqRecWrite(handle, 0x1400a128u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a064u, 0x01df0000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a068u, 0x04370000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400500cu, 0x82d00143u, 0xdfff1fffu);
	cmdqRecWrite(handle, 0x1400501cu, 0x00000002u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005020u, 0x00006200u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005024u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005028u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x1400502cu, 0x00000002u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005030u, 0x00006200u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005034u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005038u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005010u, 0x043801e0u, 0x9fff1fffu);
	cmdqRecWrite(handle, 0x14002000u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14002020u, 0x00001000u, 0x00001110u);
	cmdqRecWrite(handle, 0x140020c0u, 0x00000008u, 0x0000007fu);
	cmdqRecWrite(handle, 0x14002088u, 0x00288010u, 0x1ffff3ffu);
	cmdqRecWrite(handle, 0x140020d0u, 0x0160002cu, 0x3fff1fffu);
	cmdqRecWrite(handle, 0x140020d8u, 0x00000008u, 0x0000007fu);
	cmdqRecWrite(handle, 0x140020b8u, 0x00000000u, 0x1ffff3ffu);
	cmdqRecWrite(handle, 0x140020e8u, 0x00c00018u, 0x3fff1fffu);
	cmdqRecWrite(handle, 0x14002118u, 0x00000778u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002120u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002128u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002070u, 0x02d00144u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002078u, 0x02d00143u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002080u, 0x00000001u, 0x003f001fu);
	cmdqRecPoll(handle, 0x14002408u, 0x00000100u, 0x00000100u);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x140200e0u, 0x00000001u, 0x00000001u);
	cmdqRecPoll(handle, 0x140200e0u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000000u, 0x03ffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecWrite(handle, 0x14002000u, 0x00000000u, 0x00000001u);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000000u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);

}

static void mt21_720x480_yuyv_1920x1080(struct cmdq_pkt *handle,
		struct mdp_buffer *input, struct mdp_buffer *output)
{
	cmdqRecWrite(handle, 0x14002008u, 0x00000001u, 0x00000001u);
	cmdqRecPoll(handle, 0x14002408u, 0x00000100u, 0x00000100u);
	cmdqRecWrite(handle, 0x14002008u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14005000u, 0x00010000u, 0x00010000u);
	cmdqRecWrite(handle, 0x14005000u, 0x00000000u, 0x00010000u);
	cmdqRecWrite(handle, 0x14005000u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x1400a100u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x1400a110u, 0x00000002u, 0x00000002u);
	cmdqRecWrite(handle, 0x1400a200u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a204u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a208u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a20cu, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a210u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a214u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a218u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a21cu, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a220u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a224u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a228u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a22cu, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a230u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a234u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a238u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a23cu, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a240u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a244u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14008010u, 0x00000001u, 0x00000001u);
	cmdqRecPoll(handle, 0x14008014u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14008010u, 0x00000000u, 0x00000001u);
	cmdqRecPoll(handle, 0x14008014u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14002270u, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14002028u, 0x00010070u, 0x00030070u);
	cmdqRecWrite(handle, 0x14002248u, 0x08080808u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002258u, 0x04040404u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002268u, 0x04040404u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002030u, 0x0008860cu, 0x03c8fe0fu);
	cmdqRecWrite(handle, 0x14002f00u, (u32)input->addr_mva[0],
			0xffffffffu);
	cmdqRecWrite(handle, 0x14002f08u, (u32)input->addr_mva[1],
			0xffffffffu);
	cmdqRecWrite(handle, 0x14002f10u, (u32)input->addr_mva[2],
			0xffffffffu);
	cmdqRecWrite(handle, 0x14002100u, 0xfcfd4600u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002108u, 0xfcf6a300u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002110u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002060u, 0x00005a00u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14002090u, 0x00002d00u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14002200u, 0x00000000u, 0x0f110000u);
	cmdqRecWrite(handle, 0x14002240u, 0x07000024u, 0x0700007fu);
	cmdqRecWrite(handle, 0x14002250u, 0x03000013u, 0x0700003fu);
	cmdqRecWrite(handle, 0x14002260u, 0x03000013u, 0x0700001fu);
	cmdqRecWrite(handle, 0x14005014u, 0x00002ff5u, 0x007fffffu);
	cmdqRecWrite(handle, 0x14005018u, 0x000038d3u, 0x007fffffu);
	cmdqRecWrite(handle, 0x14005004u, 0x037b0013u, 0x07ff1df3u);
	cmdqRecWrite(handle, 0x1400a0bcu, 0x00200000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0c0u, 0x00600040u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0c4u, 0x00a00080u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0c8u, 0x00e000c0u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0ccu, 0x01200100u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0d0u, 0x01600140u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0d4u, 0x01a00180u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0d8u, 0x01e001c0u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0dcu, 0x00000200u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0e4u, 0x00000000u, 0x0000007fu);
	cmdqRecWrite(handle, 0x1400a0e0u, 0x00800000u, 0x00ff0000u);
	cmdqRecWrite(handle, 0x1400a000u, 0x10000000u, 0x10000000u);
	cmdqRecWrite(handle, 0x1400a0bcu, 0x00200000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0c0u, 0x00600040u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0c4u, 0x00a00080u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0c8u, 0x00e000c0u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0ccu, 0x01200100u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0d0u, 0x01600140u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0d4u, 0x01a00180u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0d8u, 0x01e001c0u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0dcu, 0x00000200u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a0e4u, 0x00000000u, 0x0000007fu);
	cmdqRecWrite(handle, 0x1400a0e0u, 0x00800000u, 0x00ff0000u);
	cmdqRecWrite(handle, 0x1400a000u, 0x10000000u, 0x10000000u);
	cmdqRecWrite(handle, 0x1400a000u, 0x80102010u, 0xefffffffu);
	cmdqRecWrite(handle, 0x1400a004u, 0x04020208u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a008u, 0x20300300u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a00cu, 0x8802020au, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a014u, 0x00183048u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a018u, 0x58687078u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a01cu, 0x807b7670u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a020u, 0x6b66605bu, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a024u, 0x56104000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a040u, 0x8c02730cu, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a044u, 0x7f1a1550u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a048u, 0x15410c20u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a04cu, 0xc4020618u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a050u, 0x60200ad0u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a054u, 0x0ac54c20u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a058u, 0x8c011420u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a05cu, 0xe03c1010u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a060u, 0x08010c20u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a110u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14008f00u, (u32)output->addr_mva[0],
			0xffffffffu);
	cmdqRecWrite(handle, 0x14008f04u, (u32)output->addr_mva[1],
			0xffffffffu);
	cmdqRecWrite(handle, 0x14008f08u, (u32)output->addr_mva[2],
			0xffffffffu);
	cmdqRecWrite(handle, 0x14008000u, 0x00005008u, 0xf131510fu);
	cmdqRecWrite(handle, 0x14008030u, 0x00000780u, 0x0000ffffu);
	cmdqRecWrite(handle, 0x1400803cu, 0x000003c0u, 0x0000ffffu);
	cmdqRecWrite(handle, 0x1400806cu, 0x000003c0u, 0x0000ffffu);
	cmdqRecWrite(handle, 0x14008004u, 0x07000000u, 0x1f000000u);
	cmdqRecWrite(handle, 0x14008084u, 0x00000000u, 0x000000f3u);
	cmdqRecWrite(handle, 0x14008054u, 0xff000000u, 0xff000000u);
	cmdqRecWrite(handle, 0x14008070u, 0x80000000u, 0x80000000u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000252u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f0u, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000400u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000001u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000002u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000001u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x1400802cu, 0x00000000u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008038u, 0x00000000u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008068u, 0x00000000u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008078u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008024u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008020u, 0x00000000u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008008u, 0x01e02000u, 0x1fff7f00u);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x1400a120u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a124u, 0x00000000u, 0x00ff00ffu);
	cmdqRecWrite(handle, 0x1400a128u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a064u, 0x01df0000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a068u, 0x04370000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400500cu, 0x01e000b7u, 0xdfff1fffu);
	cmdqRecWrite(handle, 0x1400501cu, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005020u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005024u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005028u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x1400502cu, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005030u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005034u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005038u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005010u, 0x843801e0u, 0x9fff1fffu);
	cmdqRecWrite(handle, 0x14002000u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14002020u, 0x00000010u, 0x00001110u);
	cmdqRecWrite(handle, 0x14002118u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002120u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002128u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002070u, 0x01e000c0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002078u, 0x01e000b7u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002080u, 0x00000000u, 0x003f001fu);
	cmdqRecPoll(handle, 0x14002408u, 0x00000100u, 0x00000100u);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x140200e0u, 0x00000001u, 0x00000001u);
	cmdqRecPoll(handle, 0x140200e0u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000000u, 0x03ffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecWrite(handle, 0x14002000u, 0x00000000u, 0x00000001u);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000000u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000252u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f0u, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000400u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000001u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000002u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000001u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x1400802cu, 0x000001e0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008038u, 0x000000f0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008068u, 0x000000f0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008078u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008024u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008020u, 0x00000000u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008008u, 0x01e02000u, 0x1fff7f00u);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x1400a120u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a124u, 0x00000000u, 0x00ff00ffu);
	cmdqRecWrite(handle, 0x1400a128u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a064u, 0x01df0000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a068u, 0x04370000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400500cu, 0x01e000bau, 0xdfff1fffu);
	cmdqRecWrite(handle, 0x1400501cu, 0x00000002u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005020u, 0x00006b60u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005024u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005028u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x1400502cu, 0x00000002u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005030u, 0x00006b60u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005034u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005038u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005010u, 0x843801e0u, 0x9fff1fffu);
	cmdqRecWrite(handle, 0x14002000u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14002020u, 0x00000010u, 0x00001110u);
	cmdqRecWrite(handle, 0x14002118u, 0x00001600u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002120u, 0x00000b00u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002128u, 0x00000b00u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002070u, 0x01e000c0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002078u, 0x01e000bau, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002080u, 0x00000001u, 0x003f001fu);
	cmdqRecPoll(handle, 0x14002408u, 0x00000100u, 0x00000100u);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x140200e0u, 0x00000001u, 0x00000001u);
	cmdqRecPoll(handle, 0x140200e0u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000000u, 0x03ffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecWrite(handle, 0x14002000u, 0x00000000u, 0x00000001u);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000000u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000252u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f0u, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000400u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000001u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000002u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000001u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x1400802cu, 0x000003c0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008038u, 0x000001e0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008068u, 0x000001e0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008078u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008024u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008020u, 0x00000000u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008008u, 0x01e02000u, 0x1fff7f00u);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x1400a120u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a124u, 0x00000000u, 0x00ff00ffu);
	cmdqRecWrite(handle, 0x1400a128u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a064u, 0x01df0000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a068u, 0x04370000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400500cu, 0x01e000bau, 0xdfff1fffu);
	cmdqRecWrite(handle, 0x1400501cu, 0x00000002u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005020u, 0x000056c0u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005024u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005028u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x1400502cu, 0x00000002u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005030u, 0x000056c0u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005034u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005038u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005010u, 0x843801e0u, 0x9fff1fffu);
	cmdqRecWrite(handle, 0x14002000u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14002020u, 0x00000010u, 0x00001110u);
	cmdqRecWrite(handle, 0x14002118u, 0x00002c00u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002120u, 0x00001600u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002128u, 0x00001600u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002070u, 0x01e000c0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002078u, 0x01e000bau, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002080u, 0x00000005u, 0x003f001fu);
	cmdqRecPoll(handle, 0x14002408u, 0x00000100u, 0x00000100u);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x140200e0u, 0x00000001u, 0x00000001u);
	cmdqRecPoll(handle, 0x140200e0u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000000u, 0x03ffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecWrite(handle, 0x14002000u, 0x00000000u, 0x00000001u);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000000u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000252u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x140200f0u, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000400u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000001u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000002u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000001u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x1400802cu, 0x000005a0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008038u, 0x000002d0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008068u, 0x000002d0u, 0x0fffffffu);
	cmdqRecWrite(handle, 0x14008078u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008024u, 0x043801e0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008020u, 0x00000000u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14008008u, 0x01e02000u, 0x1fff7f00u);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x1400a120u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a124u, 0x00000000u, 0x00ff00ffu);
	cmdqRecWrite(handle, 0x1400a128u, 0x01e00438u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x1400a064u, 0x01df0000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400a068u, 0x04370000u, 0xffffffffu);
	cmdqRecWrite(handle, 0x1400500cu, 0x81e000b7u, 0xdfff1fffu);
	cmdqRecWrite(handle, 0x1400501cu, 0x00000002u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005020u, 0x00004220u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005024u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005028u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x1400502cu, 0x00000002u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005030u, 0x00004220u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005034u, 0x00000000u, 0x00001fffu);
	cmdqRecWrite(handle, 0x14005038u, 0x00000000u, 0x001fffffu);
	cmdqRecWrite(handle, 0x14005010u, 0x043801e0u, 0x9fff1fffu);
	cmdqRecWrite(handle, 0x14002000u, 0x00000001u, 0x00000001u);
	cmdqRecWrite(handle, 0x14002020u, 0x00000010u, 0x00001110u);
	cmdqRecWrite(handle, 0x14002118u, 0x00004200u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002120u, 0x00002100u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002128u, 0x00002100u, 0xffffffffu);
	cmdqRecWrite(handle, 0x14002070u, 0x01e000c0u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002078u, 0x01e000b7u, 0x1fff1fffu);
	cmdqRecWrite(handle, 0x14002080u, 0x00000009u, 0x003f001fu);
	cmdqRecPoll(handle, 0x14002408u, 0x00000100u, 0x00000100u);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecClear(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x140200e0u, 0x00000001u, 0x00000001u);
	cmdqRecPoll(handle, 0x140200e0u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x140200ecu, 0x00000000u, 0x03ffffffu);
	cmdqRecWrite(handle, 0x140200f4u, 0x00000000u, 0xffffffffu);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_RDMA1_EOF);
	cmdqRecWrite(handle, 0x14002000u, 0x00000000u, 0x00000001u);
	cmdqRecWait(handle, CMDQ_EVENT_MDP_WROT1_W_EOF);
	cmdqRecWrite(handle, 0x1400807cu, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000020u, 0x00000000u, 0x00000700u);
	cmdqRecWrite(handle, 0x1400002cu, 0x00000000u, 0x00000007u);
	cmdqRecWrite(handle, 0x14000034u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000060u, 0x00000000u, 0x00000003u);
	cmdqRecWrite(handle, 0x14000068u, 0x00000000u, 0x00000001u);
	cmdqRecWrite(handle, 0x14000080u, 0x00000000u, 0x00000003u);

}

int mtk_mdp_frvc_process(mdp_device *dev, struct video_buffer *buffer_in, struct video_buffer *buffer_out)
{
	struct mdp_config *src_cfg, *dst_cfg;
	struct cmdq_client *client;
	struct cmdq_pkt *pkt;
	int ret;
	int support = 1;
	struct mdp_buffer *src_buffer, *dst_buffer;
	uint32_t size;

	client = dev->client;
	pkt = dev->pkt;
	src_buffer = &dev->src_buffer;
	dst_buffer = &dev->dst_buffer;

	src_cfg = &dev->mdp_cfg_src;
	dst_cfg = &dev->mdp_cfg_dst;
	MDP_LOG_D("[MDP]%s, %d, mdp_cfg_src: %p, mdp_cfg_dst: %p", __func__, __LINE__, src_cfg, dst_cfg);

	MDP_LOG_D("[MDP]%s, %d, src_fourcc: %ld, dst_fourcc: %ld, src_crop_w: %d, src_crop_h: %d, dst_crop_w: %d, dst_crop_h: %d\n",
		__func__, __LINE__, src_cfg->format, dst_cfg->format, src_cfg->crop_w, src_cfg->crop_h, dst_cfg->crop_w, dst_cfg->crop_h);

	switch(src_cfg->format){
	case PIX_FMT_YUYV:
		size = src_cfg->h*src_cfg->w;
		src_buffer->addr_mva[0] = (unsigned long)buffer_in->image;
		src_buffer->addr_mva[1] = (unsigned long)(buffer_in->image + size);
		src_buffer->addr_mva[2] = (unsigned long)(buffer_in->image + (size>>1));

		src_buffer->plane_size[0] = size;
		src_buffer->plane_size[1] = size>>1;
		src_buffer->plane_size[2] = size>>1;
		src_buffer->plane_num = 3;
		break;
	case PIX_FMT_MT21:
		size = src_cfg->h*src_cfg->w;
		src_buffer->addr_mva[0] = (unsigned long)buffer_in->image;
		src_buffer->addr_mva[1] = (unsigned long)(buffer_in->image + size);
		src_buffer->addr_mva[2] = 0;

		src_buffer->plane_size[0] = size;
		src_buffer->plane_size[1] = 0;
		src_buffer->plane_size[2] = 0;
		src_buffer->plane_num = 3;
		break;
	}

	size = dst_cfg->h*dst_cfg->w;
	dst_buffer->addr_mva[0] = (unsigned long)buffer_out->image;
	dst_buffer->addr_mva[1] = (unsigned long)(buffer_out->image + size);
	dst_buffer->addr_mva[2] = (unsigned long)(buffer_out->image + (size>>1));

	dst_buffer->plane_size[0] = size;
	dst_buffer->plane_size[1] = size>>1;
	dst_buffer->plane_size[2] = size>>1;
	dst_buffer->plane_num = 3;
	MDP_LOG_D("[MDP]%s, %d\n", __func__, __LINE__);

	pkt->cmd_buf_size = 0;

	MDP_LOG_D("[MDP]%s, %d, format: %x, src_crop_w: %d, src_crop_h: %d, dst_crop_w: %d, dst_crop_h: %d\n",
		__func__, __LINE__, dst_cfg->format, src_cfg->crop_w, src_cfg->crop_h, dst_cfg->crop_w, dst_cfg->crop_h);

	if (src_cfg->format == PIX_FMT_YUYV &&
	src_cfg->crop_x == 0 &&
	src_cfg->crop_y == 0) {

		if (dst_cfg->format == PIX_FMT_YUYV &&
			src_cfg->crop_w == 1280 && src_cfg->crop_h == 720 &&
			dst_cfg->crop_w == 1920 && dst_cfg->crop_h == 1080)
			yuyv_1280x720_yuyv_1920x1080(pkt,
				src_buffer, dst_buffer);
		else
			support = 0;

	} else if (src_cfg->format == PIX_FMT_MT21 &&
		src_cfg->crop_x == 0 &&
		src_cfg->crop_y == 0) {

		if (dst_cfg->format == PIX_FMT_YUYV &&
			src_cfg->crop_w == 720 && src_cfg->crop_h == 480 &&
			dst_cfg->crop_w == 1920 && dst_cfg->crop_h == 1080)
			mt21_720x480_yuyv_1920x1080(pkt,
				src_buffer, dst_buffer);
		else
			support = 0;

	} else {
		support = 0;
	}

	if (support == 1) {
		ret = cmdq_pkt_flush(client, pkt);
		if (ret < 0)
			MDP_LOG_E("cmdq flush failed!!!\n");
	} else {
		ret = -1;
		MDP_LOG_D("[MDP]frvc unsupport:");
		MDP_LOG_D("[MDP]in: (%d, %d, %d, %d, %ld)\n",
			src_cfg->w, src_cfg->h, src_cfg->crop_w, src_cfg->crop_h, src_cfg->format);
		MDP_LOG_D("[MDP]out: (%d, %d, %d, %d, %d, %d, %ld)\n",
			dst_cfg->w, dst_cfg->h, dst_cfg->crop_w, dst_cfg->crop_h, dst_cfg->format);
	}

	return ret;
}

static void mdp_thread(void *args)
{
	mdp_device *dev = (mdp_device*)args;
	video_buffer *buffer_in;
	video_buffer *buffer_out;
	u32 input_buffer_flag = 0;
	u32 output_buffer_flag = 0;

	MDP_LOG_D("[MDP]%s, %d, dev: %p\n", __func__, __LINE__, dev);

	while (dev->stop_flag != 1) {
		if (input_buffer_flag && output_buffer_flag) {
			WARN_ON(mtk_mdp_frvc_process(dev, buffer_in, buffer_out));
			/*buffer done*/
			dev->buffer_done_callback(dev->callback_data, VIDEO_BUFFER_INPUT, buffer_in);
			dev->buffer_done_callback(dev->callback_data, VIDEO_BUFFER_OUTPUT, buffer_out);
			input_buffer_flag = 0;
			output_buffer_flag = 0;
		}

		if(input_buffer_flag == 0) {
			if ((xQueueReceive(dev->queue_handle_in, &buffer_in, 100/portTICK_RATE_MS) == pdPASS))
				input_buffer_flag = 1;
		}

		if (output_buffer_flag == 0) {
			if ((xQueueReceive(dev->queue_handle_out, &buffer_out, 100/portTICK_RATE_MS) == pdPASS))
				output_buffer_flag = 1;
		}
	}
	dev->stop_flag = 2;
	vTaskDelete(NULL);
}

video_handle mdp_open()
{
	mdp_device *dev;
	MDP_LOG_D("[MDP]%s, %d\n", __func__, __LINE__);

	dev = (mdp_device*)malloc(sizeof(*dev));
	memset(dev, 0, sizeof(*dev));
	dev->client = cmdq_msg_create(GCE_CM4_MDP, false);
	cmdq_pkt_create_ex(&dev->pkt, 5*1024);

	dev->queue_handle_in = xQueueCreate(5, sizeof(video_buffer*));
	if (!dev->queue_handle_in) {
		MDP_LOG_E("mdp: xQueueCreate error\n");
		goto error_1;
	}

	dev->queue_handle_out = xQueueCreate(5, sizeof(video_buffer*));
	if (!dev->queue_handle_out) {
		MDP_LOG_E("mdp: xQueueCreate error\n");
		goto error_1;
	}

	dev->stop_flag = 0;
	if ( xTaskCreate(
		mdp_thread,
		"Mdp",
		400,
		(void*)dev,
		0,
		&(dev->thread_handle)
		) != pdPASS) {
		MDP_LOG_E("mdp: xTaskCreate error\n");
		goto error_2;
	}
	MDP_LOG_D("[MDP]%s, %d, dev: %p\n", __func__, __LINE__, dev);

	return (video_handle)dev;

error_2:
	if (dev->queue_handle_in) {
		vQueueDelete(dev->queue_handle_in);
		dev->queue_handle_in= NULL;
	}

	if (dev->queue_handle_out) {
		vQueueDelete(dev->queue_handle_out);
		dev->queue_handle_out= NULL;
	}
error_1:
	free(dev);
	return NULL;
}

int32_t mdp_close(video_handle handle)
{
	mdp_device *dev = (mdp_device*)handle;
	MDP_LOG_D("[MDP]%s, %d, dev: %p\n", __func__, __LINE__, dev);

	dev->stop_flag = 1;
	while (dev->stop_flag != 2)
		vTaskDelay(pdMS_TO_TICKS(1));

	if (dev->queue_handle_in)
		vQueueDelete(dev->queue_handle_in);

	if (dev->queue_handle_out)
		vQueueDelete(dev->queue_handle_out);

	if (dev->pkt != NULL)
		cmdq_pkt_destroy(dev->pkt);


	free(dev);
	return 0;
}

int32_t mdp_config(video_handle handle, video_config_type type, void *config)
{
	mdp_device * dev = (mdp_device *)handle;

	video_format * format = NULL;
	video_rect * rect = NULL;
	video_buffer_type config_type = VIDEO_BUFFER_INPUT;
	struct mdp_config *cfg;

	MDP_LOG_D("[MDP]%s, %d, %p, %d\n", __func__, __LINE__, dev, type);

	switch(type){
	case VIDEO_CONFIG_FORMAT:
		format = (video_format*)config;
		config_type = format->type;
		MDP_LOG_D("[MDP]%s, %d, VIDEO_CONFIG_FORMAT\n", __func__, __LINE__);
		break;
	case VIDEO_CONFIG_CROP:
		rect = (video_rect*)config;
		config_type = rect->type;
		MDP_LOG_D("[MDP]%s, %d, VIDEO_CONFIG_CROP\n", __func__, __LINE__);
		break;
	case VIDEO_CONFIG_AREA:
	case NR_CONFIG_LEVEL:
		break;
	}

	MDP_LOG_D("[MDP]%s, %d, mdp_cfg_src: %p, mdp_cfg_dst: %p", __func__, __LINE__, dev->mdp_cfg_src, dev->mdp_cfg_dst);

	if (config_type == VIDEO_BUFFER_INPUT) {
		MDP_LOG_D("[MDP]%s, %d, VIDEO_BUFFER_INPUT\n", __func__, __LINE__);
		cfg = &dev->mdp_cfg_src;
		switch(type){
		case VIDEO_CONFIG_FORMAT:
			format = (video_format*)config;
			cfg->h = format->height;
			cfg->w = format->width;
			cfg->type = format->type;
			cfg->format = format->fourcc;
			MDP_LOG_D("[MDP]%s, %d,format:%p, h:%d, w: %d,  type: %d,  fourcc: %ld\n",
				__func__, __LINE__, format, cfg->h, cfg->w,cfg->type, cfg->format);
			break;
		case VIDEO_CONFIG_CROP:
		case VIDEO_CONFIG_AREA:
			rect = (video_rect*)config;
			cfg->crop_h = rect->height;
			cfg->crop_w = rect->width;
			cfg->crop_x = rect->left;
			cfg->crop_y = rect->top;
			cfg->type = rect->type;
			MDP_LOG_D("[MDP]%s, %d,rect:%p, crop_h:%d, crop_w: %d,  crop_x: %d, crop_y: %d, type: %d\n",
				__func__, __LINE__, rect, cfg->crop_h, cfg->crop_w,cfg->crop_x, cfg->crop_y, cfg->type);
			break;
		default:
			break;
		}
	} else if (config_type == VIDEO_BUFFER_OUTPUT) {
		MDP_LOG_D("[MDP]%s, %d, VIDEO_BUFFER_OUTPUT\n", __func__, __LINE__);
		cfg = &dev->mdp_cfg_dst;
		switch(type){
		case VIDEO_CONFIG_FORMAT:
			format = (video_format*)config;
			cfg->h = format->height;
			cfg->w = format->width;
			cfg->type = format->type;
			cfg->format = format->fourcc;
			MDP_LOG_D("[MDP]%s, %d,format:%p, h:%d, w: %d,  type: %d,  fourcc: %ld\n",
				__func__, __LINE__, format, cfg->h, cfg->w,cfg->type, cfg->format);
			break;
		case VIDEO_CONFIG_CROP:
		case VIDEO_CONFIG_AREA:
			rect = (video_rect*)config;
			cfg->crop_h = rect->height;
			cfg->crop_w = rect->width;
			cfg->crop_x = rect->left;
			cfg->crop_y = rect->top;
			cfg->type = rect->type;
			MDP_LOG_D("[MDP]%s, %d,rect:%p, h:%d, w: %d,  crop_x: %d, crop_y: %d, type: %d\n",
				__func__, __LINE__, rect, cfg->crop_h, cfg->crop_w,cfg->crop_x, cfg->crop_y, cfg->type);
			break;
		default:
			break;
		}
	} else {
		MDP_LOG_E("[MDP][Warning] Invalid format.\n");
		return -1;
	}

	MDP_LOG_D("[MDP]%s, %d\n", __func__, __LINE__);
	return 0;
}

int32_t mdp_commit(video_handle handle, video_buffer_type type, video_buffer *buffer)
{
	mdp_device *dev = (mdp_device*)handle;
	MDP_LOG_D("[MDP]%s, %d, dev: %p\n", __func__, __LINE__, dev);
	switch(type) {
	case VIDEO_BUFFER_INPUT:
		if (xQueueSendToBack(dev->queue_handle_in, &buffer, 0) != pdPASS) {
			MDP_LOG_E("[MDP]mdp: %s fail\n", __func__);
			return -1;
		}
		break;
	case VIDEO_BUFFER_OUTPUT:
		if (xQueueSendToBack(dev->queue_handle_out, &buffer, 0) != pdPASS) {
			MDP_LOG_E("[MDP]mdp: %s fail\n", __func__);
			return -1;
		}
		break;
	default:
		break;
	}

	return 0;
}

int32_t mdp_register_handler(void *handle,
					void (*buffer_done_handler)(void *usr_data, video_buffer_type type, video_buffer *buffer),
					void *usr_data)
{
	mdp_device *dev = (mdp_device*)handle;
	MDP_LOG_D("[MDP]%s, %d, dev: %p\n", __func__, __LINE__, dev);

	dev->buffer_done_callback = buffer_done_handler;
	dev->callback_data = usr_data;
	return 0;
}
