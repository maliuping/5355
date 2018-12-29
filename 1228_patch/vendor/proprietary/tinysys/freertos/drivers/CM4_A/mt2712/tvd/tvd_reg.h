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

#ifndef TVD_REG_BASE_H
#define TVD_REG_BASE_H

#define REG_STA_REG11				(0x044)
#define BLANK_LV				(0x3FFU << 0)

#define REG_STA_REG13				(0x04C)
#define PY_LV					(0x3FFU << 0)

#define REG_STA_REG14				(0x050)
#define AGAIN_CODE				(0x7FU << 8)
#define FINE_AGC				BIT(2)

#define REG_STA_REG15				(0x054)
#define CP_CUR					(0x7FU << 16)

#define REG_STA_REG18				(0x060)
#define COCH_DETECTED				BIT(0)

#define REG_STA_CDET_00				(0x080)
#define CKILL					BIT(31)
#define MODE_TVD3D				(0x7U << 28)
#define BLOCK4DET				BIT(27)
#define V525_TVD3D				BIT(26)
#define IS443_TVD3D				BIT(25)
#define PHALT_TVD3D				BIT(24)
#define FH_NEG					BIT(23)
#define FH_POS					BIT(22)
#define STD_V625				BIT(21)
#define STD_V525				BIT(20)
#define NSTD_V625				BIT(19)
#define NSTD_V525				BIT(18)
#define NARRSYNC				BIT(17)
#define SB4CTG_FLAG				BIT(16)
#define PALSW_TGL_FLAG				BIT(15)
#define SVF_BSTDET_F				BIT(14)
#define VPRES_SVF				BIT(13)
#define VPRES_TVD3D				BIT(12)
#define SCF					BIT(11)
#define PH_OLD					BIT(10)
#define NA_STATE				(0x3U << 8)
#define NR_LEVEL				(0xFFU << 0)

#define TG_STA_00				(0x088)
#define HEAD_SWITHC				BIT(19)
#define TRICK					BIT(15)
#define HEAD_SWITHC				BIT(19)
#define TRICK					BIT(15)
#define VLOCK					BIT(3)
#define HSYNC_LOCK				BIT(2)
#define HLOCK					BIT(1)
#define LLOCK					BIT(0)
#define LOCK_STATUS				(0xFU << 0)

#define REG_STA_REG23				(0x08C)
#define AVG_VLEN				(0x3FFU << 0)

#define REG_STA_REG24				(0x090)
#define VCR_BV					BIT(26)

#define REG_STA_REG2A				(0x0A8)
#define ADC_CODE				(0xFFFU << 0)

#define REG_STA_REG2B				(0x0AC)
#define VAR_CVBS_CLIP				(0x3FFFFU << 0)

#define REG_STA_REG2D				(0x0B4)
#define LINE_STDFH_FLAG				BIT(29)
#define STA_LCNT				(0xFFFU << 0)

#define REG_SYS_00				(0x400)

#define TVD_INTR_EN				(0x40C)
#define TVD_INTR_STA				(0x000)
#define INTR_VSYNC_TVD				BIT(9)
#define INTR_MODE_TVD				BIT(1)
#define INTR_VPRES_TVD				BIT(0)

#define REG_VSRC_07				(0x434)
#define AAF_SEL					(0x7U << 17)
#define RG_VSRC_INV_AIDX			BIT(8)
#define fgHwTvdSVID				BIT(7)


#define REG_DEF_24				(0x488)


#define REG_DFE_01				(0x4C4)
#define BLANK_WIN_START				(0xFFU << 0)
#define DFE_BLANK_WIN_START_STD_L		(0x45)
#define DFE_BLANK_WIN_START_NSTD_L		(0x3F)

#define REG_DFE_02				(0x4C8)
#define Y4H_BW					(0x3U << 0)

#define REG_DFE_03				(0x4CC)
#define AGC2_MODE				(0xFU << 28)
#define AGC2_PK_MODE				(0xFU << 24)

#define REG_DFE_07				(0x4DC)
#define AGC2_MANUAL_ACODE			(0x7FU << 16)

#define REG_DFE_08				(0x4E0)
#define DCLAMP_Y_EN				BIT(29)

#define REG_DFE_09				(0x4E4)


#define REG_DFE_0A				(0x4E8)
#define CLAMP_TARGET_BLANK_LV			(0x3FFU << 10)

#define REG_DFE_0B				(0x4EC)


