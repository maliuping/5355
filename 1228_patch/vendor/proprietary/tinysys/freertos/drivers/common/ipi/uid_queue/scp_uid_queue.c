
#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <semphr.h>
#include <string.h>
#include <mt_printf.h>

#include "scp_uid_queue.h"
#include "scp_ipi.h"
#include "uid_queue_task.h"
#include "ipi_assert.h"

static void unique_id_queue_loop(void *pvParameters);

static queue_msg_t *send_queue_addr;

static UIDQTask *task_uidq;
static UIDQHEADER *uid_send_header;
static uint32_t uid_counter;

static SemaphoreHandle_t mark_id_mutex;




static void unique_id_queue_loop(void *pvParameters)
{
	UIDQTask *task_q = (UIDQTask *)pvParameters;
	while (1) {
		vTaskSuspend(task_q->u_task);
		 xSemaphoreTake(mark_id_mutex, portMAX_DELAY);
		 uid_counter = 0;
		 xSemaphoreGive(mark_id_mutex);
		 scp_ipi_send(IPI_FUNC_UID_QUEUE, uid_send_header);
	}
}


void create_queue_loop(UIDQTask *this)
{
	PRINTF_E("%s  \n", __func__);
	BaseType_t xReturn = pdFAIL;
	xReturn = xTaskCreate(
				unique_id_queue_loop,
				QUEUE_TASK_NAME,
				LOCAL_TASK_STACK_SIZE,
				this,
				LOCAL_TASK_PRIORITY,
				&this->u_task);
	IPI_ASSERT(xReturn == pdPASS);

}

void mark_msg_id_init(void)
{
	for (int i = 0; i < UNITUQ_QUEUE_ELEMENT_COUNT; i++)
		uid_send_header->mark_id[i] = MSG_ID_UNMARKED;
}


IPI_UID_RET_STATUS scp_uid_queue_send(queue_msg_t *msg)
{
	queue_msg_t *temp_msg;
	int cmp;
//	PRINTF_E("%s+ \n", __func__);

	if (!get_send_ipc_ready()) {
		PRINTF_E("%s+ IPI_UID_RET_IRQ_UNREADY \n", __func__);
		return IPI_UID_RET_IRQ_UNREADY;
	}

	if (msg->magic != IPI_MSG_MAGIC_NUMBER) {
		PRINTF_E("%s  PUSH_MAGIC_FAIL \n", __func__);
		return IPI_UID_RET_NG;
	}

	xSemaphoreTake(mark_id_mutex, portMAX_DELAY);
//	  PRINTF_E("%s+ msg_id %d  mark_id %d \n", __func__, msg->msg_id, uid_send_header->mark_id[msg->msg_id]);
	if (uid_send_header->mark_id[msg->msg_id] == MSG_ID_MARKED) {
		temp_msg = send_queue_addr + msg->msg_id;
		cmp = memcmp(temp_msg->data, msg->data, UNIQUE_ELEMENT_SIZE - 8);
		if (!cmp) {
			//PRINTF_E("%s	msg_id already marked, data same  \n", __func__);
			xSemaphoreGive(mark_id_mutex);
			return IPI_UID_RET_BUSY_DATA_SAME;
		} else {
			//PRINTF_E("%s	msg_id already marked, data diff  \n", __func__);
			xSemaphoreGive(mark_id_mutex);
			return IPI_UID_RET_BUSY_DATA_DIFF;
		}
	}

//	PRINTF_E("%s memcpy \n", __func__);
	memcpy_to_scp((void *)(send_queue_addr + msg->msg_id), (void *)msg, sizeof(queue_msg_t));

	uid_send_header->mark_id[msg->msg_id] = MSG_ID_MARKED;


	uid_counter++;
	xSemaphoreGive(mark_id_mutex);
	if (uid_counter > 0)
		vTaskResume(task_uidq->u_task);

	return IPI_UID_RET_OK;
}


void uid_queue_init(void)
{
	PRINTF_E("%s 1 \n", __func__);

	mark_id_mutex = xSemaphoreCreateMutex();


	uid_send_header = (UIDQHEADER *)IPI_UID_QUEUE_SEND_HEADER;
	//PRINTF_E("%s 2  uid_header = %p size = %d \n", __func__, uid_header, sizeof(UIDQHEADER));

	uid_send_header->magic = IPI_MSG_MAGIC_NUMBER;
	mark_msg_id_init();

	//PRINTF_E("%s 3  state = %d , id[0] %d \n", __func__, uid_header->state, uid_header->mark_id[0]);
	send_queue_addr = (queue_msg_t *)IPI_UID_QUEUE_SEND_BUFFER;
	memset_to_scp(send_queue_addr, 0, sizeof(queue_msg_t)*UNITUQ_QUEUE_ELEMENT_COUNT);
	//queue_buf_init(queue_buf);
	PRINTF_E("%s IPI_UID_QUEUE_SEND_HEADER = 0x%x, uid_send_header = %p, IPI_UID_QUEUE_SEND_BUFFER = 0x%x queue_buf = %p \n",
		__func__, IPI_UID_QUEUE_SEND_HEADER, uid_send_header, IPI_UID_QUEUE_SEND_BUFFER, send_queue_addr);

	task_uidq = (UIDQTask *)pvPortMalloc(sizeof(UIDQTask));
	if (task_uidq == NULL)
		PRINTF_E("%s pvPortMalloc UIDQTask fail \n", __func__);
	memset(task_uidq, 0, sizeof(UIDQTask));
	create_queue_loop(task_uidq);

}
