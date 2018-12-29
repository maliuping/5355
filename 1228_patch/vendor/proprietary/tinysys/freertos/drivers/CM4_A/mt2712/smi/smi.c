
#include "main.h"
#include "FreeRTOS.h"
#include <driver_api.h>
#include <mt_reg_base.h>

#define SMI_LARB_NONSEC_CON 	0x380
#define MMU_MASK		(1 << 0)
#define MMU_EN(en)		((en) << 0)

#define SCP_BASE(a)		((a) + SCP_OFFSET_AP)

struct mtk_smi_larb_config {
	uint32_t base;    /* base register */
	uint32_t port_nr; /* Valid port number in each larb. */
};

static const struct mtk_smi_larb_config mtk_smi_larb_cfg[] = {
	{SCP_BASE(0x14021000), 8},/* larb0 */
	{SCP_BASE(0x16010000), 11}, /* larb1: vdec */
	{SCP_BASE(0x15001000), 3}, /* larb2: cam */
	{SCP_BASE(0x18001000), 9}, /* larb3: venc */
	{SCP_BASE(0x14027000), 7},/* larb4 */
	{SCP_BASE(0x14030000), 4},
	{SCP_BASE(0x18002000), 4}, /* larb6:jpg */
	{SCP_BASE(0x14032000), 2},
};

static int mtk_smi_larb_port_disable_mmu(const uint32_t larb_id)
{
	uint32_t base, reg, reg_confirm;
	uint32_t i;

	base = mtk_smi_larb_cfg[larb_id].base + SMI_LARB_NONSEC_CON;

	for (i = 0; i<mtk_smi_larb_cfg[larb_id].port_nr; i++) {
		reg = DRV_Reg32(base + (i << 2));
		reg &= ~MMU_MASK;
		reg |= MMU_EN(0); /* Disable MMU */
		DRV_WriteReg32(base + (i << 2), reg);

		/* Confirm */
		reg_confirm = DRV_Reg32(base + (i << 2));
		if (reg_confirm != reg) {
			printf("SMI port fail %d-%d 0x%x 0x%x, Take care the power and clock\n",
			       larb_id, i, reg, reg_confirm);
			return -1;
		}
	}
	return 0;
}

#define BDPSYS_MMU_MASK  ((1<<7) | (1<<23))
static void mtk_smi_bdpsys_larb_disable_mmu(void)
{
	uint32_t larb8_base = SCP_BASE(0x1501a004), larb9_base = SCP_BASE(0x1501a00c);
	uint32_t reg;

	reg = DRV_Reg32(larb8_base);
	reg &= ~BDPSYS_MMU_MASK;
	DRV_WriteReg32(larb8_base, reg);

	reg = DRV_Reg32(larb9_base);
	reg &= ~BDPSYS_MMU_MASK;
	DRV_WriteReg32(larb9_base, reg);
}

/* CM4 Don't support IOMMU. Disable IOMMU for MM HW. */
void mtk_smi_larb_init(void)
{
	int ret;

	ret = mtk_smi_larb_port_disable_mmu(0);
	ret = mtk_smi_larb_port_disable_mmu(2);
	ret = mtk_smi_larb_port_disable_mmu(4);
	ret = mtk_smi_larb_port_disable_mmu(5);
	ret = mtk_smi_larb_port_disable_mmu(7);

	mtk_smi_bdpsys_larb_disable_mmu();
	if (ret)
		printf("smi init fail\n");
}