#define REG_DFE_0E				(0x4F8)
#define VPRES_EN				BIT(31)
#define VPRES_FORCE_ON				BIT(30)
#define MVPRES_TVD_EN				BIT(29)
#define MVPRES_CLAMP_EN				BIT(28)
#define MVPRES_AGC_EN				BIT(27)
#define MVPRES_BLANK_EN				BIT(26)
#define MVPRES_SYNC_EN				BIT(25)
#define MVPRES_HDET_EN				BIT(24)
#define MVPRES_VDET_EN				BIT(23)
#define MVPRES_TVD				BIT(22)
#define MVPRES_CLAMP				BIT(21)
#define MVPRES_AGC				BIT(20)
#define MVPRES_BLANK				BIT(19)
#define MVPRES_SYNC				BIT(18)
#define MVPRES_HDET				BIT(17)
#define MVPRES_VDET				BIT(16)
#define DCLAMP_UP_LIM(x)			(((x) & 0xFF) << 8)
#define DCLAMP_CHECK_LIM			BIT(7)
#define NR_DET_VPRES_SEL			BIT(6)
#define VPRES4TVD_MODE				BIT(5)
#define VPRES4PIC_MODE				BIT(4)
#define VPRES_SEL				BIT(3)
#define VPRES_MASK(x)				(((x) & 0x7) << 0)

#define REG_DFE_19				(0x524)
#define AGC_SEL					BIT(26)

#define REG_DFE_1F				(0x53C)

#define REG_CDET_00				(0x540)
#define BST_DET_ADAP				BIT(31)
#define DET443_SEL_ADAP				BIT(30)
#define PALSW_FAST_ADAP				BIT(29)
#define CTG_ADAP				BIT(28)
#define CDET_START_ADAP				BIT(27)
#define CKILL_ADAP				BIT(26)
#define PALSW_ADAP				BIT(25)
#define CAGC_ADAP				BIT(24)
#define CKILL_SEL(x)				(((x) & 0x3) << 22)
#define MODE_CKILL_EN				BIT(21)
#define SCF_SEL					BIT(20)
#define MDET_V525_SEL				BIT(19)
#define MDET_SCF_EN				BIT(18)
#define NR_OUT_SEL(x)				(((x) & 0x3) << 16)
#define DET443_SEL				BIT(15)
#define HN_DET443_EN				BIT(14)
#define NTSC443_EN				BIT(13)
#define PALM_EN					BIT(11)
#define PAL60_EN				BIT(10)
#define SECAM_EN				BIT(9)
#define INI_IS443				BIT(8)
#define INI_PHALT				BIT(7)
#define INI_V525				BIT(6)
#define L525					BIT(5)
#define MODE000					BIT(4)
#define TVD_MMODE				BIT(3)


#define REG_CDET_04				(0x550)
#define CTG_NA_SEL				BIT(29)
#define CDET_NA_SEL				BIT(28)

#define REG_TG_04				(0x590)
#define LF_OFFSET_EN				BIT(12)

#define REG_TG_08				(0x5A0)
#define VALIGN_SPEED(x)				(((x) & 0x3) << 30)
#define SKIP_VSPIKE				BIT(29)
#define FAST_VALIGN				BIT(28)
#define TGEN_DEBUG				BIT(27)
#define HVDET_FIXBLK				BIT(26)
#define FAST_VLOCK				BIT(25)
#define LLOCK_GUARD				BIT(24)
#define TKMODE_SEL				BIT(23)
#define TKMODE_THR(x)				(((x) & 0xF) << 19)
#define TVD_VCR_EN				BIT(18)
#define FDET_SEL				BIT(16)
#define VFIR_EN					BIT(15)
#define VFIR_SEL(x)				(((x) & 0x3) << 12)
#define NARRSYNC_TH(x)				(((x) & 0xF) << 8)
#define LCNT_DLY(x)				(((x) & 0xFF))

#define REG_TG_0B				(0x5AC)
#define AUTO_AVDELAY				BIT(31)
#define AUTO_MLLOCK				BIT(30)
#define AUTO_LF_OFFSET				BIT(29)
#define AUTO_FAST_KP_GAIN			BIT(28)
#define AUTO_FASTV				BIT(27)
#define AUTO_FASTV_2S				BIT(26)
#define TG_VPRES_FORCE(x)			(((x) & 0x3) << 24)
#define L525_FORCE(x)				(((x) & 0x3) << 22)
#define LOCKVLEN_FORCE(x)			(((x) & 0x3) << 20)
#define DEF_VLEN_SEL				BIT(19)
#define TVD_PATGEN_EN				BIT(18)
#define TVD_PATGEN_MODE(x)			(((x) & 0x3) << 16)
#define IIR_SLICE_LIM(x)			(((x) & 0xFF) << 8)
#define HVDET_MBLK(x)				(((x) & 0xFF) << 0)

#define REG_TG_0D				(0x5B4)
#define MXSD_FHNEG_EN				BIT(15)
#define HLEN_FHPOS_EN				BIT(14)

#define REG_CTG_00				(0x5E0)
#define SOBVLD_MASK_EN				BIT(26)
#define CTG_SWLBF				BIT(21)
#define BST_0DEG				BIT(10)

