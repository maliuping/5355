/*
 * Copyright (c) 2015 MediaTek Inc.
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

#ifndef __MTK_CMDQ_CONTROL_H__
#define __MTK_CMDQ_CONTROL_H__

#include "FreeRTOS.h"
#include "list.h"
#include "timers.h"
#include <bit_op.h>
#include "mt_systmr.h"
#include "mtk-cmdq.h"
#include <string.h>
#include <assert.h>
#include "mt2712.h"

#define CMDQ_INST_SIZE			8 /* instruction is 64-bit */
#define CMDQ_SUBSYS_SHIFT		16
#define CMDQ_OP_CODE_SHIFT		24
#define CMDQ_JUMP_PASS		CMDQ_INST_SIZE

#define CMDQ_WFE_UPDATE		BIT(31)
#define CMDQ_WFE_WAIT			BIT(15)
#define CMDQ_WFE_WAIT_VALUE		0x1
#define ETIMEDOUT			60        /* Connection timed out */
#define EFAULT				14        /* Bad address */
#define EINVAL				22         /* Invalid argument */
#define EBUSY				16         /* Device or resource busy */
#define ENOMEM				12         /* Out of memory */
#define PLATFORM_TYPE			2712u

#define WARN_ON(x) ({					\
	int __ret_warn_on = !!(x);			\
	if (__ret_warn_on) {	\
		PRINTF_E("[Warning!] %s, %d", __FUNCTION__, __LINE__); \
	} \
	__ret_warn_on;			\
})

#define GCE_LOG_LEVEL					1

#define GCE_DEBUG_LOG_LEVEL 				2
#define GCE_ERROR_LOG_LEVEL 				1

#if (GCE_DEBUG_LOG_LEVEL <= GCE_LOG_LEVEL)
#define GCE_LOG_D(x...) printf(x)
#else
#define GCE_LOG_D(x...)
#endif

#if (GCE_ERROR_LOG_LEVEL <= GCE_LOG_LEVEL)
#define GCE_LOG_E(x...) printf(x)
#else
#define GCE_LOG_E(x...)
#endif

#ifndef bool
   #define bool int
#endif

#ifndef false
   #define false  0
#endif

#ifndef true
   #define true  1
#endif

/*
 * The length of circular buffer for queuing messages from a client.
 * 'msg_count' tracks the number of buffered messages while 'msg_free'
 * is the index where the next message would be buffered.
 * We shouldn't need it too big because every transfer is interrupt
 * triggered and if we have lots of data to transfer, the interrupt
 * latencies are going to be the bottleneck, not the buffer length.
 * Besides, msg_send_message could be called from atomic context and
 * the client could also queue another message from the notifier 'tx_done'
 * of the last transfer done.
 * REVISIT: If too many platforms see the "Try increasing MSG_TX_QUEUE_LEN"
 * print, it needs to be taken from config option or somesuch.
 */
#define MSG_TX_QUEUE_LEN	20

static const int  BITS_PER_LONG = sizeof(long) * 8;
#define min(x,y) ((x) < (y) ? x : y)
#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))

/*
 * This looks more complex than it should be. But we need to
 * get the type for the ~ right in round_down (it needs to be
 * as wide as the result!), and we want to evaluate the macro
 * arguments just once each.
 */
#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry_safe(pos, n, head)			\
	for (pos = listGET_HEAD_ENTRY(head), \
		n = listGET_NEXT(pos);	\
	     pos != listGET_END_MARKER(head);					\
	     pos = n, n = listGET_NEXT(pos))

/**
* list_first_entry - get the first element from a list
* @ptr:	the list head to take the element from.
*
* Note, that list is expected to be not empty.
*/
#define list_first_entry(ptr) \
		listGET_LIST_ITEM_OWNER(listGET_NEXT(ptr))

/**
* list_first_entry_or_null - get the first element from a list
* @ptr:	the list head to take the element from.
*
* Note that if the list is empty, it returns NULL.
*/
#define list_first_entry_or_null(ptr) \
		(!listLIST_IS_EMPTY(ptr) ? list_first_entry((ptr)->pxIndex) : NULL)

