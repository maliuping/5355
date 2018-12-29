#include "main.h"
#include "FreeRTOS.h"
#include <driver_api.h>
#include <stdint.h>
#include <mt_reg_base.h>
#include <panel.h>

#define LVDS_BASE	LVDS0_BASE
#define LVDATX0		LVDSOTX0_BASE
#define LVDATX1		LVDSETX0_BASE

/* lvds ana reg define*/
#define LVDSTX_CTL1	0x00
#define LVDSTX_CTL2	0x04
#define RG_LVDSTX_TVO	(0xf << 0)
#define RG_LVDSTX_TVCM	(0xf << 4)
#define RG_LVDSTX_TSTCLK_SEL	(0x3 << 8)
#define RG_LVDSTX_TSTCLKDIV_EN	BIT(10)
#define RG_LVDSTX_TSTCLK_EN	BIT(11)
#define RG_LVDSTX_TSTCLKDIV_SEL	(3U << 12)
#define RG_LVDSTX_MPX_SEL	(3U << 14)
#define RG_LVDSTX_BIAS_SEL	(3U << 16)
#define RG_LVDSTX_R_TERM	(3U << 18)
#define RG_LVDSTX_SEL_CKTST	BIT(20)
#define RG_LVDSTX_SEL_MERGE	BIT(21)
#define RG_LVDSTX_LDO_EN	BIT(22)
#define RG_LVDSTX_BIAS_EN	BIT(23)
#define RG_LVDSTX_SER_ABIST_EN	BIT(24)
#define RG_LVDSTX_SER_ABEDG_EN	BIT(25)
#define RG_LVDSTX_SER_BIST_TOG	BIT(26)

#define LVDSTX_CTL3	0x08
#define RG_LVDSTX_VOUTABIST_EN	(0x1f << 0)
#define RG_LVDSTX_EXT_EN	(0x1f << 5)
#define RG_LVDSTX_DRV_EN	(0x1f << 10)
#define RG_LVDSTX_SER_DIN_SEL	BIT(16)
#define RG_LVDSTX_SER_CLKDIG_INV	BIT(17)

#define LVDSTX_CTL4	0x0c
#define RG_LVDSTX_TSTPAD_EN	BIT(20)
#define RG_LVDSTX_ABIST_EN	BIT(21)
#define RG_LVDSTX_MPX_EN	BIT(22)
#define RG_LVDSTX_LDOLPF_EN	BIT(23)
#define RG_LVDSTX_TEST_BYPASSBUF	BIT(24)
#define RG_LVDSTX_BIASLPF_EN	BIT(25)
#define RG_LVDSTX_SER_ABMUX_SEL	(7U << 26)
#define RG_LVDSTX_SER_PEM_EN	BIT(29)
#define RG_LVDSTX_LVROD	(3U << 30)

#define LVDSTX_CTL5	0x10
#define RG_LVDSTX_MIPICK_SEL	BIT(4)
#define RG_LVDSTX_INCK_SEL	BIT(5)
#define RG_LVDSTX_SWITCH_EN	BIT(6)

#define VOPLL_CTL1		0x14
#define RG_VPLL_TXMUXDIV2_EN	BIT(0)
#define RG_VPLL_FBKSEL	(3U << 6)
#define RG_VPLL_FBKDIV	(0x7f << 12)

#define VOPLL_CTL2		0x18
#define RG_VPLL_EN	BIT(7)
#define RG_VPLL_TXDIV1	(3U << 8)
#define RG_VPLL_TXDIV2	(3U << 10)
#define RG_VPLL_LVDS_EN	BIT(12)
#define RG_VPLL_LVDS_DPIX_DIV2	BIT(13)
#define RG_VPLL_TTLDIV	(3U << 16)
#define RG_VPLL_TXDIV5_EN	BIT(21)
#define RG_VPLL_BIAS_EN	BIT(24)
#define RG_VPLL_BIASLPF_EN	BIT(25)

#define VOPLL_CTL3		0x1c
#define LVDS_ISO_EN		BIT(8)
#define DA_LVDSTX_PWR_ON	BIT(9)
/* lvds ana reg define end*/

