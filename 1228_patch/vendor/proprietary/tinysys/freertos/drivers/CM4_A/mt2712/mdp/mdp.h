#ifndef __MDP_H__
#define __MDP_H__

#include "video_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tinysys_config.h>
#include "mtk-cmdq.h"
#include "mtk-cmdq-control.h"

#define MTK_MDP_MAX_NUM_PLANE		3
/*typedef unsigned long				u32;*/
#define u32 uint32_t
#define u64 uint64_t

#define WARN_ON(x) ({					\
	int __ret_warn_on = !!(x);			\
	if (__ret_warn_on) {	\
		PRINTF_E("[Warning!] %s, %d", __FUNCTION__, __LINE__); \
	} \
	__ret_warn_on;			\
})

#define MDP_LOG_LEVEL					1

#define MDP_DEBUG_LOG_LEVEL 				2
#define MDP_ERROR_LOG_LEVEL 				1

#if (MDP_DEBUG_LOG_LEVEL <= MDP_LOG_LEVEL)
#define MDP_LOG_D(x...) printf(x)
#else
#define MDP_LOG_D(x...)
#endif

#if (MDP_ERROR_LOG_LEVEL <= MDP_LOG_LEVEL)
#define MDP_LOG_E(x...) printf(x)
#else
#define MDP_LOG_E(x...)
#endif


/**
 * struct mdp_config - configured for source/destination image
 * @x        : left
 * @y        : top
 * @w        : width
 * @h        : height
 * @crop_x   : cropped left
 * @crop_y   : cropped top
 * @crop_w   : cropped width
 * @crop_h   : cropped height
 * @format   : color format
 * @pitch    : bytes per line for each plane
 */
struct mdp_config {
	video_buffer_type type;
	video_handle handle;
	uint32_t w;
	uint32_t h;
	uint32_t crop_x;
	uint32_t crop_y;
	uint32_t crop_w;
	uint32_t crop_h;
	uint32_t format;
	uint32_t pitch[MTK_MDP_MAX_NUM_PLANE];
};

struct mdp_buffer {
	unsigned long addr_mva[MTK_MDP_MAX_NUM_PLANE];
	uint32_t plane_size[MTK_MDP_MAX_NUM_PLANE];
	uint32_t plane_num;
};

video_handle mdp_open();
int32_t mdp_close(video_handle);
int32_t mdp_config(video_handle, video_config_type, void *);
int32_t mdp_commit(video_handle, video_buffer_type, video_buffer *);
int32_t mdp_register_handler(video_handle, void (*)(void *, video_buffer_type, video_buffer *), void *);
#endif /*__MDP_H__*/
