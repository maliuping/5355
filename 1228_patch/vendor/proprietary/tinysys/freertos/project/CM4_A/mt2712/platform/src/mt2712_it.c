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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "platform_mtk.h"
#include "stdarg.h"
#include "string.h"
#include <mt_reg_base.h>
#include <driver_api.h>

enum { r0, r1, r2, r3, r12, lr, pc, psr };

struct TaskContextType {
    unsigned int r0;
    unsigned int r1;
    unsigned int r2;
    unsigned int r3;
    unsigned int r4;
    unsigned int r5;
    unsigned int r6;
    unsigned int r7;
    unsigned int r8;
    unsigned int r9;
    unsigned int r10;
    unsigned int r11;
    unsigned int r12;
    unsigned int sp; /* after pop r0-r3, lr, pc, xpsr                   */
    unsigned int lr; /* lr before exception                             */
    unsigned int pc; /* pc before exception                             */
    unsigned int psr; /* xpsr before exeption                            */
    unsigned int control;  /* nPRIV bit & FPCA bit meaningful,
                * SPSEL bit = 0
                */
    unsigned int exc_return; /* current lr */
    unsigned int msp; /* msp                                             */
};

#define EXCEPTION_HALT 1
#define NEW_LINE "\n"
/* #define DEBUG_MON_DUMP_EN */

static struct TaskContextType taskCtxDump __attribute__((section(".share")));
struct TaskContextType *pTaskContext = &taskCtxDump;

void triggerException(void)
{
    SCB->CCR |= 0x10;   /* DIV_0_TRP */
    __asm volatile("mov r0, #0");
    __asm volatile("UDIV r0, r0");

    do {
    } while (1);
}

void printUsageErrorMsg(uint32_t CFSRValue)
{
    PRINTF("Usage fault: ");
    CFSRValue >>= 16;   /* right shift to lsb */
    if ((CFSRValue & (1 << 9)) != 0)
        PRINTF("Divide by zero\n\r");

    if ((CFSRValue & (1 << 8)) != 0)
        PRINTF("Unaligned access\n\r");

    if ((CFSRValue & (1 << 0)) != 0)
        PRINTF("Undefined instruction\n\r");
}

void printBusFaultErrorMsg(uint32_t CFSRValue)
{
    PRINTF("Bus fault: ");
    CFSRValue &= 0x0000FF00;    /* mask just bus faults */
    CFSRValue >>= 8;
    if ((CFSRValue & (1 << 5)) != 0) {
        PRINTF(
            "A bus fault occurred during FP lazy state preservation\n\r");
    }
    if ((CFSRValue & (1 << 4)) != 0) {
        PRINTF(
            "A derived bus fault has occurred on exception entry\n\r");
    }
    if ((CFSRValue & (1 << 3)) != 0) {
        PRINTF(
            "A derived bus fault has occurred on exception return\n\r");
    }
    if ((CFSRValue & (1 << 2)) != 0) {
        PRINTF(
            "Imprecise data access error has occurred\n\r");
    }
    if ((CFSRValue & (1 << 1)) != 0) {
        /* Need to check valid bit (bit 7 of CFSR)? */
        PRINTF(
            "A precise data access error has occurred @x%08x\n\r",
            (unsigned int)SCB->BFAR);
    }
    if ((CFSRValue & (1 << 0)) != 0) {
        PRINTF(
            "A bus fault on an instruction prefetch has occurred\n\r");
    }
    if ((CFSRValue & (1 << 7)) != 0) {
        /* To review: remove this if redundant */
        PRINTF("SCB->BFAR = 0x%08x\n\r", (unsigned int)SCB->BFAR);
    }
}

void printMemoryManagementErrorMsg(uint32_t CFSRValue)
{
    PRINTF("Memory Management fault: ");
    CFSRValue &= 0x000000FF;    /* mask just mem faults */
    if ((CFSRValue & (1 << 5)) != 0) {
        PRINTF(
            "A MemManage fault occurred during FP lazy state preservation\n\r");
    }
    if ((CFSRValue & (1 << 4)) != 0) {
        PRINTF(
            "A derived MemManage fault occurred on exception entry\n\r");
    }
    if ((CFSRValue & (1 << 3)) != 0) {
        PRINTF(
            "A derived MemManage fault occurred on exception return\n\r");
    }
    if ((CFSRValue & (1 << 1)) != 0) {
        /* Need to check valid bit (bit 7 of CFSR)? */
        PRINTF("Data access violation @0x%08x\n\r",
               (unsigned int)SCB->MMFAR);
    }
    if ((CFSRValue & (1 << 0)) != 0) {
        PRINTF(
            "MPU or Execute Never (XN) default memory map access violation\n\r");
    }
    if ((CFSRValue & (1 << 7)) != 0) {
        /* To review: remove this if redundant */
        PRINTF("SCB->MMFAR = 0x%08x\n\r", (unsigned int)SCB->MMFAR);
    }
}

