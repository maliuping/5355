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
#include "FreeRTOS.h"
#include "mtk-cmdq-control.h"
#include "mtk-cmdq.h"
#include "timers.h"
#include <stdlib.h>
#include "list.h"
#include "main.h"
#include "task.h"
#include "interrupt.h"
#include "queue.h"

#include <driver_api.h>
#include <mt_reg_base.h>
#include <string.h>
#include <assert.h>
#include "dram_malloc.h"
#include "mt2712.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define CMDQ_OP_CODE_MASK		(0xff << CMDQ_OP_CODE_SHIFT)
#define CMDQ_TIMEOUT_MS			1000
#define CMDQ_IRQ_MASK			(((1 << 7) | (1 << 8)) & 0xffff) /* CM4 use 7, 8 */
#define CMDQ_NUM_CMD(t)			(t->cmd_buf_size / CMDQ_INST_SIZE)

#define CMDQ_CURR_IRQ_STATUS		0x10
#define CMDQ_THR_SLOT_CYCLES		0x30

#define CMDQ_THR_BASE			0x100
#define CMDQ_THR_SIZE			0x80
#define CMDQ_THR_WARM_RESET		0x00
#define CMDQ_THR_ENABLE_TASK		0x04
#define CMDQ_THR_SUSPEND_TASK		0x08
#define CMDQ_THR_CURR_STATUS		0x0c
#define CMDQ_THR_IRQ_STATUS		0x10
#define CMDQ_THR_IRQ_ENABLE		0x14
#define CMDQ_THR_SECURITY		0x18
#define CMDQ_THR_CURR_ADDR		0x20
#define CMDQ_THR_END_ADDR		0x24
#define CMDQ_THR_WAIT_TOKEN		0x30

#define CMDQ_THR_ENABLED		0x1
#define CMDQ_THR_DISABLED		0x0
#define CMDQ_THR_SUSPEND		0x1
#define CMDQ_THR_RESUME			0x0
#define CMDQ_THR_STATUS_SUSPENDED	BIT(1)
#define CMDQ_THR_DO_WARM_RESET		BIT(0)
#define CMDQ_THR_ACTIVE_SLOT_CYCLES	0x3200
#define CMDQ_THR_IRQ_DONE		0x1
#define CMDQ_THR_IRQ_ERROR		0x12
#define CMDQ_THR_IRQ_EN			(CMDQ_THR_IRQ_ERROR | CMDQ_THR_IRQ_DONE)
#define CMDQ_THR_IS_WAITING		BIT(31)

#define CMDQ_JUMP_BY_OFFSET		0x10000000
#define CMDQ_JUMP_BY_PA			0x10000001
#define ARRAY_SIZE(a)       (sizeof(a) / sizeof(a[0]))
#if 0
#ifdef writel
#undef writel
void writel_ex(unsigned int v, void *a, const char *f, int line)
{
	PRINTF_D("[GCE]writel(%u,%p) @ %s, %d\n", v, a, f, line);
	unsigned int *tmp = (unsigned int *)a;
	*tmp = v;
}
#define writel(v, a) writel_ex(v, a, __func__, __LINE__)
#endif

#ifdef readl
#undef readl
unsigned int readl_ex(void *a, const char *f, int line)
{
	unsigned int v;
	unsigned int *tmp = (unsigned int *)a;

	v = *tmp;
	PRINTF_D("[GCE]readl(%p) = %u @ %s, %d\n", a, v, f, line);

	return v;
}
#define readl(a) readl_ex(a, __func__, __LINE__)
#endif
#endif

struct cmdq {
	void 		             *base;
	int			num_clients;
	uint32_t			irq;
	struct cmdq_client	client[GCE_CM4_NUM];
	bool			suspended;
};

static struct cmdq  *s_cmdq;

