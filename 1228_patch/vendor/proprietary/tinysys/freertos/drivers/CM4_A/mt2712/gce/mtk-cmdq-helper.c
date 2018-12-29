/*
 * Copyright (c) 2015 MediaTek Inc.
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
#include "mtk-cmdq.h"
#include "FreeRTOS.h"
#include <semphr.h>
#include "video_core.h"
#include "dram_malloc.h"
#include "portmacro.h"
#include "queue.h"
#include <string.h>
#include <assert.h>

#define CMDQ_ARG_A_WRITE_MASK	0xffff
#define CMDQ_WRITE_ENABLE_MASK	BIT(0)
#define CMDQ_EOC_IRQ_EN		BIT(0)
#define CMDQ_EOC_CMD		((u64)((CMDQ_CODE_EOC << CMDQ_OP_CODE_SHIFT)) \
				<< 32 | CMDQ_EOC_IRQ_EN)

struct cmdq_subsys {
	u32	base;
	int	id;
};

static const struct cmdq_subsys gce_subsys[] = {
	{0x1400, 1},
	{0x1401, 2},
	{0x1402, 3},
	{0x1403, 23},
	{0x0, -1},
};

static int cmdq_subsys_base_to_id(u32 base)
{
	int i;

	for (i = 0; gce_subsys[i].base != 0u; i++)
		if (gce_subsys[i].base == base)
			return gce_subsys[i].id;
	return -EFAULT;
}

u32 cmdq_subsys_id_to_base(int id)
{
	int i;

	for (i = 0; gce_subsys[i].base != 0u; i++)
		if (gce_subsys[i].id == id)
			return gce_subsys[i].base;
	return 0;
}

int cmdq_pkt_realloc_cmd_buffer(struct cmdq_pkt *pkt, size_t size)
{
	void *new_buf;
	if (pkt->va_base) {
		GCE_LOG_E("[GCE]cmdq_buf_realloc buf_size: %u ->%u, cmdq size: %u,  pa: %p, va: %p",
			pkt->buf_size, size, pkt->cmd_buf_size, pkt->pa_base, pkt->va_base);
	}

	new_buf = dram_malloc(size);
	if (!new_buf)
		return -ENOMEM;

	if (pkt->va_base) {
		memcpy(new_buf, pkt->va_base, pkt->buf_size);
		dram_free(pkt->va_base);
	}

	pkt->va_base = new_buf;
	pkt->buf_size = size;
	return 0;
}

struct cmdq_base *cmdq_register_device(uint32_t res)
{
	struct cmdq_base *cmdq_base;
	int subsys;
	u32 base;

	base = (u32)res;

	subsys = cmdq_subsys_base_to_id(base >> 16);
	if (subsys < 0)
		return NULL;

	cmdq_base = pvPortMalloc(sizeof(*cmdq_base));
	if (!cmdq_base)
		return NULL;
	cmdq_base->subsys = subsys;
	cmdq_base->base = base;

	return cmdq_base;
}

struct cmdq_client *cmdq_msg_create(GCE_THREAD_INDEX index, bool automic_flag)
{
             struct cmdq_client *client;
             struct gce_index gce_ind;

	if (index > GCE_CM4_NUM) {
		GCE_LOG_D("GCE index should be < %u\n", GCE_CM4_NUM);
		return NULL;
	}
             memset(&gce_ind, 0, sizeof(gce_ind));
             gce_ind.atomic_flag = automic_flag;
             gce_ind.gce_index = index;
             client = cmdq_xlate(&gce_ind);

              return client;
}

int cmdq_pkt_create_ex(struct cmdq_pkt **pkt_ptr, u32 size)
{
	struct cmdq_pkt *pkt;
	int err;

	pkt = pvPortMalloc(sizeof(*pkt));
	memset(pkt, 0, sizeof(*pkt));
	if (!pkt)
		return -ENOMEM;
	err = cmdq_pkt_realloc_cmd_buffer(pkt, size);
	if (err < 0) {
		vPortFree(pkt);
		return err;
	}
	*pkt_ptr = pkt;

	return 0;
}

int cmdq_pkt_create(struct cmdq_pkt **pkt_ptr)
{
	return cmdq_pkt_create_ex(pkt_ptr, 1024);
}

void cmdq_pkt_destroy(struct cmdq_pkt *pkt)
{
	dram_free(pkt->va_base);
	vPortFree(pkt);
}

static bool cmdq_pkt_is_finalized(struct cmdq_pkt *pkt)
{
	u64 *expect_eoc;

	if (pkt->cmd_buf_size < CMDQ_INST_SIZE << 1)
		return false;

	expect_eoc = pkt->va_base + pkt->cmd_buf_size - (CMDQ_INST_SIZE << 1);
	if (*expect_eoc == CMDQ_EOC_CMD)
		return true;

	return false;
}

static int cmdq_pkt_append_command(struct cmdq_pkt *pkt, enum cmdq_code code,
				   u32 arg_a, u32 arg_b)
{
	u64 *cmd_ptr;
	int err;

	if (WARN_ON(cmdq_pkt_is_finalized(pkt)))
		return -EBUSY;
	if (pkt->cmd_buf_size + CMDQ_INST_SIZE > pkt->buf_size) {
		err = cmdq_pkt_realloc_cmd_buffer(pkt, pkt->buf_size << 1);
		if (err < 0)
			return err;
	}
	cmd_ptr = pkt->va_base + pkt->cmd_buf_size;
	(*cmd_ptr) = (u64)((code << CMDQ_OP_CODE_SHIFT) | arg_a) << 32 | arg_b;
	pkt->cmd_buf_size += CMDQ_INST_SIZE;
	return 0;
}

int cmdq_pkt_read(struct cmdq_pkt *pkt,
			struct cmdq_base *base, u32 offset, u32 writeAddress,
			enum cmdq_gpr_reg valueRegId,
			enum cmdq_gpr_reg destRegId)
{
	int ret;
	/* physical reg address. */
	u32 arg_a = ((base->base + offset) & CMDQ_ARG_A_WRITE_MASK) |
		    (base->subsys << CMDQ_SUBSYS_SHIFT);

	/* Load into 32-bit GPR (R0-R15) */
	ret = cmdq_pkt_append_command(pkt, CMDQ_CODE_READ,
				arg_a | (2 << 21), valueRegId);


	/* CMDQ_CODE_MASK=CMDQ_CODE_MOVE argB is 48-bit */
	/* so writeAddress is split into 2 parts */
	/* and we store address in 64-bit GPR (P0-P7) */
	ret += cmdq_pkt_append_command(pkt, CMDQ_CODE_MASK,
				((destRegId & 0x1f) << 16) | (4 << 21),
				writeAddress);

	/* write to memory */
	ret += cmdq_pkt_append_command(pkt, CMDQ_CODE_WRITE,
				((destRegId & 0x1f) << 16) | (6 << 21),
				valueRegId);

	GCE_LOG_E("COMMAND: copy reg:0x%08x to phys:%pa, GPR(%d, %d)\n",
		arg_a, &writeAddress, valueRegId, destRegId);
	return ret;
}