/* LVDS TOP */
#define LVDSTOP_REG00	0x000
#define LVDSTOP_REG01	0x004
#define LVDSTOP_REG02	0x008
#define LVDSTOP_REG03	0x00c
#define LVDSTOP_REG04	0x010
#define LVDSTOP_REG05	0x014
#define RG_LVDS_CLKDIV_CTRL	(0xf << 23)
#define RG_FIFO_CTRL	(0x3 << 20)
#define RG_FIFO_EN	(3 << 16)

/* PATTERN GEN */
#define PATGEN_REG00	0x604
#define PATGEN_REG01	0x608
#define PATGEN_REG02	0x60c
#define PATGEN_REG03	0x610
#define PATGEN_REG04	0x614
#define PATGEN_REG05	0x618
#define PATGEN_REG06	0x620

#define CFG_REG00	0x700
#define DETECT_REG0	0x704
#define DETECT_REG1	0x708
#define DETECT_REG2	0x70c
#define TD_CTRL_MON	0x714
#define CRC_CHECK_REG0	0x738
#define CRC_CHECK_REG1	0x73c
#define CRC_CHECK_REG2	0x740
#define DETECT_REG3	0x750
#define DETECT_REG4	0x754

#define MODE0	0x800
#define LVDS_CTRL	0x814
#define ANA_TEST	0x824

#define LVDS_CTRL00	0xa00
#define RG_NS_VESA_EN	BIT(1)
#define RG_DUAL			BIT(12)

#define LVDS_CTRL01	0xa04
#define LVDS_CTRL02	0xa08
#define RG_DPMODE	BIT(12)
#define RG_LVDS_74FIFO_EN	BIT(13)

#define CRC0	0xa10
#define CRC1	0xa14
#define CRC2	0xa18
#define CRC3	0xa1c
#define LVDS_TEST01	0xa30
#define LVDS_TEST02	0xa34
#define CRC4	0xa38
#define CRC5	0xa3c
#define CRC6	0xa40

#define LVDSTX_REG00	0xa80

#define LLV_DO_SEL	0x904
#define CKO_SEL	0x90c
#define PN_SWAP	0x930
#define LVDS_CRC0	0x934
#define LVDS_CRC1	0x938
#define LVDS_CRC2	0x93c
#define LVDS_CRC3	0x940
#define LVDS_CRC4	0x944
#define LVDS_CRC5	0x948
#define LVDS_CRC6	0x94c

static void mtk_lvds_tx_power_on_signal(struct display_mode_info *mode)
{
	uint32_t reg;

	PRINTF_D("mtk_lvds_tx_power_on_signal\n");

	if (mode->type == LCM_TYPE_DUAL_LVDS || mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX0)
		writel(DA_LVDSTX_PWR_ON, LVDATX0 + VOPLL_CTL3);

	writel(DA_LVDSTX_PWR_ON, LVDATX1 + VOPLL_CTL3);
	reg = RG_VPLL_TXMUXDIV2_EN | 1U << 6 | 0x1cU << 12 | 1U << 20;
	if (mode->type == LCM_TYPE_DUAL_LVDS || mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX0)
		writel(reg, LVDATX0 + VOPLL_CTL1);
	writel(reg, LVDATX1 + VOPLL_CTL1);

	reg = RG_VPLL_EN | 1U << 8 | (mode->type == LCM_TYPE_DUAL_LVDS ? 0UL : 1UL) << 10 |
	      RG_VPLL_LVDS_EN | RG_VPLL_LVDS_DPIX_DIV2 |
	      (mode->type == LCM_TYPE_DUAL_LVDS ? 1UL : 0UL) << 16 | RG_VPLL_TXDIV5_EN |
	      RG_VPLL_BIAS_EN | RG_VPLL_BIASLPF_EN;
	writel(reg, LVDATX1 + VOPLL_CTL2);
	if (mode->type == LCM_TYPE_DUAL_LVDS || mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX0)
		writel(reg, LVDATX0 + VOPLL_CTL2);
	reg = 5U | 0xbU << 4 | 3U << 8 | RG_LVDSTX_TSTCLKDIV_EN |
	      RG_LVDSTX_TSTCLK_EN | 1U << 16 | RG_LVDSTX_LDO_EN |
	      RG_LVDSTX_BIAS_EN;
	writel(reg, LVDATX1 + LVDSTX_CTL2);

	if (mode->type == LCM_TYPE_DUAL_LVDS || mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX0)
		writel(reg, LVDATX0 + LVDSTX_CTL2);

	reg = 0x1fU << 5 | 0x1fU << 10;
	writel(reg, LVDATX1 + LVDSTX_CTL3);

	if (mode->type == LCM_TYPE_DUAL_LVDS || mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX0)
		writel(reg, LVDATX0 + LVDSTX_CTL3);

	reg = RG_LVDSTX_LDOLPF_EN | RG_LVDSTX_BIASLPF_EN;
	writel(reg, LVDATX1 + LVDSTX_CTL4);

	if (mode->type == LCM_TYPE_DUAL_LVDS || mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX0)
		writel(reg, LVDATX0 + LVDSTX_CTL4);
}

