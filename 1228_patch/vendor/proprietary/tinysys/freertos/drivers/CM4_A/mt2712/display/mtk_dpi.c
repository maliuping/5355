
#include "main.h"
#include "FreeRTOS.h"
#include <driver_api.h>
#include <bit_op.h>
#include <stdint.h>
#include <mt_reg_base.h>
#include <panel.h>

#define false 0
#define true 1

#define DPI_BASE  DPI0_BASE
#define LVDSPLL_BASE  LVDSPLL0_BASE

#define do_div(n, base) ({					\
		uint32_t __base = (base);			\
		uint32_t __rem;					\
		__rem = ((uint64_t)(n)) % __base;		\
		(n) = ((uint64_t)(n)) / __base;			\
		__rem;						\
})

#define DPI_EN			0x00
#define EN				BIT(0)

#define DPI_RET			0x04
#define RST				BIT(0)

#define DPI_INTEN		0x08
#define INT_VSYNC_EN			BIT(0)
#define INT_VDE_EN			BIT(1)
#define INT_UNDERFLOW_EN		BIT(2)

#define DPI_INTSTA		0x0C
#define INT_VSYNC_STA			BIT(0)
#define INT_VDE_STA			BIT(1)
#define INT_UNDERFLOW_STA		BIT(2)

#define DPI_CON			0x10
#define BG_ENABLE			BIT(0)
#define IN_RB_SWAP			BIT(1)
#define INTL_EN				BIT(2)
#define TDFP_EN				BIT(3)
#define CLPF_EN				BIT(4)
#define YUV422_EN			BIT(5)
#define CSC_ENABLE			BIT(6)
#define R601_SEL			BIT(7)
#define EMBSYNC_EN			BIT(8)
#define VS_LODD_EN			BIT(16)
#define VS_LEVEN_EN			BIT(17)
#define VS_RODD_EN			BIT(18)
#define VS_REVEN			BIT(19)
#define FAKE_DE_LODD			BIT(20)
#define FAKE_DE_LEVEN			BIT(21)
#define FAKE_DE_RODD			BIT(22)
#define FAKE_DE_REVEN			BIT(23)

#define DPI_OUTPUT_SETTING	0x14
#define CH_SWAP				0
#define CH_SWAP_MASK			(0x7 << 0)
#define SWAP_RGB			0x00
#define SWAP_GBR			0x01
#define SWAP_BRG			0x02
#define SWAP_RBG			0x03
#define SWAP_GRB			0x04
#define SWAP_BGR			0x05
#define BIT_SWAP			BIT(3)
#define B_MASK				BIT(4)
#define G_MASK				BIT(5)
#define R_MASK				BIT(6)
#define DE_MASK				BIT(8)
#define HS_MASK				BIT(9)
#define VS_MASK				BIT(10)
#define DE_POL				BIT(12)
#define HSYNC_POL			BIT(13)
#define VSYNC_POL			BIT(14)
#define CK_POL				BIT(15)
#define OEN_OFF				BIT(16)
#define EDGE_SEL			BIT(17)
#define OUT_BIT				18
#define OUT_BIT_MASK			(0x3 << 18)
#define OUT_BIT_8			0x00
#define OUT_BIT_10			0x01
#define OUT_BIT_12			0x02
#define OUT_BIT_16			0x03
#define YC_MAP				20
#define YC_MAP_MASK			(0x7 << 20)
#define YC_MAP_RGB			0x00
#define YC_MAP_CYCY			0x04
#define YC_MAP_YCYC			0x05
#define YC_MAP_CY			0x06
#define YC_MAP_YC			0x07

#define DPI_SIZE		0x18
#define HSIZE				0
#define HSIZE_MASK			(0x1FFF << 0)
#define VSIZE				16
#define VSIZE_MASK			(0x1FFF << 16)

#define DPI_DDR_SETTING		0x1C
#define DDR_EN				BIT(0)
#define DDDR_SEL			BIT(1)
#define DDR_4PHASE			BIT(2)
#define DDR_WIDTH			(0x3 << 4)
#define DDR_PAD_MODE			(0x1 << 8)

