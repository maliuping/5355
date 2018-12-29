/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#ifndef SOC_DRIVER_MT2721_INCLUDE_SOC_GPIO_IN_H
#define SOC_DRIVER_MT2721_INCLUDE_SOC_GPIO_IN_H

struct val_regs {
	uint16_t val;
	uint16_t _align1;
	uint16_t set;
	uint16_t _align2;
	uint16_t rst;
	uint16_t _align3[3];
};

struct  gpio_regs {
	struct val_regs dir[14];    /*0x0000 ~ 0x00DF: 14*16  bytes*/
	uint8_t   rsv00[32];      /*0x00E0 ~ 0x00FF: 2*16  bytes*/
	struct val_regs pullen[14];    /*0x0100 ~ 0x01DF: 14*16  bytes*/
	uint8_t   rsv01[32];      /*0x01E0 ~ 0x01FF: 2*16  bytes*/
	struct val_regs pullsel[14];    /*0x0200 ~ 0x02DF: 14*16  bytes*/
	uint8_t   rsv02[32];      /*0x02E0 ~ 0x02FF: 2*16  bytes*/
	struct val_regs dout[14];    /*0x0300 ~ 0x03DF: 14*16  bytes*/
	uint8_t   rsv03[32];      /*0x03E0 ~ 0x03FF: 2*16  bytes*/
	struct val_regs din[14];    /*0x0400 ~ 0x04DF: 14*16  bytes*/
	uint8_t   rsv04[32];      /*0x04E0 ~ 0x04FF: 2*16  bytes*/
	struct val_regs mode[14];    /*0x0500 ~ 0x079F: 42*16  bytes*/
	uint8_t   rsv05[240];      /*0x0780 ~ 0x086F: 15*16  bytes*/
	struct val_regs bank[1];    /*0x0870 ~ 0x087F: 1*16  bytes*/
	struct val_regs bankup[1];    /*0x0880 ~ 0x088F: 1*16  bytes*/
	struct val_regs ies[4];    /*0x0890 ~ 0x08CF: 4*16  bytes*/
	struct val_regs smt[4];    /*0x08D0 ~ 0x090F: 4*16  bytes*/
	struct val_regs tdsel[16];    /*0x0910 ~ 0x0A0F: 16*16  bytes*/
	struct val_regs rdsel[18];    /*0x0A10 ~ 0x0B2F: 18*16  bytes*/
	uint8_t   rsv06[16];      /*0xB30 ~ 0x0B3F: 1*16  bytes*/
	struct val_regs drv_mode[16]; /*0x0B40 ~ 0x0C3F: 16*16  bytes*/
	struct val_regs msdc0_crtl0[1]; /*0x0C40 ~ 0x0C4F: 1*16  bytes*/
	struct val_regs msdc0_crtl1[1]; /*0x0C50 ~ 0x0C5F: 1*16  bytes*/
	struct val_regs msdc0_crtl2[1]; /*0x0C60 ~ 0x0C6F: 1*16  bytes*/
	struct val_regs msdc0_crtl5[1]; /*0x0C70 ~ 0x0C7F: 1*16  bytes*/
	struct val_regs msdc1_crtl0[1]; /*0x0C80 ~ 0x0C8F: 1*16  bytes*/
	struct val_regs msdc1_crtl1[1]; /*0x0C90 ~ 0x0C9F: 1*16  bytes*/
	struct val_regs msdc1_crtl2[1]; /*0x0CA0 ~ 0x0CAF: 1*16  bytes*/
	struct val_regs msdc1_crtl5[1]; /*0x0CB0 ~ 0x0CBF: 1*16  bytes*/
	struct val_regs msdc1_crtl6[1]; /*0x0CC0 ~ 0x0CCF: 1*16  bytes*/
	struct val_regs msdc1_crtl7[1]; /*0x0CD0 ~ 0x0CDF: 1*16  bytes*/
	struct val_regs msdc2_crtl0[1]; /*0x0CE0 ~ 0x0CEF: 1*16  bytes*/
	struct val_regs msdc2_crtl1[1]; /*0x0CF0 ~ 0x0CFF: 1*16  bytes*/
	struct val_regs msdc2_crtl2[1]; /*0x0D00 ~ 0x0D0F: 1*16  bytes*/
	struct val_regs msdc2_crtl5[1]; /*0x0D10 ~ 0x0D1F: 1*16  bytes*/
	struct val_regs msdc2_crtl6[1]; /*0x0D20 ~ 0x0D2F: 1*16  bytes*/
	struct val_regs msdc2_crtl7[1]; /*0x0D30 ~ 0x0D3F: 1*16  bytes*/
	struct val_regs msdc3_crtl0[1]; /*0x0D40 ~ 0x0D4F: 1*16  bytes*/
	struct val_regs msdc3_crtl1[1]; /*0x0D50 ~ 0x0D5F: 1*16  bytes*/
	struct val_regs msdc3_crtl2[1]; /*0x0D60 ~ 0x0D6F: 1*16  bytes*/
	struct val_regs msdc3_crtl5[1]; /*0x0D70 ~ 0x0D7F: 1*16  bytes*/
	struct val_regs msdc3_crtl6[1]; /*0x0D80 ~ 0x0D8F: 1*16  bytes*/
	struct val_regs msdc0_crtl3[1]; /*0x0D90 ~ 0x0D9F: 1*16  bytes*/
	struct val_regs msdc0_crtl4[1]; /*0x0DA0 ~ 0x0DAF: 1*16  bytes*/
	struct val_regs msdc1_crtl3[1]; /*0x0DB0 ~ 0x0DBF: 1*16  bytes*/
	struct val_regs msdc1_crtl4[1]; /*0x0DC0 ~ 0x0DCF: 1*16  bytes*/
	struct val_regs msdc2_crtl3[1]; /*0x0DD0 ~ 0x0DDF: 1*16  bytes*/
	struct val_regs msdc2_crtl4[1]; /*0x0DE0 ~ 0x0DEF: 1*16  bytes*/
	struct val_regs msdc3_crtl3[1]; /*0x0DF0 ~ 0x0DFF: 1*16  bytes*/
	struct val_regs msdc3_crtl4[1]; /*0x0E00 ~ 0x0E0F: 1*16  bytes*/
	uint8_t   rsv07[48];      /*0xE10 ~ 0x0E3F: 3*16  bytes*/
	struct val_regs exmd_ctrl0[1]; /*0x0E40 ~ 0x0E4F: 1*16  bytes*/
	struct val_regs kctrl[3]; /*0x0E50 ~ 0x0E7F: 3*16  bytes*/
	struct val_regs hisc_ctrl[4]; /*0x0E80 ~ 0x0EBF: 4*16  bytes*/
	struct val_regs gpio_tm[1]; /*0x0EC0 ~ 0x0ECF: 1*16  bytes*/
	struct val_regs gpio_backup0_crtl[3]; /*0x0ED0 ~ 0x0EFF: 3*16  bytes*/
	struct val_regs gpio_backup1_crtl[3]; /*0x0F00 ~ 0x0F2F: 3*16  bytes*/
	struct val_regs nc_ctrl[4]; /*0x0F30 ~ 0x0F6F: 4*16  bytes*/
	struct val_regs norm_trap[1]; /*0x0F70 ~ 0x0F7F: 1*16  bytes*/
};
#endif
