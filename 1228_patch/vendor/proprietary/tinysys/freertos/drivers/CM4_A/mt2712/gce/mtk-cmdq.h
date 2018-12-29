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

#ifndef __MTK_CMDQ_H__
#define __MTK_CMDQ_H__

#include "mtk-cmdq-control.h"
#include "queue.h"
#include <driver_api.h>
#include <string.h>
#include <mt_reg_base.h>
#include <bit_op.h>
#include <assert.h>
#include <stdint.h>

#define PAGE_SIZE              4096
#define MAX_ERRNO            4095

#define u32 uint32_t
#define u64 uint64_t

#ifndef bool
   #define bool int
#endif

#ifndef false
   #define false  0
#endif

#ifndef true
   #define true  1
#endif


#define IS_ERR_VALUE(x)   ((x) >= (u32)-MAX_ERRNO)

static inline long IS_ERR(const void *ptr)
{
        return IS_ERR_VALUE((u32)ptr);
}

typedef enum GCE_IND {
	GCE_CM4_DRM = 0,
	GCE_CM4_MDP,
	GCE_CM4_NUM
} GCE_THREAD_INDEX;

/* display events in command queue(CMDQ) */
enum cmdq_event {
	/* MDP start of frame(SOF) events */
	CMDQ_EVENT_MDP_RDMA0_SOF,
	CMDQ_EVENT_MDP_RDMA1_SOF,
	CMDQ_EVENT_MDP_RDMA2_SOF,
	CMDQ_EVENT_MDP_RDMA3_SOF,
	CMDQ_EVENT_MDP_DSI0_TE_SOF,
	CMDQ_EVENT_MDP_DSI1_TE_SOF,
	CMDQ_EVENT_MDP_MVW_SOF,
	CMDQ_EVENT_MDP_TDSHP0_SOF,
	CMDQ_EVENT_MDP_TDSHP1_SOF,
	CMDQ_EVENT_MDP_TDSHP2_SOF,
	CMDQ_EVENT_MDP_WDMA_SOF,
	CMDQ_EVENT_MDP_WROT0_SOF,
	CMDQ_EVENT_MDP_WROT1_SOF,
	CMDQ_EVENT_MDP_WROT2_SOF,
	CMDQ_EVENT_MDP_CROP_SOF,

	/* Display start of frame(SOF) events */
	CMDQ_EVENT_DISP_OVL0_SOF,
	CMDQ_EVENT_DISP_OVL1_SOF,
	CMDQ_EVENT_DISP_OVL2_SOF,
	CMDQ_EVENT_DISP_RDMA0_SOF,
	CMDQ_EVENT_DISP_RDMA1_SOF,
	CMDQ_EVENT_DISP_RDMA2_SOF,
	CMDQ_EVENT_DISP_WDMA0_SOF,
	CMDQ_EVENT_DISP_WDMA1_SOF,
	CMDQ_EVENT_DISP_WDMA2_SOF,
	CMDQ_EVENT_DISP_COLOR0_SOF,
	CMDQ_EVENT_DISP_COLOR1_SOF,
	CMDQ_EVENT_DISP_COLOR2_SOF,
	CMDQ_EVENT_DISP_AAL0_SOF,
	CMDQ_EVENT_DISP_AAL1_SOF,
	CMDQ_EVENT_DISP_GAMMA_SOF,
	CMDQ_EVENT_DISP_UFOE_SOF,
	CMDQ_EVENT_DISP_PWM0_SOF,
	CMDQ_EVENT_DISP_PWM1_SOF,
	CMDQ_EVENT_DISP_OD0_SOF,
	CMDQ_EVENT_DISP_OD1_SOF,