#define DPI_TGEN_HWIDTH		0x20
#define HPW				0
#define HPW_MASK			(0xFFF << 0)

#define DPI_TGEN_HPORCH		0x24
#define HBP				0
#define HBP_MASK			(0xFFF << 0)
#define HFP				16
#define HFP_MASK			(0xFFF << 16)

#define DPI_TGEN_VWIDTH		0x28
#define DPI_TGEN_VPORCH		0x2C

#define VSYNC_WIDTH_SHIFT		0
#define VSYNC_WIDTH_MASK		(0xFFF << 0)
#define VSYNC_HALF_LINE_SHIFT		16
#define VSYNC_HALF_LINE_MASK		BIT(16)
#define VSYNC_BACK_PORCH_SHIFT		0
#define VSYNC_BACK_PORCH_MASK		(0xFFF << 0)
#define VSYNC_FRONT_PORCH_SHIFT		16
#define VSYNC_FRONT_PORCH_MASK		(0xFFF << 16)

#define DPI_BG_HCNTL		0x30
#define BG_RIGHT			(0x1FFF << 0)
#define BG_LEFT				(0x1FFF << 16)

#define DPI_BG_VCNTL		0x34
#define BG_BOT				(0x1FFF << 0)
#define BG_TOP				(0x1FFF << 16)

#define DPI_BG_COLOR		0x38
#define BG_B				(0xF << 0)
#define BG_G				(0xF << 8)
#define BG_R				(0xF << 16)

#define DPI_FIFO_CTL		0x3C
#define FIFO_VALID_SET			(0x1F << 0)
#define FIFO_RST_SEL			(0x1 << 8)

#define DPI_STATUS		0x40
#define VCOUNTER			(0x1FFF << 0)
#define DPI_BUSY			BIT(16)
#define OUTEN				BIT(17)
#define FIELD				BIT(20)
#define TDLR				BIT(21)

#define DPI_TMODE		0x44
#define DPI_OEN_ON			BIT(0)

#define DPI_CHECKSUM		0x48
#define DPI_CHECKSUM_MASK		(0xFFFFFF << 0)
#define DPI_CHECKSUM_READY		BIT(30)
#define DPI_CHECKSUM_EN			BIT(31)

#define DPI_DUMMY		0x50
#define DPI_DUMMY_MASK			(0xFFFFFFFF << 0)

#define DPI_TGEN_VWIDTH_LEVEN	0x68
#define DPI_TGEN_VPORCH_LEVEN	0x6C
#define DPI_TGEN_VWIDTH_RODD	0x70
#define DPI_TGEN_VPORCH_RODD	0x74
#define DPI_TGEN_VWIDTH_REVEN	0x78
#define DPI_TGEN_VPORCH_REVEN	0x7C

#define DPI_ESAV_VTIMING_LODD	0x80
#define ESAV_VOFST_LODD			(0xFFF << 0)
#define ESAV_VWID_LODD			(0xFFF << 16)

#define DPI_ESAV_VTIMING_LEVEN	0x84
#define ESAV_VOFST_LEVEN		(0xFFF << 0)
#define ESAV_VWID_LEVEN			(0xFFF << 16)

#define DPI_ESAV_VTIMING_RODD	0x88
#define ESAV_VOFST_RODD			(0xFFF << 0)
#define ESAV_VWID_RODD			(0xFFF << 16)

#define DPI_ESAV_VTIMING_REVEN	0x8C
#define ESAV_VOFST_REVEN		(0xFFF << 0)
#define ESAV_VWID_REVEN			(0xFFF << 16)

#define DPI_ESAV_FTIMING	0x90
#define ESAV_FOFST_ODD			(0xFFF << 0)
#define ESAV_FOFST_EVEN			(0xFFF << 16)

