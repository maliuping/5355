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

#ifndef TVD_PRIVATE_H
#define TVD_PRIVATE_H

#include "driver_api.h"
#include "tvd_if.h"

#define TVD_READ32(offset)		readl(((base) + (offset)))
#define TVD_WRITE32(offset, value)	writel(value, ((base) + (offset)));

#define TVD_SET_BIT(offset, Bit) \
	do { \
		uint32_t tv = TVD_READ32(offset); \
		tv |= (Bit); \
		TVD_WRITE32(offset, tv); \
	} while (0)

#define TVD_CLR_BIT(offset, Bit) \
	do { \
		uint32_t tv = TVD_READ32(offset); \
		tv &= (~(Bit)); \
		TVD_WRITE32(offset, tv); \
	} while (0)

#define TVD_SET_FIELD(offset, field, val) \
	do { \
		uint32_t tv = TVD_READ32(offset); \
		tv &= (~(field)); \
		tv |= (val); \
		TVD_WRITE32(offset, tv); \
	} while (0)

#define bool uint32_t
#define true 1
#define false 0

#define TVD_Y_SIZE (720U * 576U)
#define TVD_POLLING_SIGNAL_LOSS	 193U
#define TVD_OTHER_FUNCTION	 194U
#define TVD_WCH_PRIV_BUF_SIZE   (720U * 576U * 2U)
#define TVD_TDC_BUF_SIZE (4U * 1024U * 1024U)

