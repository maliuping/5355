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

#ifndef DRIVER_API_H
#define DRIVER_API_H

#include <stdint.h>

/* low level macros for accessing memory mapped hardware registers */
#define REG64(addr) ((volatile uint64_t *)(uintptr_t)(addr))
#define REG32(addr) ((volatile uint32_t *)(uintptr_t)(addr))
#define REG16(addr) ((volatile uint16_t *)(uintptr_t)(addr))
#define REG8(addr) ((volatile uint8_t *)(uintptr_t)(addr))

#define RMWREG64(addr, startbit, width, val) *REG64(addr) = (*REG64(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG32(addr, startbit, width, val) *REG32(addr) = (*REG32(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG16(addr, startbit, width, val) *REG16(addr) = (*REG16(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG8(addr, startbit, width, val) *REG8(addr) = (*REG8(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))

#define writel(v, a) (*REG32(a) = (v))
#define readl(a) (*REG32(a))
#define writeb(v, a) (*REG8(a) = (v))
#define readb(a) (*REG8(a))

/// I/O    ////////////////////////////////////////////////////////////////////
#ifndef DRV_WriteReg32
#define DRV_WriteReg32(addr,data)   writel(data,addr)
#endif

#ifndef DRV_Reg32
#define DRV_Reg32(addr)             readl(addr)
#endif
#define DRV_WriteReg8(addr,data)    writeb(data, addr)
#define DRV_Reg8(addr)              readb(addr)
#define DRV_SetReg32(addr, data)    ((*(volatile uint32_t *)(uintptr_t)(addr)) |= (uint32_t)(data))
#define DRV_ClrReg32(addr, data)    ((*(volatile uint32_t *)(uintptr_t)(addr)) &= ~((uint32_t)(data)))
#define DRV_SetReg8(addr, data)    ((*(volatile uint8_t *)(uintptr_t)(addr)) |= (uint8_t)(data))
#define DRV_ClrReg8(addr, data)    ((*(volatile uint8_t *)(uintptr_t)(addr)) &= ~((uint8_t)(data)))

// lowcase version
#define outreg32(a,b)   (*(volatile uint32_t *)(uintptr_t)(a) = (uint32_t)(b))
#define inreg32(a)      (*(volatile uint32_t *)(uintptr_t)(a))

#ifndef drv_write_reg32
#define drv_write_reg32(addr,data)   ((*(volatile uint32_t *)(uintptr_t)(addr)) = (uint32_t)(data))
#endif
#ifndef drv_reg32
#define drv_reg32(addr)             (*(volatile uint32_t *)(uintptr_t)(addr))
#endif
#define drv_write_reg8(addr,data)    ((*(volatile uint8_t *)(uintptr_t)(addr)) = (uint8_t)(data))
#define drv_reg8(addr)              (*(volatile uint8_t *)(uintptr_t)(addr))
#define drv_set_reg32(addr, data)    ((*(volatile uint32_t *)(uintptr_t)(addr)) |= (uint32_t)(data))
#define drv_clr_reg32(addr, data)    ((*(volatile uint32_t *)(uintptr_t)(addr)) &= ~((uint32_t)(data)))
#define drv_set_reg8(addr, data)    ((*(volatile uint8_t *)(uintptr_t)(addr)) |= (uint8_t)(data))
#define drv_clr_reg8(addr, data)    ((*(volatile uint8_t *)(uintptr_t)(addr)) &= ~((uint8_t)(data)))

#ifndef drv_write_reg16
#define drv_write_reg16(addr,data)    ((*(volatile uint16_t *)(uintptr_t)(addr)) = (uint16_t)(data))
#endif

#ifndef drv_reg16
#define drv_reg16(addr)              (*(volatile unsigned short *)(uintptr_t)(addr))
#endif

#endif
