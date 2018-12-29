#ifndef SCP_IPI_H
#define SCP_IPI_H


#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
//#include <stdint.h>
#include <mt_reg_base.h>


#define SCP_EVENT_CTL_REG		(SCP_SHARE_NS_BASE + 0x00)
	#define IRQ_TO_AP		BIT(1)
	#define IRQ_TO_CMSYS		BIT(0)
#define SCP_EVENT_CLR_REG		(SCP_SHARE_NS_BASE + 0x04)
	#define IRQ_TO_AP_CLR		BIT(0)
	#define IRQ_TO_CMSYS_CLR	BIT(1)


void mtk_irq_clr(void);
void mtk_ap_irq_trigger(void);



#ifndef false
#define false	0
#endif

#ifndef true
#define true	1
#endif

#ifndef bool
typedef unsigned char  bool;
#endif

#define UNIQUE_ELEMENT_SIZE 256
#define UNITUQ_QUEUE_ELEMENT_COUNT 128
#define MARK_SIZE ((UNITUQ_QUEUE_ELEMENT_COUNT+31)/32)

typedef void *DATA_ADDR;

typedef enum IPI_STATUS {
	IPI_RET_OK,
	IPI_RET_NG,
	IPI_RET_BUSY,
} IPI_STATUS;

typedef enum FUNC_ID {
	IPI_FUNC_UID_QUEUE,
	IPI_FUNC_WDT,
	IPI_FUNC_SYSTEM,
	NUM_IPI_FUNC,
} FUNC_ID;

typedef enum MODULE_ID{
	MODULE_ID_AUDIO = 0,
	MODULE_ID_LOG,
	MODULE_ID_SENSOR,
	NUM_MODULE_ID,
} MODULE_ID;


typedef enum QUEUE_MSG_ID{
	MTK_DISP_IPC_OPEN,
	MTK_DISP_IPC_OPEN_RSP,
	MTK_DISP_IPC_CLOSE,
	MTK_DISP_IPC_LAYER_DATA_COMMIT,
	MTK_DISP_IPC_PAGE_FLIP_DONE,
	ID_LOG,
	NUM_MSG_ID,
} QUEUE_MSG_ID;


typedef struct ipi_msg_t {
	uint32_t  state;			 /* for polling state */
	uint32_t  func_id;
	uint64_t *payload;
	void *p;
} ipi_msg_t;

typedef struct queue_msg_t {
	uint32_t magic;
	uint32_t  msg_id;
	char data[UNIQUE_ELEMENT_SIZE - 8];
} queue_msg_t;


typedef struct UIDQHEADER {
	uint64_t magic;
	uint8_t mark_id[UNITUQ_QUEUE_ELEMENT_COUNT];
} UIDQHEADER;


#define UNCACHED_ORGIN_ADDRESS SCP_NON_CACHED_BASE
#define UNCACHED_FOR_DRIVER_LENGTH SCP_NON_CACHED_SIZE
#define UNCACHED_FOR_IPC_LENGTH SCP_IPC_SHARE_SIZE

//#define SHARE_BUFFER_START_ADDRESS (UNCACHED_ORGIN_ADDRESS + UNCACHED_FOR_DRIVER_LENGTH)
#define SHARE_BUFFER_START_ADDRESS SCP_IPC_SHARE_BASE


//#define SHARE_BUF_SIZE sizeof(ipi_msg_t)
//#define UIDQUE_HEADER_SIZE sizeof(UIDQHEADER)


#define UNIQUE_ID_QUEUE_SIZE (sizeof(queue_msg_t)*UNITUQ_QUEUE_ELEMENT_COUNT)


#define IPI_MSG_SEND_BUFFER SHARE_BUFFER_START_ADDRESS
#define IPI_UID_QUEUE_SEND_HEADER (IPI_MSG_SEND_BUFFER + sizeof(ipi_msg_t))
#define IPI_UID_QUEUE_SEND_BUFFER (IPI_UID_QUEUE_SEND_HEADER + sizeof(UIDQHEADER))


