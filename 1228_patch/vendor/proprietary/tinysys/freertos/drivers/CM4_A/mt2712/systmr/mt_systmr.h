#ifndef __MT_SYSTMR_H__
#define __MT_SYSTMR_H__


uint64_t timer_get_global_timer_tick(void);
/* max effective time provided by get_boot_time_ms & us is about 44995 year */
uint64_t get_boot_time_ms(void);
uint64_t get_boot_time_us(void);
/* max effective time provided by get_boot_time_ns is about 213 day */
uint64_t get_boot_time_ns(void);

uint32_t get_global_timer_freq(void);

/* busy waiting */
void mdelay(uint32_t msec);
void udelay(uint32_t usec);


#endif

