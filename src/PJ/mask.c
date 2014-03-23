/* mask.c - Routines to maintain and select our screen-sized bitplane
   used mostly for write-protecting pixels.  */

#include "jimk.h"
#include "auto.h"
#include "broadcas.h"
#include "errcodes.h"
#include "filemenu.h"
#include "menus.h"
#include "softmenu.h"

void free_mask(Bitmap *mask)
{
	pj_rast_free((Raster *)mask);
}
Errcode alloc_mask(Bitmap **mask,USHORT width,USHORT height)

/* allocates a single bit plane mask of width and height.
 * if either w or h is 0 it will use the the size of the penwndo */
{
Errcode err;
Rasthdr bmspec;

	copy_rasthdr(vb.pencel,&bmspec);
	bmspec.pdepth = 1; /* only one plane wanted */

	if(width)
		bmspec.width = width;
	if(height)
		bmspec.height = height;

	if((err = pj_alloc_bitmap(&bmspec,mask)) < 0)
	{
		*mask = NULL;
		return(softerr(err,"mask_alloc"));
	}
	return(0);
}
void free_the_mask(void)
{
	pj_rast_free((Raster *)mask_rast);
	mask_rast = NULL;
}
int alloc_the_mask(void)
{
	return(alloc_mask(&mask_rast,0,0));
}


static int newmask(void)
/* Free up old PJ mask plane and  allocate a new,  empty one */
{
Errcode err;
	free_the_mask();
	if((err = alloc_the_mask()) >= 0)
		pj_clear_rast(mask_rast);
	return(err);
}

int save_the_mask(char *name)
{
	/* note even though mask has no cmap save pic will work since 
	 * save_colors is FALSE */

	if(mask_rast != NULL)
		return(save_pic(name,(Rcel *)mask_rast,0,FALSE));
	return(Err_nogood);
}

int load_the_mask(char *name)
{
Errcode err;

	/* note even though mask has no cmap save pic will work since 
	 * load_colors is FALSE */

	if((err = newmask()) < 0)
		return(err);
	return(load_pic(name,(Rcel *)mask_rast,0,FALSE));
}


static void qcreate_mask()
{
	vs.make_mask = !vs.make_mask;
	if(vs.make_mask)
	{
		if(newmask() >= 0)
		{
			vs.make_mask = 1;
			vs.use_mask = 0;
		}
	}
}

static Boolean tog_mask()
{
	vs.use_mask = !vs.use_mask;
	if (vs.use_mask)
	{
		vs.make_mask = 0;
		if (mask_rast == NULL)
		{
			vs.use_mask = 0;
			return(0);
		}
	}
	do_rmode_redraw(RSTAT_MASK);
	return(1);
}
void mb_toggle_mask(Button *b)
{
	tog_mask();
	draw_button(b);
}

static Errcode
paste1_mask(void *data, int ix, int intween, int scale, Autoarg *aa)
{
	Errcode err;
	(void)data;
	(void)ix;
	(void)intween;
	(void)scale;
	(void)aa;

	if(mask_rast == NULL)
		return(Err_nogood);

	set_full_gradrect();
	if((err = make_render_cashes()) >= 0)
	{
		render_mask_blit(mask_rast->bm.bp[0], mask_rast->bm.bpr, 0, 0, 
						 vb.pencel, 0, 0, 
						 mask_rast->width, mask_rast->height );
		free_render_cashes();
	}
	return(err);
}


static void blitmask(Pixel color)
{
 	pj_mask1blit(mask_rast->bm.bp[0],mask_rast->bm.bpr,0,0,
			vb.pencel,0,0,mask_rast->width,mask_rast->height,color);
	zoom_it();
}

