#ifndef UNIQUE_ID_QUEUE_H
#define UNIQUE_ID_QUEUE_H


#include <FreeRTOS.h>
#include <stdint.h>
#include <task.h>
#include <semphr.h>

#include "scp_ipi.h"

#define QUEUE_TASK_NAME "scp_uid_queue"


typedef struct UIDQTask {
	uint32_t mark_id[4];
	TaskHandle_t u_task;
} UIDQTask;

typedef enum IPI_UID_RET_STATUS {
	IPI_UID_RET_OK,
	IPI_UID_RET_NG,
	IPI_UID_RET_BUSY_DATA_SAME,
	IPI_UID_RET_BUSY_DATA_DIFF,
	IPI_UID_RET_IRQ_UNREADY,
} IPI_UID_RET_STATUS;

/*
typedef struct my_semaphore {
	uint32_t done;
	SemaphoreHandle_t done_mutex;;
}my_semaphore;
*/

void create_queue_loop(UIDQTask *this);
IPI_UID_RET_STATUS scp_uid_queue_send(queue_msg_t *msg);

void uid_queue_init(void);
//uint8_t count_one(uint32_t num);
//void msg_ack(void);
bool check_msg_id_marked(UIDQTask *task_q, uint16_t msg_id);
//bool check_mark_id_empty(struct UIDQTask *task_q);
void mark_msg_id(UIDQTask *task_q, uint16_t msg_id);
void mark_msg_id_init(void);


extern IPI_UID_RET_STATUS scp_uid_queue_register(QUEUE_MSG_ID msg_id,
	void (*uid_msg_handler)(void *private, char *data), void *private);


#endif
