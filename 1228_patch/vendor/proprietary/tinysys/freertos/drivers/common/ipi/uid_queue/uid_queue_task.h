#ifndef UID_QUEUE_TASK_H
#define UID_QUEUE_TASK_H

#include <FreeRTOS.h>
#include <stdint.h>
#include <task.h>
#include <queue.h>
#include "scp_ipi.h"
#include "scp_uid_queue.h"


#define LOCAL_TASK_NAME "ipc"
#define LOCAL_TASK_STACK_SIZE (512)
#define LOCAL_TASK_PRIORITY (2)

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

typedef int status_t;

#define TIMEREG_LOW 0x1000c008
#define TIMEREG_HIGH 0x1000c00c


typedef enum {
	TASK_IDLE,
	TASK_INIT,
	TASK_WORKING,
	TASK_DEINIT
} task_state_t;


/* the definition for the AudioTask class */
typedef struct DspTask {
	/* attributes */
	uint32_t state;

	TaskHandle_t freertos_task;

	uint8_t queue_idx;
	// uint32_t (*msg_array)[MARK_SIZE];
	uint8_t num_queue_element;
	xQueueHandle msg_idx_queue;

	/* constructor/destructor */
	void (*constructor)(struct DspTask *this);
	void (*destructor)(struct DspTask *this);

	/* create RTOS task */
	void (*create_task_loop)(struct DspTask *this);
	void (*destroy_task_loop)(struct DspTask *this);

	/* receive ipi message */
	IPI_STATUS (*recv_message)(
		struct DspTask *this,
		void *data);

} DspTask;


void memcpy_from_scp(void *trg, void *src, int size);
void memset_to_scp(void *trg, int val, int size);
void memcpy_to_scp(void *trg, const void *src, int size);
void unmark_recv_mark_id(int i, int j);




DspTask *get_task(void);
void task_ipi_init(void);

IPI_STATUS scp_ipi_register(FUNC_ID func_id, void (*ipi_handler)(DATA_ADDR data_addr));





#endif

