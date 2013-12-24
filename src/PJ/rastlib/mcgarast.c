#include "errcodes.h"
#include "memory.h"
#include "rastlib.h"
#include "cmap.h"


extern void *pj_get_bytemap_lib();

extern void pj_mcga_put_dot(Bytemap *v, Pixel color, Coor x, Coor y);
extern void pj_mcga_get_dot(Bytemap *v, Pixel color, Coor x, Coor y);
extern void pj_mcga_cput_dot(Bytemap *v, Pixel color, Coor x, Coor y);
extern void pj_mcga_cget_dot(Bytemap *v, Pixel color, Coor x, Coor y);

void pj_mcga_get_hseg(Bytemap *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor width);
void pj_mcga_put_hseg(Bytemap *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor width);

void pj_mcga_set_colors(Bytemap *v, int startix, int count, Rgb3 *cmap);
void pj_mcga_uncc64(Bytemap *v, void *cbuf);
void pj_mcga_uncc256(Bytemap *v, void *cbuf);

static Errcode mcga_close_rast(McgaRast *r)
{
	return(Success);
}
static void mcga_wait_vsync(Raster *r)
{
	pj_wait_vsync();
}
static void *get_mcga_lib(void)
{
static Rastlib mcga_lib;
static int loaded = 0;

	if(!loaded)
	{

		/*--------------------------------------------------------------------
		 * first plug in the entire bytemap lib...
		 *------------------------------------------------------------------*/

		pj_copy_bytes(pj_get_bytemap_lib(),&mcga_lib,sizeof(mcga_lib));

		/*--------------------------------------------------------------------
		 * then override things we have specific code for...
		 *------------------------------------------------------------------*/

		mcga_lib.close_raster = (rl_type_close_raster)mcga_close_rast;

		mcga_lib.put_dot  = (rl_type_put_dot)pj_mcga_put_dot;
		mcga_lib.cput_dot = (rl_type_cput_dot)pj_mcga_cput_dot;
		mcga_lib.get_dot  = (rl_type_get_dot)pj_mcga_get_dot;
		mcga_lib.cget_dot = (rl_type_cget_dot)pj_mcga_cget_dot;

		mcga_lib.get_hseg = (rl_type_get_hseg)pj_mcga_get_hseg;
		mcga_lib.put_hseg = (rl_type_put_hseg)pj_mcga_put_hseg;

		mcga_lib.set_colors = (rl_type_set_colors)pj_mcga_set_colors;
		mcga_lib.uncc64 	= (rl_type_uncc64)pj_mcga_uncc64;
		mcga_lib.uncc256	= (rl_type_uncc256)pj_mcga_uncc256;
		mcga_lib.wait_vsync = (rl_type_wait_vsync)mcga_wait_vsync;

		mcga_lib.get_rectpix = NULL; /* don't use bytemap call */
		mcga_lib.put_rectpix = NULL; /* don't use bytemap call */

		pj_set_grc_calls(&mcga_lib);

		loaded = 1;
	}
	return(&mcga_lib);
}

Errcode pj_open_mcga_raster(Raster *r)
/* assuming display is already set up this will load a raster to represent a
 * vanilla mcga display card as a 320 X 200 raster */
{
#define VGA_SCREEN ((void *)0xa0000)

static Rasthdr defaults = {
	RT_MCGA,	 /* type */
	8,			 /* effective bit depth */
	NULL,		 /* lib */
	6,5,		 /* aspect ratio */
	0,0,		 /* reserved */
	320,200,0,0  /* w,h,x,y rectangle */
};
static Bmap bm = {
	0,						/* realmem seg descriptor, filled in at runtime */
	1,						/* number of bplanes at least 1 */
	Bytemap_bpr(320),		/* rowbytes */
	Byteplane_size(320,200),/* size of plane (saves code) */
	{ VGA_SCREEN }, 		/* at least one plane, the pixelated data */
};
McgaRast *mr = (McgaRast *)r;

	*((Rasthdr *)mr) = defaults;
	mr->hw.bm = bm;
	mr->hw.bm.segment = pj_set_gs();
	mr->lib = get_mcga_lib();
	return(Success);

#undef VGA_SCREEN
}
