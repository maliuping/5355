#include <FreeRTOS.h>
#include <mt_reg_base.h>
#include <driver_api.h>
#include "scp_sem.h"

/*
 * set or clear register bit
 */
static inline void mtk_set_clr_reg_bit(int bit, unsigned int reg)
{
	writel(1 << bit, reg);
}

/*
 * acquire a hardware semaphore
 * @param flag: semaphore id
 * return  0 :get sema success
 * 	  nz :get sema fail
 */
int mtk_get_scp_semaphore(int index)
{
	int count = 0;
	unsigned int reg, bit;

	if (index > SEMAPHORE_MAX_CNT) {
		PRINTF_E("semaphore out of range[0 - %d]\r\n", SEMAPHORE_MAX_CNT);
		return -1;
	}

	bit = index % 32;
	reg = SCP_SEMAPHORE_REG + (index / 32) * 4;

	if (!mtk_check_reg_bit(bit, reg)) {
		mtk_set_clr_reg_bit(bit, reg);
		while (count != SEMAPHORE_TIMEOUT) {
			/* repeat test untill we get semaphore */
			if(mtk_check_reg_bit(bit, reg))
				return 0;

			mtk_set_clr_reg_bit(bit, reg);
			count++;
		}
		PRINTF_E("get scp sema [%d] TIMEOUT...!\n", index);
		return -1;
	} else {
		PRINTF_E("already hold scp sema [%d]\n", index);
		return -1;
	}
}

/*
 * release a hardware semaphore
 * @param flag: semaphore id
 * return  0 :release sema success
 *        nz :release sema fail
 */
int mtk_release_scp_semaphore(int index)
{
	unsigned int reg, bit;

	bit = index % 32;
	reg = SCP_SEMAPHORE_REG + (index / 32) * 4;

	if(mtk_check_reg_bit(bit, reg)) {
		/* write 1 clear */
		mtk_set_clr_reg_bit(bit, reg);

		if(mtk_check_reg_bit(bit, reg)) {
			PRINTF_E("release scp sema [%d] failed\n", index);
			return -1;
		}
	} else {
		PRINTF_E("try to release sema [%d] not own by me\n", index);
		return -1;
	}
	return 0;
}