#define REG_CTG_05				(0x5F4)
#define BST_START_SEL				BIT(30)

#define CTG_07					(0x5fc)
#define UV_DELAY				(0x3U << 2)
#define Y_DELAY					(0x3U << 0)

#define COMB_CTRL_01				(0x644)
#define DRAMBASEADR				(0x1FFFFFFU << 0)

#define COMB_CTRL_02				(0x648)
#define EN3D					BIT(31)

#define COMB_CTRL_03				(0x64C)
#define FSKBACK					BIT(4)

#define COMB2D_0D				(0x748)
#define CHANNEL_B_SEL				BIT(29)
#define DRAMBASEADR_MSB				BIT(28)

#define REG_SECAM_07				(0x61C)
#define YUV_CATCH_SEL				(0xFU << 28)

#define REG_SYS_06				(0x7c4)
#define REG_SYS_07				(0x7c8)
#define REG_SYS_08				(0x7cc)
#define REG_SYS_09				(0x7D0)
#define REG_SYS_0A				(0x7D4)

#define REG_DFE_21				(0x7EC)
#define AGC_MLAGC_EN				BIT(30)

#define REG_DFE_22				(0x7F0)
#define AGC_MAGC				(0xFFU << 16)

#define fgHwTvdSVID_TDC()	\
		((TVD_READ32(REG_VSRC_07) & fgHwTvdSVID) >> 7)
#define fgHwTvdHeadSwitch()	\
		((TVD_READ32(TG_STA_00) & HEAD_SWITHC) >> 19)
#define fgHwTvdTrick()		\
		((TVD_READ32(TG_STA_00) & TRICK) >> 15)
#define fgHwTvdFineAGC()	\
		((TVD_READ32(REG_STA_REG14) & FINE_AGC) >> 2)
#define wHwTvdCPCur()		\
		((TVD_READ32(REG_STA_REG15) & CP_CUR) >> 16)
#define fgHwTvdCoChannel()	\
		(TVD_READ32(REG_STA_REG18) & COCH_DETECTED)
#define fgHwTvdCKill()		\
		((TVD_READ32(REG_STA_CDET_00) & CKILL) >> 31)
#define bHwTvdMode()		\
		((TVD_READ32(REG_STA_CDET_00) & MODE_TVD3D) >> 28)
#define fgHwTvd525()		\
		((TVD_READ32(REG_STA_CDET_00) & V525_TVD3D) >> 26)
#define fgHwTvd443()		\
		((TVD_READ32(REG_STA_CDET_00) & IS443_TVD3D) >> 25)
#define fgHwTvdFHNeg()		\
		((TVD_READ32(REG_STA_CDET_00) & FH_NEG) >> 23)
#define fgHwTvdFHPos()		\
		((TVD_READ32(REG_STA_CDET_00) & FH_POS) >> 22)
#define fgHwTvdVPresSVF()	\
		((TVD_READ32(REG_STA_CDET_00) & VPRES_SVF) >> 13)
#define fgHwTvdVPresTVD3D()	\
		((TVD_READ32(REG_STA_CDET_00) & VPRES_TVD3D) >> 12)
#define bHwTvdNAState()		\
		((TVD_READ32(REG_STA_CDET_00) & NA_STATE) >> 8)
#define bHwTvdNRLevel()		\
		((TVD_READ32(REG_STA_CDET_00) & NR_LEVEL))
#define wHwTvdAvgVLen()		\
		((TVD_READ32(REG_STA_REG23) & AVG_VLEN))
#define fgHwTvdLineSTDFH()	\
		((TVD_READ32(REG_STA_REG2D) & LINE_STDFH_FLAG) >> 29)
#define wHwTvdAvgLineCnt()	\
		((TVD_READ32(REG_STA_REG2D) & STA_LCNT))
#define TVDAbsDiff(a, b)  (((a) > (b))?((a)-(b)):((b)-(a)))
#define StatusLock()	\
		(((TVD_READ32(TG_STA_00) & LOCK_STATUS) == 0xFU) ? 1U : 0U)


#define VPRESCOUNT				(120)
#define WAIT_TVD_VSYNC_COUNT			(16)
#define LINE_NON_STD_MIN			(1695)
#define LINE_NON_STD_MAX			(1761)
#define LINE_STD_MIN				(1701)
#define LINE_STD_MAX				(1749)

#define IS_LINE_STD(line)	\
	((((line) < LINE_STD_MAX) && ((line) > LINE_STD_MIN))	\
	? true : false)

#define IS_LINE_NON_STD(line)	\
	((((line) > LINE_NON_STD_MAX) || ((line) < LINE_NON_STD_MIN))	\
	? true : false)

#define IS_FH_NON_STD()		(fgHwTvdFHPos() || fgHwTvdFHNeg())

#endif
