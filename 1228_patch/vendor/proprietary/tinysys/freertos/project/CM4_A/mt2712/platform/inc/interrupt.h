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

/**
 * @file interrupt.h
 * Heander of interrupt.c
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "FreeRTOS.h"
#include <task.h>

#include <platform_mtk.h>
#include <driver_api.h>
#include <mt_reg_base.h>
#include <scp_regs.h>
#include <mt_list.h>

enum {
        IRQ_TYPE_NONE           = 0x00000000,
        IRQ_TYPE_EDGE_RISING    = 0x00000001,
        IRQ_TYPE_EDGE_FALLING   = 0x00000002,
        IRQ_TYPE_EDGE_BOTH      = (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING),
        IRQ_TYPE_LEVEL_HIGH     = 0x00000004,
        IRQ_TYPE_LEVEL_LOW      = 0x00000008,
        IRQ_TYPE_LEVEL_MASK     = (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH),
        IRQ_TYPE_SENSE_MASK     = 0x0000000f,
        IRQ_TYPE_DEFAULT        = IRQ_TYPE_SENSE_MASK,

        IRQ_TYPE_PROBE          = 0x00000010,

        IRQ_LEVEL               = (1 <<  8),
        IRQ_PER_CPU             = (1 <<  9),
        IRQ_NOPROBE             = (1 << 10),
        IRQ_NOREQUEST           = (1 << 11),
        IRQ_NOAUTOEN            = (1 << 12),
        IRQ_NO_BALANCING        = (1 << 13),
        IRQ_MOVE_PCNTXT         = (1 << 14),
        IRQ_NESTED_THREAD       = (1 << 15),
        IRQ_NOTHREAD            = (1 << 16),
        IRQ_PER_CPU_DEVID       = (1 << 17),
        IRQ_IS_POLLED           = (1 << 18),
        IRQ_DISABLE_UNLAZY      = (1 << 19),
};

#define DEFAULT_IRQ_PRIORITY    (0x8)
/** @ingroup type_group_scp_intc_ext_typedef
 * @{
 */
typedef void (*irq_handler_t)(int, void *);
/** @}
 */

/** @ingroup type_group_scp_intc_ext_struct
 * @brief INTC irq event structure\n
 */
struct irq_desc_t {
    /** member of interrupt list */
    struct list_head irq_list;
    /** interrupt event handler */
    irq_handler_t handler;
    /** interrupt event name */
    const char *name;
    /** interrupt event private data */
    void *pdata;
    /** interrupt hardware number */
    uint32_t hwirq;
    /** interrupt polarity */
    uint32_t priority;
    /** interrupt flags */
    uint32_t flags;
    /** interrupt event count */
    uint32_t irq_count;
    /** interrupt wakeup count */
    uint32_t wakeup_count;
    /** initialization times of an interrupt */
    uint32_t init_count;
    /** last event handled enter time */
    uint64_t last_enter;
    /** last event handled exit time */
    uint64_t last_exit;
    /** interrupt event duraiton  */
    uint64_t max_duration;
};

#define list_to_irq_desc(list) \
	list_entry(list, struct irq_desc_t, irq_list)


#define CM4_portVECTACTIVE_MASK (0xFFUL)

__STATIC_INLINE uint32_t is_in_isr(void)
{
    /*  Masks off all bits but the VECTACTIVE bits in the ICSR register. */
    if ((portNVIC_INT_CTRL_REG & CM4_portVECTACTIVE_MASK) != 0)
        return 1;   /*in isr */
    else
        return 0;
}

#define local_irq_save(flag)                \
    {                       \
        flag = __get_PRIMASK();         \
        __disable_irq();            \
    }


#define local_irq_restore(flag)         \
    {                   \
        __set_PRIMASK(flag);        \
    }


int irq_set_type(uint32_t irq, uint32_t flags);
void request_irq(uint32_t irq, irq_handler_t handler, uint32_t flags,
		 const char *name, void *pdata);
void mask_irq(IRQn_Type IRQn);
void unmask_irq(IRQn_Type IRQn);
void irq_status_dump(void);
void irq_init(void);
void free_irq(uint32_t irq);
void set_max_cs_limit(uint64_t limit_time);
void disable_cs_limit(void);
void wrap_vPortEnterCritical(void);
void wrap_vPortExitCritical(void);
uint64_t get_max_cs_duration_time(void);
uint64_t get_max_cs_limit(void);
#endif
