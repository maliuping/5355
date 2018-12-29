#include "scp_ipi.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <driver_api.h>
#include <mt_printf.h>
#include <interrupt.h>
#include <semphr.h>
#include "scp_sem.h"


#include "FreeRTOS.h"
//#include "gicv2.h"
#include "uid_queue_task.h"
#include "scp_uid_queue.h"
#ifdef CFG_DSP_IPC_SUPPORT_TEST
#include "test.h"
#endif
#ifdef CFG_DSP_IPC_LOG
#include "ipi_log.h"
#endif
#ifdef CFG_DSP_IPC_WDT
#include "ipi_wdt.h"
#endif


#define MAX_POLLING_COUNT 50

static ipi_msg_t *scp_send_obj, *scp_rcv_obj;
static uint64_t p_driver;
static uint8_t num_noncached_alloc;
static NONCACHED_INFO *p_first, *p_temp, *p_header;
static uint32_t size_noncached_mem_allocated;
static IPC_MEM_HEADER *ipc_mem_header;

//static IPI_IRQ_STATUS *ipi_irq_send_status;
//static IPI_IRQ_STATUS *ipi_irq_recv_status;


static uint32_t *rcv_ipc_status;
static uint32_t *send_ipc_status;


static SemaphoreHandle_t m_d_mutex;
static SemaphoreHandle_t m_i_mutex;
static SemaphoreHandle_t f_mutex;

static SemaphoreHandle_t ipi_send_mutex;


ipi_msg_t *get_rcv_obj(void)
{
	return scp_rcv_obj;
}

ipi_msg_t *get_send_obj(void)
{
	return scp_send_obj;
}

void mtk_irq_clr(void)
{
	writel(IRQ_TO_CMSYS_CLR, SCP_EVENT_CLR_REG);
}

void mtk_ap_irq_trigger(void)
{
	writel(IRQ_TO_AP, SCP_EVENT_CTL_REG);
}

/*
 * check if the ap irq processing is complete, ap should
 * clear irq status at the end of irq handler.
 * @param : none
 * return 0 : ap is processing irq
 *		  1 : ap irq processing is complete
 */

inline int mtk_ap_irq_status(void)
{
	return mtk_check_reg_bit(0, SCP_EVENT_CTL_REG);
}

void set_rcv_ipc_ready(void)
{
	*rcv_ipc_status = IRQ_STATUS_READY;
}

bool get_send_ipc_ready(void)
{
	if (*send_ipc_status == IRQ_STATUS_READY)
		return true;
	else
		return false;
}


void scp_ipi_init(void)
{
	int i;
	scp_send_obj = (ipi_msg_t *)IPI_MSG_SEND_BUFFER;
	scp_rcv_obj = (ipi_msg_t *)IPI_MSG_RECV_BUFFER;
	PRINTF_E("scp_send_obj = %p \n\r", scp_send_obj);

	PRINTF_E("SCP_IPC_SHARE_BASE = 0x%x  SCP_OS_HEAP_BASE 0x%x SCP_SHARE_MEM_OFFSET 0x%x\n\r",
		SCP_IPC_SHARE_BASE, SCP_OS_HEAP_BASE, SCP_SHARE_MEM_OFFSET);
	PRINTF_E("scp_rcv_obj = %p \n\r", scp_rcv_obj);
	memset_to_scp(scp_send_obj, 0, sizeof(ipi_msg_t));

	send_ipc_status = (uint32_t *)(IPI_MEM_IRQ_STATUS);
	rcv_ipc_status = (uint32_t *)(IPI_MEM_IRQ_STATUS + sizeof(uint32_t));

	p_driver = UNCACHED_ORGIN_ADDRESS;
	ipc_mem_header = (IPC_MEM_HEADER *)IPI_ALLOCATOR_HEADER;
	for (i = 0; i < IPI_ALLOCATOR_MARK_SIZE; i++)
		ipc_mem_header->mark_req_mem[i] = 0;
	for (i = 0; i < IPI_ALLOCATOR_ELEMENT_COUNT; i++)
		ipc_mem_header->record_size[i] = 0;
	m_d_mutex = xSemaphoreCreateMutex();
	m_i_mutex = xSemaphoreCreateMutex();
	f_mutex = xSemaphoreCreateMutex();
	ipi_send_mutex = xSemaphoreCreateMutex();
	uid_queue_init();

	p_first = (NONCACHED_INFO *)pvPortMalloc(sizeof(NONCACHED_INFO));
	p_first->data_addr = NULL;
	p_first->data_len = 0;
	p_first->p_next = NULL;
	p_temp = p_first;
	p_header = (NONCACHED_INFO *)pvPortMalloc(sizeof(NONCACHED_INFO));
	p_header->p_next = NULL;
	num_noncached_alloc = 0;
	size_noncached_mem_allocated = 0;

	task_ipi_init();
	set_rcv_ipc_ready();
#ifdef CFG_DSP_IPC_LOG
	ipi_log_init();
#endif

#ifdef CFG_DSP_IPC_WDT
	ipi_wdt_init();
#endif

#ifdef CFG_DSP_IPC_SUPPORT_TEST
	test_init();
#endif

	PRINTF_E("scp_ipi_init(-) IPI_MSG_SEND_BUFFER 0x%08x \n", IPI_MSG_SEND_BUFFER);
}