int cmdq_pkt_write(struct cmdq_pkt *pkt, u32 value, struct cmdq_base *base,
		   u32 offset)
{
	u32 arg_a = ((base->base + offset) & CMDQ_ARG_A_WRITE_MASK) |
		    (base->subsys << CMDQ_SUBSYS_SHIFT);
	return cmdq_pkt_append_command(pkt, CMDQ_CODE_WRITE, arg_a, value);
}

int cmdq_pkt_write_mask(struct cmdq_pkt *pkt, u32 value,
			struct cmdq_base *base, u32 offset, u32 mask)
{
	u32 offset_mask = offset;
	int err;

	if (mask != 0xffffffff) {
		err = cmdq_pkt_append_command(pkt, CMDQ_CODE_MASK, 0, ~mask);
		if (err < 0)
			return err;
		offset_mask |= CMDQ_WRITE_ENABLE_MASK;
	}

	return cmdq_pkt_write(pkt, value, base, offset_mask);
}

int cmdq_pkt_poll(struct cmdq_pkt *pkt, u32 value, struct cmdq_base *base,
		   u32 offset)
{
	u32 arg_a = ((base->base + offset) & CMDQ_ARG_A_WRITE_MASK) |
		    (base->subsys << CMDQ_SUBSYS_SHIFT);
	return cmdq_pkt_append_command(pkt, CMDQ_CODE_POLL, arg_a, value);
}

