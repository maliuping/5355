/* RGB - next is	0x1018 */
#define MEDIA_BUS_FMT_RGB444_1X12		0x1016
#define MEDIA_BUS_FMT_RGB444_2X8_PADHI_BE	0x1001
#define MEDIA_BUS_FMT_RGB444_2X8_PADHI_LE	0x1002
#define MEDIA_BUS_FMT_RGB555_2X8_PADHI_BE	0x1003
#define MEDIA_BUS_FMT_RGB555_2X8_PADHI_LE	0x1004
#define MEDIA_BUS_FMT_RGB565_1X16		0x1017
#define MEDIA_BUS_FMT_BGR565_2X8_BE		0x1005
#define MEDIA_BUS_FMT_BGR565_2X8_LE		0x1006
#define MEDIA_BUS_FMT_RGB565_2X8_BE		0x1007
#define MEDIA_BUS_FMT_RGB565_2X8_LE		0x1008
#define MEDIA_BUS_FMT_RGB666_1X18		0x1009
#define MEDIA_BUS_FMT_RBG888_1X24		0x100e
#define MEDIA_BUS_FMT_RGB666_1X24_CPADHI	0x1015
#define MEDIA_BUS_FMT_RGB666_1X7X3_SPWG		0x1010
#define MEDIA_BUS_FMT_BGR888_1X24		0x1013
#define MEDIA_BUS_FMT_GBR888_1X24		0x1014
#define MEDIA_BUS_FMT_RGB888_1X24		0x100a
#define MEDIA_BUS_FMT_RGB888_2X12_BE		0x100b
#define MEDIA_BUS_FMT_RGB888_2X12_LE		0x100c
#define MEDIA_BUS_FMT_RGB888_1X7X4_SPWG		0x1011
#define MEDIA_BUS_FMT_RGB888_1X7X4_JEIDA	0x1012
#define MEDIA_BUS_FMT_ARGB8888_1X32		0x100d
#define MEDIA_BUS_FMT_RGB888_1X32_PADHI		0x100f

/* Video mode flags */
/* bit compatible with the xorg definitions. */
#define DRM_MODE_FLAG_PHSYNC			(1<<0)
#define DRM_MODE_FLAG_NHSYNC			(1<<1)
#define DRM_MODE_FLAG_PVSYNC			(1<<2)
#define DRM_MODE_FLAG_NVSYNC			(1<<3)
#define DRM_MODE_FLAG_INTERLACE			(1<<4)
#define DRM_MODE_FLAG_DBLSCAN			(1<<5)
#define DRM_MODE_FLAG_CSYNC			(1<<6)
#define DRM_MODE_FLAG_PCSYNC			(1<<7)
#define DRM_MODE_FLAG_NCSYNC			(1<<8)
#define DRM_MODE_FLAG_HSKEW			(1<<9) /* hskew provided */
#define DRM_MODE_FLAG_BCAST			(1<<10)
#define DRM_MODE_FLAG_PIXMUX			(1<<11)
#define DRM_MODE_FLAG_DBLCLK			(1<<12)
#define DRM_MODE_FLAG_CLKDIV2			(1<<13)

#define	DRM_MODE_FLAG_3D_MASK			(0x1f<<14)
#define DRM_MODE_FLAG_3D_NONE			(0<<14)
#define DRM_MODE_FLAG_3D_FRAME_PACKING		(1<<14)
#define DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE	(2<<14)
#define DRM_MODE_FLAG_3D_LINE_ALTERNATIVE	(3<<14)
#define DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL	(4<<14)
#define DRM_MODE_FLAG_3D_L_DEPTH		(5<<14)
#define DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH	(6<<14)
#define DRM_MODE_FLAG_3D_TOP_AND_BOTTOM	(7<<14)
#define DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF	(8<<14)

enum lcm_if_type {
	LCM_TYPE_DBI = 0,
	LCM_TYPE_DPI,
	LCM_TYPE_DSI,
	LCM_TYPE_SINGLE_LVDS_ON_TX0,
	LCM_TYPE_SINGLE_LVDS_ON_TX1,
	LCM_TYPE_DUAL_LVDS
};

/* display_mode_info from panel params */
struct display_mode_info {
	enum lcm_if_type type;

	int32_t clock;		/* in kHz */
	int32_t hdisplay;
	int32_t hsync_start;
	int32_t hsync_end;
	int32_t htotal;
	int32_t vdisplay;
	int32_t vsync_start;
	int32_t vsync_end;
	int32_t vtotal;
	int32_t vrefresh;
	uint32_t flags;
	char name[32];
	int32_t bus_formats;
	int32_t bpc;
} ;

extern int32_t lcm_if_enable(struct display_mode_info *mode);
extern int32_t lcm_if_disable(struct display_mode_info *mode);
extern int32_t lcm_if_getmode(struct display_mode_info *mode);
extern int32_t lcm_if_set_backlight(uint32_t level);

