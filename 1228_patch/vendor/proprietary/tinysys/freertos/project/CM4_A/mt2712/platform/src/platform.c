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

#include "main.h"
/*  Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "interrupt.h"

#ifdef TINYSYS_DEBUG_BUILD
static int task_monitor_log = 1;
static unsigned int ix;
static void vTaskMonitor(void *pvParameters);
static char list_buffer[512];
#define mainCHECK_DELAY ((portTickType)1000 / portTICK_RATE_MS)
static void vTaskMonitor(void *pvParameters)
{
    portTickType xLastExecutionTime, xDelayTime;

    xLastExecutionTime = xTaskGetTickCount();
    xDelayTime = mainCHECK_DELAY;

    do {
        if (task_monitor_log == 1) {
            vTaskList(list_buffer);
            PRINTF_D("[%d]Heap: free size/total size:%d/%d\n", ix,
                     xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);
            PRINTF_D("Task Status:\n\r%s", list_buffer);
            PRINTF_D("max duration: %llu, limit: %llu\n\r",
                     get_max_cs_duration_time(),
                     get_max_cs_limit());
        }
        ix++;
        vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
    } while (1);
}
#endif
void enable_task_monitor_log(void)
{
#ifdef TINYSYS_DEBUG_BUILD
    task_monitor_log = 1;
#endif
}

void disable_task_monitor_log(void)
{
#ifdef TINYSYS_DEBUG_BUILD
    task_monitor_log = 0;
#endif
}

void platform_init(void)
{

#ifdef TINYSYS_DEBUG_BUILD
    uint32_t reg;
    xTaskCreate(vTaskMonitor, "TMon", 384, (void *)4, 0, NULL);
    reg = readl(CMSYS_DBG_CTRL);
    writel(reg | ENABLE_DEBUG, CMSYS_DBG_CTRL);
#endif

    PRINTF_D("in %s\n\r", __func__);
    //irq_status_dump();

}