#define DPI_CLPF_SETTING	0x94
#define CLPF_TYPE			(0x3 << 0)
#define ROUND_EN			BIT(4)

#define DPI_Y_LIMIT		0x98
#define Y_LIMINT_BOT			0
#define Y_LIMINT_BOT_MASK		(0xFFF << 0)
#define Y_LIMINT_TOP			16
#define Y_LIMINT_TOP_MASK		(0xFFF << 16)

#define DPI_C_LIMIT		0x9C
#define C_LIMIT_BOT			0
#define C_LIMIT_BOT_MASK		(0xFFF << 0)
#define C_LIMIT_TOP			16
#define C_LIMIT_TOP_MASK		(0xFFF << 16)

#define DPI_YUV422_SETTING	0xA0
#define UV_SWAP				BIT(0)
#define CR_DELSEL			BIT(4)
#define CB_DELSEL			BIT(5)
#define Y_DELSEL			BIT(6)
#define DE_DELSEL			BIT(7)

#define DPI_EMBSYNC_SETTING	0xA4
#define EMBSYNC_R_CR_EN			BIT(0)
#define EMPSYNC_G_Y_EN			BIT(1)
#define EMPSYNC_B_CB_EN			BIT(2)
#define ESAV_F_INV			BIT(4)
#define ESAV_V_INV			BIT(5)
#define ESAV_H_INV			BIT(6)
#define ESAV_CODE_MAN			BIT(8)
#define VS_OUT_SEL			(0x7 << 12)

#define DPI_ESAV_CODE_SET0	0xA8
#define ESAV_CODE0			(0xFFF << 0)
#define ESAV_CODE1			(0xFFF << 16)

#define DPI_ESAV_CODE_SET1	0xAC
#define ESAV_CODE2			(0xFFF << 0)
#define ESAV_CODE3_MSB			BIT(16)

#define DPI_H_FRE_CON		0xE0
#define H_FRE_2N			BIT(25)


enum mtk_dpi_out_bit_num {
	MTK_DPI_OUT_BIT_NUM_8BITS,
	MTK_DPI_OUT_BIT_NUM_10BITS,
	MTK_DPI_OUT_BIT_NUM_12BITS,
	MTK_DPI_OUT_BIT_NUM_16BITS
};

enum mtk_dpi_out_yc_map {
	MTK_DPI_OUT_YC_MAP_RGB,
	MTK_DPI_OUT_YC_MAP_CYCY,
	MTK_DPI_OUT_YC_MAP_YCYC,
	MTK_DPI_OUT_YC_MAP_CY,
	MTK_DPI_OUT_YC_MAP_YC
};

enum mtk_dpi_out_channel_swap {
	MTK_DPI_OUT_CHANNEL_SWAP_RGB,
	MTK_DPI_OUT_CHANNEL_SWAP_GBR,
	MTK_DPI_OUT_CHANNEL_SWAP_BRG,
	MTK_DPI_OUT_CHANNEL_SWAP_RBG,
	MTK_DPI_OUT_CHANNEL_SWAP_GRB,
	MTK_DPI_OUT_CHANNEL_SWAP_BGR
};

enum mtk_dpi_out_color_format {
	MTK_DPI_COLOR_FORMAT_RGB,
	MTK_DPI_COLOR_FORMAT_RGB_FULL,
	MTK_DPI_COLOR_FORMAT_YCBCR_444,
	MTK_DPI_COLOR_FORMAT_YCBCR_422,
	MTK_DPI_COLOR_FORMAT_XV_YCC,
	MTK_DPI_COLOR_FORMAT_YCBCR_444_FULL,
	MTK_DPI_COLOR_FORMAT_YCBCR_422_FULL
};

enum mtk_dpi_polarity {
	MTK_DPI_POLARITY_RISING,
	MTK_DPI_POLARITY_FALLING,
};

struct mtk_dpi_polarities {
	enum mtk_dpi_polarity de_pol;
	enum mtk_dpi_polarity ck_pol;
	enum mtk_dpi_polarity hsync_pol;
	enum mtk_dpi_polarity vsync_pol;
};

