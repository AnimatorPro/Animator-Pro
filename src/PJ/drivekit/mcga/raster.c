#include "mcga.h"

/* Assembly language function C prototypes */
void mcga_wait_vsync(McgaRast *r);
Pixel pj_mcga_get_dot(McgaRast *r,Coor x,Coor y);
void pj_mcga_put_dot(McgaRast *r,Pixel color,Coor x, Coor y);
void pj_mcga_cput_dot(McgaRast *r,Pixel color,Coor x, Coor y);
void pj_mcga_set_colors(McgaRast *r,LONG start, LONG count, void *cmap);
void mcga_get_hseg(McgaRast *r,Pixel *buf, Ucoor x, Ucoor y, Ucoor width);
void mcga_get_vseg(McgaRast *r,Pixel *buf, Ucoor x, Ucoor y, Ucoor height);
void mcga_put_hseg(McgaRast *r,Pixel *buf, Ucoor x, Ucoor y, Ucoor width);
void mcga_d_hline(McgaRast *r, Pixel color, Coor x, Coor y, Ucoor width);
void mcga_mask1blit(UBYTE *mbytes, LONG mbpr, Coor mx, Coor my,
			   McgaRast *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
			   Pixel oncolor );
void mcga_mask2blit(UBYTE *mbytes, LONG mbpr, Coor mx, Coor my,
			   McgaRast *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
			   Pixel oncolor, Pixel offcolor );
void blit(McgaRast *source, Coor src_x, Coor src_y,
			 McgaRast *dest, Coor dest_x, Coor dest_y,
			 Coor width, Coor height);
void tblit(McgaRast *s, Coor sx, Coor sy,
		  McgaRast *d, Coor dx, Coor dy, Coor width, Coor height,
		  Pixel tcolor );
void mcga_set_rect(McgaRast *r,Pixel color,Coor x,Coor y,Ucoor w,Ucoor h);
void mcga_xor_rect(McgaRast *r,Pixel color,Coor x,Coor y,Ucoor w,Ucoor h);
void pj_unbrun(McgaRast *r,void *ucbuf, LONG pixsize,
					 Coor x,Coor y,Ucoor width,Ucoor height);
void pj_unlccomp(McgaRast *r,void *ucbuf, LONG pixsize,
					 Coor x,Coor y,Ucoor width,Ucoor height);
void pj_mcga_uncc64(McgaRast *r, void *ucbuf);
void pj_mcga_uncc256(McgaRast *r, void *ucbuf);
void pj_unss2(void *cbuf, UBYTE *screen, long bpr, int screen_seg, int width);

static void unss2_rect(McgaRast *r,void *ucbuf, LONG pixsize,
				   LONG x,LONG y,ULONG width,ULONG height)
/* a little shell to main delta decompression routine. */
{
/* convert x,y starting point to address offset in byte-plane */
	pj_unss2(ucbuf, r->hw.bm.bp[0] + y*r->hw.bm.bpr + x, 
			 r->hw.bm.bpr, r->hw.bm.segment, width);
}

static struct rastlib mcga_raster_library;

struct rastlib *mcga_get_rlib(Vdevice *dev, int mode, McgaRast *r)
{
static got_lib = 0;

if (!got_lib)
	{
	mcga_raster_library.set_colors = pj_mcga_set_colors;
	mcga_raster_library.put_dot = pj_mcga_put_dot;
	mcga_raster_library.cput_dot = pj_mcga_cput_dot;
	mcga_raster_library.get_dot = pj_mcga_get_dot;
	mcga_raster_library.set_hline = mcga_d_hline;
	mcga_raster_library.set_rect = mcga_set_rect;
	mcga_raster_library.xor_rect = mcga_xor_rect;
	mcga_raster_library.get_hseg = mcga_get_hseg;
	mcga_raster_library.put_hseg = mcga_put_hseg;
	mcga_raster_library.mask1blit = mcga_mask1blit;
	mcga_raster_library.mask2blit = mcga_mask2blit;
	mcga_raster_library.blitrect[RL_TO_SAME] = blit;
	mcga_raster_library.blitrect[RL_TO_BYTEMAP] = blit;
	mcga_raster_library.blitrect[RL_FROM_BYTEMAP] = blit;
	mcga_raster_library.tblitrect[RL_TO_SAME] = tblit;
	mcga_raster_library.tblitrect[RL_TO_BYTEMAP] = tblit;
	mcga_raster_library.tblitrect[RL_FROM_BYTEMAP] = tblit;
	mcga_raster_library.get_vseg = mcga_get_vseg;
	mcga_raster_library.unbrun_rect = pj_unbrun;
	mcga_raster_library.unlccomp_rect = pj_unlccomp;
	mcga_raster_library.uncc64 = pj_mcga_uncc64;
	mcga_raster_library.uncc256 = pj_mcga_uncc256;
   	mcga_raster_library.unss2_rect = unss2_rect; 
	mcga_raster_library.wait_vsync = mcga_wait_vsync;
	got_lib = 1;
	}
return(&mcga_raster_library);
}