IPI_STATUS scp_ipi_send(FUNC_ID func_id, void *data_addr)
{

	int polling_try_count;
	polling_try_count = 0;

	if (is_in_isr()) {
			PRINTF_E("scp_ipi_send: cannot use in isr\n");
			return IPI_RET_NG;
	}

	xSemaphoreTake(ipi_send_mutex, portMAX_DELAY);

	while (scp_send_obj->state != IPI_RET_OK && polling_try_count < MAX_POLLING_COUNT) {
		vTaskDelay(1);
		polling_try_count++;
	}

	if (polling_try_count == MAX_POLLING_COUNT) {
		PRINTF_E("scp_ipi_send busy\n");
		xSemaphoreGive(ipi_send_mutex);
		return IPI_RET_BUSY;
	}
	PRINTF_E("%s  \n", __func__);
	scp_send_obj->func_id = func_id;
	scp_send_obj->payload = data_addr;
	scp_send_obj->state = IPI_RET_BUSY;

	mtk_ap_irq_trigger();

	xSemaphoreGive(ipi_send_mutex);

	return IPI_RET_OK;
}

void print_mm_header(void)
{
	int i;
	for (i = 0; i < IPI_ALLOCATOR_MARK_SIZE; i++)
		PRINTF_E("ALLOCATOR_MARK_SIZE mm_header[%d] = 0x%x	\n", i, ipc_mem_header->mark_req_mem[i]);
}

DATA_ADDR noncached_mem_alloc(uint32_t size)
{
	DATA_ADDR p = NULL;
	xSemaphoreTake(m_d_mutex, portMAX_DELAY);
	if ((size_noncached_mem_allocated + size) < (UNCACHED_ORGIN_ADDRESS + UNCACHED_FOR_DRIVER_LENGTH)) {
		if (num_noncached_alloc == 0) {
			p_first->data_addr = (DATA_ADDR)((uint64_t)UNCACHED_ORGIN_ADDRESS);
			p_first->data_len = size;
			p_temp->p_next = p_first;
			PRINTF_E("first data_addr %p data_len 0x%x	\n", p_temp->data_addr, p_temp->data_len);
		} else {
			NONCACHED_INFO *p_new = (NONCACHED_INFO *)pvPortMalloc(sizeof(NONCACHED_INFO));
			p_new->p_next = NULL;
			p_new->data_addr = p_temp->data_addr + p_temp->data_len;
			p_new->data_len = size;
			p_temp->p_next = p_new;
			p_temp = p_new;
			PRINTF_E("data_addr %p data_len 0x%x  \n", p_temp->data_addr, p_temp->data_len);
		}
		num_noncached_alloc++;
		size_noncached_mem_allocated += size;
		p = p_temp->data_addr;
		xSemaphoreGive(m_d_mutex);
		return p;
	} else {
		xSemaphoreGive(m_d_mutex);
		return NULL;
	}

}


