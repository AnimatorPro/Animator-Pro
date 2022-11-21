/* vdr_sdl.c */

#include <assert.h>
#include <SDL.h>
#define VDEV_INTERNALS
#include "errcodes.h"
#include "libdummy.h"
#include "memory.h"
#include "ptrmacro.h"
#include "rastcall.h"
#include "rastlib.h"
#include "vdevcall.h"

// shared space for all the SDL structures and queries
#include "pj_sdl.h"

#define REX_VDRIVER     0x0101U
#define VDEV_VERSION    0

static Errcode sdl_detect(Vdevice *vd);
static Errcode sdl_get_modes(Vdevice *vd, USHORT mode, Vmode_info *pvm);
static char *sdl_mode_text(Vdevice *vd, USHORT mode);

static Errcode
sdl_open_graphics(Vdevice *vd, LibRast *r, LONG w, LONG h, USHORT mode);

static Errcode sdl_close_graphics(Vdevice *vd);
static Rastlib *get_sdl_lib(void);

static struct vdevice_lib sdl_device_library = {
	sdl_detect,
	sdl_get_modes,
	sdl_mode_text,
	NOFUNC, /* set_max_height */
	sdl_open_graphics,
	sdl_close_graphics,
	NOFUNC, /* open_cel */
	NOFUNC, /* show_rast */
};

#define MAKE_VMODE_INFO(MODE_IX, WIDTH, HEIGHT) \
	{                                    \
		sizeof(Vmode_info),              \
		MODE_IX,                         \
		"SDL",                           \
		8, /* bits */                    \
		1, /* planes */                  \
		{WIDTH,WIDTH,WIDTH,1},           \
		{HEIGHT,HEIGHT,HEIGHT,1},        \
		TRUE, /* readable */             \
		TRUE, /* writeable */            \
		TRUE, /* displayable */          \
		0, /* fields_per_frame */        \
		1, /* display_pages */           \
		1, /* store_pages */             \
		WIDTH*HEIGHT,                    \
		WIDTH*HEIGHT,                    \
		TRUE, /* palette_vblank_only */  \
		0, /* screen_swap_vblank_only */ \
		70, /* field_rate */             \
		0 /* vblank_period */            \
	}

static Vmode_info sdl_infos[] = {
	MAKE_VMODE_INFO(0,  320,  200),
	MAKE_VMODE_INFO(1,  640,  480),
	MAKE_VMODE_INFO(2,  800,  600),
	MAKE_VMODE_INFO(3, 1024,  768),
	MAKE_VMODE_INFO(4, 1280,  800),
	MAKE_VMODE_INFO(5, 1920, 1080),
};

#undef MAKE_VMODE_INFO

static Vdevice sdl_driver = {
	{ REX_VDRIVER, VDEV_VERSION, NULL, NULL, NULL, NULL, NULL }, /* hdr */
	0, /* first_rtype */
	1, /* num_rtypes */
	Array_els(sdl_infos), /* mode_count */
	sizeof(Vdevice_lib) / sizeof(int(*)()), /* dev_lib_count */
	&sdl_device_library, /* lib */
	NULL, /* grclib */
	NUM_LIB_CALLS, /* rast_lib_count */
	{0}
};


/*--------------------------------------------------------------*/

int
pj_get_vmode(void)
{
	return 0;
}

void
restore_ivmode(void)
{
}

void
pj_wait_vsync(void)
{
}

/*--------------------------------------------------------------*/
/* SDL Vdevice library.                                         */
/*--------------------------------------------------------------*/

static Errcode
sdl_detect(Vdevice *vd)
{
	(void)vd;
	return Success;
}

static Errcode
sdl_get_modes(Vdevice *vd, USHORT mode, Vmode_info *pvm)
{
	(void)vd;

	if (mode >= Array_els(sdl_infos))
		return Err_no_such_mode;

	*pvm = sdl_infos[mode];
	return Success;
}

static char *
sdl_mode_text(Vdevice *vd, USHORT mode)
{
	(void)vd;
	(void)mode;

	return "";
}

static void sdl_open_raster(Raster* r, LONG w, LONG h)
{
	static const Rasthdr defaults = {
		RT_MCGA,			/* type */
		8,					/* effective bit depth */
		NULL,				/* lib */
		6,		  5,		/* aspect ratio */
		{ 0, 0 },			/* reserved */
		320,	  200, 0, 0 /* w,h,x,y rectangle */
	};
	static const Bmap bm = {
		0,						  /* realmem seg descriptor, filled in at runtime */
		1,						  /* number of bplanes at least 1 */
		Bytemap_bpr(320),		  /* rowbytes */
		Byteplane_size(320, 200), /* size of plane (saves code) */
		{ NULL },				  /* at least one plane, the pixelated data */
	};

	*((Rasthdr*)r) = defaults;
	r->lib		   = get_sdl_lib();
	r->width	   = w;
	r->height	   = h;

	r->hw.bm	   = bm;
	r->hw.bm.bpr   = w;
	r->hw.bm.psize = w * h;
}

