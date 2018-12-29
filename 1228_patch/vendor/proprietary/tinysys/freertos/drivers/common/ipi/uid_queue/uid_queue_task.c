//#include <wrapped_errors.h>

#include "FreeRTOS.h"
#include <FreeRTOSConfig.h>
#include <string.h>
#include <driver_api.h>
#include <mt_printf.h>
#include "scp_regs.h"
#include "scp_sem.h"
#include "interrupt.h"


#include "timers.h"
#include "semphr.h"
#include "uid_queue_task.h"
#include "ipi_assert.h"
//#include "gicv2.h"
#include "scp_uid_queue.h"


#define size_buf_1k 0x400
#define size_buf_128k 0x20000
#define size_buf_1M 0x100000

static DspTask *ipi_task;
//static struct ipi_msg_t *test_msg;
//static uint32_t time_1, time_2, time_thread;
//static uint64_t time_3, time_4;
//static ipi_msg_t ipi_queue_

//#define MAX_MSG_QUEUE_SIZE 128


queue_msg_t *uid_queue_rcv_addr;
static SCP_IPI_DESC scp_ipi_desc[NUM_IPI_FUNC];
static IPI_UID_QUEUE_DESC uid_queue_desc[NUM_MSG_ID];

UIDQHEADER *uid_queue_rcv_header;


static uint32_t recv_mark_id[MARK_SIZE];
static uint32_t recv_mark_id_before[MARK_SIZE];
static uint32_t msg_array[NUM_MSG_ID][MARK_SIZE];


static void		task_ipi_constructor(DspTask *this);
static void		task_ipi_destructor(DspTask *this);

static void		task_ipi_create_task_loop(DspTask *this);
static void		task_ipi_destroy_task_loop(DspTask *this);

static void		task_ipi_task_loop(void *pvParameters);
//static void	  task_ipi_task_loop_test(void *pvParameters);

static IPI_STATUS task_ipi_recv_message(
		struct DspTask *this,
		void *data);

void memcpy_from_scp(void *trg, void *src, int size)
{
	int i;
	uint32_t *t = trg;
	uint32_t *s = src;

	for (i = 0; i < ((size + 3) >> 2); i++)
		*t++ = *s++;
}

void memcpy_to_scp(void *trg, const void *src, int size)
{
	int i;
	uint32_t *t = trg;
	const uint32_t *s = src;

	for (i = 0; i < ((size + 3) >> 2); i++)
		*t++ = *s++;
}


void memset_to_scp(void *trg, int val, int size)
{
	int i;
	uint32_t *t = trg;
	for (i = 0; i < ((size + 3) >> 2); i++)
		*t++ = val;
}





void unmark_recv_mark_id(int i, int j)
{
	UBaseType_t recv_mark_id_lock;
	recv_mark_id_lock = portSET_INTERRUPT_MASK_FROM_ISR();
	recv_mark_id_before[i] &= ~(1<<j);
	portCLEAR_INTERRUPT_MASK_FROM_ISR(recv_mark_id_lock);

}


IPI_STATUS scp_ipi_register(FUNC_ID func_id,
	void (*ipi_handler)(DATA_ADDR data_addr))
{
	if (func_id < NUM_IPI_FUNC) {
		if (ipi_handler == NULL)
			return IPI_RET_NG;
		scp_ipi_desc[func_id].handler = ipi_handler;
		return IPI_RET_OK;
	} else {
		return IPI_RET_NG;
	}
}


IPI_UID_RET_STATUS scp_uid_queue_register(QUEUE_MSG_ID msg_id,
	void (*uid_msg_handler)(void *private, char *data), void *private)
{

	if (msg_id < NUM_MSG_ID) {
		uid_queue_desc[msg_id].private = private;
		if (uid_msg_handler == NULL)
			return IPI_UID_RET_NG;
		uid_queue_desc[msg_id].handler = uid_msg_handler;
		return IPI_UID_RET_OK;
	} else {
		return IPI_UID_RET_NG;
	}

}

static uint8_t get_queue_idx(DspTask *this)
{
	uint8_t queue_idx = this->queue_idx;

	this->queue_idx++;
	if (this->queue_idx == NUM_MSG_ID) {
		this->queue_idx = 0;
	}

	return queue_idx;
}

static void task_ipi_constructor(DspTask *this)
{

	PRINTF_D("%s(+),\n", __func__);

	IPI_ASSERT(this != NULL);

	this->state = TASK_IDLE;

	this->queue_idx = 0;
	this->num_queue_element = 0;

	this->msg_idx_queue = xQueueCreate(NUM_MSG_ID, sizeof(uint8_t));

	IPI_ASSERT(this->msg_idx_queue != NULL);

	PRINTF_D("%s(-)\n", __func__);

}

static void task_ipi_destructor(DspTask *this)
{
	if (this == NULL) {
		PRINTF_E("%s(), this is NULL!!\n", __func__);
		return;
	}

	PRINTF_D("%s(+)\n", __func__);

	if (this->msg_idx_queue != NULL) {
		vQueueDelete(this->msg_idx_queue);
	}
	PRINTF_D("%s(-)\n", __func__);
}