static void mtk_lvds_tx_power_off_signal(struct display_mode_info *mode)
{
	uint32_t reg;

	PRINTF_D("mtk_lvds_tx_power_off_signal\n");

	if (mode->type == LCM_TYPE_DUAL_LVDS || mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX0)
		writel(DA_LVDSTX_PWR_ON | LVDS_ISO_EN,
		       LVDATX0 + VOPLL_CTL3);

	writel(DA_LVDSTX_PWR_ON | LVDS_ISO_EN, LVDATX1 + VOPLL_CTL3);

	writel(0U, LVDATX1 + VOPLL_CTL2);
	if (mode->type == LCM_TYPE_DUAL_LVDS || mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX0)
		writel(0U, LVDATX0 + VOPLL_CTL2);

	reg = readl(LVDATX1 + LVDSTX_CTL2) &
		    (~(RG_LVDSTX_BIAS_EN | RG_LVDSTX_LDO_EN));

	writel(reg, LVDATX1 + LVDSTX_CTL2);

	if (mode->type == LCM_TYPE_DUAL_LVDS || mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX0)
		writel(reg, LVDATX0 + LVDSTX_CTL2);

	writel(0, LVDATX1 + LVDSTX_CTL3);

	if (mode->type == LCM_TYPE_DUAL_LVDS || mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX0)
		writel(0U, LVDATX0 + LVDSTX_CTL3);
}

int32_t mtk_lvds_enable(struct display_mode_info *mode)
{
	uint32_t reg;

	if (mode->type == LCM_TYPE_DUAL_LVDS) {
		/* clk_dpilvds_sel  switch to lvdspll_d2 */
		writel((readl(TOPCK_BASE + 0xc0) & 0xf8ffffff) | (2 << 24), TOPCK_BASE + 0xc0);
	} else {
		/* clk_dpilvds_sel  switch to lvdspll_ck */
		writel((readl(TOPCK_BASE + 0xc0) & 0xf8ffffff) | (1 << 24), TOPCK_BASE + 0xc0);
	}

	//usleep_range(20, 100);

	mtk_lvds_tx_power_on_signal(mode);

	writel(RG_FIFO_EN | RG_FIFO_CTRL |
	       (mode->type == LCM_TYPE_DUAL_LVDS ? 1UL : 0UL) << 23, LVDS_BASE + LVDSTOP_REG05);

	writel((mode->type == LCM_TYPE_DUAL_LVDS ? RG_DUAL : 0UL) |
	       (mode->bus_formats == MEDIA_BUS_FMT_RGB888_1X7X4_JEIDA ? 0U :
		RG_NS_VESA_EN), LVDS_BASE + LVDS_CTRL00);

	reg = (readl(LVDS_BASE + LVDS_CTRL02) | RG_LVDS_74FIFO_EN) &
	      (~RG_DPMODE);
	writel(reg, LVDS_BASE + LVDS_CTRL02);

	return 0;
}

int32_t mtk_lvds_disable(struct display_mode_info *mode)
{
	uint32_t reg;

	reg = (readl(LVDS_BASE + LVDS_CTRL02) | RG_DPMODE) &
	      (~RG_LVDS_74FIFO_EN);
	writel(reg, LVDS_BASE + LVDS_CTRL02);

	mtk_lvds_tx_power_off_signal(mode);

	return 0;
}

