/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
* Author: Huangfei Xiao <huangfei.xiao@mediatek.com>
*/

#ifndef VIDEO_CORE_H
#define VIDEO_CORE_H

#include <stdint.h>
#include <tinysys_config.h>

//#define VIDEO_CORE_DEBUG

typedef void* video_handle;

typedef enum video_config_type {
	VIDEO_CONFIG_FORMAT,		/* struct  video_format */
	VIDEO_CONFIG_CROP,		/* struct  video_rect, crop area */
	VIDEO_CONFIG_AREA,		/* struct  video_rect, target area to be processed or output */
	NR_CONFIG_LEVEL,		/* struct nr_level, set total/bnr/mnr/fnr level */
} video_config_type;

typedef enum video_buffer_type {
	VIDEO_BUFFER_INPUT,		/* normal device input buffer */
	VIDEO_BUFFER_OUTPUT,		/* normal device output buffer */
	OVL_BUFFER_INPUT_0,		/* OVL layer0 input buffer */
	OVL_BUFFER_INPUT_1,		/* OVL layer1 input buffer */
	OVL_BUFFER_INPUT_2,		/* OVL layer2 input buffer */
	OVL_BUFFER_INPUT_3,		/* OVL layer3 input buffer */
	OVL_BUFFER_OUTPUT,		/* OVL output buffer */
} video_buffer_type;

typedef struct video_format {
	video_buffer_type type;
	uint32_t	fourcc;		/* Four-character-code color format */
	uint32_t	width;		/* image width */
	uint32_t	height;		/* image height */
} video_format;

typedef struct video_rect {
	video_buffer_type type;
	uint32_t	left;
	uint32_t	top;
	uint32_t	width;
	uint32_t	height;
} video_rect;

typedef struct nr_level {
	uint32_t u4BnrLevel;
	uint32_t u4MnrLevel;
	uint32_t u4FnrLevel;
} nr_level;

typedef struct video_buffer {
	void		*image;		/* start address of image, contiguous physical memory */
	uint64_t	timestamp;	/* filled by camera module and should be copyed from input buffer to output buffer */
	uint64_t	latency;	/* time costed by module */
} video_buffer;


/*
* video_core_open()	create a device instance
* @return: handle of instance, NULL means fail
* @name: device name
*/
void* video_core_open(char *name);

/*
* video_core_close()	close device instance
* @return: NON-zero means fail
* @handle: handle by video_core_open returned
*/
int32_t video_core_close(void *handle);

/*
* video_core_config()	set device configuration
* @return: NON-zero means fail
* @handle: handle by video_core_open
* @type: video_config_type
* @config: address of configuration data
*/
int32_t video_core_config(void *handle, video_config_type type, void *config);

/*
* video_core_commit()	commit buffer
* @return: NON-zero means fail
* @handle: handle by video_core_open
* @type: video_buffer_type
* @buffer: address of buffer to commit (*)
*
* NOTE: (*)the same buffer address should be outputed by buffer_done_handler
*/
int32_t video_core_commit(void *handle, video_buffer_type type, video_buffer *buffer);

/*
* buffer_done_handler()	handler to receive buffer processed done notification
* @return: void
* @usr_data: the same 'usr_data' with video_core_register_handler
* @type: video_buffer_type
* @buffer: address of buffer processed done (*)
*
* NOTE: (*)the buffer address should be the same with video_core_commit
*/
typedef void (*buffer_done_handler)(void *usr_data, video_buffer_type type, video_buffer *buffer);

/*
* video_core_register_handler()	register buffer processed done callback
* @return: NON-zero means fail
* @buffer_done_handler: callback handler address
* @usr_data: address of user private data
*/
int32_t video_core_register_handler(void *handle, buffer_done_handler handler, void *usr_data);

