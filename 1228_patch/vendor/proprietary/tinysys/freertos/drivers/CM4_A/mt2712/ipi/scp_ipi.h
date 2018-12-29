#ifndef __MT_IPI_H__
#define __MT_IPI_H__
#include <mt_reg_base.h>

#define SCP_EVENT_CTL_REG		(SCP_SHARE_NS_BASE + 0x00)
	#define IRQ_TO_AP		BIT(1)
	#define IRQ_TO_CMSYS		BIT(0)
#define SCP_EVENT_CLR_REG		(SCP_SHARE_NS_BASE + 0x04)
	#define IRQ_TO_AP_CLR		BIT(0)
	#define IRQ_TO_CMSYS_CLR	BIT(1)

int scp_ipi_init(void);

#endif
