#include "main.h"
#include "FreeRTOS.h"
#include <driver_api.h>
#include <interrupt.h>
#include <mt_reg_base.h>
#include "mt_systmr.h"

#define SYSTMR_FREQ_SEC 13000000
#define SYSTMR_FREQ_MS 13000
#define SYSTMR_FREQ_US 13

#define systmr_ms2cnt(ms) ((ms) * SYSTMR_FREQ_MS)
#define systmr_us2cnt(us) ((us) * SYSTMR_FREQ_US)


/** internal function **/

uint32_t check_cur_tick_timeout(uint64_t obj_tick)
{
	uint64_t cur_tick;

	cur_tick = timer_get_global_timer_tick();

	return (cur_tick > obj_tick) ? 1 : 0;
}


/** external function **/

uint64_t timer_get_global_timer_tick(void)
{
	uint64_t tmr_cnt_tick, tmr_cnt_h;
	UBaseType_t uxIntSta;

	uxIntSta = taskENTER_CRITICAL_FROM_ISR();

	tmr_cnt_tick = (uint64_t)DRV_Reg32(SYSTMR_CNTCV_L);
	tmr_cnt_h = (uint64_t)DRV_Reg32(SYSTMR_CNTCV_H);

	taskEXIT_CRITICAL_FROM_ISR(uxIntSta);

	tmr_cnt_tick = (tmr_cnt_tick & 0x00000000FFFFFFFFULL)
			+ (0xFFFFFFFF00000000ULL & (tmr_cnt_h << 32));

	return tmr_cnt_tick;
}


uint32_t get_global_timer_freq(void)
{
	return SYSTMR_FREQ_SEC;
}


uint64_t get_boot_time_ms(void)
{
	uint64_t cur_tick;

	cur_tick = timer_get_global_timer_tick();

	return (cur_tick / SYSTMR_FREQ_MS);
}


uint64_t get_boot_time_us(void)
{
	uint64_t cur_tick;

	cur_tick = timer_get_global_timer_tick();

	return (cur_tick / SYSTMR_FREQ_US);
}

uint64_t get_boot_time_ns(void)
{
	uint64_t cur_tick;

	cur_tick = timer_get_global_timer_tick();
	/* 1 tick = 76.923 ns */

	return (cur_tick * 76923 / 1000);
}


void mdelay(uint32_t msec)
{
	uint64_t bgn_tick, end_tick;

	bgn_tick = timer_get_global_timer_tick();
	end_tick = bgn_tick + systmr_ms2cnt(msec);
	while (!check_cur_tick_timeout(end_tick))
		;

	return;
}


void udelay(uint32_t usec)
{
	uint64_t bgn_tick, end_tick;

	bgn_tick = timer_get_global_timer_tick();
	end_tick = bgn_tick + systmr_us2cnt(usec);
	while (!check_cur_tick_timeout(end_tick))
		;

	return;
}