#define TVD_LOG_TAG "[tvd]"
#if TVD_LOG_DISABLE
#define TVD_LOG_ERR(fmt, ...)
#define TVD_LOG_DBG(fmt, ...)
#define TVD_LOG_IRQ(fmt, ...)
#define TVD_LOG_IF(fmt, ...)
#if TVD_SELF_TEST
#define TVD_LOG_TEST_OK(fmt, ...)
#define TVD_LOG_TEST_ERR(fmt, ...)
#endif
#else
#define TVD_LOG_ERR(fmt, ...) \
		printf("%s [E]"fmt"\n", TVD_LOG_TAG, ##__VA_ARGS__);
#define TVD_LOG_DBG(fmt, ...) \
		printf("%s [D]"fmt"\n", TVD_LOG_TAG, ##__VA_ARGS__);
#define TVD_LOG_IRQ(fmt, ...)
		//printf("%s [IRQ]"fmt"\n", TVD_LOG_TAG, ##__VA_ARGS__);
#define TVD_LOG_IF(fmt, ...)
		//printf("%s [IF]"fmt"\n", TVD_LOG_TAG, ##__VA_ARGS__);
#if TVD_SELF_TEST
#define TVD_LOG_TEST_OK(fmt, ...) \
		printf("%s [T-OK]"fmt"\n", TVD_LOG_TAG, ##__VA_ARGS__);
#define TVD_LOG_TEST_ERR(fmt, ...) \
		printf("%s [T-ERR]"fmt"\n", TVD_LOG_TAG, ##__VA_ARGS__);
#endif
#endif
#define TVD_LOG_FUNC_START TVD_LOG_IF("%s start", __func__);
#define TVD_LOG_FUNC_END TVD_LOG_IF("%s end", __func__);

#define TVD_RET_OK 0
#define TVD_RET_PARAM_ERR 1
#define TVD_RET_FAIL 2

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

enum TVD_MODE_E {
	AV_PAL_N = 0,
	AV_PAL,
	AV_PAL_M,
	AV_NTSC,
	AV_SECAM,
	AV_PAL_60,
	AV_UNSTABLE,
	AV_NTSC443,
	AV_NONE,
};

enum BLOCK_FORMAT {
	FORMAT_16_32 = 0,	/* wch write memory by 16*32 block mode */
	FORMAT_32_64,		/* wch write memory by 32*64 block mode */
};

enum WRITE_DRAM_METHOD {
	BLOCK = 0,		/* wch write memory by  block mode */
	SCANLINE,		/* wch write memory by scanline mode */
};

enum DATA_FORMAT {
	FORMAT_420 = 0,		/* wch write memory by 420 format */
	FORMAT_422,		/* wch write memory by 422 format */
};

enum TVD_DRV_ST {
	DRV_NONE = 0,
	DRV_START,
	DRV_READY,
	DRV_MAX
};

enum TVD_THREAD_ST {
	THREAD_NONE = 0,
	THREAD_IDLE,
	THREAD_GET_BUF,
	THREAD_CB_BUF,
	THREAD_RELEASE,
	THREAD_MAX
};

/**
 *struct mode_switch_control
 *@tvd_vpres_count:         used for vpress count when mode change
 *@tvd_mode_switch_flag: used for recoder the mode change or not
 *@tvd_vsync_count:         used for recoder the vsync count when mode change
 *@tvd_vpres_flag:           used for tvd vpress flag
 *@current_mode:           tvd current mode
 */
struct TVD_MODE_SWITCH_CTRL {
	int32_t tvd_vpres_count;
	int32_t tvd_mode_switch_flag;
	int32_t tvd_vsync_count;
	int32_t tvd_vpres_flag;
	uint32_t current_mode;
	uint32_t mode_is_fix;
};

/**
 * struct tvd_status
 * @h_lock:   tvd h_lock status
 * @v_lock:   tvd v_lock status
 * @vpress_on: tvd vpress lock status
 * @line_lock: tvd line lock status
 */
struct TVD_SIGNAL_ST {
	int32_t vpress_on;
	int32_t h_lock;
	int32_t v_lock;
	int32_t line_lock;
};

/**
 *struct tvd_info
 *@tvd_base:             tvd base address
 *@cvbs_base:           cvbs base address
 *@dispsys_config:      clear interrupt status base address
 *@wch:                    related wch point
 *@tdc:                     related tdc point
 *@irq:                     tvd irq vector
 *@support_3d:            tvd support 3d feature or not
 *@mode_switch_ctr:    mode change realted control parameter
 *@tvd_status:    tvd status
 */
struct TVD_INFO {
	uint32_t irq;
	uint32_t support_3d;
	enum TVD_MODE_E mode;
	struct TVD_MODE_SWITCH_CTRL mode_switch_ctr;
	struct TVD_SIGNAL_ST tvd_signal_st;
};

/**
 *struct wch_info
 *@wch_base:     tvd base address
 *@dram2axi_bridge:    enable iommu base address
 *@tvd:              related tvd point
 *@irq:              current wch irq vector
 *@buffer_addr:  init wch y/c address
 *@block_format:            wch block mode(16*32/32*64)
 *@write_dram_method:   wch write memory method(420/422)
 *@data_format:       data format in memory(block/scanline)
 *@init_done:      wch init done
 *@first_field:     fisrt field or not
 *@discard_frame:   need discard field count
 */
struct WCH_INFO {
	uint32_t irq;
	uint32_t width;
	uint32_t height;
	uint32_t first_field;
	uint32_t discard_frame;
	uint32_t wch_fmt_fourcc;
	enum DATA_FORMAT data_format;
	enum BLOCK_FORMAT block_format;
	enum WRITE_DRAM_METHOD write_dram_method;
};

struct TVD_REG_BASE {
	void *tvd_base;
	void *cvbs_base;
	void *dispsys_config_base;
	void *tdc_base;
	void *wch_base;
	void *dram2axi_bridge_base;
};

struct TVD_BUF_INFO {
	unsigned long y;
	unsigned long c;
	video_buffer *store;
};

struct TVD_BUF {
	unsigned long tdc_buf;
	struct TVD_BUF_INFO wch_pre_buf;
	struct TVD_BUF_INFO wch_curr_buf;
	struct TVD_BUF_INFO wch_next_buf;
	struct TVD_BUF_INFO wch_priv_buf;
};

struct TVD_DRV {
	struct TVD_INFO tvd;
	struct WCH_INFO wch;
	struct TVD_REG_BASE reg;
	struct TVD_BUF buf;

	enum TVD_THREAD_ST thr_st;

	TaskHandle_t thread_handle;
	QueueHandle_t queue_handle;

	uint32_t color_format;
	uint32_t thread_stop;

	void (*cb_func)(void *cb_data, video_buffer_type type, video_buffer *buf);
	void *cb_data;
};

void tvd_2d_setting(struct TVD_DRV *drv);
void tvd_3d_setting(struct TVD_DRV *drv);
void tvd_thread_st_change(struct TVD_DRV *drv, enum TVD_THREAD_ST st);
void tvd_thread(void *args);
void tvd_irq_req(struct TVD_DRV *drv);
void tvd_irq_unreq(struct TVD_DRV *drv);

#endif /* TVD_PRIVATE_H */