static int cmdq_thread_suspend(struct cmdq *cmdq, struct cmdq_client *client)
{
	u32 status = 0;
	GCE_LOG_D("[GCE]%s, %d",__func__, __LINE__);

	writel(CMDQ_THR_SUSPEND, client->base + CMDQ_THR_SUSPEND_TASK);

	/* If already disabled, treat as suspended successful. */
	if (!(readl(client->base + CMDQ_THR_ENABLE_TASK) & CMDQ_THR_ENABLED))
		return 0;

	if (readl_poll_timeout_atomic(client->base + CMDQ_THR_CURR_STATUS,
			status, status & CMDQ_THR_STATUS_SUSPENDED, 10, 1)) {
		GCE_LOG_E("suspend GCE thread 0x%x failed\n",
			(u32)(client->base - cmdq->base));
		return -EFAULT;
	}

	return 0;
}

static void cmdq_thread_resume(struct cmdq_client *client)
{           GCE_LOG_D("[GCE]%s, %d",__func__, __LINE__);
	writel(CMDQ_THR_RESUME, client->base + CMDQ_THR_SUSPEND_TASK);
}

static int cmdq_thread_reset(struct cmdq *cmdq, struct cmdq_client *client)
{
	u32 warm_reset = 0;
             GCE_LOG_D("[GCE]%s, %d",__func__, __LINE__);
	writel(CMDQ_THR_DO_WARM_RESET, client->base + CMDQ_THR_WARM_RESET);
	if (readl_poll_timeout_atomic(client->base + CMDQ_THR_WARM_RESET,
			warm_reset, !(warm_reset & CMDQ_THR_DO_WARM_RESET),
			10, 1)) {
		GCE_LOG_E("reset GCE thread 0x%x failed\n",
			(u32)(client->base - cmdq->base));
		return -EFAULT;
	}
	writel(CMDQ_THR_ACTIVE_SLOT_CYCLES, cmdq->base + CMDQ_THR_SLOT_CYCLES);
	return 0;
}

static void cmdq_thread_disable(struct cmdq *cmdq, struct cmdq_client *client)
{           GCE_LOG_D("[GCE]%s, %d",__func__, __LINE__);
	cmdq_thread_reset(cmdq, client);
	writel(CMDQ_THR_DISABLED, client->base + CMDQ_THR_ENABLE_TASK);
}

/* notify GCE to re-fetch commands by setting GCE thread PC */
static void cmdq_thread_invalidate_fetched_data(struct cmdq_client *client)
{
	writel(readl(client->base + CMDQ_THR_CURR_ADDR),
	       client->base + CMDQ_THR_CURR_ADDR);
}

static void cmdq_task_insert_into_thread(struct cmdq_task *task)
{           GCE_LOG_D("[GCE]%s, %d",__func__, __LINE__);
	struct cmdq_client *client = task->client;
	struct cmdq_task *prev_task = listGET_LIST_ITEM_OWNER(client->task_busy_list.pxIndex->pxPrevious);
	u64 *prev_task_base = prev_task->pkt->va_base;

	/* let previous task jump to this task */
	prev_task_base[CMDQ_NUM_CMD(prev_task->pkt) - 1] =
		(u64)CMDQ_JUMP_BY_PA << 32 | task->pa_base;

	cmdq_thread_invalidate_fetched_data(client);
}

static bool cmdq_command_is_wfe(u64 cmd)
{
	u64 wfe_option = CMDQ_WFE_UPDATE | CMDQ_WFE_WAIT | CMDQ_WFE_WAIT_VALUE;
	u64 wfe_op = (u64)(CMDQ_CODE_WFE << CMDQ_OP_CODE_SHIFT) << 32;
	u64 wfe_mask = (u64)CMDQ_OP_CODE_MASK << 32 | 0xffffffff;

	return ((cmd & wfe_mask) == (wfe_op | wfe_option));
}

/* we assume tasks in the same display GCE thread are waiting the same event. */
static void cmdq_task_remove_wfe(struct cmdq_task *task)
{
	u64 *base = task->pkt->va_base;
	int i;

	for (i = 0; i < CMDQ_NUM_CMD(task->pkt); i++)
		if (cmdq_command_is_wfe(base[i]))
			base[i] = (u64)CMDQ_JUMP_BY_OFFSET << 32 |
				  CMDQ_JUMP_PASS;
}