/**
 * @brief   This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

void Hard_Fault_Handler(uint32_t stack[])
{
    PRINTF("\n\rIn Hard Fault Handler\n\r");
    PRINTF("SCB->HFSR = 0x%08x\n\r", (unsigned int)SCB->HFSR);
    if ((SCB->HFSR & (1 << 30)) != 0) {
        PRINTF("Forced Hard Fault\n\r");
        PRINTF("SCB->CFSR = 0x%08x\n\r", (unsigned int)SCB->CFSR);

        if ((SCB->CFSR & 0xFFFF0000) != 0)  /* Usage Fault */
            printUsageErrorMsg(SCB->CFSR);

        if ((SCB->CFSR & 0x0000FF00) != 0)  /* Bus Fault */
            printBusFaultErrorMsg(SCB->CFSR);

        if ((SCB->CFSR & 0x000000FF) != 0)  /* MemManage Fault */
            printMemoryManagementErrorMsg(SCB->CFSR);

    }
    while (1);
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
    __asm volatile(
        "ldr r0, =pTaskContext" NEW_LINE
        "ldr r0, [r0]"      NEW_LINE
        "add r0, r0, #16"   NEW_LINE /* point to context.r4 */
        "stmia r0!, {r4-r11}"   NEW_LINE /* store r4-r11 */
        "add r0, #20"       NEW_LINE /* point to context.control */
        "mrs r1, control"   NEW_LINE /* move CONTROL to r1 */
        "str r1, [r0], #4"  NEW_LINE /* store CONTROL */
        "str lr, [r0], #4"  NEW_LINE /* store EXC_RETURN */
        "mrs r1, msp"       NEW_LINE /* move MSP to r1 */
        "str r1, [r0]"      NEW_LINE /* store MSP */
        "tst lr, #4"        NEW_LINE /* thread or handler mode ? */
        "ite eq"        NEW_LINE
        "mrseq r0, msp"     NEW_LINE
        "mrsne r0, psp"     NEW_LINE
        "push {lr}"     NEW_LINE
        "bl Hard_Fault_Handler" NEW_LINE
        "pop {lr}"      NEW_LINE
        "bx lr"         NEW_LINE
    );
}

