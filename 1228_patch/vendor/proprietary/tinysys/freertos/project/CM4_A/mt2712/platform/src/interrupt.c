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

#include "FreeRTOS.h"
#include "portmacro.h"
#include <malloc.h>
#include <interrupt.h>
#include <task.h>
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif
#include <limits.h>
#include <scp_regs.h>


/**
 * @defgroup IP_group_scp_intc INTC
 *     SCP INTC related functions.\n
 *     @{
 *       @defgroup IP_group_scp_intc_external EXTERNAL
 *         The external API document for SCP INTC. \n
 *         @{
 *           @defgroup type_group_scp_intc_ext_API 1.function
 *               This is SCP_INTC external function.
 *           @defgroup type_group_scp_intc_ext_struct 2.structure
 *               This is SCP_INTC external structure.
 *           @defgroup type_group_scp_intc_ext_typedef 3.typedef
 *               This is SCP_INTC external typedef.
 *           @defgroup type_group_scp_intc_ext_enum 4.enumeration
 *               This is SCP_INTC external enumeration.
 *           @defgroup type_group_scp_intc_ext_def 5.define
 *               This is SCP_INTC external define.
 *         @}
 *
 *       @defgroup IP_group_scp_intc_internal INTERNAL
 *         The internal API document for SCP INTC. \n
 *         @{
 *           @defgroup type_group_scp_intc_InFn 1.function
 *               This is SCP_INTC internal function.
 *           @defgroup type_group_scp_intc_struct 2.structure
 *               None.
 *           @defgroup type_group_scp_intc_typedef 3.typedef
 *               None.
 *           @defgroup type_group_scp_intc_enum 4.enumeration
 *               None.
 *           @defgroup type_group_scp_intc_def 5.define
 *               None.
 *         @}
 *     @}
 */
#ifdef CFG_XGPT_SUPPORT
static uint64_t start_cs_time;
static uint64_t end_cs_time;
static uint64_t cs_duration_time;
#endif
static uint64_t max_cs_duration_time;

static uint64_t max_cs_limit;
extern UBaseType_t uxCriticalNesting;

static LIST_HEAD(irq_desc_head);

/** @ingroup type_group_scp_intc_struct
 * @brief INTC_PWR irq event structure\n
 */
struct irq_pwr_descr_t {
    /** intc_pwr id */
    unsigned short id;
    /** intc_pwr polarity */
    unsigned short polarity;
    /** intc_pwr group*/
    unsigned short group;
    /** intc_pwr handler */
    irq_handler_t handler;
    /** intc_pwr user defined data */
    void *userdata;
};

static struct irq_desc_t *irq_to_desc(uint32_t irq)
{
    struct irq_desc_t *irq_desc;
    struct list_head *list;

    list_for_each(list, &irq_desc_head) {
	irq_desc = list_to_irq_desc(list);

	if (irq == irq_desc->hwirq)
	    return irq_desc;
    }

    PRINTF_E("can't find irq [%d]\r\n", irq);
    return NULL;
}

