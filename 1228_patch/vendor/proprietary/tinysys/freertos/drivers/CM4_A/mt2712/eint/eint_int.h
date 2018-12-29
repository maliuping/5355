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

#ifndef __MT_EINT_INT_H__
#define __MT_EINT_INT_H__
/* EINT register mapping & definition */
/** @ingroup type_group_scp_eint_def
 * @{
 */
#define SCP_EINT_BASE       (0x40900000)
#define EINT_STA_BASE       ((SCP_EINT_BASE + SCP_EINT_STA0))
#define EINT_INTACK_BASE    ((SCP_EINT_BASE + SCP_EINT_INTACK0))
#define EINT_MASK_BASE      ((SCP_EINT_BASE + SCP_EINT_MASK0))
#define EINT_MASK_SET_BASE  ((SCP_EINT_BASE + SCP_EINT_MASK_SET0))
#define EINT_MASK_CLR_BASE  ((SCP_EINT_BASE + SCP_EINT_MASK_CLR0))
#define EINT_SENS_BASE      ((SCP_EINT_BASE + SCP_EINT_SENS0))
#define EINT_SENS_SET_BASE  ((SCP_EINT_BASE + SCP_EINT_SENS_SET0))
#define EINT_SENS_CLR_BASE  ((SCP_EINT_BASE + SCP_EINT_SENS_CLR0))
#define EINT_SOFT_BASE      ((SCP_EINT_BASE + SCP_EINT_SOFT0))
#define EINT_SOFT_SET_BASE  ((SCP_EINT_BASE + SCP_EINT_SOFT_SET0))
#define EINT_SOFT_CLR_BASE  ((SCP_EINT_BASE + SCP_EINT_SOFT_CLR0))
#define EINT_POL_BASE       ((SCP_EINT_BASE + SCP_EINT_POL0))
#define EINT_POL_SET_BASE   ((SCP_EINT_BASE + SCP_EINT_POL_SET0))
#define EINT_POL_CLR_BASE   ((SCP_EINT_BASE + SCP_EINT_POL_CLR0))
#define EINT_D0_EN_BASE     ((SCP_EINT_BASE + SCP_EINT_D0EN0))
#define EINT_DBNC_BASE      ((SCP_EINT_BASE + SCP_EINT_DBNC_3_0))
#define EINT_DBNC_SET_BASE  ((SCP_EINT_BASE + SCP_EINT_DBNC_SET_3_0))
#define EINT_DBNC_CLR_BASE  ((SCP_EINT_BASE + SCP_EINT_DBNC_CLR_3_0))
#define EINT_RAW_STA_BASE   ((SCP_EINT_BASE + SCP_EINT_RAW_STA0))
#ifdef CFG_FPGA_SUPPORT
#define EINT_FPGA_TRIG_BASE ((SCP_EINT_BASE + 0x000c3f00))
#endif

#define EINT_DBNC_SET_EN_BITS   (0)
#define EINT_DBNC_CLR_EN_BITS   (0)
#define EINT_DBNC_SET_RST_BITS  (1)
#define EINT_DBNC_CLR_RST_BITS  (1)
#define EINT_DBNC_SET_DBNC_BITS (4)
#define EINT_DBNC_CLR_DBNC_BITS (4)

#define EINT_DBNC_SET_EN    (0x1)
#define EINT_DBNC_CLR_EN    (0x1)
#define EINT_DBNC_RST_EN    (0x1)
#define EINT_DBNC_0_MS      (0x7)

#define EINT_STA_DEFAULT    0x00000000
#define EINT_INTACK_DEFAULT 0x00000000
#define EINT_MASK_DEFAULT   0x00007FFF
#define EINT_MASK_SET_DEFAULT   0x00000000
#define EINT_MASK_CLR_DEFAULT   0x00000000
#define EINT_SENS_DEFAULT   0x00007FFF
#define EINT_SENS_SET_DEFAULT   0x00000000
#define EINT_SENS_CLR_DEFAULT   0x00000000
#define EINT_SOFT_DEFAULT   0x00000000
#define EINT_SOFT_SET_DEFAULT   0x00000000
#define EINT_SOFT_CLR_DEFAULT   0x00000000
#define EINT_POL_DEFAULT    0x00000000
#define EINT_POL_SET_DEFAULT    0x00000000
#define EINT_POL_CLR_DEFAULT    0x00000000
#define EINT_D0EN_DEFAULT   0x00000000
#define EINT_DBNC_DEFAULT   0x00000000
#define EINT_DBNC_SET_DEFAULT   0x00000000
#define EINT_DBNC_CLR_DEFAULT   0x00000000

#define EINT_MAX_CHANNEL    15
#define MAX_HW_DEBOUNCE_TIME    512000
#define EINT_REG_SET_SIZE   ((EINT_MAX_CHANNEL + 31) / 32)

#define WRITE_REG(v, a) DRV_WriteReg32(a, v)
#define READ_REG(a) DRV_Reg32(a)

static struct eint_func eint;
static struct eint_conf eint_config;

unsigned int eint_mask_store[EINT_REG_SET_SIZE];
unsigned int eint_domain0_store[EINT_REG_SET_SIZE];
/** @}
 */

/** @ingroup type_group_scp_eint_struct
 * @brief EINT function attribute structures\n
 */
struct eint_func {
    /** EINT handler */
    eint_handler_t eint_func[EINT_MAX_CHANNEL];
    /** EINT auto-unmask */
    unsigned int eint_auto_umask[EINT_MAX_CHANNEL];
    /** EINT debounce enable */
    unsigned int is_deb_en[EINT_MAX_CHANNEL];
    /** EINT debounce time */
    unsigned int deb_time[EINT_MAX_CHANNEL];
};

/** @ingroup type_group_scp_eint_struct
 * @brief EINT configuration attribute structures\n
 */
struct eint_conf {
    /** EINT handler */
    eint_handler_t eint_func[EINT_MAX_CHANNEL];
    /** EINT auto-unmask */
    unsigned int eint_auto_umask[EINT_MAX_CHANNEL];
    /** EINT debounce enable */
    unsigned int is_deb_en[EINT_MAX_CHANNEL];
    /** EINT debounce time */
    unsigned int deb_time[EINT_MAX_CHANNEL];
    /** EINT trigger mode */
    unsigned int sens[EINT_MAX_CHANNEL];
    /** EINT polarity */
    unsigned int pol[EINT_MAX_CHANNEL];
};

unsigned int mt_eint_read_mask(unsigned int eint_num);
unsigned int mt_eint_read_sens(unsigned int eint_num);
void mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
unsigned int mt_eint_read_polarity(unsigned int eint_num);
void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
unsigned int mt_eint_read_soft(unsigned int eint_num);
void mt_eint_soft_set(unsigned int eint_num);
void mt_eint_soft_clr(unsigned int eint_num);
void mt_eint_send_pulse(unsigned int eint_num);
void mt_eint_dis_hw_debounce(unsigned int eint_num);
void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
unsigned int mt_eint_get_status(unsigned int eint_num);
unsigned int mt_eint_read_status(unsigned int eint_num);
unsigned int mt_eint_read_raw_status(unsigned int eint_num);
void mt_eint_dump_config(unsigned int eint_num);
void mt_eint_dump_all_config(void);
#endif