/*
* device driver callback structure
*/
typedef struct device_driver {
	/*
	* video_core_open
	*/
	video_handle (*open)();
	/*
	* video_core_close
	*/
	int32_t (*close)(video_handle);
	/*
	* video_core_config
	*/
	int32_t (*config)(video_handle, video_config_type, void *);
	/*
	* video_core_commit
	*/
	int32_t (*commit)(video_handle, video_buffer_type, video_buffer *);
	/*
	* video_core_register_handler
	*/
	int32_t (*register_handler)(void *handle, buffer_done_handler, void *);
} device_driver;

/*
* device name
*/
#ifdef CFG_VIRTUAL_CAMERA_SUPPORT
#define VIRTUAL_CAMERA_DEV_NAME		"virt_cam"
#endif
#ifdef CFG_VIRTUAL_DISPLAY_SUPPORT
#define VIRTUAL_DISPLAY_DEV_NAME	"virt_disp"
#endif
#ifdef CFG_VIRTUAL_M2M_SUPPORT
#define VIRTUAL_M2M_DEV_NAME	"virt_m2m"
#endif
#ifdef CFG_DISPLAY_SUPPORT
#define DISPLAY_DEV_NAME	"disp"
#endif
#ifdef CFG_MDP_SUPPORT
#define MDP_DEV_NAME		"mdp"
#endif
#ifdef CFG_OVERLAY2_SUPPORT
#define OVL2_DEV_NAME		"ovl2"
#endif
#ifdef CFG_TVD_SUPPORT
#define TVD_DEV_NAME		"tvd"
#endif
#ifdef CFG_MTK_NR_SUPPORT
#define NR_DEV_NAME		"nr"
#endif

/*  Four-character-code (FOURCC) */
#define FOURCC(a, b, c, d)\
	((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#define PIX_FMT_ARGB32	FOURCC('B', 'A', '2', '4') /* 32  ARGB-8-8-8-8  */
#define PIX_FMT_ABGR32	FOURCC('A', 'R', '2', '4') /* 32  BGRA-8-8-8-8  */
#define PIX_FMT_YUYV	FOURCC('Y', 'U', 'Y', 'V') /* 16  YUV 4:2:2     */
#define PIX_FMT_UYVY	FOURCC('U', 'Y', 'V', 'Y') /* 16  YUV 4:2:2     */
#define PIX_FMT_MT21	FOURCC('M', 'M', '2', '1') /* Mediatek Block Mode  */
#define PIX_FMT_BGR24	FOURCC('B', 'G', 'R', '3') /* 24  BGR-8-8-8     */
#define PIX_FMT_RGB24	FOURCC('R', 'G', 'B', '3') /* 24  RGB-8-8-8     */

#define PIX_DISP_ARGB32	FOURCC('A', 'R', '2', '4') /* [31:0] A:R:G:B 8:8:8:8 little endian */
#define PIX_DISP_ABGR32	FOURCC('A', 'B', '2', '4') /* [31:0] A:B:G:R 8:8:8:8 little endian */
#define PIX_DISP_YUYV	FOURCC('Y', 'U', 'Y', 'V') /* [31:0] Cr0:Y1:Cb0:Y0 8:8:8:8 little endian */
#define PIX_DISP_UYVY	FOURCC('U', 'Y', 'V', 'Y') /* [31:0] Y1:Cr0:Y0:Cb0 8:8:8:8 little endian */
#define PIX_DISP_RGB24	FOURCC('R', 'G', '2', '4') /* [23:0] R:G:B little endian */
#define PIX_DISP_BGR24	FOURCC('B', 'G', '2', '4') /* [23:0] B:G:R little endian */


/*
* VA: address of scp dram
* IOVA: address for HW (no matter enabled/disabled iommu)
*
* kernel reserved iommu area for scp whole dram range
* and the mapping rule is IOVA = PA (physical address)
* so that IOVA can keep the same whether iommu is on or not
*/
#define VA_TO_IOVA(va)		(void*)(((unsigned long)va) - SCP_SHARE_MEM_OFFSET)
#define IOVA_TO_VA(iv)		(void*)(((unsigned long)iv) + SCP_SHARE_MEM_OFFSET)


#endif //VIDEO_CORE_H
