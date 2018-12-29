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
/* MediaTek Inc. (C) 2010. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software")
 * have been modified by MediaTek Inc. All revisions are subject to any
 * receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <platform_mtk.h>
#include <interrupt.h>
#include <driver_api.h>
#include <utils.h>
#include "FreeRTOS.h"
#include "task.h"
#include "eint.h"
#include "scp_regs.h"
#include "eint_int.h"

/**
 * @defgroup IP_group_scp_eint EINT
 *     SCP EINT related functions.\n
 *     @{
 *       @defgroup IP_group_scp_eint_external EXTERNAL
 *         The external API document for SCP EINT. \n
 *         @{
 *           @defgroup type_group_scp_eint_ext_API 1.function
 *               This is SCP_EINT external function.
 *           @defgroup type_group_scp_eint_ext_struct 2.structure
 *               None.
 *           @defgroup type_group_scp_eint_ext_typedef 3.typedef
 *               This is SCP_EINT external typedef.
 *           @defgroup type_group_scp_eint_ext_enum 4.enumeration
 *               None.
 *           @defgroup type_group_scp_eint_ext_def 5.define
 *               This is SCP_EINT external define.
 *         @}
 *
 *       @defgroup IP_group_scp_eint_internal INTERNAL
 *         The internal API document for SCP EINT. \n
 *         @{
 *           @defgroup type_group_scp_eint_InFn 1.function
 *               This is SCP_EINT internal function.
 *           @defgroup type_group_scp_eint_struct 2.structure
 *               This is SCP_EINT internal structure.
 *           @defgroup type_group_scp_eint_typedef 3.typedef
 *               None.
 *           @defgroup type_group_scp_eint_enum 4.enumeration
 *               None.
 *           @defgroup type_group_scp_eint_def 5.define
 *               This is SCP_EINT internal define.
 *         @}
 *     @}
 */

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Mask all EINT sources.
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
void mt_eint_mask_all(void)
{
    unsigned int i;
    unsigned int val = 0xFFFFFFFF;

    for (i = 0; i < EINT_REG_SET_SIZE; i++) {
        WRITE_REG(val, (EINT_MASK_SET_BASE + (i * 4)));

        error_msg("[EINT] mask addr:%x = %x\n\r",
                  EINT_MASK_BASE + (i * 4),
                  READ_REG(EINT_MASK_BASE + (i * 4)));
    }
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Unmask all EINT sources.
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
void mt_eint_unmask_all(void)
{
    unsigned int i;
    unsigned int val = 0xFFFFFFFF;

    for (i = 0; i < EINT_REG_SET_SIZE; i++) {
        WRITE_REG(val, (EINT_MASK_CLR_BASE + (i * 4)));

        error_msg("[EINT] unmask addr:%x = %x\n\r",
                  EINT_MASK_BASE + (i * 4),
                  READ_REG(EINT_MASK_BASE + (i * 4)));
    }
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Save current mask setting then mask all EINT sources.
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
void mt_eint_mask_save_all(void)
{
    unsigned int i;
    unsigned int val = 0xFFFFFFFF;

    for (i = 0; i < EINT_REG_SET_SIZE; i++) {
        eint_mask_store[i] = READ_REG((EINT_MASK_BASE + (i * 4)));
        WRITE_REG(val, (EINT_MASK_SET_BASE + (i * 4)));
    }
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Recover the prestored EINT mask setting.
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
void mt_eint_mask_restore_all(void)
{
    unsigned int i;
    unsigned int val = 0xFFFFFFFF;

    for (i = 0; i < EINT_REG_SET_SIZE; i++) {
        WRITE_REG(val, (EINT_MASK_CLR_BASE + (i * 4)));
        WRITE_REG(eint_mask_store[i], (EINT_MASK_SET_BASE + (i * 4)));
    }
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Read specific EINT mask configuration.
 * @param[in]
 *     eint_num: EINT number to be read.
 * @return
 *     0: Error.\n
 *     others: EINT mask register value.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
unsigned int mt_eint_read_mask(unsigned int eint_num)
{
    unsigned int st;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL) {
        st = READ_REG(((eint_num / 32) * 4 + EINT_MASK_BASE));
        return (st & bit);
    }
    error_msg("[EINT] num. exceeds the max num.\n\r");
    return 0;

}

/** @ingroup type_group_scp_eint_ext_API
 * @par Description
 *     Mask a specific EINT source.
 * @param[in]
 *     eint_num: EINT number to be masked.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_mask(unsigned int eint_num)
{
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL)
        WRITE_REG(bit, ((eint_num / 32) * 4 + EINT_MASK_SET_BASE));
    else
        error_msg("[EINT] num. exceeds the max num.\n\r");
}

/** @ingroup type_group_scp_eint_ext_API
 * @par Description
 *     Unmask a specific EINT source.
 * @param[in]
 *     eint_num: EINT number to be unmasked.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL .
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_unmask(unsigned int eint_num)
{
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL)
        WRITE_REG(bit, ((eint_num / 32) * 4 + EINT_MASK_CLR_BASE));
    else
        error_msg("[EINT] num. exceeds the max num.\n\r");
}

/** @ingroup type_group_scp_eint_ext_API
 * @par Description
 *     Save and Mask the specified EINT number.
 * @param[in]
 *     eint_num: EINT number to mask.
 * @return
 *     0: Error.\n
 *     others: EINT mask register value.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
unsigned int mt_eint_mask_save(unsigned int eint_num)
{
    unsigned int st;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL) {
        st = READ_REG(((eint_num / 32) * 4 + EINT_MASK_BASE));
        WRITE_REG(bit, ((eint_num / 32) * 4 + EINT_MASK_SET_BASE));
        return (st & bit);
    }
    error_msg("[EINT] num. exceeds the max num.\n\r");
    return 0;
}

/** @ingroup type_group_scp_eint_ext_API
 * @par Description
 *     Restore the specified EINT number.
 * @param[in]
 *     eint_num: EINT number to reconfigure.
 * @param[in]
 *     val: 0, clear mask.\n
 *          1, set mask.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_mask_restore(unsigned int eint_num, unsigned int val)
{
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL) {
        if (val == 0) {
            WRITE_REG(bit,
                      ((eint_num / 32) * 4 + EINT_MASK_CLR_BASE));
        } else {
            WRITE_REG(bit,
                      ((eint_num / 32) * 4 + EINT_MASK_SET_BASE));
        }
    } else {
        error_msg("[EINT] num. exceeds the max num.\n\r");
    }
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     To get the eint trigger mode.
 * @param[in]
 *     eint_num: EINT number to get.
 * @return
 *     Trigger mode setting of the selected EINT.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
unsigned int mt_eint_read_sens(unsigned int eint_num)
{
    unsigned int st;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL) {
        st = READ_REG(((eint_num / 32) * 4 + EINT_SENS_BASE));
        return (st & bit);
    }

    error_msg("[EINT] num. exceeds the max num.\n\r");
    return 0;
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     To set trigger mode of an EINT.
 * @param[in]
 *     eint_num: EINT number to set.
 * @param[in]
 *     sens: EINT trigger mode.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_set_sens(unsigned int eint_num, unsigned int sens)
{
    unsigned int bit = 1 << (eint_num % 32);

    if (sens == EDGE_SENSITIVE) {
        if (eint_num < EINT_MAX_CHANNEL) {
            WRITE_REG(bit,
                      ((eint_num / 32) * 4 + EINT_SENS_CLR_BASE));
        } else {
            error_msg("[EINT] num. exceeds the max num.\n\r");
        }
    } else if (sens == LEVEL_SENSITIVE) {
        if (eint_num < EINT_MAX_CHANNEL) {
            WRITE_REG(bit,
                      ((eint_num / 32) * 4 + EINT_SENS_SET_BASE));
        } else {
            error_msg("[EINT] num. exceeds the max num.\n\r");
        }
    } else {
        error_msg("[EINT] wrong sensitivity parameter!");
    }

}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     To get polarity setting of an EINT.
 * @param[in]
 *     eint_num: EINT number to get.
 * @return
 *     Polarity setting of the selected EINT.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
unsigned int mt_eint_read_polarity(unsigned int eint_num)
{
    unsigned int st;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL) {
        st = READ_REG(((eint_num / 32) * 4 + EINT_POL_BASE));
        return (st & bit);
    }

    error_msg("[EINT] enit number exceeds the max number\n\r");
    return 0;
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     To set polarity of an EINT.
 * @param[in]
 *     eint_num: EINT number to set.
 * @param[in]
 *     pol: EINT polarity.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol)
{
    unsigned int mask_flag;
    unsigned int bit = 1 << (eint_num % 32);

    mask_flag = mt_eint_mask_save(eint_num);

    if (pol == LOW_LEVEL_TRIGGER) {
        if (eint_num < EINT_MAX_CHANNEL) {
            WRITE_REG(
                bit, ((eint_num / 32) * 4 + EINT_POL_CLR_BASE));
        } else {
            error_msg("[EINT] num. exceeds the max num.\n\r");
        }
    } else if (pol == HIGH_LEVEL_TRIGGER) {
        if (eint_num < EINT_MAX_CHANNEL) {
            WRITE_REG(
                bit, ((eint_num / 32) * 4 + EINT_POL_SET_BASE));
        } else {
            error_msg("[EINT] num. exceeds the max num.\n\r");
        }
    } else {
        error_msg("[EINT] wrong polarity parameter!");
    }

    mt_eint_mask_restore(eint_num, mask_flag);
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     To get software trigger setting of an EINT.
 * @param[in]
 *     eint_num: EINT number to get.
 * @return
 *     Software trigger setting of the selected EINT.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
unsigned int mt_eint_read_soft(unsigned int eint_num)
{
    unsigned int st;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL) {
        st = READ_REG(((eint_num / 32) * 4 + EINT_SOFT_BASE));
        return (st & bit);
    }

    error_msg("[EINT] num. exceeds the max num.\n\r");
    return 0;
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Assert a software trigger setting of an EINT.
 * @param[in]
 *     eint_num: EINT number to assert.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_soft_set(unsigned int eint_num)
{
    unsigned int bit = 1 << (eint_num % 32);

#ifdef CFG_FPGA_SUPPORT /* for FPGA */
    if (eint_num < EINT_MAX_CHANNEL)
        WRITE_REG(bit, ((eint_num / 32) * 4 + EINT_FPGA_TRIG_BASE));
    else
        error_msg("[EINT] num. exceeds the max num.\n\r");
#else
    if (eint_num < EINT_MAX_CHANNEL)
        WRITE_REG(bit, ((eint_num / 32) * 4 + EINT_SOFT_SET_BASE));
    else
        error_msg("[EINT] num. exceeds the max num.\n\r");
#endif
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     De-assert a software trigger of an EINT.
 * @param[in]
 *     eint_num: EINT number to de-assert.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_soft_clr(unsigned int eint_num)
{
    unsigned int bit = 1 << (eint_num % 32);

#ifdef CFG_FPGA_SUPPORT /* for FPGA */
    bit &= ~bit;
    if (eint_num < EINT_MAX_CHANNEL)
        WRITE_REG(bit, ((eint_num / 32) * 4 + EINT_FPGA_TRIG_BASE));
    else
        error_msg("[EINT] num. exceeds the max num.\n\r");
#else
    if (eint_num < EINT_MAX_CHANNEL)
        WRITE_REG(bit, ((eint_num / 32) * 4 + EINT_SOFT_CLR_BASE));
    else
        error_msg("[EINT] num. exceeds the max num.\n\r");
#endif
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Send a software trigger pulse to an EINT.
 * @param[in]
 *     eint_num: EINT number to trgger.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_send_pulse(unsigned int eint_num)
{
    unsigned int mask_flag;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL) {
        mask_flag = mt_eint_mask_save(eint_num);

        WRITE_REG(bit, ((eint_num / 32) * 4 + EINT_SOFT_SET_BASE));
        udelay(50);
        WRITE_REG(bit, ((eint_num / 32) * 4 + EINT_SOFT_CLR_BASE));

        mt_eint_mask_restore(eint_num, mask_flag);
        return;
    }

    error_msg("[EINT] num. exceeds the max num.\n\r");
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Disable HW debounce function of an EINT.
 * @param[in]
 *     eint_num: EINT number to disable.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_dis_hw_debounce(unsigned int eint_num)
{
    unsigned int bit;

    eint.is_deb_en[eint_num] = 0;
    eint.deb_time[eint_num] = 0;

    bit = (EINT_DBNC_CLR_EN << EINT_DBNC_CLR_EN_BITS)
          << ((eint_num % 4) * 8);
    WRITE_REG(bit, ((eint_num / 4) * 4 + EINT_DBNC_CLR_BASE));
    __DMB();
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Set HW debounce function of an EINT.
 * @param[in]
 *     eint_num: EINT number to disable.
 * @param[in]
 *     us: EINT HW debounce time in us.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int us)
{
    unsigned int dbnc, bit, clr_bit, rst;
    unsigned int mask_flag;

    if (us >= MAX_HW_DEBOUNCE_TIME) {
        error_msg(
            "Can't enable debounce time %d eint_num:%d, max debounce time is %d\n\r",
            us, eint_num, MAX_HW_DEBOUNCE_TIME);
        return;
    }

    if (us == 0)
        dbnc = 0;
    else if (us <= 125)
        dbnc = 0;
    else if (us <= 250)
        dbnc = 1;
    else if (us <= 500)
        dbnc = 2;
    else if (us <= 1000)
        dbnc = 3;
    else if (us <= 16000)
        dbnc = 4;
    else if (us <= 32000)
        dbnc = 5;
    else if (us <= 64000)
        dbnc = 6;
    else if (us <= 128000)
        dbnc = 7;
    else if (us <= 256000)
        dbnc = 8;
    else
        dbnc = 9;

    /* setp 1: mask the EINT */
    mask_flag = mt_eint_mask_save(eint_num);

    /* step 2: Check hw debouce number to decide which type should be used
     */

    /* step 2.1: disable debounce */
    mt_eint_dis_hw_debounce(eint_num);

    eint.is_deb_en[eint_num] = 1;
    eint.deb_time[eint_num] = us;

    /* step 2.2: clear register */
    clr_bit = 0xFF << ((eint_num % 4) * 8);
    WRITE_REG(clr_bit, ((eint_num / 4) * 4 + EINT_DBNC_CLR_BASE));
    __DMB();

    /* step 2.3: reset counter and dsb */
    rst = (EINT_DBNC_RST_EN << EINT_DBNC_SET_RST_BITS)
          << ((eint_num % 4) * 8);
    WRITE_REG(rst, ((eint_num / 4) * 4 + EINT_DBNC_SET_BASE));
    __DMB();

    /* step 2.4: set new debounce value & enable function */
    bit = ((dbnc << EINT_DBNC_SET_DBNC_BITS) |
           (EINT_DBNC_SET_EN << EINT_DBNC_SET_EN_BITS))
          << ((eint_num % 4) * 8);
    WRITE_REG(bit, ((eint_num / 4) * 4 + EINT_DBNC_SET_BASE));
    __DMB();

    /* step 3: unmask the EINT */
    mt_eint_mask_restore(eint_num, mask_flag);
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Enable all EINT on domain0.
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
void mt_eint_domain0_set_all(void)
{
    unsigned int val = 0xFFFFFFFF, i;

    for (i = 0; i < EINT_REG_SET_SIZE; i++)
        WRITE_REG(val, (EINT_D0_EN_BASE + (i * 4)));

}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Disable all EINT on domain0.
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
void mt_eint_domain0_clr_all(void)
{
    unsigned int val = 0x0, i;

    for (i = 0; i < EINT_REG_SET_SIZE; i++)
        WRITE_REG(val, (EINT_D0_EN_BASE + (i * 4)));
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Save and disable all EINT on domain0.
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
void mt_eint_domain0_save_all(void)
{
    unsigned int val = 0x0, i;

    for (i = 0; i < EINT_REG_SET_SIZE; i++) {
        eint_domain0_store[i] = READ_REG((EINT_D0_EN_BASE + (i * 4)));
        WRITE_REG(val, (EINT_D0_EN_BASE + (i * 4)));
    }
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Restore the previously backup domain0 EINT.
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
void mt_eint_domain0_restore_all(void)
{
    unsigned int i;

    for (i = 0; i < EINT_REG_SET_SIZE; i++)
        WRITE_REG(eint_domain0_store[i], (EINT_D0_EN_BASE + (i * 4)));
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Enable a specific EINT on domain0.
 * @param[in]
 *     eint_num: EINT number to enable.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_domain0_set(unsigned int eint_num)
{
    unsigned int st;

    if (eint_num < EINT_MAX_CHANNEL) {
        st = READ_REG(((eint_num / 32) * 4 + EINT_D0_EN_BASE));
        st |= (1 << (eint_num % 32));
        WRITE_REG(st, ((eint_num / 32) * 4 + EINT_D0_EN_BASE));
    } else {
        error_msg("[EINT] num. exceeds the max num.\n\r");
    }
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Disable a specific EINT on domain0.
 * @param[in]
 *     eint_num: EINT number to disable.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_domain0_clr(unsigned int eint_num)
{
    unsigned int st;

    if (eint_num < EINT_MAX_CHANNEL) {
        st = READ_REG(((eint_num / 32) * 4 + EINT_D0_EN_BASE));
        st &= ~(1 << (eint_num % 32));
        WRITE_REG(st, ((eint_num / 32) * 4 + EINT_D0_EN_BASE));
    } else {
        error_msg("[EINT] num. exceeds the max num.\n\r");
    }
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Save and disable a specific EINT on domain0.
 * @param[in]
 *     eint_num: EINT number to disable and save.
 * @return
 *     Domain0 configuration of the seleted EINT.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
unsigned int mt_eint_domain0_save(unsigned int eint_num)
{
    unsigned int st;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL) {
        st = READ_REG(((eint_num / 32) * 4 + EINT_D0_EN_BASE));
        WRITE_REG(
            (st & (~bit)), ((eint_num / 32) * 4 + EINT_D0_EN_BASE));
        return (st & bit);
    }

    error_msg("[EINT] num. exceeds the max num.\n\r");
    return 0;
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Enable or disable a specific EINT on domain0.
 * @param[in]
 *     eint_num: EINT number to configure.
 * @param[in]
 *     val: 1, enable.\n
 *          0, disable.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_domain0_restore(unsigned int eint_num, unsigned int val)
{
    unsigned int st;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL) {
        if (val == 0) {
            st = READ_REG(((eint_num / 32) * 4 + EINT_D0_EN_BASE));
            WRITE_REG((st & (~bit)),
                      ((eint_num / 32) * 4 + EINT_D0_EN_BASE));
        } else {
            st = READ_REG(((eint_num / 32) * 4 + EINT_D0_EN_BASE));
            WRITE_REG((st | bit),
                      ((eint_num / 32) * 4 + EINT_D0_EN_BASE));
        }
    } else {
        error_msg("[EINT] num. exceeds the max num.\n\r");
    }
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Get status register of all EINT.
 * @param[in]
 *     eint_num: EINT number to get.
 * @return
 *     EINT status register value.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
unsigned int mt_eint_get_status(unsigned int eint_num)
{
    unsigned int st;

    if (eint_num < EINT_MAX_CHANNEL) {
        st = READ_REG(((eint_num / 32) * 4 + EINT_STA_BASE));
        return st;
    }

    error_msg("[EINT] num. exceeds the max num.\n\r");
    return 0;
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Get status of a specific EINT.
 * @param[in]
 *     eint_num: EINT number to get.
 * @return
 *     Status of a specific EINT.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
unsigned int mt_eint_read_status(unsigned int eint_num)
{
    unsigned int st;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL) {
        st = READ_REG(((eint_num / 32) * 4 + EINT_STA_BASE));
        return (st & bit);
    }

    error_msg("[EINT] num. exceeds the max num.\n\r");
    return 0;
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Get raw status of a specific EINT.
 * @param[in]
 *     eint_num: EINT number to get.
 * @return
 *     Status of a specific EINT.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
unsigned int mt_eint_read_raw_status(unsigned int eint_num)
{
    unsigned int st;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL) {
        st = ~(READ_REG(((eint_num / 32) * 4 + EINT_RAW_STA_BASE)));
        return (st & bit);
    }

    error_msg("[EINT] num. exceeds the max num.\n\r");
    return 0;
}

/** @ingroup type_group_scp_eint_ext_API
 * @par Description
 *     To ack specific EINT interrupt.
 * @param[in]
 *     eint_num: EINT number to ack.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_ack(unsigned int eint_num)
{
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_MAX_CHANNEL)
        WRITE_REG(bit, ((eint_num / 32) * 4 + EINT_INTACK_BASE));
    else
        error_msg("[EINT] num. exceeds the max num.\n\r");
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     EINT LISR.
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
static void mt_eint_lisr(void)
{
    unsigned int index;
    unsigned int status = 0;
    unsigned int status_check;
    unsigned int xLastExecutionStamp, xFirstExecutionStamp;

    debug_msg("[EINT]ISR Start\n\r");

    for (index = 0; index < EINT_MAX_CHANNEL; index++) {
        /* read & print status register every 32 interrupts */
        if ((index % 32) == 0) {
            status = mt_eint_get_status(index);
            debug_msg("[EINT]REG_SET_INDEX = %d, STATUS = 0x%x\n\r",
                      index, status);
        }

        status_check = status & (1 << (index % 32));

        /* execute EINT callback function */
        if (status_check) {
            mt_eint_mask(index);

            if (eint.eint_func[index] != NULL) {
#ifdef CFG_XGPT_SUPPORT
                /* Get start time */
                xFirstExecutionStamp = timestamp_get_ns();
#else
                xFirstExecutionStamp = 0;
#endif
                eint.eint_func[index]();
#ifdef CFG_XGPT_SUPPORT
                /* Get finish time */
                xLastExecutionStamp = timestamp_get_ns();
#else
                xLastExecutionStamp = 0;
#endif
                /* debug_msg("[EINT]Callback First Stamp = */
                /* %u\n\r", xFirstExecutionStamp); */
                /* debug_msg("[EINT]Callback Last  Stamp = */
                /* %u\n\r", xLastExecutionStamp); */
                if (xFirstExecutionStamp >
                        xLastExecutionStamp) {
                    xLastExecutionStamp =
                        (xFirstExecutionStamp -
                         xLastExecutionStamp);
                } else {
                    xLastExecutionStamp =
                        (xLastExecutionStamp -
                         xFirstExecutionStamp);
                }

                debug_msg(
                    "[EINT]Callback Execution Stamp = %u\n\r",
                    xLastExecutionStamp);
            }

            mt_eint_ack(index);

            if (eint.eint_auto_umask[index])
                mt_eint_unmask(index);

        }
    }

    debug_msg("[EINT]ISR END\n\r");
}

/** @ingroup type_group_scp_eint_ext_API
 * @par Description
 *     Register an EINT event.
 * @param[in]
 *     eint_num: EINT number.
 * @param[in]
 *     sens: EINT trigger mode.
 * @param[in]
 *     pol: EINT polarity.
 * @param[in]
 *     handler: EINT handler.
 * @param[in]
 *     unmask: unmask EINT after registration.
 * @param[in]
 *     is_auto_umask: unmask EINT after the event is handled.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_registration(unsigned int eint_num, unsigned int sens,
                          unsigned int pol, eint_handler_t handler, unsigned int unmask,
                          unsigned int is_auto_umask)
{
    if (eint_num < EINT_MAX_CHANNEL) {

        eint.eint_func[eint_num] = handler;

        eint.eint_auto_umask[eint_num] = is_auto_umask;

        mt_eint_domain0_set(eint_num);

        mt_eint_set_sens(eint_num, sens);

        mt_eint_set_polarity(eint_num, pol);

        mt_eint_ack(eint_num);

        if (unmask)
            mt_eint_unmask(eint_num);

    } else {
        error_msg("[EINT] num. exceeds the max num.\n\r");
    }
}

/** @ingroup type_group_scp_eint_ext_API
 * @par Description
 *     Unregister an EINT event.
 * @param[in]
 *     eint_num: EINT number.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     eint_num < EINT_MAX_CHANNEL.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_unregistration(unsigned int eint_num)
{
    if (eint_num < EINT_MAX_CHANNEL) {

        eint.eint_func[eint_num] = NULL;
        eint.eint_auto_umask[eint_num] = 0;
        eint.is_deb_en[eint_num] = 0;
        eint.deb_time[eint_num] = 0;

        mt_eint_domain0_clr(eint_num);

        mt_eint_ack(eint_num);

        mt_eint_mask(eint_num);

        mt_eint_dis_hw_debounce(eint_num);
    } else {
        error_msg("[EINT] num. exceeds the max num.\n\r");
    }
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Save all EINT configurations.
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
void mt_eint_save_all_config(void)
{
    int i;

    mt_eint_domain0_save_all();

    mt_eint_mask_save_all();

    for (i = 0; i < EINT_MAX_CHANNEL; i++) {
        eint_config.eint_func[i] = eint.eint_func[i];
        eint_config.eint_auto_umask[i] = eint.eint_auto_umask[i];
        eint_config.is_deb_en[i] = eint.is_deb_en[i];
        eint_config.deb_time[i] = eint.deb_time[i];
        eint_config.sens[i] = mt_eint_read_sens(i);
        eint_config.pol[i] = mt_eint_read_polarity(i);
    }
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Restore all EINT configurations.
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
void mt_eint_store_all_config(void)
{
    int i;

    for (i = 0; i < EINT_MAX_CHANNEL; i++) {
        eint.eint_func[i] = eint_config.eint_func[i];
        eint.eint_auto_umask[i] = eint_config.eint_auto_umask[i];
        eint.is_deb_en[i] = eint_config.is_deb_en[i];
        eint.deb_time[i] = eint_config.deb_time[i];
        mt_eint_set_sens(i,
                         (eint_config.sens[i] ? LEVEL_SENSITIVE : EDGE_SENSITIVE));
        mt_eint_set_polarity(i, (eint_config.pol[i] ? HIGH_LEVEL_TRIGGER
                                 : LOW_LEVEL_TRIGGER));
        mt_eint_ack(i);
    }

    mt_eint_mask_restore_all();

    mt_eint_domain0_restore_all();
}

/** @ingroup type_group_scp_eint_ext_API
 * @par Description
 *     EINT module initialization.
 * @par Parameters
 *     None.
 * @return
 *     Return 0.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
int mt_eint_init(void)
{
    unsigned int i;

    debug_msg("[EINT]driver init\n\r");

    for (i = 0; i < EINT_MAX_CHANNEL; i++) {
        eint.eint_func[i] = NULL;
        eint.eint_auto_umask[i] = 0;
        eint.is_deb_en[i] = 0;
        eint.deb_time[i] = 0;
    }

    for (i = 0; i < EINT_MAX_CHANNEL; i++) {
        eint_config.eint_func[i] = NULL;
        eint_config.eint_auto_umask[i] = 0;
        eint_config.is_deb_en[i] = 0;
        eint_config.deb_time[i] = 0;
        eint_config.sens[i] = 0;
        eint_config.pol[i] = 0;
    }

    /* init EINT interrupt ack  */
    for (i = 0; i < EINT_MAX_CHANNEL; i++)
        mt_eint_ack(i);

    /* request & unmask SCP IRQ  */
    /* unmask_irq(EINT_IRQn); */
    request_irq(EINT_IRQn, mt_eint_lisr, "EINT");
    return 0;
}

/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     To get a specific EINT configurations.
 * @param[in]
 *     eint_num: EINT number to get.
 * @return
 *     None.
 * @par Boundary case and Limitation
 *     None.
 * @par Error case and Error handling
 *     None.
 * @par Call graph and Caller graph (refer to the graph below)
 * @par Refer to the source code
 */
void mt_eint_dump_config(unsigned int eint_num)
{
    unsigned int st;
    unsigned int bit;
    unsigned int eint_mask;
    unsigned int eint_sens;
    unsigned int eint_pol;
    unsigned int is_deb_en;
    unsigned int deb_time;
    unsigned int deb_reg_value;
    unsigned int int_status;
    eint_handler_t eint_func;

    if (eint_num < EINT_MAX_CHANNEL) {
        bit = 1 << (eint_num % 32);
        st = READ_REG(((eint_num / 32) * 4 + EINT_MASK_BASE));
        eint_mask = bit & st;
        if (eint_mask > 0)
            eint_mask = 1;
        st = READ_REG(((eint_num / 32) * 4 + EINT_SENS_BASE));
        eint_sens = bit & st;
        if (eint_sens > 0)
            eint_sens = 1;
        st = READ_REG(((eint_num / 32) * 4 + EINT_POL_BASE));
        eint_pol = bit & st;
        if (eint_pol > 0)
            eint_pol = 1;
        deb_time = eint.deb_time[eint_num];

        deb_reg_value = READ_REG(((eint_num / 4) * 4 + EINT_DBNC_BASE));
        deb_reg_value = deb_reg_value >> ((eint_num % 4) * 8);
        is_deb_en = deb_reg_value & 0x01;
        deb_reg_value = deb_reg_value & 0x70;
        deb_reg_value = deb_reg_value >> 4;
        eint_func = eint.eint_func[eint_num];
        int_status = mt_eint_get_status(eint_num);

        debug_msg("[EINT] number   = %d\n\r", eint_num);
        debug_msg("[EINT] mask     = %d\n\r", eint_mask);
        debug_msg("[EINT] sens     = %d\n\r", eint_sens);
        debug_msg("[EINT] pol      = %d\n\r", eint_pol);
        debug_msg("[EINT] deb_en   = %d\n\r", is_deb_en);
        debug_msg("[EINT] deb_time = %d\n\r", deb_time);
        debug_msg("[EINT] deb_reg_value = %d\n\r", deb_reg_value);
        debug_msg("[EINT] eint_func = %p\n\r", eint_func);
        debug_msg("[EINT] int_status = %d\n\r", int_status);

        return;
    }

    error_msg("[EINT] num. exceeds the max num.\n\r");
}


/** @ingroup type_group_scp_eint_InFn
 * @par Description
 *     Dump all EINT configurations.
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
void mt_eint_dump_all_config(void)
{
    unsigned int st;
    unsigned int bit;
    unsigned int eint_mask;
    unsigned int eint_sens;
    unsigned int eint_pol;
    unsigned int is_deb_en;
    unsigned int deb_time;
    unsigned int deb_reg_value;
    unsigned int int_status;
    eint_handler_t eint_func;
    int eint_num;

    debug_msg("[EINT]\n\r");
    debug_msg(
        "eint_id\tmask\tsens\tpol\tdeb_en\tdeb_ms\tdeb_reg\tfunc\tint_sta\n\r");
    for (eint_num = 0; eint_num < EINT_MAX_CHANNEL; eint_num++) {
        bit = 1 << (eint_num % 32);
        st = READ_REG(((eint_num / 32) * 4 + EINT_MASK_BASE));
        eint_mask = bit & st;
        if (eint_mask > 0)
            eint_mask = 1;
        st = READ_REG(((eint_num / 32) * 4 + EINT_SENS_BASE));
        eint_sens = bit & st;
        if (eint_sens > 0)
            eint_sens = 1;
        st = READ_REG(((eint_num / 32) * 4 + EINT_POL_BASE));
        eint_pol = bit & st;
        if (eint_pol > 0)
            eint_pol = 1;
        deb_time = eint.deb_time[eint_num];

        deb_reg_value = READ_REG(((eint_num / 4) * 4 + EINT_DBNC_BASE));
        deb_reg_value = deb_reg_value >> ((eint_num % 4) * 8);
        is_deb_en = deb_reg_value & 0x01;
        deb_reg_value = deb_reg_value & 0x70;
        deb_reg_value = deb_reg_value >> 4;
        eint_func = eint.eint_func[eint_num];
        int_status = mt_eint_get_status(eint_num);

        debug_msg("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%p\t%d\n\r", eint_num,
                  eint_mask, eint_sens, eint_pol, is_deb_en, deb_time,
                  deb_reg_value, eint_func, int_status);
    }
}