DATA_ADDR ipc_mem_alloc(uint32_t size)
{
	int i, j, k, m, n, t_start, t_end, t_count;
	void *p = NULL;
	if (size > IPI_ALLOCATOR_ELEMENT_SIZE*IPI_ALLOCATOR_ELEMENT_COUNT) {
		return NULL;
	}

	if (size%IPI_ALLOCATOR_ELEMENT_SIZE) {
		return NULL;
	}
	PRINTF_E("%s(+) \n", __func__);

	xSemaphoreTake(m_i_mutex, portMAX_DELAY);
	m = size/IPI_ALLOCATOR_ELEMENT_SIZE;

	for (i = 0; i < IPI_ALLOCATOR_ELEMENT_COUNT-m; i++) {
		PRINTF_E("malloc_ipc i = %d \n", i);
		n = i + m;
		t_start = i/32;
		t_end = n/32;
		t_count = t_end - t_start;
		if (t_count == 0) {
			if (ipc_mem_header->mark_req_mem[t_start] & ((1 << m)-1) << i) {
				continue;
			} else {
				PRINTF_E("malloc_ipc  __i = %d  m = %d, ipc_mem_header = 0x%x new mark = 0x%x \n", i, m, ipc_mem_header->mark_req_mem[t_start], ((1<<m)-1)<<i);
				ipc_mem_header->mark_req_mem[t_start] |= (((1<<m)-1)<<i);
				ipc_mem_header->record_size[i] = m;
				p = (void *)(IPI_ALLOCATOR_BUFFER + IPI_ALLOCATOR_ELEMENT_SIZE*i);
				PRINTF_E("malloc_ipc p= %p, \n", p);
				print_mm_header();
				xSemaphoreGive(m_i_mutex);
				return p;
			}
		} else if (t_count > 0) {
			if (ipc_mem_header->mark_req_mem[t_start] &  ~((1 << i%32) - 1)) {
				continue;
			} else if (ipc_mem_header->mark_req_mem[t_end] & ((1 << n%32) - 1)) {
				continue;
			}

			if (t_count == 1) {
				ipc_mem_header->mark_req_mem[t_start] |=  ~((1 << i%32) - 1);
				ipc_mem_header->mark_req_mem[t_end] |= ((1 << n%32) - 1);
				ipc_mem_header->record_size[i] = m;
				p = (void *)(IPI_ALLOCATOR_BUFFER + IPI_ALLOCATOR_ELEMENT_SIZE*i);
				PRINTF_E("malloc_ipc p= %p, \n", p);
				print_mm_header();
				xSemaphoreGive(m_i_mutex);
				return p;
			} else if (t_count > 1) {
				for (j = t_start + 1; j < t_end; j++) {
					if (ipc_mem_header->mark_req_mem[j] > 0) {
						break;
					}
				}
				if (j == t_end) {
					ipc_mem_header->mark_req_mem[t_start] |=  ~((1 << i%32) - 1);
					ipc_mem_header->mark_req_mem[t_end] |= ((1 << n%32) - 1);
					for (k = t_start + 1; k < t_end; k++)
						ipc_mem_header->mark_req_mem[k] = ~ipc_mem_header->mark_req_mem[k];
					ipc_mem_header->record_size[i] = m;
					p = (void *)(IPI_ALLOCATOR_BUFFER + IPI_ALLOCATOR_ELEMENT_SIZE*i);
					PRINTF_E("malloc_ipc p= %p, \n", p);
					xSemaphoreGive(m_i_mutex);
					print_mm_header();
					return p;
				} else {
					continue;
				}
			}
		}
	}
	xSemaphoreGive(m_i_mutex);
	print_mm_header();

	return NULL;

}

void ipc_mem_free(DATA_ADDR ptr)
{
	int i, id, t_start, t_end, t_count;
	//uint32_t p_inter = (signed int)((uint64_t)p);
   // uint64_t p_inter = (uint64_t)p;
	PRINTF_E("free_ipc ptr= %p, \n", ptr);
	id = ((uint32_t)ptr - IPI_ALLOCATOR_BUFFER)/IPI_ALLOCATOR_ELEMENT_SIZE;
	xSemaphoreTake(f_mutex, portMAX_DELAY);
	t_start = id/32;
	t_end = (id + ipc_mem_header->record_size[id])/32;
	t_count = t_end - t_start;

	PRINTF_E("free_ipc id =%d, record_size %d \n", id, ipc_mem_header->record_size[id]);
	if (t_count == 0) {
		PRINTF_E("free_ipc t_start 0x%x  .. 0x%x\n", ipc_mem_header->mark_req_mem[t_start], ~(((1 << ipc_mem_header->record_size[id]) - 1) << (id%32)));
		ipc_mem_header->mark_req_mem[t_start] &= ~(((1 << ipc_mem_header->record_size[id]) - 1)<<(id%32));
	} else if (t_count > 0) {
		ipc_mem_header->mark_req_mem[t_start] &= ((1 << id%32) - 1);
		ipc_mem_header->mark_req_mem[t_end] &= ~((1 << (id + ipc_mem_header->record_size[id])%32) - 1);
		if (t_count > 1) {
			for (i = t_start + 1; i < t_end; i++)
				ipc_mem_header->mark_req_mem[i] = 0;
		}
	}
	ipc_mem_header->record_size[id] = 0;
	ptr = NULL;
	print_mm_header();
	xSemaphoreGive(f_mutex);


}

