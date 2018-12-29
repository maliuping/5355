
#include "main.h"
#include "FreeRTOS.h"
#include <driver_api.h>
#include <bit_op.h>
#include <panel.h>
#include <gpio.h>

#define PANEL_AUO_G133HAN01
extern int32_t mtk_dpi_enable(struct display_mode_info *mode);
extern int32_t mtk_lvds_enable(struct display_mode_info *mode);
extern int32_t mtk_dpi_disable(struct display_mode_info *mode);
extern int32_t mtk_lvds_disable(struct display_mode_info *mode);

int32_t lcm_if_enable(struct display_mode_info *mode)
{
	#ifdef PANEL_AUO_G133HAN01
	/* to do
		config panel power and control pin
	*/

	#endif

	if ((mode->type == LCM_TYPE_DUAL_LVDS)
		|| (mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX0)
		|| (mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX1)) {
		mtk_dpi_enable(mode);
		mtk_lvds_enable(mode);
	} else if (mode->type == LCM_TYPE_DSI) {
		/* TO DO: for dsi panel */

	} else {
		/* 2712 not support the mode type */
	}

	return 0;
}

int32_t lcm_if_disable(struct display_mode_info *mode)
{
#ifdef PANEL_AUO_G133HAN01
	/* to do
		shutdown panel power and control pin
	*/
#endif

	if ((mode->type == LCM_TYPE_DUAL_LVDS)
		|| (mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX0)
		|| (mode->type == LCM_TYPE_SINGLE_LVDS_ON_TX1)) {
		mtk_lvds_disable(mode);
		mtk_dpi_disable(mode);
	} else if (mode->type == LCM_TYPE_DSI) {
		/* TO DO: for dsi panel */

	} else {
		/* 2712 not support the mode type */
	}

	return 0;
}

int32_t lcm_if_getmode(struct display_mode_info *mode)
{
#ifdef PANEL_AUO_G133HAN01
	mode->type = LCM_TYPE_DUAL_LVDS;
	mode->clock = 141200;
	mode->hdisplay = 1920;
	mode->hsync_start = 1920 + 108;
	mode->hsync_end = 1920 + 108 + 20;
	mode->htotal = 1920 + 108 + 20 + 60;
	mode->vdisplay = 1080;
	mode->vsync_start = 1080 + 16;
	mode->vsync_end = 1080 + 16 + 10;
	mode->vtotal = 1080 + 16 + 10 + 10;
	mode->vrefresh = 60;
	mode->bpc = 8;
	mode->bus_formats = MEDIA_BUS_FMT_RGB888_1X7X4_JEIDA;
#endif

	return 0;
}

int32_t lcm_if_set_backlight(uint32_t level)
{
	/* config disp_pwm pin as GPIO, then the pwm duty will be 100% high */
	gpio_set_mode(GPIO6, GPIO_MODE_GPIO);
	gpio_set_dir(GPIO6, GPIO_DIR_OUT);
	gpio_set_out(GPIO6, GPIO_OUT_ONE);

	gpio_set_mode(GPIO4, GPIO_MODE_GPIO);
	gpio_set_dir(GPIO4, GPIO_DIR_OUT);
	gpio_set_out(GPIO4, GPIO_OUT_ONE);

	return 0;
}
