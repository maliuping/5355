/*
 * Copyright (c) 2018 MediaTek Inc.
 * Author: Qing Li <qing.li@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef TVD_CVBS_REG_BASE_H
#define TVD_CVBS_REG_BASE_H

#define CVBSPLL_CON0			(0x310)
#define CVBSPLL_EN			BIT(31)

#define CVBSPLL_CON1			(0x314)
#define CVBSPLL_DIV2_EN			BIT(0)

#define CVBSREFPLL_CON0			(0x318)
#define CVBSREFPLL_EN			BIT(31)

#define CVBSREFPLL_CON1			(0x31c)
#define CVBSREFPLL_DIV26_EN		BIT(5)

#define PA_PLL_CON0			(0x900)
#define  RG_CLKSQ_EN			BIT(0)

#define CVBS_CON0			(0x930)
#define  RG_AISEL			(0xffU << 24)
#define  RG_BLANK_EN			BIT(9)
#define  RG_CHA_MIDDLE_EN		BIT(8)

#define CVBS_CON1			(0x934)
#define  RG_CVBS_PWD			BIT(9)
#define  RG_CLAMP_PWD			BIT(8)

#define CVBS_CON2			(0x938)
#define  RG_INMUX_PWD			BIT(30)
#define  RG_PROT_PWD			BIT(29)
#define  RG_IGEN_PWD			BIT(28)
#define  RG_PGABUFNA_PWD		BIT(26)
#define  RG_CORE_PWD			BIT(27)
#define  RG_SHIFTA_PWD			BIT(24)
#define  RG_CVBSADC1_IO_PWD		BIT(6)
#define  RG_CVBSADC1_VREF_SEL		(0x3U << 0)
#define  RG_OFFCURA			(0x7U << 20)

#define CVBS_CON3			(0x93c)
#define  RG_CVBSADC1_CK_PWD		BIT(26)
#define  RG_CVBSADC1_CORE_PWD		BIT(25)
#define  RG_CVBSADC1_DC_EN		BIT(28)
#define CVBS_CON6			(0x600)
#define RG_BGR_PI_PWD			BIT(5)

#endif
