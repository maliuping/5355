
#include "FreeRTOS.h"
#include <FreeRTOSConfig.h>
#include <string.h>
#include <driver_api.h>
#include <mt_printf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include "ipi_assert.h"
#include "test.h"
#include "scp_uid_queue.h"
#include "uid_queue_task.h"
#include "ipi_log.h"

#include "scp_ipi.h"

#define LOCAL_TASK_STACK_SIZE (512)
#define LOCAL_TASK_PRIORITY (2)

static UIDQHEADER *uid_send_header;
static uint8_t msg_id_array[NUM_MSG_ID];



TaskHandle_t test_task;
//TaskHandle_t test_task_1;

static void create_task_test(void);
static void task_loop_test(void *pvParameters);


void queue_msg_handler(void *private, char *data)
{

	uint8_t *p_id = (uint8_t *)private;
	uint8_t *data_to_printk = (uint8_t *)data;
//	  memcpy_from_scp((void *)uid_recv_data[*p_id], (void *)data, UNIQUE_ELEMENT_SIZE - 8);
	PRINTF_E("%s(+) private %d uid_recv_data %d \n", __func__, *p_id, data_to_printk[0]);

	//  PRINTF_E("%s(+) data[0] %d \n", __func__, data[0]);

}



//IPI_UID_RET_STATUS scp_uid_queue_register(QUEUE_MSG_ID msg_id,
//	void (*uid_msg_handler)(void *private, char *data), void *private)

void test_handler_register(void)
{
	int i;
	PRINTF_E("%s(+)  \n", __func__);
	for (i = 0; i < NUM_MSG_ID; i++) {
		msg_id_array[i] = i;
		scp_uid_queue_register(i, queue_msg_handler, &msg_id_array[i]);
	}

}


static void task_loop_test(void *pvParameters)
{
	int cnt = 0;
	uint8_t tmp = 0;
	int i;
	queue_msg_t *test_msg = (queue_msg_t *)pvPortMalloc(sizeof(queue_msg_t));
	test_msg->magic = IPI_MSG_MAGIC_NUMBER;

	DATA_ADDR p_test;
  //  test_msg->msg_id = 0;
	while (1) {
		//PRINTF_E("%s(+) cnt = %d \n", __func__, cnt);

		if (cnt == 0) {
			// mtk_ap_irq_trigger();
		}


		if (cnt < 5) {
			PRINTF_E("%s(+) cnt = %d \n", __func__, cnt);
			p_test = noncached_mem_alloc(0x1000);
			PRINTF_E("%s(+) p_test = %p \n", __func__, p_test);
			tmp = cnt%NUM_MSG_ID;
			test_msg->msg_id = tmp;
			test_msg->data[1] = tmp;
			for (i = 0; i < UNIQUE_ELEMENT_SIZE - 8; i++)
				test_msg->data[i] = tmp;
			scp_uid_queue_send(test_msg);

		 }

		if (cnt == 0)
			vTaskDelay(5000);
		else
			vTaskDelay(100);
		cnt++;
	}
}


void create_task_test(void)
{

	PRINTF_E("%s(+)\n", __func__);
	BaseType_t xReturntest = pdFAIL;
	xReturntest = xTaskCreate(
				task_loop_test,
				"test",
				LOCAL_TASK_STACK_SIZE,
				NULL,
				LOCAL_TASK_PRIORITY,
				&test_task);

	IPI_ASSERT(xReturntest == pdPASS);

}


void test_init(void)
{
	uid_send_header = (UIDQHEADER *)IPI_UID_QUEUE_SEND_HEADER;
	create_task_test();
	test_handler_register();
}