int cmdq_pkt_poll_mask(struct cmdq_pkt *pkt, u32 value,
			 struct cmdq_base *base, uint32_t offset, uint32_t mask)
{
	uint32_t offset_mask = offset;
	int err;

	if (mask != 0xffffffff) {
		err = cmdq_pkt_append_command(pkt, CMDQ_CODE_MASK, 0, ~mask);
		if (err < 0)
			return err;
		offset_mask |= CMDQ_WRITE_ENABLE_MASK;
	}
	return cmdq_pkt_poll(pkt, value, base, offset_mask);
}

static const u32 cmdq_event_value[CMDQ_MAX_EVENT] = {
	[CMDQ_EVENT_MDP_RDMA0_SOF] = 0,
	[CMDQ_EVENT_MDP_RDMA1_SOF] = 1,
	[CMDQ_EVENT_MDP_TDSHP0_SOF] = 5,
	[CMDQ_EVENT_MDP_TDSHP1_SOF] = 6,
	[CMDQ_EVENT_MDP_WDMA_SOF] = 7,
	[CMDQ_EVENT_MDP_WROT0_SOF] = 8,
	[CMDQ_EVENT_MDP_WROT1_SOF] = 9,
	[CMDQ_EVENT_MDP_CROP_SOF] = 10,
	[CMDQ_EVENT_DISP_OVL0_SOF] = 11,
	[CMDQ_EVENT_DISP_OVL1_SOF] = 12,
	[CMDQ_EVENT_DISP_RDMA0_SOF] = 13,
	[CMDQ_EVENT_DISP_RDMA1_SOF] = 14,
	[CMDQ_EVENT_DISP_RDMA2_SOF] = 15,
	[CMDQ_EVENT_DISP_WDMA0_SOF] = 16,
	[CMDQ_EVENT_DISP_WDMA1_SOF] = 17,
	[CMDQ_EVENT_DISP_COLOR0_SOF] = 18,
	[CMDQ_EVENT_DISP_COLOR1_SOF] = 19,
	[CMDQ_EVENT_DISP_AAL0_SOF] = 20,
	[CMDQ_EVENT_DISP_GAMMA_SOF] = 21,
	[CMDQ_EVENT_DISP_UFOE_SOF] = 22,
	[CMDQ_EVENT_DISP_PWM0_SOF] = 23,
	[CMDQ_EVENT_DISP_PWM1_SOF] = 24,
	[CMDQ_EVENT_DISP_OD0_SOF] = 25,
	[CMDQ_EVENT_MDP_RDMA2_SOF] = 26,
	[CMDQ_EVENT_MDP_RDMA3_SOF] = 27,
	[CMDQ_EVENT_MDP_TDSHP2_SOF] = 28,
	[CMDQ_EVENT_MDP_WROT2_SOF] = 29,
	[CMDQ_EVENT_DISP_OVL2_SOF] = 30,
	[CMDQ_EVENT_DISP_WDMA2_SOF] = 31,
	[CMDQ_EVENT_DISP_COLOR2_SOF] = 32,
	[CMDQ_EVENT_DISP_AAL1_SOF] = 33,
	[CMDQ_EVENT_DISP_OD1_SOF] = 34,
	[CMDQ_EVENT_MDP_RDMA0_EOF] = 37,
	[CMDQ_EVENT_MDP_RDMA1_EOF] = 38,
	[CMDQ_EVENT_MDP_RSZ0_EOF] = 39,
	[CMDQ_EVENT_MDP_RSZ1_EOF] = 40,
	[CMDQ_EVENT_MDP_RSZ2_EOF] = 41,
	[CMDQ_EVENT_MDP_TDSHP0_EOF] = 42,
	[CMDQ_EVENT_MDP_TDSHP1_EOF] = 43,
	[CMDQ_EVENT_MDP_WDMA_EOF] = 44,
	[CMDQ_EVENT_MDP_WROT0_W_EOF] = 45,
	[CMDQ_EVENT_MDP_WROT0_R_EOF] = 46,
	[CMDQ_EVENT_MDP_WROT1_W_EOF] = 47,
	[CMDQ_EVENT_MDP_WROT1_R_EOF] = 48,
	[CMDQ_EVENT_MDP_CROP_EOF] = 49,
	[CMDQ_EVENT_DISP_OVL0_EOF] = 50,
	[CMDQ_EVENT_DISP_OVL1_EOF] = 51,
	[CMDQ_EVENT_DISP_RDMA0_EOF] = 52,
	[CMDQ_EVENT_DISP_RDMA1_EOF] = 53,
	[CMDQ_EVENT_DISP_RDMA2_EOF] = 54,
	[CMDQ_EVENT_DISP_WDMA0_EOF] = 55,
	[CMDQ_EVENT_DISP_WDMA1_EOF] = 56,
	[CMDQ_EVENT_DISP_COLOR0_EOF] = 57,
	[CMDQ_EVENT_DISP_COLOR1_EOF] = 58,
	[CMDQ_EVENT_DISP_AAL0_EOF] = 59,
	[CMDQ_EVENT_DISP_GAMMA_EOF] = 60,
	[CMDQ_EVENT_DISP_UFOE_EOF] = 61,
	[CMDQ_EVENT_DISP_DPI0_EOF] = 62,
	[CMDQ_EVENT_DISP_DPI1_EOF] = 63,
	[CMDQ_EVENT_MDP_RDMA2_EOF] = 64,
	[CMDQ_EVENT_MDP_RDMA3_EOF] = 65,
	[CMDQ_EVENT_MDP_WROT2_W_EOF] = 66,
	[CMDQ_EVENT_MDP_WROT2_R_EOF] = 67,
	[CMDQ_EVENT_MDP_TDSHP2_EOF] = 68,
	[CMDQ_EVENT_DISP_OVL2_EOF] = 69,
	[CMDQ_EVENT_DISP_WDMA2_EOF] = 70,
	[CMDQ_EVENT_DISP_COLOR2_EOF] = 71,
	[CMDQ_EVENT_DISP_AAL1_EOF] = 72,
	[CMDQ_EVENT_DISP_OD0_EOF] = 73,
	[CMDQ_EVENT_DISP_OD1_EOF] = 74,
	[CMDQ_EVENT_DISP_DSI0_EOF] = 75,
	[CMDQ_EVENT_DISP_DSI1_EOF] = 76,
	[CMDQ_EVENT_DISP_DSI2_EOF] = 77,
	[CMDQ_EVENT_DISP_DSI3_EOF] = 78,
	[CMDQ_EVENT_MUTEX0_STREAM_EOF] = 79,
	[CMDQ_EVENT_MUTEX1_STREAM_EOF] = 80,
	[CMDQ_EVENT_MUTEX2_STREAM_EOF] = 81,
	[CMDQ_EVENT_MUTEX3_STREAM_EOF] = 82,
	[CMDQ_EVENT_MUTEX4_STREAM_EOF] = 83,
	[CMDQ_EVENT_DISP_RDMA0_UNDERRUN] = 89,
	[CMDQ_EVENT_DISP_RDMA1_UNDERRUN] = 90,
	[CMDQ_EVENT_DISP_RDMA2_UNDERRUN] = 91,
};

