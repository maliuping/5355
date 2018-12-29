#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <stdint.h>
#include "timers.h"
#include "mt_printf.h"


#include "ipi_wdt.h"
#include "scp_ipi.h"


#define WDT_TIME 2000
#define WDT_DELAY_TIME 5000

static TimerHandle_t xTimer_wdt;



void vWdtTimerCallback(TimerHandle_t xTimer)
{
	if (xTimer) {
		PRINTF_E("%s \n", __func__);
		scp_ipi_send(IPI_FUNC_WDT, NULL);
	}
}


void ipi_wdt_init(void)
{

	xTimer_wdt = xTimerCreate("wdt_Timer", WDT_TIME, pdTRUE,
			  0, vWdtTimerCallback);
	xTimerStart(xTimer_wdt, 5000);

}