static bool cmdq_thread_is_in_wfe(struct cmdq_client *client)
{
	return readl(client->base + CMDQ_THR_WAIT_TOKEN) & CMDQ_THR_IS_WAITING;
}

static void cmdq_thread_wait_end(struct cmdq_client *client,
				 unsigned long end_pa)
{
	unsigned long curr_pa;

	if (readl_poll_timeout_atomic(client->base + CMDQ_THR_CURR_ADDR,
			curr_pa, curr_pa == end_pa, 20, 1))
		GCE_LOG_E("GCE thread cannot run to end.\n");
}

/*
 * Enable MTK CMDQ(GCE) Security config.
 */
#define CMDQ_DOMAIN(n)			((n) << 1)
#define CMDQ_DOMAIN_1			CMDQ_DOMAIN(1)
#define CMDQ_DOMAIN_MASK		(0x3 << 1)

void mt_sip_set_cmdq_sreg(struct cmdq_client *client)
{
	uint32_t v;
	void *r;

	/*
	 * 0x10212118[2:1], axi domain
	 * 0x10212118[0], thread security, 0:non-secure, 1:secure
	 */
	/* GCE as master: Domain 1 (cluster), NS */
	r = client->base + CMDQ_THR_SECURITY;
	v = readl(r);
	v &= ~CMDQ_DOMAIN_MASK;
	v |= CMDQ_DOMAIN_1;
	writel(v, r);
}

static void cmdq_task_exec(struct cmdq_pkt *pkt, struct cmdq_client *client)
{
	struct cmdq_task *task;
	unsigned long curr_pa, end_pa;

	/* Client should not flush new tasks if suspended. */
	WARN_ON(s_cmdq->suspended);

	task = &pkt->task;
	task->cmdq = s_cmdq;
	vListInitialiseItem(&task->list_entry);
	listSET_LIST_ITEM_OWNER(&task->list_entry, task);
	task->pa_base = pkt->pa_base;
	task->client = client;
	task->pkt = pkt;

	GCE_LOG_D("[GCE]%s, %d",__func__,__LINE__);

	if (listLIST_IS_EMPTY(&client->task_busy_list)) {
		GCE_LOG_D("[GCE]%s, %d",__func__,__LINE__);
		vListInsertEnd(&client->task_busy_list, &task->list_entry);
		WARN_ON(cmdq_thread_reset(s_cmdq, client) < 0);
		mt_sip_set_cmdq_sreg(client);

		GCE_LOG_D("cmdq task %p~%p, thread->base=%p\n",
			    task->pa_base,
			    (task->pa_base+pkt->cmd_buf_size),
			    client->base);

		writel(task->pa_base, client->base + CMDQ_THR_CURR_ADDR);
		writel(task->pa_base + pkt->cmd_buf_size,
		       client->base + CMDQ_THR_END_ADDR);
		writel(CMDQ_THR_IRQ_EN, client->base + CMDQ_THR_IRQ_ENABLE);
		writel(CMDQ_THR_ENABLED, client->base + CMDQ_THR_ENABLE_TASK);

	} else {
		GCE_LOG_D("[GCE]%s, %d",__func__,__LINE__);
		vListInsertEnd(&client->task_busy_list, &task->list_entry);
		WARN_ON(cmdq_thread_suspend(s_cmdq, client) < 0);
		curr_pa = readl(client->base + CMDQ_THR_CURR_ADDR);
		end_pa = readl(client->base + CMDQ_THR_END_ADDR);

		GCE_LOG_D("cmdq curr task %p~%p, thread->base=%p\n",
					(void *)curr_pa,
					(void *)end_pa, client->base);

		/*
		 * Atomic execution should remove the following wfe, i.e. only
		 * wait event at first task, and prevent to pause when running.
		 */
		if (client->atomic_exec) {
			/* GCE is executing if command is not WFE */
			if (!cmdq_thread_is_in_wfe(client)) {
				cmdq_thread_resume(client);
				cmdq_thread_wait_end(client, end_pa);
				WARN_ON(cmdq_thread_suspend(s_cmdq, client) < 0);
				/* set to this task directly */
				writel(task->pa_base,
				       client->base + CMDQ_THR_CURR_ADDR);
			} else {
				PRINTF_D("[GCE]%s, %d",__func__,__LINE__);
				cmdq_task_insert_into_thread(task);
				cmdq_task_remove_wfe(task);
			}
		} else {
			/* check boundary */
			if (curr_pa == end_pa - CMDQ_INST_SIZE ||
			    curr_pa == end_pa) {
				/* set to this task directly */
				writel(task->pa_base,
				       client->base + CMDQ_THR_CURR_ADDR);
			} else {
				cmdq_task_insert_into_thread(task);
			}
		}
		writel(task->pa_base + pkt->cmd_buf_size,
		       client->base + CMDQ_THR_END_ADDR);
		cmdq_thread_resume(client);
	}

}

