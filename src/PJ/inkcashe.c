/* inkcashe.c - stuff for precomputing as much as possible for render_dot */

#include "errcodes.h"
#include "jimk.h"
#include "inks.h"
#include "render.h"
#include "util.h"

void free_render_cashes();

Rgb3 tcolor_src;

void free_ink_bhash(Ink *inky)
{
(void)inky;
free_bhash();
}

Errcode make_ink_bhash(Ink *inky)
{
(void)inky;
return(make_bhash());
}

/*******  Stuff for 'thash' which is used in glass ink non-dither mode.
 The basic idea is that since the ink strength and the current color are
 constant,  there are only COLORS values possible, one for each color
 in the destination screen. ********/

void free_ink_thash(Ink *inky)
{
	if (inky->dither)
		free_ink_bhash(inky);
	else
		pj_freez(&inky->inkdata);
}

Errcode make_ink_thash(Ink *inky)
{
unsigned size;

	if (inky->dither)
		return(make_ink_bhash(inky));
	size = COLORS*sizeof(Thash);
	if ((inky->inkdata = pj_malloc(size)) == NULL)
		return(Err_no_memory);
	clear_mem(inky->inkdata, size);
	return(Success);
}

Errcode make_tsp_cashe(Ink *inky)
{
get_color_rgb(inky->aid->ccolor, vb.pencel->cmap, &tcolor_src);
return(make_ink_thash(inky));
}

Errcode make_dar_cashe(Ink *inky)
{
tcolor_src.r = tcolor_src.g = tcolor_src.b = 0;
return(make_ink_thash(inky));
}

/************** Glow ink can get by with a table lookup on the destination
	 color.  */

Errcode make_glow_cashe(Ink *inky)
{
int i;
UBYTE *bun, c, c1;
UBYTE *dat;

if ((dat = inky->inkdata = pj_malloc(COLORS))== NULL)
	return(Err_no_memory);
for (i=0;i<COLORS;i++)
	dat[i]=i;
i = vs.buns[vs.use_bun].bun_count;
bun = vs.buns[vs.use_bun].bundle;
c1 = *bun;
while (--i > 0)
	{
	c = *bun++;
	dat[c] = *bun;
	}
dat[*bun] = c1;
return(Success);
}

void free_glow_cashe(Ink *inky)
{
pj_gentle_free(inky->inkdata);
inky->inkdata = NULL;
}

/* Spread cashes...
	Dithering spreads need to clear random seed for consistency from frame to
    frame. */
Errcode clear_random_cashe(Ink *inky)
{
	(void)inky;

	pj_srandom(1);
	return(Success);
}

/********* Dispatcher to inks' cashe set up and free routines *******/

void free_ink_cashes(Ink *inky)
{
	if (inky->free_cashe != NULL)
		(*inky->free_cashe)(inky);
	inky->needs &= ~INK_CASHE_MADE;
}

void free_render_cashes(void)
{
	free_ink_cashes(vl.ink);
}

static Errcode make_ink_cashes(Ink *inky)
{
	set_render_fast();
	if(inky->make_cashe != NULL)
	{
		inky->needs |= INK_CASHE_MADE;
		return((*inky->make_cashe)(inky));
	}
	return(Success);
}

Errcode make_render_cashes(void)
{
	return(make_ink_cashes(vl.ink));
}