void Bus_Fault_Handler(uint32_t stack[])
{
    PRINTF("\n\rIn Bus Fault Handler\n\r");
    PRINTF("SCB->CFSR = 0x%08x\n\r", (unsigned int)SCB->CFSR);
    if ((SCB->CFSR & 0xFF00) != 0)  /* Bus Fault */
        printBusFaultErrorMsg(SCB->CFSR);

    do {
    } while (1);
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
    __asm volatile(
        "ldr r0, =pTaskContext"     NEW_LINE
        "ldr r0, [r0]"          NEW_LINE
        "add r0, r0, #16"       NEW_LINE/* point to context.r4 */
        "stmia r0!, {r4-r11}"       NEW_LINE/* store r4-r11 */
        "add r0, #20"           NEW_LINE/* point to context.control */
        "mrs r1, control"       NEW_LINE/* move CONTROL to r1 */
        "str r1, [r0], #4"      NEW_LINE/* store CONTROL */
        "str lr, [r0], #4"      NEW_LINE/* store EXC_RETURN */
        "mrs r1, msp"           NEW_LINE/* move MSP to r1 */
        "str r1, [r0]"          NEW_LINE/* store MSP */
        "tst lr, #4"            NEW_LINE/* thread or handler mode? */
        "ite eq"            NEW_LINE
        "mrseq r0, msp"         NEW_LINE
        "mrsne r0, psp"         NEW_LINE
        "push {lr}"         NEW_LINE
        "bl Bus_Fault_Handler"      NEW_LINE
        "pop {lr}"          NEW_LINE
        "bx lr"             NEW_LINE
    );
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Fault_Handler(uint32_t stack[])
{
    PRINTF("\n\rIn MemManage Fault Handler\n\r");
    PRINTF("SCB->CFSR = 0x%08x\n\r", (unsigned int)SCB->CFSR);
    if ((SCB->CFSR & 0xFF) != 0)    /* MemManage Fault */
        printMemoryManagementErrorMsg(SCB->CFSR);
    do {
    } while (1);
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
    __asm volatile(
        "ldr r0, =pTaskContext"     NEW_LINE
        "ldr r0, [r0]"          NEW_LINE
        "add r0, r0, #16"       NEW_LINE/* point to context.r4 */
        "stmia r0!, {r4-r11}"       NEW_LINE/* store r4-r11 */
        "add r0, #20"           NEW_LINE/* point to context.control */
        "mrs r1, control"       NEW_LINE/* move CONTROL to r1 */
        "str r1, [r0], #4"      NEW_LINE/* store CONTROL */
        "str lr, [r0], #4"      NEW_LINE/* store EXC_RETURN */
        "mrs r1, msp"           NEW_LINE/* move MSP to r1 */
        "str r1, [r0]"          NEW_LINE/* store MSP */
        "tst lr, #4"            NEW_LINE/* thread or handler mode? */
        "ite eq"            NEW_LINE
        "mrseq r0, msp"         NEW_LINE
        "mrsne r0, psp"         NEW_LINE
        "push {lr}"         NEW_LINE
        "bl MemManage_Fault_Handler"    NEW_LINE
        "pop {lr}"          NEW_LINE
        "bx lr"             NEW_LINE
    );
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    do {
    } while (1);
}

void Debug_Mon_Handler(uint32_t stack[])
{
    PRINTF("\n\rIn Debug Mon Fault Handler\n\r");
    PRINTF("SCB->CFSR = 0x%08x\n\r", (unsigned int)SCB->CFSR);

    if (SCB->DFSR & SCB_DFSR_DWTTRAP_Msk) {
        PRINTF("A debug event generated by the DWT\n\r");
#if DEBUG_MON_DUMP_EN
        PRINTF_D("COMP0: %8lx \t MASK0: %8lx \t FUNC0: %8lx\n\r",
                 DWT->COMP0, DWT->MASK0, DWT->FUNCTION0);
        PRINTF_D("COMP1: %8lx \t MASK1: %8lx \t FUNC1: %8lx\n\r",
                 DWT->COMP1, DWT->MASK1, DWT->FUNCTION1);
        PRINTF_D("COMP2: %8lx \t MASK2: %8lx \t FUNC2: %8lx\n\r",
                 DWT->COMP2, DWT->MASK2, DWT->FUNCTION2);
        PRINTF_D("COMP3: %8lx \t MASK3: %8lx \t FUNC3: %8lx\n\r",
                 DWT->COMP3, DWT->MASK3, DWT->FUNCTION3);
#endif
    }
    do {
    } while (1);
#if DEBUG_MON_DUMP_EN
    PRINTF_D("LR:0x%x\n", drv_reg32(REG_LR));
    PRINTF_D("PC:0x%x\n", drv_reg32(REG_PC));
    PRINTF_D("PSP:0x%x\n", drv_reg32(REG_PSP));
    PRINTF_D("SP:0x%x\n", drv_reg32(REG_SP));
#endif
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
    /* TODO: implement here !!!
    *    PRINTF("\n\rEnter Debug Monitor Handler\n\r");
    *    PRINTF("SCB->DFSR = 0x%08x\n\r", (unsigned int)SCB->DFSR );
    */
    __asm volatile(
        "ldr r0, =pTaskContext" NEW_LINE
        "ldr r0, [r0]"      NEW_LINE
        "add r0, r0, #16"   NEW_LINE/* point to context.r4 */
        "stmia r0!, {r4-r11}"   NEW_LINE/* store r4-r11 */
        "add r0, #20"       NEW_LINE/* point to context.control */
        "mrs r1, control"   NEW_LINE/* move CONTROL to r1 */
        "str r1, [r0], #4"  NEW_LINE/* store CONTROL */
        "str lr, [r0], #4"  NEW_LINE/* store EXC_RETURN */
        "mrs r1, msp"       NEW_LINE/* move MSP to r1 */
        "str r1, [r0]"      NEW_LINE/* store MSP */
        "tst lr, #4"        NEW_LINE/* thread or handler mode? */
        "ite eq"        NEW_LINE
        "mrseq r0, msp"     NEW_LINE
        "mrsne r0, psp"     NEW_LINE
        "push {lr}"     NEW_LINE
        "bl Debug_Mon_Handler"  NEW_LINE
        "pop {lr}"      NEW_LINE
        "bx lr"         NEW_LINE
    );
}