static Errcode sdl_open_graphics(Vdevice* vd, Raster* r, LONG w, LONG h, USHORT mode)
{
	(void)mode;

	/* kiki note: Not using SDL_Renderer because we want to grab the
	 * window surface directly, and creating a renderer makes
	 * SDL_GetWindowSurface() return NULL. */

	const LONG video_scale = pj_sdl_get_display_scale();

	/* kiki note: on video resize, the app calls open_graphics
	 * again-- need to make sure this window is dead. */

	if (window) {
		SDL_DestroyWindow(window);
		window = NULL;
	}

	window = SDL_CreateWindow("PJ Paint",
							  SDL_WINDOWPOS_UNDEFINED,
							  SDL_WINDOWPOS_UNDEFINED,
							  w * video_scale,
							  h * video_scale,
							  0);

	SDL_SetWindowResizable(window, SDL_TRUE);

	s_window_surface = SDL_GetWindowSurface(window);
	if (!s_window_surface) {
		return Err_no_display;
	}

	// don't kill my beautiful pixels with smoothing
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	s_surface = SDL_CreateRGBSurface(0, w, h, 8, 0, 0, 0, 0);
	if (!s_surface) {
		return Err_no_display;
	}

	s_buffer = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
	if (!s_buffer) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return Err_no_display;
	}

	sdl_open_raster(r, w, h);
	r->hw.bm.bp[0] = s_surface->pixels;
	r->hw.bm.bpr   = s_surface->pitch;
	r->type		   = vd->first_rtype;

	return Success;
}

static Errcode sdl_close_graphics(Vdevice* vd)
{
	(void)vd;

	return Success;
}

/*--------------------------------------------------------------*/
/* SDL Rastlib.                                                 */
/*--------------------------------------------------------------*/

static Errcode sdl_close_rast(Raster* r)
{
	(void)r;
	return Success;
}

static void
sdl_set_colors(Raster *r, LONG start, LONG count, void *cbuf)
{
	const uint8_t *cmap = cbuf;
	SDL_Colour colors[256];
	int c;

	(void)r;
//	(void)start;

	assert(0 < count && count <= 256);

	for (c = 0; c < count; c++) {
		colors[c].r = cmap[3 * c + 0];
		colors[c].g = cmap[3 * c + 1];
		colors[c].b = cmap[3 * c + 2];
	}

	SDL_SetPaletteColors(s_surface->format->palette, colors, start, 256);
}

static void sdl_wait_vsync(Raster* r)
{
	(void)r;

	/* SDL_BlitScaled doesn't work from 8 bit to screen,
	 * so I'm copying to a second buffer first and then
	 * doing my stretched blit. */
	SDL_BlitSurface(s_surface, NULL, s_buffer, NULL);

	/* With window scaling, we want to center the target rect. */
	SDL_Rect target_rect;
	const float aspect = ((float)s_window_surface->w) / ((float)s_window_surface->h);

	//!TODO: use aspect!

	const int width_is_longest = s_window_surface->w > s_window_surface->h ? 1 : 0;
	// converting to int here to deliberately round
	const int surf_scaled_w = s_surface->w * (s_window_surface->w / s_surface->w);
	const int surf_scaled_h = s_surface->h * (s_window_surface->h / s_surface->h);

	if (width_is_longest) {
		// width is greater -- height straight copy
		target_rect.y = 0;
		target_rect.h = s_window_surface->h;

//		float delta = (((float)s_window_surface->w) - surf_scaled_w) * 0.5;

		target_rect.x = (s_window_surface->w - surf_scaled_w) / 2;
		target_rect.w = surf_scaled_w;
	}
	else {
		// height is greater - width straight copy
		target_rect.x = 0;
		target_rect.w = s_window_surface->w;

//		float delta = (((float)s_window_surface->h) - surf_scaled_h) * 0.5;

		target_rect.y = (s_window_surface->h - surf_scaled_h) / 2;
		target_rect.h = surf_scaled_h;
	}

	/* Draw to the window surface, scaled */
	//!FIXME: Only clear the parts that aren't drawn from the scaled blit
	SDL_FillRect(s_window_surface, NULL, 0x000000);
	SDL_BlitScaled(s_buffer, &s_buffer->clip_rect, s_window_surface, &target_rect);

	SDL_UpdateWindowSurface(window);
	//	SDL_RenderClear(renderer);
	//	SDL_RenderCopy(renderer, texture, NULL, NULL);
	//	SDL_RenderPresent(renderer);
}

static Rastlib* get_sdl_lib(void)
{
	static Rastlib sdl_lib;
	static Boolean loaded = FALSE;

	if (!loaded) {
		pj_copy_bytes(pj_get_bytemap_lib(), &sdl_lib, sizeof(sdl_lib));

		sdl_lib.close_raster = sdl_close_rast;

		sdl_lib.set_colors = sdl_set_colors;
		sdl_lib.wait_vsync = sdl_wait_vsync;

		sdl_lib.get_rectpix = NULL; /* don't use bytemap call */
		sdl_lib.put_rectpix = NULL; /* don't use bytemap call */

		pj_set_grc_calls(&sdl_lib);

		loaded = TRUE;
	}

	return &sdl_lib;
}

/*--------------------------------------------------------------*/
/**
 * Function: sdl_open_vdriver
 *
 *  Based on pj_open_mcga_vdriver, pj__vdr_init_open, mcga_get_driver.
 */
static Errcode
sdl_open_vdriver(Vdevice **pvd)
{
	Vdevice *vd = &sdl_driver;

	sdl_driver.hdr.init = pj_errdo_success;
	*pvd = vd;

	if (vd->num_rtypes == 0) {
		pj_close_vdriver(pvd);
		return Err_driver_protocol;
	}

	vd->first_rtype = RT_FIRST_VDRIVER;
	vd->grclib = pj_get_grc_lib();

	return Success;
}

/**
 * Function: pj_open_ddriver
 */
Errcode
pj_open_ddriver(Vdevice **pvd, char *name)
{
	(void)name;
	return sdl_open_vdriver(pvd);
}