struct mtk_dpi_sync_param {
	uint32_t sync_width;
	uint32_t front_porch;
	uint32_t back_porch;
	uint32_t shift_half_line;
};

struct mtk_dpi_yc_limit {
	uint16_t y_top;
	uint16_t y_bottom;
	uint16_t c_top;
	uint16_t c_bottom;
};

void mtk_dpi_mask(uint32_t offset, uint32_t val, uint32_t mask)
{
	uint32_t tmp = readl(DPI_BASE + offset) & ~mask;

	tmp |= (val & mask);
	writel(tmp, DPI_BASE + offset);
}

void mtk_dpi_sw_reset(uint32_t reset)
{
	mtk_dpi_mask(DPI_RET, reset ? RST : 0, RST);
}

void mtk_dpi_config_hsync(struct mtk_dpi_sync_param *sync)
{
	mtk_dpi_mask(DPI_TGEN_HWIDTH,
		     sync->sync_width << HPW, HPW_MASK);
	mtk_dpi_mask(DPI_TGEN_HPORCH,
		     sync->back_porch << HBP, HBP_MASK);
	mtk_dpi_mask(DPI_TGEN_HPORCH, sync->front_porch << HFP,
		     HFP_MASK);
}

void mtk_dpi_config_vsync(struct mtk_dpi_sync_param *sync,
				 uint32_t width_addr, uint32_t porch_addr)
{
	mtk_dpi_mask(width_addr,
		     sync->sync_width << VSYNC_WIDTH_SHIFT,
		     VSYNC_WIDTH_MASK);
	mtk_dpi_mask(width_addr,
		     sync->shift_half_line << VSYNC_HALF_LINE_SHIFT,
		     VSYNC_HALF_LINE_MASK);
	mtk_dpi_mask(porch_addr,
		     sync->back_porch << VSYNC_BACK_PORCH_SHIFT,
		     VSYNC_BACK_PORCH_MASK);
	mtk_dpi_mask(porch_addr,
		     sync->front_porch << VSYNC_FRONT_PORCH_SHIFT,
		     VSYNC_FRONT_PORCH_MASK);
}

void mtk_dpi_config_vsync_lodd(struct mtk_dpi_sync_param *sync)
{
	mtk_dpi_config_vsync(sync, DPI_TGEN_VWIDTH, DPI_TGEN_VPORCH);
}

void mtk_dpi_config_vsync_leven(struct mtk_dpi_sync_param *sync)
{
	mtk_dpi_config_vsync(sync, DPI_TGEN_VWIDTH_LEVEN,
			     DPI_TGEN_VPORCH_LEVEN);
}

void mtk_dpi_config_vsync_rodd(struct mtk_dpi_sync_param *sync)
{
	mtk_dpi_config_vsync(sync, DPI_TGEN_VWIDTH_RODD,
			     DPI_TGEN_VPORCH_RODD);
}

void mtk_dpi_config_vsync_reven(struct mtk_dpi_sync_param *sync)
{
	mtk_dpi_config_vsync(sync, DPI_TGEN_VWIDTH_REVEN,
			     DPI_TGEN_VPORCH_REVEN);
}

void mtk_dpi_config_pol(struct mtk_dpi_polarities *dpi_pol)
{
	uint32_t pol;

	pol = (dpi_pol->ck_pol == MTK_DPI_POLARITY_RISING ? 0 : CK_POL) |
	      (dpi_pol->de_pol == MTK_DPI_POLARITY_RISING ? 0 : DE_POL) |
	      (dpi_pol->hsync_pol == MTK_DPI_POLARITY_RISING ? 0 : HSYNC_POL) |
	      (dpi_pol->vsync_pol == MTK_DPI_POLARITY_RISING ? 0 : VSYNC_POL);
	mtk_dpi_mask(DPI_OUTPUT_SETTING, pol,
		     CK_POL | DE_POL | HSYNC_POL | VSYNC_POL);
}