	/* MDP end of frame(EOF) events */
	CMDQ_EVENT_MDP_RDMA0_EOF,
	CMDQ_EVENT_MDP_RDMA1_EOF,
	CMDQ_EVENT_MDP_RDMA2_EOF,
	CMDQ_EVENT_MDP_RDMA3_EOF,
	CMDQ_EVENT_MDP_RSZ0_EOF,
	CMDQ_EVENT_MDP_RSZ1_EOF,
	CMDQ_EVENT_MDP_RSZ2_EOF,
	CMDQ_EVENT_MDP_TDSHP0_EOF,
	CMDQ_EVENT_MDP_TDSHP1_EOF,
	CMDQ_EVENT_MDP_TDSHP2_EOF,
	CMDQ_EVENT_MDP_WDMA_EOF,
	CMDQ_EVENT_MDP_WROT0_W_EOF,
	CMDQ_EVENT_MDP_WROT0_R_EOF,
	CMDQ_EVENT_MDP_WROT1_W_EOF,
	CMDQ_EVENT_MDP_WROT1_R_EOF,
	CMDQ_EVENT_MDP_WROT2_W_EOF,
	CMDQ_EVENT_MDP_WROT2_R_EOF,
	CMDQ_EVENT_MDP_CROP_EOF,

	/* Display end of frame(EOF) events */
	CMDQ_EVENT_DISP_OVL0_EOF,
	CMDQ_EVENT_DISP_OVL1_EOF,
	CMDQ_EVENT_DISP_OVL2_EOF,
	CMDQ_EVENT_DISP_RDMA0_EOF,
	CMDQ_EVENT_DISP_RDMA1_EOF,
	CMDQ_EVENT_DISP_RDMA2_EOF,
	CMDQ_EVENT_DISP_WDMA0_EOF,
	CMDQ_EVENT_DISP_WDMA1_EOF,
	CMDQ_EVENT_DISP_WDMA2_EOF,
	CMDQ_EVENT_DISP_COLOR0_EOF,
	CMDQ_EVENT_DISP_COLOR1_EOF,
	CMDQ_EVENT_DISP_COLOR2_EOF,
	CMDQ_EVENT_DISP_AAL0_EOF,
	CMDQ_EVENT_DISP_AAL1_EOF,
	CMDQ_EVENT_DISP_GAMMA_EOF,
	CMDQ_EVENT_DISP_UFOE_EOF,
	CMDQ_EVENT_DISP_DPI0_EOF,
	CMDQ_EVENT_DISP_DPI1_EOF,
	CMDQ_EVENT_DISP_OD0_EOF,
	CMDQ_EVENT_DISP_OD1_EOF,
	CMDQ_EVENT_DISP_DSI0_EOF,
	CMDQ_EVENT_DISP_DSI1_EOF,
	CMDQ_EVENT_DISP_DSI2_EOF,
	CMDQ_EVENT_DISP_DSI3_EOF,

	/* Mutex end of frame(EOF) events */
	CMDQ_EVENT_MUTEX0_STREAM_EOF,
	CMDQ_EVENT_MUTEX1_STREAM_EOF,
	CMDQ_EVENT_MUTEX2_STREAM_EOF,
	CMDQ_EVENT_MUTEX3_STREAM_EOF,
	CMDQ_EVENT_MUTEX4_STREAM_EOF,
	/* Display underrun events */
	CMDQ_EVENT_DISP_RDMA0_UNDERRUN,
	CMDQ_EVENT_DISP_RDMA1_UNDERRUN,
	CMDQ_EVENT_DISP_RDMA2_UNDERRUN,
	/* Keep this at the end */
	CMDQ_MAX_EVENT,
};

/* General Purpose Register */
enum cmdq_gpr_reg {
	/* Value Reg, we use 32-bit */
	/* Address Reg, we use 64-bit */
	/* Note that R0-R15 and P0-P7 actullay share same memory */
	/* and R1 cannot be used. */

	CMDQ_DATA_REG_JPEG = 0x00,	/* R0 */
	CMDQ_DATA_REG_JPEG_DST = 0x11,	/* P1 */

	CMDQ_DATA_REG_PQ_COLOR = 0x04,	/* R4 */
	CMDQ_DATA_REG_PQ_COLOR_DST = 0x13,	/* P3 */

	CMDQ_DATA_REG_2D_SHARPNESS_0 = 0x05,	/* R5 */
	CMDQ_DATA_REG_2D_SHARPNESS_0_DST = 0x14,	/* P4 */

	CMDQ_DATA_REG_2D_SHARPNESS_1 = 0x0a,	/* R10 */
	CMDQ_DATA_REG_2D_SHARPNESS_1_DST = 0x16,	/* P6 */