/** @ingroup type_group_scp_intc_InFn
 * @par Description
 *     get irq mask status.
 * @param[in]
 *     irq: irq interrupt number.
 * @return
 *     irq mask status.
 * @par Boundary case and Limitation
 *     irq < IRQ_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
uint32_t get_mask_irq(IRQn_Type irq)
{
    return NVIC->ISER[((uint32_t)((int32_t) irq) >> 5)] &
           (1 << ((uint32_t)((int32_t) irq) & 0x1F));
}

/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     Enable irq.
 * @param[in]
 *     irq: irq interrupt number.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     irq < IRQ_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void unmask_irq(IRQn_Type irq)
{
    struct irq_desc_t *irq_desc = irq_to_desc(irq);

    if (irq_desc && irq_desc->handler != NULL)
        NVIC_EnableIRQ(irq);
    else
        PRINTF_E("ERROR Interrupt ID %d\n\r", irq);

}

/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     Disable irq.
 * @param[in]
 *     irq: irq interrupt number.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     irq < IRQ_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mask_irq(IRQn_Type irq)
{
    struct irq_desc_t *irq_desc = irq_to_desc(irq);

    if (irq_desc && irq_desc->handler != NULL)
        NVIC_DisableIRQ(irq);
    else
        PRINTF_E("ERROR IRQ ID %d\n\r", irq);
}

static void irq_set_sen(unsigned int irq, unsigned int sen)
{
	return;
}

static void irq_set_pol(unsigned int irq, unsigned int pol)
{
	unsigned int mask;
	unsigned int value;

	if (IRQ_TYPE_LEVEL_LOW & pol) {
		mask = irq % 32;
		value = readl(CMSYS_POLARITY_BASE + (irq / 32) * 4);
		value |= (1 << mask);
		writel(value, CMSYS_POLARITY_BASE + (irq / 32) * 4);
	} else if (IRQ_TYPE_LEVEL_HIGH & pol) {
		mask = irq % 32;
		value = readl(CMSYS_POLARITY_BASE + (irq / 32) * 4);
		value &= ~(1 << mask);
		writel(value, CMSYS_POLARITY_BASE + (irq / 32) * 4);
	}
}

int irq_set_type(uint32_t irq, uint32_t flags)
{
    struct irq_desc_t *irq_desc = irq_to_desc(irq);

    if (irq_desc == NULL)
	return -1;

    irq_set_sen(irq, flags);
    irq_set_pol(irq, flags);
    irq_desc->flags = flags;
    return 0;
}
/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     Record INTC interrupt wakeup count.
 * @param[in]
 *     irq_status_msb: INTC interrupt status msb.
 * @param[in]
 *     irq_status: INTC interrupt status.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void wakeup_source_count(uint32_t irq_status_msb, uint32_t irq_status)
{
#if 0
    int32_t id;
    uint64_t intc_irq_status;

    intc_irq_status = ((uint64_t)(irq_status_msb) << 32)
                      | ((uint64_t)irq_status);

    for (id = 0; id <= PWRAP_P2P_GSPI; id++) {
        if (intc_irq_status & (1ULL << id))
            irq_desc[id].wakeup_count++;
    }
#endif
}

/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     Register a irq handler.
 * @param[in]
 *     irq: irq interrupt number.
 * @param[in]
 *     handler: irq handler.
 * @param[in]
 *     *name: interrupt name.
 * @param[in]
 *     *pdata: private data passed back to the handler function
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     irq < IRQ_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void request_irq(uint32_t irq, irq_handler_t handler, uint32_t flags,
		 const char *name, void *pdata)
{
    struct irq_desc_t *irq_desc = malloc(sizeof(struct irq_desc_t));

    if (irq_desc == NULL) {
	PRINTF_E("No space for irq [%d]\n\r", irq);
	return;
    }

    irq_desc->name = name;
    irq_desc->handler = handler;
    irq_desc->pdata = pdata;
    irq_desc->hwirq = irq;
    irq_desc->priority = DEFAULT_IRQ_PRIORITY;
    irq_desc->irq_count = 0;
    irq_desc->wakeup_count = 0;
    irq_desc->init_count++;
    irq_desc->last_enter = 0;
    irq_desc->last_exit = 0;
    irq_desc->max_duration = 0;
    NVIC_SetPriority(irq, irq_desc->priority);

    list_add_tail(&irq_desc->irq_list, &irq_desc_head);

    if (!(flags & IRQ_NOAUTOEN))
	NVIC_EnableIRQ(irq);
    irq_set_type(irq, flags);
}

/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     Unregister an irq handler.
 * @param[in]
 *     irq: irq interrupt number.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     irq < IRQ_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void free_irq(uint32_t irq)
{
    struct irq_desc_t *irq_desc = irq_to_desc(irq);

    if (irq_desc) {
	NVIC_DisableIRQ(irq);

	if (irq_desc->last_exit < irq_desc->last_enter)
	    PRINTF_W("free_irq:IRQ ID %lu maybe processing\n\r", irq);

	list_del(&irq_desc->irq_list);
	free(irq_desc);
    } else
	PRINTF_E("free irq [%d] error\n", irq);
}

/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     IRQ data structure initialization.
 * @par Parameters
 *     None.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void irq_init(void)
{
    uint32_t reg;
    uint32_t irq;

    for (irq = 0; irq < IRQ_MAX_CHANNEL; irq++)
        NVIC_DisableIRQ(irq);

    //enable system interrupt
    reg = readl(CMSYS_DBG_CTRL);
    writel(reg | ENABLE_INTERRUPT, CMSYS_DBG_CTRL);
}

/** @ingroup type_group_scp_intc_InFn
 * @par Description
 *     Low level interrupt service routine.
 * @par Parameters
 *     None.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void hw_isr_dispatch(void)
{
    uint32_t ulCurrentInterrupt;
    uint64_t duration;
    void *pdata;
    struct irq_desc_t *irq_desc;

    /* Obtain the number of the currently executing interrupt. */
    __asm volatile("mrs %0, ipsr":"=r"(ulCurrentInterrupt));

    /* skip the CM4 built-in interrupts:16 */
    ulCurrentInterrupt = ulCurrentInterrupt - 16;

    irq_desc = irq_to_desc(ulCurrentInterrupt);
    if (irq_desc) {
	if(irq_desc->handler) {
	    irq_desc->irq_count ++;
	    pdata = irq_desc->pdata;
	    /* record the last handled interrupt duration, unit: (ns) */
	    //irq_desc[ulCurrentInterrupt].last_enter =
	    //read_xgpt_stamp_ns();
	    irq_desc->handler(ulCurrentInterrupt, pdata);
	    //irq_desc[ulCurrentInterrupt].last_exit =
	    //read_xgpt_stamp_ns();
	    duration = irq_desc->last_exit - irq_desc->last_enter;

	    /* handle the xgpt overflow case
	     * discard the duration time when exit time < enter time
	     */
	    if (irq_desc->last_exit > irq_desc->last_enter)
		irq_desc->max_duration = duration;
	} else
	    PRINTF_E("IRQ ID %lu handler is null\n\r", ulCurrentInterrupt);

    } else
	PRINTF_E("ERROR IRQ ID %lu\n\r", ulCurrentInterrupt);
}

