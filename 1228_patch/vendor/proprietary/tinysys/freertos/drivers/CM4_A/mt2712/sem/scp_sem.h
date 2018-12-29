#ifndef __SCP_SEM_H__
#define __SCP_SEM_H__

#define SCP_SEMAPHORE_REG	(SCP_SHARE_NS_BASE + 0x08)
#define SCP_SEMAPHORE0_REG	(SCP_SEMAPHORE_REG)
#define SCP_SEMAPHORE1_REG	(SCP_SEMAPHORE_REG + 0x04)

#define SEMAPHORE_MAX_CNT	63
#define SEMAPHORE_TIMEOUT	5000

/*
 * check register bit value
 */
inline int mtk_check_reg_bit(int bit, unsigned int reg)
{
	return (readl(reg) >> bit) & 0x01;
}

int mtk_get_scp_semaphore(int index);
int mtk_release_scp_semaphore(int index);

#endif
