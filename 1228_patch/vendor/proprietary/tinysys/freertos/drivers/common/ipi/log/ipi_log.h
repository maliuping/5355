#ifndef IPI_LOG_H
#define IPI_LOG_H

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <stdint.h>


#include "mt_printf.h"
#include "scp_ipi.h"

#define MAX_LOG_NUMBER 100
#define LOG_BUFFER_SIZE 0x10000


#define LOCAL_TASK_STACK_SIZE (512)
#define LOCAL_TASK_PRIORITY (2)


void scp_ipi_printf(uint32_t log_level, char *fmt, ...);

#define LOG_DEBUG	3
#define LOG_INFO	2
#define LOG_WARN	1
#define LOG_ERROR	0


#define IPI_LOG(x...)	IPI_LOG_D(x)
#define IPI_LOG_D(x...)	scp_ipi_printf(LOG_DEBUG, x)
#define IPI_LOG_I(x...)	scp_ipi_printf(LOG_INFO, x)
#define IPI_LOG_W(x...)	scp_ipi_printf(LOG_WARN, x)
#define IPI_LOG_E(x...)	scp_ipi_printf(LOG_ERROR, x)


// circular buffer is good for FIFO mechanism
/*
typedef struct RingBuf{
	char *pBufBase;
	char *pBufEnd;
	char *pRead;
	char *pWrite;
	int   bufLen;
} RingBuf; 
*/

//sizeof(LOG_INFO) must be less than  (UNIQUE_ELEMENT_SIZE - 8)
/*
typedef struct IPI_LOG_INFO{
	uint16_t log_number;
	uint16_t log_size[MAX_LOG_NUMBER];
}IPI_LOG_INFO;
*/

typedef struct IPI_LOG_RING_BUFFER{
	uint32_t magic;
	uint16_t id_read;
	uint16_t id_write;
} IPI_LOG_RING_BUFFER;


void ipi_log_init(void);


#endif