/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     IRQ status dump.
 * @par Parameters
 *     None.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void irq_status_dump(void)
{
    int32_t id;
    struct irq_desc_t *irq_desc;

    PRINTF_E(
        "id\tname\tpriority(HW\tcount\tlast\tenable\tpending\tactive\n\r");
    for (id = NonMaskableInt_IRQn; id < 0; id++) {
        PRINTF_E("%lu\t%s\t%d(%lu)\t\t%d\t%d\t%s\t%s\t%s\n\r", id, "n/a",
                 0, NVIC_GetPriority(id), 0, 0,
                 get_mask_irq(id) ? "enable" : "disable", "n/a", "n/a");
    }
    for (id = 0; id < IRQ_MAX_CHANNEL; id++) {
	irq_desc = irq_to_desc(id);
        PRINTF_E("%lu\t%s\t%lu(%lu)\t\t%lu\t%llu\t%s\t%s\t%s\n\r", id,
                 (irq_desc->name) ? irq_desc->name : "n/a",
                 irq_desc->priority, NVIC_GetPriority(id),
                 irq_desc->irq_count, irq_desc->last_exit,
                 get_mask_irq(id) ? "enable" : "disable",
                 NVIC_GetPendingIRQ(id) ? "enable" : "no",
                 NVIC_GetActive(id) ? "enable" : "inactive");
    }
}

/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     Set critical section limitation time.
 * @param[in]
 *     limit_time: Limitation time (ns) for critical section.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void set_max_cs_limit(uint64_t limit_time)
{
    if (max_cs_limit == 0)
        max_cs_limit = limit_time;
    else {
        PRINTF_D("set_max_cs_limit(%llu) failed\n\r", limit_time);
        PRINTF_D(
            "Already set max context switch limit:%llu,\n\r", limit_time);
        PRINTF_D(
            "Please run disable_cs_limit() to disiable it first\n");
    }
}

/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     Reset critical section limitation time to 0.
 * @par Parameters
 *     None.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void disable_cs_limit(void)
{
    max_cs_limit = 0;
}

/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     Wrapper of vPortEnterCritical, with limitation time check.
 * @par Parameters
 *     None.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void wrap_vPortEnterCritical(void)
{
    portENTER_CRITICAL();
#ifdef CFG_XGPT_SUPPORT
    if (uxCriticalNesting == 1 && max_cs_limit != 0)
        //start_cs_time = read_xgpt_stamp_ns();
#endif
    }

/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     Wrapper of vPortExitCritical, with limitation time check.
 * @par Parameters
 *     None.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void wrap_vPortExitCritical(void)
{
#ifdef CFG_XGPT_SUPPORT
    if ((uxCriticalNesting - 1) == 0 && max_cs_limit != 0) {
        end_cs_time = read_xgpt_stamp_ns();

        if (end_cs_time > start_cs_time) {
            cs_duration_time = end_cs_time - start_cs_time;
            if (cs_duration_time > max_cs_duration_time)
                max_cs_duration_time = cs_duration_time;
            if (max_cs_duration_time > max_cs_limit) {
                PRINTF_D(
                    "violate the critical section time limit:%llu>%llu\n",
                    max_cs_duration_time, max_cs_limit);
                configASSERT(0);
            }
        }
    }
#endif
    portEXIT_CRITICAL();
}

/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     Get max critical section duration time.
 * @par Parameters
 *     None.
 * @return
 *     Critical section duration time in ns.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
uint64_t get_max_cs_duration_time(void)
{
    return max_cs_duration_time;
}

/** @ingroup type_group_scp_intc_ext_API
 * @par Description
 *     Get max critical section limitation time.
 * @par Parameters
 *     None.
 * @return
 *     Critical section limitation time in ns.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
uint64_t get_max_cs_limit(void)
{
    return max_cs_limit;
}

#ifdef CFG_PD_SUPPORT
/** @ingroup type_group_scp_intc_def
 * @{
 */
static int irq_lock_count;
/** @}
 */

/** @ingroup type_group_scp_intc_InFn
 * @par Description
 *     Wrapper of IRQ enable..
 * @par Parameters
 *     None.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void irq_lock(void)
{
    __disable_irq();
    irq_lock_count++;
}

/** @ingroup type_group_scp_intc_InFn
 * @par Description
 *     Wrapper of IRQ disable.
 * @par Parameters
 *     None.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void irq_unlock(void)
{
    irq_lock_count--;
    if (!irq_lock_count)
        __enable_irq();
}
#endif
