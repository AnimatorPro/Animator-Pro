/* vdr_sdl.c */

#include <assert.h>
#include <SDL/SDL.h>
#define VDEV_INTERNALS
#include "errcodes.h"
#include "libdummy.h"
#include "memory.h"
#include "ptrmacro.h"
#include "rastcall.h"
#include "rastlib.h"
#include "vdevcall.h"

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

static Vmode_info sdl_infos[1] = {
	{
		sizeof(Vmode_info),
		0, /* mode_ix */
		"SDL",
		8, /* bits */
		1, /* planes */
		{320,320,320,1}, /* width */
		{200,200,200,1}, /* height */
		TRUE, TRUE, /* readable, writeable */
		TRUE, /* displayable */
		0, /* fields_per_frame */
		1, /* display_pages */
		1, /* store_pages */
		320*200, /* display_bytes */
		320*200, /* store_bytes */
		TRUE, /* palette_vblank_only */
		0, /* screen_swap_vblank_only */
		70, /* field_rate */
		0 /* vblank_period */
	}
};

static Vdevice sdl_driver = {
	{ REX_VDRIVER, VDEV_VERSION, NULL, NULL, NULL, NULL, NULL }, /* hdr */
	0, /* first_rtype */
	1, /* num_rtypes */
	Array_els(sdl_infos), /* mode_count */
	sizeof(Vdevice_lib) / sizeof(int(*)()), /* dev_lib_count */
	&sdl_device_library, /* lib */
	NULL, /* grclib */
	NUM_LIB_CALLS, /* rast_lib_count */
};

/* TODO: should be part of sdl_raster. */
static SDL_Surface *s_surface;

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

static void
sdl_open_raster(Raster *r)
{
	static const Rasthdr defaults = {
		RT_MCGA, /* type */
		8, /* effective bit depth */
		NULL, /* lib */
		6,5, /* aspect ratio */
		{0,0}, /* reserved */
		320,200,0,0 /* w,h,x,y rectangle */
	};
	static const Bmap bm = {
		0, /* realmem seg descriptor, filled in at runtime */
		1, /* number of bplanes at least 1 */
		Bytemap_bpr(320), /* rowbytes */
		Byteplane_size(320,200), /* size of plane (saves code) */
		{ NULL }, /* at least one plane, the pixelated data */
	};

	*((Rasthdr *)r) = defaults;
	r->hw.bm = bm;
	r->hw.bm.segment = 0;
	r->lib = get_sdl_lib();
}

static Errcode
sdl_open_graphics(Vdevice *vd, Raster *r, LONG w, LONG h, USHORT mode)
{
	(void)mode;

	if (w != 320 || h != 200)
		return Err_wrong_res;

	s_surface = SDL_SetVideoMode(w, h, 8, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if (s_surface == NULL)
		return Err_no_display;

	SDL_WM_SetCaption("PJ Paint", NULL);

	sdl_open_raster(r);
	r->hw.bm.bp[0] = s_surface->pixels;
	r->type = vd->first_rtype;

	return Success;
}

static Errcode sdl_close_graphics(Vdevice *vd)
{
	(void)vd;
	return Success;
}

/*--------------------------------------------------------------*/
/* SDL Rastlib.                                                 */
/*--------------------------------------------------------------*/

static Errcode
sdl_close_rast(Raster *r)
{
	(void)r;
	return Success;
}

static void
sdl_set_colors(Raster *r, LONG start, LONG count, void *cbuf)
{
	const uint8_t *cmap = cbuf;
	SDL_Colour col[256];
	int c;
	(void)r;
	assert(0 < count && count <= 256);

	for (c = 0; c < count; c++) {
		col[c].r = cmap[3 * c + 0];
		col[c].g = cmap[3 * c + 1];
		col[c].b = cmap[3 * c + 2];
	}

	SDL_SetPalette(s_surface, SDL_LOGPAL | SDL_PHYSPAL, col, start, count);
}

static void
sdl_wait_vsync(Raster *r)
{
	/* TODO: dodgy. */
	(void)r;
	SDL_Delay(5);
	SDL_Flip(s_surface);
}

static Rastlib *
get_sdl_lib(void)
{
	static Rastlib sdl_lib;
	static Boolean loaded = FALSE;

	if (!loaded) {
		pj_copy_bytes(pj_get_bytemap_lib(), &sdl_lib, sizeof(sdl_lib));

		sdl_lib.close_raster = sdl_close_rast;

		sdl_lib.set_colors  = sdl_set_colors;
		sdl_lib.wait_vsync  = sdl_wait_vsync;

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