static void cmdq_task_exec_done(struct cmdq_task *task, bool err)
{
	struct cmdq_cb_data cmdq_cb_data;

	if (task->pkt->cb.cb) {

		cmdq_cb_data.err = err;
		cmdq_cb_data.data = task->pkt->cb.data;
		task->pkt->cb.cb(cmdq_cb_data);
	}

             uxListRemove(&task->list_entry);

}

static void cmdq_task_handle_error(struct cmdq_task *task, u32 pa_curr)
{
	struct cmdq_client *client = task->client;
	struct cmdq_task *next_task;
	List_t *list = &client->task_busy_list;

	PRINTF_E("task 0x%p error\n", task);
	WARN_ON(cmdq_thread_suspend(task->cmdq, client) < 0);
	next_task = list_first_entry_or_null(list);
	if (next_task)
		writel(next_task->pa_base, client->base + CMDQ_THR_CURR_ADDR);
	cmdq_thread_resume(client);
}

void cmdq_task_status(struct cmdq_client *client, struct cmdq_pkt *pkt)
{
	GCE_LOG_E("curr_pa: %p task_end_pa:%p, pkt:%p, cmd_buf_size: %u",
		(void *) readl(client->base + CMDQ_THR_CURR_ADDR), (void *)readl(client->base + CMDQ_THR_END_ADDR),
		(void *)pkt->pa_base, pkt->cmd_buf_size);
}

static void cmdq_thread_irq_handler(struct cmdq *cmdq,
				    struct cmdq_client *client)
{
	struct cmdq_task *task,  *curr_task = NULL;
	u32 curr_pa, irq_flag, task_end_pa;
	bool err;
	List_t * list_tmp = &client->task_busy_list;
	ListItem_t *pos, *n;

	irq_flag = readl(client->base + CMDQ_THR_IRQ_STATUS);
	GCE_LOG_D("[GCE]%s, %d, irq_flag: %d\n",__func__, __LINE__, irq_flag);
	writel(~irq_flag, client->base + CMDQ_THR_IRQ_STATUS);

	/*
	 * When ISR call this function, another CPU core could run
	 * "release task" right before we acquire the spin lock, and thus
	 * reset / disable this GCE thread, so we need to check the enable
	 * bit of this GCE thread.
	 */
	if (!(readl(client->base + CMDQ_THR_ENABLE_TASK) & CMDQ_THR_ENABLED))
		return;

	if (irq_flag & CMDQ_THR_IRQ_ERROR)
		err = true;
	else if (irq_flag & CMDQ_THR_IRQ_DONE)
		err = false;
	else
		return;
	curr_pa = readl(client->base + CMDQ_THR_CURR_ADDR);
	task_end_pa = readl(client->base + CMDQ_THR_END_ADDR);

	GCE_LOG_D("cmdq status %p~%p, flag=%x\n", (void *)(unsigned long)curr_pa,
		(void *)(unsigned long)task_end_pa, irq_flag);