static inline unsigned long __ffs(unsigned long word)
{
    int num = 0;

    #if BITS_PER_LONG == 64
    if ((word & 0xffffffff) == 0) {
        num += 32;
        word >>= 32;
    }
    #endif

    if ((word & 0xffff) == 0) {
        num += 16;
        word >>= 16;
    }
    if ((word & 0xff) == 0) {
        num += 8;
        word >>= 8;
    }
    if ((word & 0xf) == 0) {
        num += 4;
        word >>= 4;
    }
    if ((word & 0x3) == 0) {
        num += 2;
        word >>= 2;
    }
    if ((word & 0x1) == 0)
        num += 1;
    return num;
}

/*
 * This is a common helper function for find_next_bit and
 * find_next_zero_bit.  The difference is the "invert" argument, which
 * is XORed with each fetched word before searching it for one bits.
 */
static inline unsigned long _find_next_bit_(const unsigned long *addr,
		unsigned long nbits, unsigned long start, unsigned long invert)
{
	unsigned long tmp;

	if (!nbits || start >= nbits)
		return nbits;

	tmp = addr[start / BITS_PER_LONG] ^ invert;

	/* Handle 1st word. */
	tmp &= BITMAP_FIRST_WORD_MASK(start);
	start = round_down(start, BITS_PER_LONG);

	while (!tmp) {
		start += BITS_PER_LONG;
		if (start >= nbits)
			return nbits;

		tmp = addr[start / BITS_PER_LONG] ^ invert;
	}

	return min(start + __ffs(tmp), nbits);
}

/**
*readl_poll_timeout_atomic - Periodically poll an address until a condition is met or timeout occurs
*@addr: Address to poll
*@val: Variable to read the value into
*@cond: Break condition (usually involving @val)
*@max_reads: Maxinum number of reads before giving up
*@time_between_us: Time to vTaskDelay between successive reads
*
*Return 0 on success and -ETIMEDOUT upon a timeout.
*/
#define readl_poll_timeout_atomic(addr, val, cond, max_reads, time_between_us) \
({ \
                  int count = 0; \
                  for (count = (max_reads); count >= 0; count--) { \
	                  (val) = readl(addr); \
	                  if (cond) \
		                break; \
	                  udelay(time_between_us); \
                  } \
                  (cond) ? 0 : -ETIMEDOUT; \
})


static inline unsigned long _find_next_o_bit_(const unsigned long *addr, unsigned long size,
					 unsigned long offset)
{
		return _find_next_bit_(addr, size, offset, ~0UL);
}

#define for_each_clear_bit(bit, addr, size) \
            for ((bit) = _find_next_o_bit_((addr), (size), 0);	  \
                   (bit) < (size);					  \
                   (bit) = _find_next_o_bit_((addr), (size), (bit) + 1))

/*
 * CMDQ_CODE_MASK:
 *   set write mask
 *   format: op mask
 * CMDQ_CODE_WRITE:
 *   write value into target register
 *   format: op subsys address value
 * CMDQ_CODE_JUMP:
 *   jump by offset
 *   format: op offset
 * CMDQ_CODE_WFE:
 *   wait for event and clear
 *   it is just clear if no wait
 *   format: [wait]  op event update:1 to_wait:1 wait:1
 *           [clear] op event update:1 to_wait:0 wait:0
 * CMDQ_CODE_EOC:
 *   end of command
 *   format: op irq_flag
 */
enum cmdq_code {
	CMDQ_CODE_READ  = 0x01,
	CMDQ_CODE_MASK = 0x02,
	CMDQ_CODE_WRITE = 0x04,
	CMDQ_CODE_POLL  = 0x08,
	CMDQ_CODE_JUMP = 0x10,
	CMDQ_CODE_WFE = 0x20,
	CMDQ_CODE_EOC = 0x40,


	/* for instruction generation */
	CMDQ_CODE_WRITE_FROM_MEM = 0x05,
	CMDQ_CODE_WRITE_FROM_REG = 0x07,
	CMDQ_CODE_SET_TOKEN = 0x21,	/* set event */
	CMDQ_CODE_WAIT_NO_CLEAR = 0x25,	/* wait event, but don't clear it */
	CMDQ_CODE_CLEAR_TOKEN = 0x23,	/* clear event */
	CMDQ_CODE_RAW = 0x24,	/* allow entirely custom arg_a/arg_b */
	CMDQ_CODE_PREFETCH_ENABLE = 0x41,	/* enable prefetch marker */
	CMDQ_CODE_PREFETCH_DISABLE = 0x42,	/* disable prefetch marker */
};

int gce_init(void);
#endif /* __MTK_CMDQ_CONTROL_H__ */