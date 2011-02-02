
/* onepal.c - A routine to force all the frames of a FLIC to a reasonable
   intermediate palette that is the same for all frames */

#include "jimk.h"
#include "fli.h"
#include "auto.h"
#include "errcodes.h"

#define MAXC (21000)

static int ccount;

static int add_clist(UBYTE *uses, Rgb3 *clist, Rgb3 *cmap)
{
int cneeded;
Rgb3 *rgb;
int i;

cneeded = ccount + COLORS;
if (cneeded > MAXC)
	{
	soft_continu_box("many_colors");
	return(0);
	}
for (i=0; i<COLORS; ++i)
	{
	if (*uses++ != 0)
		{
		rgb = &cmap[i];
		if (!in_ctable(rgb, clist, ccount) )
			{
			clist[(unsigned)ccount] = *rgb;
			++ccount;
			}
		}
	}
return(1);
}

static int force1(Cmap *dcmap, int ix,int it,int scale,Autoarg *aa)
{
	soft_top_textf("!%d%d", "top_refit", aa->cur_frame, it - aa->cur_frame);
	refit_rcel(vb.pencel, dcmap, vb.pencel->cmap);
	pj_cmap_copy(dcmap, vb.pencel->cmap);
	see_cmap();
	return(0);
}

void fold_in_mucolors(Cmap *cmap, int first_free, Wscreen *s)

/* if there are any free colors, insure that menu colors are present if the
 * colors in the color map provided aren't good enough */
{
Rgb3 *mc_curr;
Rgb3 *mc_ideal;
Rgb3 *max_ideal;
Rgb3 *ctab;
int i;

	if(has_menu_colors(cmap, s))
		return;

	ctab = cmap->ctab;
	mc_ideal = s->mc_ideals;
	max_ideal = mc_ideal + NUM_MUCOLORS;
	mc_curr = ctab + FIRST_MUCOLOR;

	i = NUM_MUCOLORS;
	while(mc_ideal < max_ideal)
	{
		if(i++ >= first_free)
			*mc_curr = *mc_ideal;
		else
		{
			/* if color in cmap is present in prior part we can replace it
			 * but only if the ideal color is not already present */

			if(in_ctable(mc_curr,ctab,FIRST_MUCOLOR)
				&& !in_ctable(mc_ideal,ctab,FIRST_MUCOLOR))
			{
				*mc_curr = *mc_ideal;
			}
		}
		++mc_curr;
		++mc_ideal;
	}
}

void one_palette(void)
{
Errcode err;
Rgb3 *clist;
Cmap *dcmap;
char *cused;
int i;
int ok;
char buf[40];
Autoarg aa;

ok = 1;		/* this routine is an optimist */
push_most();
if (pj_cmap_alloc(&dcmap,COLORS) >= Success)
	{
	if ((cused = begmem(COLORS)) != NULL)
		{
		if((err = scrub_cur_frame()) >= Success)
			{
			if((clist = begmem(MAXC*sizeof(Rgb3))) != NULL)
				{
				ccount = 0;
				for (i=0; i<flix.hdr.frame_count; ++i)
					{
					if(unfli(vb.pencel, i, 1) < 0)
						{
						ok = 0;
						break;
						}
					make_cused(vb.pencel, cused, COLORS);
					if (!add_clist(cused, clist, vb.pencel->cmap->ctab))
						{
						ok = 0;
						break;
						}
					}
				if (ok)
					{
					find_colors();
					soft_top_textf("!%d", "top_cpack", ccount);
					clear_mem(dcmap->ctab, COLORS*sizeof(Rgb3));
					pack_ctable(clist,(long)ccount,dcmap->ctab,COLORS);
					if(ccount < COLORS)
						fold_in_mucolors(dcmap,ccount,vb.screen);
					}
				pj_free(clist);
				}
			}
		pj_free(cused);
		}
	if (ok)
		{
		clear_struct(&aa);
		aa.avec = force1;
		aa.avecdat = dcmap;
		noask_do_auto(&aa,DOAUTO_ALL);
		}
	pj_cmap_free(dcmap);
	}
cleanup_toptext();
pop_most();
fli_abs_tseek(vb.pencel,vs.frame_ix);
see_cmap();
save_undo();
}