#define IPI_MSG_RECV_BUFFER (IPI_MSG_SEND_BUFFER + 0x10000)
#define IPI_UID_QUEUE_RECV_HEADER (IPI_MSG_RECV_BUFFER + sizeof(ipi_msg_t))
#define IPI_UID_QUEUE_RECV_BUFFER (IPI_UID_QUEUE_RECV_HEADER + sizeof(UIDQHEADER))



#define IPI_ALLOCATOR_HEADER (IPI_MSG_RECV_BUFFER + 0x10000)

#define IPI_ALLOCATOR_ELEMENT_SIZE 4096
#define IPI_ALLOCATOR_ELEMENT_COUNT 384
#define IPI_ALLOCATOR_MARK_SIZE  ((IPI_ALLOCATOR_ELEMENT_COUNT+31)/32)

#define IPI_MEM_IRQ_STATUS (IPI_ALLOCATOR_BUFFER + IPI_ALLOCATOR_ELEMENT_SIZE * IPI_ALLOCATOR_ELEMENT_COUNT + 0x8000)

#define IPI_LOG_HEADER (IPI_MEM_IRQ_STATUS + 0x8000)

#define IPI_LOG_ELEMENT_SIZE 260
#define IPI_LOG_ELEMENT_COUNT 256


#define IPI_MEM_LEFT  (IPI_LOG_HEADER + IPI_LOG_ELEMENT_SIZE*IPI_LOG_ELEMENT_COUNT + 0x8000)






typedef enum IRQ_STATUS {
	IRQ_STATUS_UNREADY = 0,
	IRQ_STATUS_READY = 0x8888,
} IRQ_STATUS;

/*
typedef struct IPI_IRQ_STATUS {
	IRQ_STATUS status_gic;
	IRQ_STATUS status_irq_handler;
	IRQ_STATUS status_module_handler[NUM_MODULE_ID];
} IPI_IRQ_STATUS;
*/


typedef struct IPC_MEM_HEADER {
	uint32_t mark_req_mem[IPI_ALLOCATOR_MARK_SIZE];
	uint16_t record_size[IPI_ALLOCATOR_ELEMENT_COUNT];
} IPC_MEM_HEADER;

typedef struct NONCACHED_INFO {
	DATA_ADDR data_addr;
	uint32_t data_len;
	struct NONCACHED_INFO *p_next;
} NONCACHED_INFO;

typedef enum MARK_STATUS {
	MSG_ID_UNMARKED = 0,
	MSG_ID_MARKED = 1,
} MARK_STATUS;

#define IPI_ALLOCATOR_BUFFER (IPI_ALLOCATOR_HEADER + sizeof(IPC_MEM_HEADER))

#define IPC_IRQ_RECV_NUMBER 107
#define IPC_IRQ_SEND_NUMBER 106

#define IPI_MSG_MAGIC_NUMBER	 (0x8888)


typedef struct SCP_IPI_DESC {
	void (*handler)(DATA_ADDR data_addr);
	//unsigned int recv_count;
	//unsigned int success_count;
	//unsigned int busy_count;
	//unsigned int error_count;
	//const char *name;
} SCP_IPI_DESC;


typedef struct IPI_UID_QUEUE_DESC {
	void (*handler)(void *private, char *data);
	void *private;
	//unsigned int recv_count;
	//unsigned int success_count;
	//unsigned int busy_count;
	//unsigned int error_count;
	//const char *name;
} IPI_UID_QUEUE_DESC;


IPI_STATUS scp_ipi_send(FUNC_ID func_id, void *data_addr);


void scp_ipi_init(void);
ipi_msg_t *get_rcv_obj(void);
ipi_msg_t *get_send_obj(void);

DATA_ADDR noncached_mem_alloc(uint32_t size);
void noncached_mem_free(DATA_ADDR ptr);

DATA_ADDR ipc_mem_alloc(uint32_t size);
void ipc_mem_free(DATA_ADDR ptr);

void set_rcv_ipc_ready(void);
bool get_send_ipc_ready(void);





#endif

