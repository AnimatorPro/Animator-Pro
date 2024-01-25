#include <stdio.h>
#include "gfxlib.h"
#include "ptrmacro.h"
#include "makehdr.c"

char fmt1[] = "gfxl_%s equ +0%xH\n";

#define gloset(f) outf(fmt1,#f,OFFSET(Gfxlib,f))

main(int argc,char **argv)
{
	openit(argc,argv);
	gloset(pj_clipbox_make);
	gloset(close_raster);
	gloset(pj_rast_free);
	gloset(put_hseg);
	gloset(get_hseg);
	gloset(put_vseg);
	gloset(get_vseg);
	gloset(put_rectpix);
	gloset(get_rectpix);
	gloset(set_hline);
	gloset(set_vline);
	gloset(pj_clear_rast);
	gloset(set_rect);
	gloset(xor_rect);
	gloset(mask1blit);
	gloset(mask2blit);
	gloset(xor_rast);
	gloset(_blitrect);
	gloset(blitrect);
	gloset(_tblitrect);
	gloset(tblitrect);
	gloset(_swaprect);
	gloset(swaprect);
	gloset(zoomblit);

	gloset(pj_cmap_alloc);
	gloset(pj_cmap_free);
	gloset(pj_cmap_load);
	gloset(pj_cmap_copy);

	gloset(pj_rcel_bytemap_open);
	gloset(pj_rcel_bytemap_alloc);
	gloset(pj_rcel_make_virtual);

	gloset(pj_rcel_close);
	gloset(pj_rcel_free);
	closeit();
}