int cmdq_pkt_wfe(struct cmdq_pkt *pkt, enum cmdq_event event)
{
	u32 arg_b;

	if (event >= CMDQ_MAX_EVENT || event < 0)
		return -EINVAL;

	/*
	 * WFE arg_b
	 * bit 0-11: wait value
	 * bit 15: 1 - wait, 0 - no wait
	 * bit 16-27: update value
	 * bit 31: 1 - update, 0 - no update
	 */
	arg_b = CMDQ_WFE_UPDATE | CMDQ_WFE_WAIT | CMDQ_WFE_WAIT_VALUE;
	return cmdq_pkt_append_command(pkt, CMDQ_CODE_WFE,
			cmdq_event_value[event], arg_b);
}

int cmdq_pkt_clear_event(struct cmdq_pkt *pkt, enum cmdq_event event)
{
	if (event >= CMDQ_MAX_EVENT || event < 0)
		return -EINVAL;

	return cmdq_pkt_append_command(pkt, CMDQ_CODE_WFE,
			cmdq_event_value[event], CMDQ_WFE_UPDATE);
}

static int cmdq_pkt_finalize(struct cmdq_pkt *pkt)
{
	int err;

	if (cmdq_pkt_is_finalized(pkt))
		return 0;

	/* insert EOC and generate IRQ for each command iteration */
	err = cmdq_pkt_append_command(pkt, CMDQ_CODE_EOC, 0, CMDQ_EOC_IRQ_EN);
	if (err < 0)
		return err;

	/* JUMP to end */
	err = cmdq_pkt_append_command(pkt, CMDQ_CODE_JUMP, 0, CMDQ_JUMP_PASS);
	if (err < 0)
		return err;

	GCE_LOG_D("finalize: add EOC and JUMP cmd, cmdq_buf_size: %u\n", pkt->cmd_buf_size);

	return 0;
}