void mtk_dpi_config_3d(uint32_t en_3d)
{
	mtk_dpi_mask(DPI_CON, en_3d ? TDFP_EN : 0, TDFP_EN);
}

void mtk_dpi_config_interface(uint32_t inter)
{
	mtk_dpi_mask(DPI_CON, inter ? INTL_EN : 0, INTL_EN);
}

void mtk_dpi_config_fb_size(uint32_t width, uint32_t height)
{
	mtk_dpi_mask(DPI_SIZE, width << HSIZE, HSIZE_MASK);
	mtk_dpi_mask(DPI_SIZE, height << VSIZE, VSIZE_MASK);
}

void mtk_dpi_config_channel_limit(struct mtk_dpi_yc_limit *limit)
{
	mtk_dpi_mask(DPI_Y_LIMIT, limit->y_bottom << Y_LIMINT_BOT,
		     Y_LIMINT_BOT_MASK);
	mtk_dpi_mask(DPI_Y_LIMIT, limit->y_top << Y_LIMINT_TOP,
		     Y_LIMINT_TOP_MASK);
	mtk_dpi_mask(DPI_C_LIMIT, limit->c_bottom << C_LIMIT_BOT,
		     C_LIMIT_BOT_MASK);
	mtk_dpi_mask(DPI_C_LIMIT, limit->c_top << C_LIMIT_TOP,
		     C_LIMIT_TOP_MASK);
}

void mtk_dpi_config_bit_num(enum mtk_dpi_out_bit_num num)
{
	uint32_t val;

	switch (num) {
	case MTK_DPI_OUT_BIT_NUM_8BITS:
		val = OUT_BIT_8;
		break;
	case MTK_DPI_OUT_BIT_NUM_10BITS:
		val = OUT_BIT_10;
		break;
	case MTK_DPI_OUT_BIT_NUM_12BITS:
		val = OUT_BIT_12;
		break;
	case MTK_DPI_OUT_BIT_NUM_16BITS:
		val = OUT_BIT_16;
		break;
	default:
		val = OUT_BIT_8;
		break;
	}
	mtk_dpi_mask(DPI_OUTPUT_SETTING, val << OUT_BIT,
		     OUT_BIT_MASK);
}

void mtk_dpi_config_yc_map(enum mtk_dpi_out_yc_map map)
{
	uint32_t val;

	switch (map) {
	case MTK_DPI_OUT_YC_MAP_RGB:
		val = YC_MAP_RGB;
		break;
	case MTK_DPI_OUT_YC_MAP_CYCY:
		val = YC_MAP_CYCY;
		break;
	case MTK_DPI_OUT_YC_MAP_YCYC:
		val = YC_MAP_YCYC;
		break;
	case MTK_DPI_OUT_YC_MAP_CY:
		val = YC_MAP_CY;
		break;
	case MTK_DPI_OUT_YC_MAP_YC:
		val = YC_MAP_YC;
		break;
	default:
		val = YC_MAP_RGB;
		break;
	}

	mtk_dpi_mask(DPI_OUTPUT_SETTING, val << YC_MAP, YC_MAP_MASK);
}

void mtk_dpi_config_channel_swap(enum mtk_dpi_out_channel_swap swap)
{
	uint32_t val;

	switch (swap) {
	case MTK_DPI_OUT_CHANNEL_SWAP_RGB:
		val = SWAP_RGB;
		break;
	case MTK_DPI_OUT_CHANNEL_SWAP_GBR:
		val = SWAP_GBR;
		break;
	case MTK_DPI_OUT_CHANNEL_SWAP_BRG:
		val = SWAP_BRG;
		break;
	case MTK_DPI_OUT_CHANNEL_SWAP_RBG:
		val = SWAP_RBG;
		break;
	case MTK_DPI_OUT_CHANNEL_SWAP_GRB:
		val = SWAP_GRB;
		break;
	case MTK_DPI_OUT_CHANNEL_SWAP_BGR:
		val = SWAP_BGR;
		break;
	default:
		val = SWAP_RGB;
		break;
	}

	mtk_dpi_mask(DPI_OUTPUT_SETTING, val << CH_SWAP, CH_SWAP_MASK);
}