	CMDQ_DATA_REG_DEBUG = 0x0b,	/* R11 */
	CMDQ_DATA_REG_DEBUG_DST = 0x17,	/* P7 */

	/* sentinel value for invalid register ID */
	CMDQ_DATA_REG_INVALID = -1,
};

struct cmdq_pkt;

struct cmdq_base {
	int	subsys;
	u32	base;
};

struct cmdq_cb_data {
	bool	err;
	void	*data;
};

typedef void (*cmdq_async_flush_cb)(struct cmdq_cb_data data);

struct cmdq_task_cb {
	cmdq_async_flush_cb	cb;
	void			*data;
};


struct cmdq_task {
	struct cmdq		*cmdq;
	ListItem_t                list_entry;
	unsigned long		             pa_base;
	struct cmdq_client	*client;
	struct cmdq_pkt		*pkt; /* the packet sent from task */
};

struct cmdq_pkt {
	void			*va_base;
	unsigned long		 pa_base;
	size_t			cmd_buf_size; /* command occupied size */
	size_t			buf_size; /* real buffer size */
	struct cmdq_task_cb	cb;
	struct cmdq_task                task;
};

struct cmdq_flush_completion {

	/*SemaphoreHandle_t semaphoreHandle_t;*/
	QueueHandle_t queuehandle_t;
	/*QueueHandle_t queue_handle;*/
	bool err;
};

struct cmdq_client {
	void     		*base;
	List_t		task_busy_list;
	bool                     atomic_exec;
	struct cmdq_task *task;
	char		thread; /* HW thread index */
	//char		*idx;/*soft thread index*/
	struct cmdq_flush_completion	cmplt;

};

/**
 * struct gce_index
 * @gce_ind:		The client thread index
 * @atomic_flag:		flag of automic_exec
 */
struct  gce_index{
	GCE_THREAD_INDEX gce_index;
	bool atomic_flag;
};


/**
 * cmdq_register_device() - register device which needs CMDQ
 * @res:registers start address
 *
 * Return: cmdq_base pointer or NULL for failed
 */
struct cmdq_base *cmdq_register_device(uint32_t res);

/**
 * cmdq_msg_create() - create CMDQ message client and channel
 * @index:	index of CMDQ message channel
 *
 * Return: CMDQ message client pointer
 */
struct cmdq_client *cmdq_msg_create(GCE_THREAD_INDEX index, bool automic_flag);

/**
 * cmdq_pkt_create_ex() - create a CMDQ packet
 * @pkt_ptr:	CMDQ packet pointer to retrieve cmdq_pkt
 * @size:		cmdq buffer size
 *
 * Return: 0 for success; else the error code is returned
 */
int cmdq_pkt_create_ex(struct cmdq_pkt **pkt_ptr, u32 size);

/**
 * cmdq_pkt_create() - create a CMDQ packet
 * @pkt_ptr:	CMDQ packet pointer to retrieve cmdq_pkt
 *
 * Return: 0 for success; else the error code is returned
 */
int cmdq_pkt_create(struct cmdq_pkt **pkt_ptr);

/**
 * cmdq_pkt_destroy() - destroy the CMDQ packet
 * @pkt:	the CMDQ packet
 */
void cmdq_pkt_destroy(struct cmdq_pkt *pkt);

/**
 * cmdq_pkt_realloc_cmd_buffer() - reallocate command buffer for CMDQ packet
 * @pkt:	the CMDQ packet
 * @size:	the request size
 * Return: 0 for success; else the error code is returned
 */
int cmdq_pkt_realloc_cmd_buffer(struct cmdq_pkt *pkt, size_t size);

/**
 * cmdq_pkt_read() - append read command to the CMDQ packet
 * @pkt:	the CMDQ packet
 * @base:	the CMDQ base
 * @offset:	register offset from module base
 * @writeAddress:
 * @valueRegId:
 * @destRegId:
 * Return: 0 for success; else the error code is returned
 */
int cmdq_pkt_read(struct cmdq_pkt *pkt,
			struct cmdq_base *base, u32 offset, u32 writeAddress,
			enum cmdq_gpr_reg valueRegId,
			enum cmdq_gpr_reg destRegId);

