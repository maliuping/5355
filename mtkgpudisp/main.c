#include <stdlib.h>
#include <stdio.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#include "gpu_render.h"
#include "drm_display.h"
#include "osal.h"
#include "stb_image.h"

int dynamic_log_level = 6;

static void display_set_drm_mode(struct mtk_display *display, int screen_idx, int enable)
{
	drm_add_mode_set(display, screen_idx, enable);
	drm_display_flush(display, 1);
	return;
}

int main(int argc, char *argv[])
{
	int format_type = 0;
	int fourcc = DRM_FORMAT_ARGB8888;
	struct mtk_display disp;
	void * gpu_handle;
	struct raw_texture input_tex;
	void * input_gpu_tex;
	struct raw_texture output_tex[2];
	void * output_gpu_tex[2];
	REND_COORD_T coord;
	REND_COORD_T *pcoord[2];
	int i = 0;

	if(argc >=2 ){
		format_type = strtoul(argv[1], NULL, 10);
		LOG_INFO("test info format_type %d", format_type);
	}

	fourcc = DRM_FORMAT_ARGB8888;

	LOG_INFO("test info");

	memset(&disp, 0, sizeof(struct mtk_display));
	drm_init(&disp);
	LOG_INFO("test info");
	usleep(200000);
	LOG_INFO("create gpu render handle");
	gpu_handle = gpu_render_init(disp.fd, 4);

	LOG_INFO("create input buffer");
	drm_alloc_gem(disp.fd, 1280, 720, fourcc, &input_tex);
	input_gpu_tex = gpu_render_get_tex(gpu_handle, &input_tex, 0);

	LOG_INFO("create output buffer");
	drm_alloc_gem(disp.fd, 1280, 720, fourcc, &output_tex[0]);
	output_gpu_tex[0] = gpu_render_get_tex(gpu_handle, &output_tex[0], 1);
	drm_buffer_prepare(disp.fd, &output_tex[0]);

	drm_alloc_gem(disp.fd, 1280, 720, fourcc, &output_tex[1]);
	output_gpu_tex[1] = gpu_render_get_tex(gpu_handle, &output_tex[1], 1);
	drm_buffer_prepare(disp.fd, &output_tex[1]);

	LOG_INFO("set display mode");
	display_set_drm_mode(&disp, 0, 1);

	memset(input_tex.texbuf, 90, input_tex.size);
	LOG_INFO("start drawing");
	for(i = 0; i < 30; i ++){
		coord.tex_w = input_tex.width;
		coord.tex_h = input_tex.height;
		coord.src_x = 0;
		coord.src_y = 0;
		coord.src_w = input_tex.width;
		coord.src_h = input_tex.height;
		coord.x = 0;
		coord.y = 0;
		coord.w = input_tex.width;
		coord.h = input_tex.height;
		coord.tex_type = i%5 + 1;
		// coord.tex_type = 1;
		pcoord[0] = &coord;
		gpu_render_2d_overlay(gpu_handle, 1280, 720,
			&input_gpu_tex, pcoord, 1, output_gpu_tex[i%2]);
		drm_add_plane_set(&disp, &output_tex[i%2], 0, 0, 0, 2);
		drm_display_flush(&disp, 0);
		sleep(1);
	}

	LOG_INFO("stop drawing");
	drm_add_plane_set(&disp, NULL, 0, 0, 0, 2);
	drm_display_flush(&disp, 0);
	sleep(1);

	LOG_INFO("destroy input buffer");
	if(input_gpu_tex != NULL)
		gpu_render_free_tex(gpu_handle, input_gpu_tex);
	drm_free_gem(disp.fd, &input_tex);

	LOG_INFO("destroy output buffer");
	if(output_gpu_tex[0] != NULL)
		gpu_render_free_tex(gpu_handle, output_gpu_tex[0]);
	drm_buffer_release(disp.fd, &output_tex[0]);
	drm_free_gem(disp.fd, &output_tex[0]);

	if(output_gpu_tex[1] != NULL)
		gpu_render_free_tex(gpu_handle, output_gpu_tex[1]);
	drm_buffer_release(disp.fd, &output_tex[1]);
	drm_free_gem(disp.fd, &output_tex[1]);

	LOG_INFO("destory gpu render handle");
	gpu_render_uninit(gpu_handle);
	LOG_INFO("test info");
	drm_deinit(&disp);
	LOG_INFO("test info");

	return 0;
}
