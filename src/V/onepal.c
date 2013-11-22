
/* onepal.c - A routine to force all the frames of a FLIC to a reasonable
   intermediate palette that is the same for all frames */

#include "jimk.h"
#include "fli.h"
#include "onepal.str"

#define MAXC (21000)

static UBYTE *clist;
static int ccount;
static UBYTE *dcmap;


static
add_clist(uses, cmap)
UBYTE *uses, *cmap;
{
int cneeded;
UBYTE *rgb;
int i;

cneeded = ccount + 256;
if (cneeded > MAXC)
	{
	continu_line(onepal_100 /* "Too many colors, try fewer frames." */);
	return(0);
	}
for (i=0; i<COLORS; i++)
	{
	if (*uses++ != 0)
		{
		rgb = cmap + i*3;
		if (!in_cmap(rgb, clist, ccount) )
			{
			copy_bytes(rgb,clist+3*(unsigned)ccount,3);
			ccount+=1;
			}
		}
	}
return(1);
}

static
force1()
{
refit_screen(render_form, dcmap, render_form->cmap, COLORS);
copy_cmap(dcmap, render_form->cmap);
see_cmap();
return(1);
}

one_palette()
{
char *cused;
int i;
int ok;
char buf[40];

ok = 1;		/* this routine is an optimist */
push_most();
if ((dcmap = begmem(COLORS*3)) != NULL)
	{
	if ((cused = begmem(COLORS)) != NULL)
		{
		if (scrub_cur_frame())
			{
			if ((clist = begmem(MAXC*3)) != NULL)
				{
				ccount = 0;
				for (i=0; i<fhead.frame_count; i++)
					{
					if (!unfli(render_form, i, 1))
						{
						ok = 0;
						break;
						}
					make_cused(render_form->p, cused);
					if (!add_clist(cused, render_form->cmap))
						{
						ok = 0;
						break;
						}
					}
				if (ok)
					{
					find_colors();
					sprintf(buf, onepal_101 /* "%d total colors used, packing..." */, ccount);
					stext(buf, 0, 0, sblack, swhite);
					zero_structure(dcmap, COLORS*3);
					pack_cmap(clist,(long)ccount,dcmap,COLORS);
					}
				freemem(clist);
				}
			}
		freemem(cused);
		}
	if (ok)
		{
		dauto(force1, 2);
		}
	freemem(dcmap);
	}
pop_most();
fli_abs_tseek(render_form,vs.frame_ix);
see_cmap();
save_undo();
}