int cmdq_pkt_flush_async(struct cmdq_client *client, struct cmdq_pkt *pkt,
			 cmdq_async_flush_cb cb, void *data)
{
	int err;

	err = cmdq_pkt_finalize(pkt);
	if (err < 0)
		return err;

	pkt->pa_base = (unsigned long)VA_TO_IOVA(pkt->va_base);
	GCE_LOG_D("[GCE]%s, %d, VA: %p, PA: %p\n",__func__,__LINE__,pkt->va_base, pkt->pa_base);
                 if (pkt->pa_base > 0xffffffffu) {
                        pkt->pa_base &= 0xffffffffu;/* Register of PC only 32-bit, print a warning log if address > 32bit */
                    }
	pkt->cb.cb = cb;
	pkt->cb.data = data;

	cmdq_msg_send_data(client, pkt);

	return 0;
}

static void cmdq_pkt_flush_cb(struct cmdq_cb_data data)
{
	struct cmdq_flush_completion *cmplt = data.data;
	BaseType_t xWokenSend = pdFALSE;

	cmplt->err = data.err;
	GCE_LOG_D("[GCE]%s, %d, queuehandle_t: %p\n",__func__, __LINE__, cmplt->queuehandle_t);

	xQueueSendToBackFromISR(cmplt->queuehandle_t, data.data, &xWokenSend);

}

int cmdq_pkt_flush(struct cmdq_client *client, struct cmdq_pkt *pkt)
{
	int err;
	BaseType_t ret;

	client->cmplt.err = true;
	GCE_LOG_D("[GCE]%s, %d, queuehandle_t: %p\n",__func__, __LINE__, client->cmplt.queuehandle_t);

	GCE_LOG_D("cmdq flush\n");
	err = cmdq_pkt_flush_async(client, pkt, cmdq_pkt_flush_cb, &client->cmplt);
	if (err < 0)
		return err;

	GCE_LOG_D("[GCE]%s, %d\n",__func__,__LINE__);

	ret = xQueueReceive(client->cmplt.queuehandle_t, &client->cmplt, 2000/portTICK_RATE_MS);
	GCE_LOG_D("[GCE]%s, %d, ret: %d\n",__func__,__LINE__, ret);
	if(ret == pdPASS) {
		GCE_LOG_D("xQueueReceive:%p, queuehandle_t: %p, err: %d\n",&client->cmplt, client->cmplt.queuehandle_t, client->cmplt.err);
	} else {
		cmdq_task_status(client, pkt);
	}

	GCE_LOG_D("cmdq done, err=%u\n", client->cmplt.err);
	return client->cmplt.err ? -EFAULT : 0;
}