static void task_ipi_task_loop(void *pvParameters)
{
	int i, j;
	DspTask *this = (DspTask *)pvParameters;
	queue_msg_t *temp_msg;
	uint8_t local_queue_idx = 0xFF;
	// struct UIDQHEADER *header = (struct UIDQHEADER *)(IPI_UID_QUEUE_HEADER + UIDQUE_HEADER_SIZE);
	//struct msg_t *msg;
	while (1) {
		if (xQueueReceive(this->msg_idx_queue, &local_queue_idx,
																		portMAX_DELAY) == pdTRUE) {
			PRINTF_E("task_ipi_task_loop received = %d\n", local_queue_idx);
		}

		PRINTF_E("%s(+) mark_id = 0x %08x %08x %08x %08x \n",  __func__, msg_array[local_queue_idx][3],
			msg_array[local_queue_idx][2], msg_array[local_queue_idx][1], msg_array[local_queue_idx][0]);
		//PRINTF_E("%s(+) while\n", __func__);

		for (i = 0; i < MARK_SIZE; i++) {
			if (msg_array[local_queue_idx][i] > 0) {
				for (j = 0; j < 32; j++) {
					if (msg_array[local_queue_idx][i] & 1<<j) {
							//PRINTF_E("%s(+) id %d handler %p \n", __func__, i*32+j, uid_queue_desc[i*32+j].handler);
						if (uid_queue_desc[i*32+j].handler != NULL) {
							temp_msg = uid_queue_rcv_addr + i*32 + j;
							uid_queue_desc[i*32+j].handler(uid_queue_desc[i*32+j].private, temp_msg->data);
							uid_queue_rcv_header->mark_id[i*32+j] = MSG_ID_UNMARKED;
							unmark_recv_mark_id(i, j);
						} else {
							PRINTF_E("uid_queue_desc handler is null or abnormal, msg_id = %d \n", i*32+j);
						}
					}
				}
			}
		}
		this->num_queue_element--;
	}
}

void scp_irq_handler(int irq, void *pdata)
{
	(void)irq;
	(void)pdata;
	PRINTF_E("%s+ \n", __func__);

	ipi_msg_t *scp_rcv_obj = get_rcv_obj();

	void *p_data = (void *)scp_rcv_obj->payload;

	if (scp_ipi_desc[scp_rcv_obj->func_id].handler) {
		scp_ipi_desc[scp_rcv_obj->func_id].handler(p_data);
	} else {
		PRINTF_E("%s scp_ipi_desc handler is null or abnormal \n", __func__);
	}

	mtk_irq_clr();
}


void uid_queue_handler(void *data)
{
	//PRINTF_D("%s+ \n", __func__);
	ipi_task->recv_message(ipi_task, data);

}

/*
static void task_ipi_task_loop_test(void *pvParameters)
{
	int cnt = 0;
	DspTask *task_test = (DspTask *)pvParameters;
	while (1) {
		PRINTF_E("%s(+) cnt = %d \n", __func__, cnt);
		vTaskDelay(1000);

		if (!get_recv_irq_gic_ready()) {
			PRINTF_E("%s(+) try_count = %d \n", __func__, cnt);
			vTaskDelay(200);
		} else {
			PRINTF_E("%s(+) get_recv_irq_gic_ready	\n", __func__);
			//request_irq(IPC_IRQ_RECV_NUMBER, scp_irq_handler, "IPC");
			set_recv_irq_handler_ready();
			PRINTF_E("%s(+) set_recv_irq_handler_ready	\n", __func__);
			vTaskDelete(task_test->freertos_task);

		}

		cnt++;
	}

}

*/

static void task_ipi_create_task_loop(DspTask *this)
{
	/* Note: you can also bypass this function,
		and do kal_xTaskCreate until you really need it.
		Ex: create task after you do get the enable message.
	*/
	PRINTF_D("%s(+)\n", __func__);
	BaseType_t xReturn = pdFAIL;

	xReturn = xTaskCreate(
				task_ipi_task_loop,
				LOCAL_TASK_NAME,
				LOCAL_TASK_STACK_SIZE,
				(void *)this,
				LOCAL_TASK_PRIORITY,
				&this->freertos_task);

	IPI_ASSERT(xReturn == pdPASS);
	IPI_ASSERT(this->freertos_task);

}


static void task_ipi_destroy_task_loop(DspTask *this)
{
#if (INCLUDE_vTaskDelete == 1)
	if (this->freertos_task) {
		vTaskDelete(this->freertos_task);
	}
#endif
}