void mtk_dpi_config_yuv422_enable(uint32_t enable)
{
	mtk_dpi_mask(DPI_CON, enable ? YUV422_EN : 0, YUV422_EN);
}

void mtk_dpi_config_csc_enable(uint32_t enable)
{
	mtk_dpi_mask(DPI_CON, enable ? CSC_ENABLE : 0, CSC_ENABLE);
}

void mtk_dpi_config_swap_input(uint32_t enable)
{
	mtk_dpi_mask(DPI_CON, enable ? IN_RB_SWAP : 0, IN_RB_SWAP);
}

void mtk_dpi_config_2n_h_fre(void)
{
	mtk_dpi_mask(DPI_H_FRE_CON, H_FRE_2N, H_FRE_2N);
}

void mtk_dpi_config_color_format(enum mtk_dpi_out_color_format format)
{
	if ((format == MTK_DPI_COLOR_FORMAT_YCBCR_444) ||
	    (format == MTK_DPI_COLOR_FORMAT_YCBCR_444_FULL)) {
		mtk_dpi_config_yuv422_enable(false);
		mtk_dpi_config_csc_enable(true);
		mtk_dpi_config_swap_input(false);
		mtk_dpi_config_channel_swap(MTK_DPI_OUT_CHANNEL_SWAP_BGR);
	} else if ((format == MTK_DPI_COLOR_FORMAT_YCBCR_422) ||
		   (format == MTK_DPI_COLOR_FORMAT_YCBCR_422_FULL)) {
		mtk_dpi_config_yuv422_enable(true);
		mtk_dpi_config_csc_enable(true);
		mtk_dpi_config_swap_input(true);
		mtk_dpi_config_channel_swap(MTK_DPI_OUT_CHANNEL_SWAP_RGB);
	} else {
		mtk_dpi_config_yuv422_enable(false);
		mtk_dpi_config_csc_enable(false);
		mtk_dpi_config_swap_input(false);
		mtk_dpi_config_channel_swap(MTK_DPI_OUT_CHANNEL_SWAP_RGB);
	}
}

static uint32_t mt2712_calculate_factor(uint32_t clock)
{
	if (clock <= 16000)
		return 8;
	else if (clock <= 32000)
		return 4;
	else if (clock <= 63000)
		return 2;
	else
		return 1;
}

