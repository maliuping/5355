#include <platform_mtk.h>
#include <driver_api.h>
#include <scp_regs.h>

#ifndef CONFIG_MTK_FPGA
uint32_t SystemCoreClock = 416000000;
#else
uint32_t SystemCoreClock = 12000000;
#endif
void SystemInit(void)
{
    /* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    uint32_t reg;

    reg = readl(CMSYS_CONFIG_REG);
    reg &= ~ENABLE_FPU;
    writel(reg, CMSYS_CONFIG_REG);
    /* set CP10 and CP11 Full Access */
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));
#endif
    /* Configure the Vector Table location add offset address
     *
     */
    SCB->VTOR = ((uint32_t) 0x0);
}