static IPI_STATUS task_ipi_recv_message(
	DspTask *this,
	void *data)
{
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	UBaseType_t recv_mark_id_lock;
	uint8_t queue_idx = 0;

	this->num_queue_element++;
	queue_idx = get_queue_idx(this);
	UIDQHEADER *queue_header = (UIDQHEADER *)data;

	int i, j;
	for (i = 0; i < MARK_SIZE; i++) {
		for (j = 0; j < 32; j++) {
			//PRINTF_D("%s(+) i*32+j %d %d \n", __func__, i*32+j, queue_header->mark_id[i*32+j]);
			recv_mark_id[i] |= queue_header->mark_id[i*32+j] << j;
		}
	}
	recv_mark_id_lock = portSET_INTERRUPT_MASK_FROM_ISR();

	for (i = 0; i < MARK_SIZE; i++) {
		msg_array[queue_idx][i] = recv_mark_id[i] & (~recv_mark_id_before[i]);
	}

	for (i = 0; i < MARK_SIZE; i++) {
		recv_mark_id_before[i] = recv_mark_id[i];
		recv_mark_id[i] = 0;
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR(recv_mark_id_lock);

	if (xQueueSendToBackFromISR(this->msg_idx_queue, &queue_idx,
														&xHigherPriorityTaskWoken) != pdTRUE) {
		PRINTF_E("%s(+) xQueueSendToBackFromISR fail \n", __func__);
		//header->state = HEADER_FREE;
		return IPI_RET_NG;
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return IPI_RET_OK;
}


void create_ipi_task(void)
{
	PRINTF_D("%s(+)\n", __func__);

	/* alloc object here */
	DspTask *task = (DspTask *)pvPortMalloc(sizeof(DspTask));

	if (task == NULL) {
		PRINTF_E("%s(), pvPortMalloc fail!!\n", __func__);
		return;
	}
	memset(task, 0, sizeof(DspTask));

	/* only assign methods, but not class members here */
	task->constructor		= task_ipi_constructor;
	task->destructor		= task_ipi_destructor;

	task->create_task_loop	= task_ipi_create_task_loop;
	task->destroy_task_loop = task_ipi_destroy_task_loop;

	task->recv_message		= task_ipi_recv_message;

	if (task == NULL) {
		PRINTF_E("%s(-), task == NULL!! return\n", __func__);
		return;
	}

	/* constructor */
	if (task->constructor != NULL) {
		task->constructor(task);
	}

	if (task->create_task_loop != NULL) {
		task->create_task_loop(task);
	}

	ipi_task = task;
	PRINTF_D("%s(-), task = %p\n", __func__, task);

}

/*
void create_ipi_task_test(void)
{

	PRINTF_E("%s(+)\n", __func__);
	BaseType_t xReturntest = pdFAIL;
	DspTask *tasktest = (DspTask *)pvPortMalloc(sizeof(DspTask));
	xReturntest = xTaskCreate(
				  task_ipi_task_loop_test,
				  "test",
				  LOCAL_TASK_STACK_SIZE,
				  tasktest,
				  LOCAL_TASK_PRIORITY,
				  &tasktest->freertos_task);

	IPI_ASSERT(xReturntest == pdPASS);

}
*/

DspTask *get_ipi_task()
{
	if (ipi_task == NULL) {
		PRINTF_D("%s(), ipi_task == NULL\n", __func__);
		return NULL;
	}
	return ipi_task;
}

void task_ipi_delete(DspTask *task)
{
	PRINTF_D("%s(+)\n", __func__);

	if (task == NULL) {
		PRINTF_D("%s(), task is NULL!!\n", __func__);
		return;
	}

	vPortFree(task);

	PRINTF_D("%s(-)\n", __func__);
}


int destroy_ipi_task(void)
{
	PRINTF_D("%s(+)\n\n", __func__);

	DspTask *task = NULL;

	/* cannot destroy null */
	IPI_ASSERT(ipi_task != NULL);

	/* remove task address in factory */
	task = ipi_task;
	ipi_task = NULL;

	/* destructor */
	if (task->destroy_task_loop != NULL) {
		task->destroy_task_loop(task);
	}
	if (task->destructor != NULL) {
		task->destructor(task);
	}

	/* delete task */
	task_ipi_delete(task);

	PRINTF_D("%s(-)\n", __func__);
	return 0;
}


void task_ipi_init(void)
{
	int i;
	request_irq(SOFT_NS_IRQ_BIT, scp_irq_handler, IRQ_TYPE_LEVEL_LOW, "scp_ipi", NULL);
	PRINTF_E("%s(+),  request_irq \n", __func__);
	for (i = 0; i < MARK_SIZE; i++) {
		recv_mark_id[i] = 0;
		recv_mark_id_before[i] = 0;
	}

	create_ipi_task();

	scp_ipi_register(IPI_FUNC_UID_QUEUE, uid_queue_handler);

	uid_queue_rcv_header = (UIDQHEADER *)IPI_UID_QUEUE_RECV_HEADER;
	uid_queue_rcv_addr = (queue_msg_t *)IPI_UID_QUEUE_RECV_BUFFER;
	PRINTF_E("%s(+),  uid_queue_rcv_addr %p \n", __func__, uid_queue_rcv_addr);

	//ipi_prty_init();

}

void task_ipi_deinit(void)
{
	destroy_ipi_task();
	//mask_irq(IPC_IRQ_NUMBER);
	//ipi_prty_init();
	IPI_ASSERT(ipi_task == NULL);
}

DspTask *get_task(void)
{
	if (ipi_task != NULL)
		return ipi_task;
	else {
		PRINTF_D("%s, task == NULL\n", __func__);
		return NULL;
	}
}
