#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <driver_api.h>
#include <platform_mtk.h>
#include <interrupt.h>
#include "FreeRTOS.h"
#include "task.h"
#include "scp_ipi.h"
#include "scp_regs.h"
#include "scp_sem.h"

/*
 * clear irq from ap
 */
static inline void mtk_irq_clr(void)
{
	writel(IRQ_TO_CMSYS_CLR, SCP_EVENT_CLR_REG);
}

/*
 * trigger irq to ap
 */
static inline void mtk_ap_irq_trigger(void)
{
	writel(IRQ_TO_AP, SCP_EVENT_CTL_REG);
}

/*
 * check if the ap irq processing is complete, ap should
 * clear irq status at the end of irq handler.
 * @param : none
 * return 0 : ap is processing irq
 *        1 : ap irq processing is complete
 */
inline int mtk_ap_irq_status(void)
{
	return mtk_check_reg_bit(0, SCP_EVENT_CTL_REG);
}

static void mtk_scp_ipi_handler(int irq, void *pdata)
{
	/* clear irq */
	mtk_irq_clr();
	PRINTF_D("receive irq from ap\n\r");
}

int scp_ipi_init(void)
{
	request_irq(SOFT_NS_IRQ_BIT, mtk_scp_ipi_handler, IRQ_TYPE_LEVEL_LOW, "scp_ipi", NULL);
	return 0;
}
