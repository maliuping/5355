
#include "ipi_log.h"


#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <semphr.h>
#include <task.h>
#include "timers.h"


#include "ipi_assert.h"

#include "scp_uid_queue.h"
#include "uid_queue_task.h"


#define MAX_LOG_NUM 20
#define LOG_TIME 1000

static uint32_t g_debug_level = LOG_DEBUG;


static SemaphoreHandle_t ipi_log_mutex;

static char printf_msg_inhouse[IPI_LOG_ELEMENT_SIZE];
static IPI_LOG_RING_BUFFER *ipi_log_header;
static char *ipi_log_buffer;
static queue_msg_t *queue_log_msg;
static uint32_t log_num;

static TaskHandle_t task_log;

static TimerHandle_t xTimer_log;





void vLogTimerCallback(TimerHandle_t xTimer)
{
	if (xTimer)
		vTaskResume(task_log);
		//PRINTF_E("------------ in %s, cnt %d ------------ \n", __func__, cnt);
}



void ipi_log_send(void)
{
	int try_count = 50;
	int i;
	int retval = IPI_UID_RET_NG;
	bool send_fail = false;

	PRINTF_E("%s  1 \n", __func__);

	for (i = 0; i < try_count; i++) {
		retval = scp_uid_queue_send(queue_log_msg);
		if (retval == IPI_UID_RET_OK)
			break;
		send_fail = true;
		vTaskDelay(100);
	}

	if (send_fail)
		PRINTF_E("%s+ send_fail try_count = %d\n", __func__, try_count);

	memset((void *)(queue_log_msg->data), 0, (UNIQUE_ELEMENT_SIZE-8));

}

static void task_log_loop(void *pvParameters)
{
	bool log_need_send = false;
	while (1) {
		PRINTF_E("%s  \n", __func__);
		vTaskSuspend(task_log);
		xSemaphoreTake(ipi_log_mutex, portMAX_DELAY);
		if (log_num > 0) {
			memcpy((void *)queue_log_msg->data, (void *)&log_num, sizeof(uint32_t));
			log_num = 0;
			log_need_send = true;
		}
		xSemaphoreGive(ipi_log_mutex);

		if (log_need_send == true) {
			PRINTF_E("%s need send \n", __func__);
			ipi_log_send();
			log_need_send = false;
		}
	}
}

void create_task_log(TaskHandle_t *this)
{
	PRINTF_E("%s  \n", __func__);
	BaseType_t xReturn = pdFAIL;
	xReturn = xTaskCreate(
				task_log_loop,
				"task_log",
				LOCAL_TASK_STACK_SIZE,
				NULL,
				LOCAL_TASK_PRIORITY,
				this);
	IPI_ASSERT(xReturn == pdPASS);
/*
   BaseType_t xReturn = pdFAIL;
   xReturn = xTaskCreate(
				  task_log_loop,
				  "task_log",
				  LOCAL_TASK_STACK_SIZE,
				  this,
				  LOCAL_TASK_PRIORITY,
				  this);
   IPI_ASSERT(xReturn == pdPASS);
*/
}


void ipi_log_init(void)
{
	ipi_log_mutex = xSemaphoreCreateMutex();

	//p_buf_base = (char *)(ipi_log_header + sizeof(IPI_LOG_RING_BUFFER));
	//p_buf_end = (char *)(p_buf_base + LOG_BUFFER_SIZE);

	ipi_log_header = (IPI_LOG_RING_BUFFER *)IPI_LOG_HEADER;
	ipi_log_header->magic = IPI_MSG_MAGIC_NUMBER;
	ipi_log_header->id_read = 0;
	ipi_log_header->id_write = 0;

	ipi_log_buffer = (char *)(ipi_log_header + sizeof(IPI_LOG_RING_BUFFER));

	memset_to_scp(ipi_log_buffer, 0, IPI_LOG_ELEMENT_COUNT*IPI_LOG_ELEMENT_SIZE);

	log_num = 0;

	queue_log_msg = (queue_msg_t *)pvPortMalloc(sizeof(queue_msg_t));
	if (queue_log_msg == NULL)
		PRINTF_E("%s pvPortMalloc queue_msg_log fail \n", __func__);
	memset(queue_log_msg, 0, sizeof(TaskHandle_t));
	queue_log_msg->magic = IPI_MSG_MAGIC_NUMBER;
	queue_log_msg->msg_id = ID_LOG;

	create_task_log(&task_log);

	xTimer_log = xTimerCreate("log_Timer", LOG_TIME, pdTRUE,
			0, vLogTimerCallback);
	xTimerStart(xTimer_log, 5000);

	PRINTF_E("ipi_log_init(-) header 0x%08x \n", ipi_log_header);
	PRINTF_E("ipi_log_init(-) buffer 0x%08x \n", ipi_log_buffer);

}





bool check_buffer_full(void)
{
	uint16_t id_w_to_be;
	id_w_to_be = ipi_log_header->id_write + 1;
	if (id_w_to_be == IPI_LOG_ELEMENT_COUNT)
		id_w_to_be = 0;

	return (id_w_to_be == ipi_log_header->id_read) ? true : false;

}





//void memcpy_log_to_scp(char *)

void scp_ipi_printf(uint32_t log_level, char *fmt, ...)
{
	va_list args;

	PRINTF_E("%s(+) \n", __func__);
	if (log_level <= g_debug_level) {
		xSemaphoreTake(ipi_log_mutex, portMAX_DELAY);

		PRINTF_E("%s(+) 1 \n", __func__);
		if (check_buffer_full()) {
			PRINTF_E("%s log buffer full\n", __func__);
			xSemaphoreGive(ipi_log_mutex);
			return;
		}

		va_start(args, fmt);
		ipi_log_vsnprintf(printf_msg_inhouse, sizeof(printf_msg_inhouse), fmt, args);
		va_end(args);

		//PRINTF_E("%s(+) 3 %s	\n", __func__, printf_msg_inhouse);
		memcpy_to_scp((void *)(ipi_log_buffer + IPI_LOG_ELEMENT_SIZE*ipi_log_header->id_write), (void *)printf_msg_inhouse, IPI_LOG_ELEMENT_SIZE);
		ipi_log_header->id_write++;
		if (ipi_log_header->id_write == IPI_LOG_ELEMENT_COUNT)
			ipi_log_header->id_write = 0;

		log_num++;

		memset(printf_msg_inhouse, 0, IPI_LOG_ELEMENT_SIZE);

		if (log_num == MAX_LOG_NUM) {
			vTaskResume(task_log);
		}
		xSemaphoreGive(ipi_log_mutex);
	} else {
		return;
	}
}