int32_t mtk_dpi_enable(struct display_mode_info *mode)
{
	struct mtk_dpi_yc_limit limit;
	struct mtk_dpi_polarities dpi_pol;
	struct mtk_dpi_sync_param hsync;
	struct mtk_dpi_sync_param vsync_lodd = { 0 };
	struct mtk_dpi_sync_param vsync_leven = { 0 };
	struct mtk_dpi_sync_param vsync_rodd = { 0 };
	struct mtk_dpi_sync_param vsync_reven = { 0 };
	uint64_t pix_rate;
	uint64_t pll_rate;
	uint32_t factor;
	uint64_t _pcw;
	uint32_t val;
	uint32_t pcw, postdiv;

	pix_rate = 1000UL * mode->clock;
	factor = mt2712_calculate_factor(mode->clock);
	pll_rate = pix_rate * factor;

	PRINTF_D("Want PLL %lu Hz, pixel clock %lu Hz\n", pll_rate, pix_rate);


	for (val = 0; val < 5; val++) {
		postdiv = 1 << val;
		if ((uint64_t)pll_rate * postdiv >= 1000000000)
			break;
	}

	/* _pcw = freq * postdiv / fin * 2^pcwfbits */
	_pcw = ((uint64_t)pll_rate << val) << 24;
	do_div(_pcw, 26000000);

	pcw = (uint32_t)_pcw;

	writel(pcw, LVDSPLL_BASE + 4);
	writel((readl(LVDSPLL_BASE) & 0xffffff00) | (val << 4), LVDSPLL_BASE); /* config postdiv and disable pll */
	writel(readl(LVDSPLL_BASE) | 1, LVDSPLL_BASE); /* enable pll */

	PRINTF_D("Got  PLL %lu Hz, pixel clock %lu Hz\n",
		pll_rate, pix_rate);

	limit.c_bottom = 0x0000;
	limit.c_top = 0x0fff;
	limit.y_bottom = 0x0000;
	limit.y_top = 0x0fff;

	dpi_pol.ck_pol = MTK_DPI_POLARITY_FALLING;
	dpi_pol.de_pol = MTK_DPI_POLARITY_RISING;
	dpi_pol.hsync_pol = mode->flags & DRM_MODE_FLAG_PHSYNC ?
			    MTK_DPI_POLARITY_FALLING : MTK_DPI_POLARITY_RISING;
	dpi_pol.vsync_pol = mode->flags & DRM_MODE_FLAG_PVSYNC ?
			    MTK_DPI_POLARITY_FALLING : MTK_DPI_POLARITY_RISING;

	hsync.sync_width = mode->hsync_end - mode->hsync_start;
	hsync.back_porch = mode->htotal - mode->hsync_end;
	hsync.front_porch = mode->hsync_start - mode->hdisplay;
	hsync.shift_half_line = false;

	vsync_lodd.sync_width = mode->vsync_end - mode->vsync_start;
	vsync_lodd.back_porch = mode->vtotal - mode->vsync_end;
	vsync_lodd.front_porch = mode->vsync_start - mode->vdisplay;
	vsync_lodd.shift_half_line = false;

	if (mode->flags & DRM_MODE_FLAG_INTERLACE &&
	    mode->flags & DRM_MODE_FLAG_3D_MASK) {
		vsync_leven = vsync_lodd;
		vsync_rodd = vsync_lodd;
		vsync_reven = vsync_lodd;
		vsync_leven.shift_half_line = true;
		vsync_reven.shift_half_line = true;
	} else if (mode->flags & DRM_MODE_FLAG_INTERLACE &&
		   !(mode->flags & DRM_MODE_FLAG_3D_MASK)) {
		vsync_leven = vsync_lodd;
		vsync_leven.shift_half_line = true;
	} else if (!(mode->flags & DRM_MODE_FLAG_INTERLACE) &&
		   mode->flags & DRM_MODE_FLAG_3D_MASK) {
		vsync_rodd = vsync_lodd;
	}

	mtk_dpi_mask(DPI_EN, EN, EN);
	mtk_dpi_sw_reset(true);
	mtk_dpi_config_pol(&dpi_pol);

	mtk_dpi_config_hsync(&hsync);
	mtk_dpi_config_vsync_lodd(&vsync_lodd);
	mtk_dpi_config_vsync_rodd(&vsync_rodd);
	mtk_dpi_config_vsync_leven(&vsync_leven);
	mtk_dpi_config_vsync_reven(&vsync_reven);

	mtk_dpi_config_3d(false);
	mtk_dpi_config_interface(false);
	mtk_dpi_config_fb_size(mode->hdisplay, mode->vdisplay);

	mtk_dpi_config_channel_limit(&limit);
	mtk_dpi_config_bit_num(MTK_DPI_OUT_BIT_NUM_8BITS);
	mtk_dpi_config_channel_swap(MTK_DPI_OUT_CHANNEL_SWAP_RGB);
	mtk_dpi_config_yc_map(MTK_DPI_OUT_YC_MAP_RGB);
	mtk_dpi_config_color_format(MTK_DPI_COLOR_FORMAT_RGB);
	mtk_dpi_config_2n_h_fre();
	mtk_dpi_sw_reset(false);

	return 0;
}

void mtk_dpi_disable(struct display_mode_info *mode)
{
	mtk_dpi_mask(DPI_EN, 0, EN);
}