/**
 * cmdq_pkt_write() - append write command to the CMDQ packet
 * @pkt:	the CMDQ packet
 * @value:	the specified target register value
 * @base:	the CMDQ base
 * @offset:	register offset from module base
 *
 * Return: 0 for success; else the error code is returned
 */
int cmdq_pkt_write(struct cmdq_pkt *pkt, u32 value,
		   struct cmdq_base *base, u32 offset);

/**
 * cmdq_pkt_write_mask() - append write command with mask to the CMDQ packet
 * @pkt:	the CMDQ packet
 * @value:	the specified target register value
 * @base:	the CMDQ base
 * @offset:	register offset from module base
 * @mask:	the specified target register mask
 *
 * Return: 0 for success; else the error code is returned
 */
int cmdq_pkt_write_mask(struct cmdq_pkt *pkt, u32 value,
			struct cmdq_base *base, u32 offset, u32 mask);

/**
 * cmdq_pkt_poll() - append polling command with mask to the CMDQ packet
 * @pkt:	the CMDQ packet
 * @value:	the specified target register value
 * @base:	the CMDQ base
 * @offset:	register offset from module base
 *
 * Return: 0 for success; else the error code is returned
 */
int cmdq_pkt_poll(struct cmdq_pkt *pkt, u32 value,
			 struct cmdq_base *base, u32 offset);

/**
 * cmdq_pkt_poll_t() - append polling command with mask to the CMDQ packet
 * @pkt:	the CMDQ packet
 * @value:	the specified target register value
 * @base:	the CMDQ base
 * @offset:	register offset from module base
 * @mask:	the specified target register mask
 *
 * Return: 0 for success; else the error code is returned
 */
int cmdq_pkt_poll_mask(struct cmdq_pkt *pkt, u32 value,
			struct cmdq_base *base, uint32_t offset, uint32_t mask);

/**
 * cmdq_pkt_wfe() - append wait for event command to the CMDQ packet
 * @pkt:	the CMDQ packet
 * @event:	the desired event type to "wait and CLEAR"
 *
 * Return: 0 for success; else the error code is returned
 */
int cmdq_pkt_wfe(struct cmdq_pkt *pkt, enum cmdq_event event);

/**
 * cmdq_pkt_clear_event() - append clear event command to the CMDQ packet
 * @pkt:	the CMDQ packet
 * @event:	the desired event to be cleared
 *
 * Return: 0 for success; else the error code is returned
 */
int cmdq_pkt_clear_event(struct cmdq_pkt *pkt, enum cmdq_event event);

/**
 * cmdq_pkt_flush() - trigger CMDQ to execute the CMDQ packet
 * @client:	the CMDQ thread
 * @pkt:	the CMDQ packet
 *
 * Return: 0 for success; else the error code is returned
 *
 * Trigger CMDQ to execute the CMDQ packet. Note that this is a
 * synchronous flush function. When the function returned, the recorded
 * commands have been done.
 */
int cmdq_pkt_flush(struct cmdq_client *client, struct cmdq_pkt *pkt);

/**
 * cmdq_pkt_flush_async() - trigger CMDQ to asynchronously execute the CMDQ
 *                          packet and call back at the end of done packet
 * @client:	the CMDQ thread
 * @pkt:	the CMDQ packet
 * @cb:		called at the end of done packet
 * @data:	this data will pass back to cb
 *
 * Return: 0 for success; else the error code is returned
 *
 * Trigger CMDQ to asynchronously execute the CMDQ packet and call back
 * at the end of done packet. Note that this is an ASYNC function. When the
 * function returned, it may or may not be finished.
 */
int cmdq_pkt_flush_async(struct cmdq_client *client, struct cmdq_pkt *pkt,
			 cmdq_async_flush_cb cb, void *data);

u32 cmdq_subsys_id_to_base(int id);
int cmdq_msg_send_data(struct cmdq_client *client, struct cmdq_pkt *data);
struct cmdq_client*cmdq_xlate(const struct gce_index*sp);
void cmdq_task_status(struct cmdq_client *client, struct cmdq_pkt *pkt);

#endif	/* __MTK_CMDQ_H__ *//**/