	list_for_each_entry_safe(pos, n, list_tmp) {
		task = pos->pvOwner;
		task_end_pa = task->pa_base + task->pkt->cmd_buf_size;



		if (curr_pa >= task->pa_base && curr_pa < task_end_pa)
			curr_task = task;

		if (!curr_task || curr_pa == task_end_pa - CMDQ_INST_SIZE) {
			cmdq_task_exec_done(task, false);
			//vPortFree(task);
		} else if (err) {
			cmdq_task_exec_done(task, true);
			cmdq_task_handle_error(curr_task, curr_pa);
			//vPortFree(task);
		}

		if (curr_task)
			break;
	}

	if (listLIST_IS_EMPTY(&client->task_busy_list)) {
		cmdq_thread_disable(cmdq, client);
	} else {
		GCE_LOG_E("[ERROR]%s, %d\n",__func__, __LINE__);
	}
}

static void cmdq_irq_handler(int irq, void *pcmdq)
{

	struct cmdq *cmdq = (struct cmdq *)pcmdq;
	unsigned long irq_status;
	struct cmdq_client *client;
	GCE_THREAD_INDEX i;

	irq_status = readl(cmdq->base + CMDQ_CURR_IRQ_STATUS);
	irq_status = ~irq_status; /* Low active */
	GCE_LOG_D("cmdq IRQ_STATUS: %x, %x\n", (u32)irq_status,
		(u32)(irq_status & CMDQ_IRQ_MASK));
	irq_status &= CMDQ_IRQ_MASK;

	if (irq_status == 0)
		return;

	for (i = GCE_CM4_DRM; i < GCE_CM4_NUM; i++) {
		client = &cmdq->client[i];
		if (irq_status & BIT(client->thread)) {
			/*GCE_LOG_D("cmdq bit=%d, thread->base=%p\n",
				client->thread, client->base);*/
			cmdq_thread_irq_handler(cmdq, client);
		}
	}

	return ;
}

int cmdq_msg_send_data(struct cmdq_client *client, struct cmdq_pkt *data)
{
	taskENTER_CRITICAL();
	cmdq_task_exec(data, client);
	taskEXIT_CRITICAL();
	return 0;
}

struct cmdq_client*cmdq_xlate(const struct gce_index*sp)
{
	GCE_THREAD_INDEX ind = sp->gce_index;
	struct cmdq_client *client;

	if (ind >= s_cmdq->num_clients)
		return (void*)(-EINVAL);

	client = &s_cmdq->client[ind];
	client->atomic_exec = sp->atomic_flag ;

	return client;
}

int gce_init()
{
	int i;

	s_cmdq = pvPortMalloc(sizeof(struct cmdq));
	memset(s_cmdq, 0, sizeof(*s_cmdq));
	if (!s_cmdq)
		return -ENOMEM;

             s_cmdq->base = (void *)GCE_BASE;

	s_cmdq->irq = CQ_DMA_IRQ_BIT;/*irq*/

	request_irq(s_cmdq->irq, cmdq_irq_handler, IRQ_TYPE_LEVEL_LOW,
			       "mtk_cmdq", s_cmdq);

	GCE_LOG_E("cmdq device: va:0x%p, irq:%d\n",
		s_cmdq->base, s_cmdq->irq);

	s_cmdq->num_clients = GCE_CM4_NUM;

	/* Map SW thread index to HW thread index */
	s_cmdq->client[GCE_CM4_DRM].thread = 7;
	s_cmdq->client[GCE_CM4_MDP].thread = 8;

	for (i = 0; i < ARRAY_SIZE(s_cmdq->client); i++) {
		s_cmdq->client[i].base = s_cmdq->base + CMDQ_THR_BASE +
				CMDQ_THR_SIZE * s_cmdq->client[i].thread;
		vListInitialise(&s_cmdq->client[i].task_busy_list);
		s_cmdq->client[i].cmplt.queuehandle_t = xQueueCreate(1,sizeof(struct cmdq_flush_completion));
	}

	return 0;
}