static void qshow_mask()
{
Errcode err;
int twocolor;
Cmap *cmap;
Rgb3 *grey;
Rgb3 *white;

	if(mask_rast == NULL)
		return;

	if((err = pj_cmap_alloc(&cmap,vb.pencel->cmap->num_colors)) < Success)
	{
		errline(err,NULL);
		return;
	}

	grey = &vb.screen->mc_ideals[MC_GREY];
	white = &vb.screen->mc_ideals[MC_WHITE];

	save_undo();
	find_colors();
	hide_mouse();

	/* build a little cmap with all colors white but mask color */

	stuff_cmap(cmap,white);
	set_color_rgb(grey, sgrey, cmap);
	set_color_rgb(grey, sgrey, vb.pencel->cmap); /* make sure we have this */ 
	blitmask(sgrey);

	/* on pen clicks toggle cmaps to reveal of hide the picture around the 
	 * mask right click or key breaks loop */

	twocolor = TRUE;
	for(;;)
	{
		if(twocolor)
			pj_cmap_load(vb.pencel, cmap);
		else
			pj_cmap_load(vb.pencel, vb.pencel->cmap);

		wait_click();
		if(!JSTHIT(MBPEN))
			break;
		twocolor = !twocolor;
	}
	pj_cmap_free(cmap);
	zoom_unundo();
	pj_cmap_load(vb.pencel, vb.pencel->cmap);
	show_mouse();
}


static void qgrab_mask()

/* this assumes the mask is the same dimension as the vb.pencel */
{
UBYTE *hline;
UBYTE *hbyte;
int numpix;
SHORT liney;
UBYTE mbit;
UBYTE mbits;
UBYTE *mbyte;
UBYTE *mplane;

	if(newmask() < 0)
		return;

	if((hline = pj_malloc(vb.pencel->width)) == NULL)
		goto error;

	mplane = mask_rast->bm.bp[0];

	for(liney = 0;liney < vb.pencel->height;++liney)
	{
		pj__get_hseg(vb.pencel,hline,0,liney,vb.pencel->width);
		mbit = 0x80;
		mbits = 0;
		mbyte = mplane;
		mplane += mask_rast->bm.bpr;
		hbyte = hline;

		for(numpix = vb.pencel->width;numpix > 0;--numpix)
		{
			if(*hbyte++ != vs.inks[0]) 
				mbits |= mbit;

			if((mbit >>= 1) == 0)
			{
				*mbyte++ = mbits;
				mbits = 0;
				mbit = 0x80;
			}
		}
	}
	pj_free(hline);
	qshow_mask();
	return;
error:
	free_the_mask();
}


static void do_qfunc(VFUNC gfunc, Boolean keep_undo)
{
Rcel_save undosave;

	if(keep_undo)
	{
		if(report_temp_save_rcel(&undosave,undof) < Success)
			return;
	}
	(*gfunc)();
	if(keep_undo)
	{
		report_temp_restore_rcel(&undosave,undof);
	}
}
static void qinvert_mask()
{
	pj_xor_rect(mask_rast,1,   /* xor plane [0] or mask with color 1 */
		      0, 0, mask_rast->width, mask_rast->height);
	qshow_mask();
}

static void do_qmask(Boolean keep_undo)
{
int choice;
USHORT mdis[9];

	hide_mp();
	flx_clear_olays();

	for (;;)
	{
	/* set up asterisks and disables */
		clear_mem(mdis, sizeof(mdis));

		if (mask_rast == NULL)
			mdis[0] = mdis[3] = mdis[4] = mdis[5] = mdis[6] = QCF_DISABLED;
		if(keep_undo || flxtime_data.draw_overlays != NULL)
			mdis[5] |= QCF_DISABLED; /* no pasting !! */
		if (vs.use_mask)
			mdis[0] |= QCF_ASTERISK;
		if (vs.make_mask)
			mdis[1] |= QCF_ASTERISK;
		choice = soft_qchoice(mdis, "mask");
		switch (choice)
		{
			case 0:
				tog_mask();
			break;
			case 1:
				qcreate_mask();
				break;
			case 2:
				do_qfunc(qgrab_mask,keep_undo);
				break;
			case 3:
				do_qfunc(qinvert_mask,keep_undo);
				break;
			case 4:
				do_qfunc(qshow_mask,keep_undo);
				break;
			case 5:
				uzauto(paste1_mask, NULL);
				break;
			case 6:
				if (soft_yes_no_box("mask_del"))
				{
					free_the_mask();
					set_trd_maxmem();
					vs.make_mask = vs.use_mask = 0;
				}
				break;
			case 7:
				go_files(FTP_MASK);
				break;
			default:
				goto OUT;
		}
	}
OUT:
	flx_draw_olays();
	show_mp();
	return;
}
void qmask(void)
{
	/* this is a kludge. all the things that
	 * use overlays also need the undo buffer and pasting or altering the
	 * undo will make it out of sync */

	do_qmask(flxtime_data.draw_overlays != NULL);
}
void qmask_keep_undo(void)
{
	do_qmask(TRUE);
}
