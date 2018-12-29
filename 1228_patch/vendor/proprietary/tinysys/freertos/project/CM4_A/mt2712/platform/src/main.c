/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL
 * PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR
 * ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A
 * PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER
 * WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "interrupt.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#ifdef CFG_IPC_SUPPORT
#include <scp_ipi.h>
#endif

#ifdef CFG_DSP_IPC_SUPPORT
#include <scp_ipi.h>
#endif

#include <driver_api.h>

/* timestamp */
#include <utils.h>
/*  console includes. */
#include <unwind.h>
#ifdef CFG_XGPT_SUPPORT
#include "mt_gpt.h"
#endif
#ifdef CFG_MPU_DEBUG_SUPPORT
#include "mpu_mtk.h"
#endif

#include <mt_uart.h>

/* Private functions --------------------------------------------------------*/
static void prvSetupHardware(void);
/* Extern functions ---------------------------------------------------------*/
extern void platform_init(void);
extern void project_init(void);
extern void mt_init_dramc(void);
extern void scp_remap_init(void);
extern void mt_ram_dump_init(void);
extern void gce_init(void);
extern void mtk_smi_larb_init(void);
#ifdef CFG_MPU_DEBUG_SUPPORT
extern struct mpu_region mpu_region_table_mtk[8];
#endif
#ifdef CFG_TVD_SUPPORT
extern void tvd_init(void);
#endif
#ifdef CFG_FASTRVC_SUPPORT
extern void fastrvc_main();
#endif

#ifdef CFG_DISPLAY_SUPPORT
extern int32_t display_core_init(void);
#endif

#ifdef CFG_OVERLAY2_SUPPORT
extern int32_t ovl2_init(void);
#endif

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{

    /* unsigned char buf[] = "ready"; */
    /* init irq handler */
    irq_init();

    /* Configure the hardware, after this point, the UART can be used */
    prvSetupHardware();


    /* start to use UART */
    PRINTF_E("\n\r FreeRTOS %s(build:%s %s)\n\r", tskKERNEL_VERSION_NUMBER,
             __DATE__, __TIME__);
    platform_init();
    project_init();

#ifdef CFG_FASTRVC_SUPPORT
    fastrvc_main();
#endif

    PRINTF_E("Scheduler start...\n");
    /* Start the scheduler. After this point, the interrupt is enabled */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle
     * and/or timer tasks to be created.  See the memory management section
     * on the FreeRTOS web site for more details.
     */

    for (;;)
        ;
}

/*-----------------------------------------------------------*/
/**
  * @brief  Hardware init
  * @param  None
  * @retval None
  */
static void prvSetupHardware(void)
{

    /* Set UART clock to 26MHz */
    //uart_clk_sel(UART_XTAL_26MHZ);

    /* set up debug level */
    set_debug_level(LOG_DEBUG);
    /* init UART before any printf function all */
    uart_init(UART_LOG_PORT, UART_LOG_BAUD_RATE, 0);
    DRV_SetReg32(0xA1020000, 0x47);

#ifdef CFG_MPU_DEBUG_SUPPORT
    /*set MPU */
    mpu_init();
#endif

#ifdef CFG_IPC_SUPPORT
    /* init remap register before any dram access */
    scp_remap_init();
    /* init IPI before any IPI all */
    scp_ipi_init();
#endif

#ifdef CFG_DSP_IPC_SUPPORT
	/* init IPI before any IPI all */
	scp_ipi_init();
#endif

#ifdef CFG_MPU_DEBUG_SUPPORT
    /*enable mpu to protect dram */
    dram_protector_init();
#endif
#ifdef CFG_DMA_SUPPORT
    /* init DMA */
    mt_init_dma();
#endif

#ifdef CFG_EINT_SUPPORT
    /* init EINT */
    mt_eint_init();
#endif

#ifdef CFG_FHCTL_SUPPORT
    mt_fh_hal_init();
#endif

#ifdef CFG_TVD_SUPPORT
	tvd_init();
#endif

#ifdef CFG_GCE_SUPPORT
   gce_init();
#endif

#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_init();
#endif

#ifdef CFG_TOP_CLKRST_SUPPORT
    scp_top_init();
#endif

#ifdef CFG_OVERLAY2_SUPPORT
    ovl2_init();
#endif

    /* smi should be after clk and power init. and before display. */
    mtk_smi_larb_init();
#ifdef CFG_XGPT_SUPPORT
    mt_platform_timer_init();
#endif
#ifdef CFG_PWRAP_SUPPORT
    pwrap_init_scp();
#endif
#ifdef CFG_DISPLAY_SUPPORT
    display_core_init();
#endif

}

static unsigned int last_pc;
static unsigned int current_pc;
_Unwind_Reason_Code trace_func(_Unwind_Context *ctx, void *d)
{
    current_pc = _Unwind_GetIP(ctx);
    if (current_pc != last_pc) {
        PRINTF_E("0x%08x\n", current_pc);
        last_pc = current_pc;
        return _URC_NO_REASON;
    }
    taskDISABLE_INTERRUPTS();
    PRINTF_E("==PC backtrace dump end==\n");
    do {
    } while (1);
}

void print_backtrace(void)
{
    int depth = 0;

    _Unwind_Backtrace(&trace_func, &depth);
}

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void vAssertCalled(char *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line
    * number,
    * ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
    * line)
    */
    PRINTF_E("Wrong parameters value: file %s on line %lu\r\n",
             file, line);
    PRINTF_E("==PC backtrace dump start==\n");
    print_backtrace();
    PRINTF_E("==PC backtrace dump end==\n");
    /* Infinite loop */
    do {
    } while (1);

}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(xTaskHandle pxTask, char *pcTaskName)
{
    /* If configCHECK_FOR_STACK_OVERFLOW is set to either 1 or 2 then this
    * function will automatically get called if a task overflows its stack.
    */
    (void)pxTask;
    (void)pcTaskName;
    PRINTF_E("\n\r task:%s stack overflow\n\r", pcTaskName);
    do {
    } while (1);
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
    /* If configUSE_MALLOC_FAILED_HOOK is set to 1 then this function will
    * be called automatically if a call to pvPortMalloc() fails.
    * pvPortMalloc()
    * is called automatically when a task, queue or semaphore is created.
    */
    PRINTF_E("\n\r malloc failed\n\r");
    PRINTF_D("Heap: free size/total size:%d/%d\n", xPortGetFreeHeapSize(),
             configTOTAL_HEAP_SIZE);
    configASSERT(0);
    do {
    } while (1);
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
    /* PRINTF_D("\n\r Enter IDLE\n\r"); */
}

void vApplicationTickHook(void)
{
    /* PRINTF_D("\n\r Enter TickHook\n\r"); */
}
