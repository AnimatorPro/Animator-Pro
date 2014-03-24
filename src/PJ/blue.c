/* blue.c - Stuff to implement much of the trace drop down */

#include "jimk.h"
#include "auto.h"
#include "blue.h"
#include "errcodes.h"
#include "fli.h"
#include "mask.h"

static Errcode blue1(void *data, int ix, int intween, int scale, Autoarg *aa)
/* blue pic stuff */
{
	(void)data;
	(void)ix;
	(void)intween;
	(void)scale;
	(void)aa;

	set_one_val(vb.pencel, vs.inks[0], vs.inks[1]);
	return Success;
}

void qblue_pic(void)
{
	uzauto(blue1, NULL);
}

static Errcode unblue1(void *data, int ix, int intween, int scale, Autoarg *aa)
/* unblue pic stuff */
{
	UBYTE table[COLORS];
	int i;
	(void)data;
	(void)ix;
	(void)intween;
	(void)scale;
	(void)aa;

	for (i=0; i<COLORS; i++)
		table[i] = i;
	table[vs.inks[1]] = vs.inks[0];
	xlat_rast(vb.pencel, table, 1);

	return Success;
}

void qunblue_pic(void)
{
	uzauto(unblue1, NULL);
}

static void bluesome(Rcel *src,		/* red frame */
					 Rcel *dest,	/* blue frame overwritten with composite */
					 Pixel ink)	    /* red color */

/*  Stuff for insert tween. src and dest must be same width */
{
UBYTE bink = ink;
UBYTE *spix;
UBYTE *dpix;
register UBYTE *s, *d;
SHORT y;
SHORT xcount;

	if((spix = begmem(src->width*2)) == NULL)
		return;

	dpix = spix + src->width;

	for(y=0;y < dest->height;++y)
	{
		xcount = src->width;
		pj__get_hseg(dest,dpix,0,y,xcount);
		pj__get_hseg(src,spix,0,y,xcount);

		s = spix;
		d = dpix;
		while(--xcount>=0)
		{
			if (*s++ == bink)
				*d = bink;
			++d;
		}
		pj__put_hseg(dest,dpix,0,y,src->width);
	}
	pj_free(spix);
}

void insert_tween(void)
{
int oix;

unzoom();

scrub_cur_frame();

/* Make undo buffer hold current frame, vga screen hold next screen */
oix = vs.frame_ix;
advance_frame_ix();
pj_rcel_copy(vb.pencel, undof);
unfli(vb.pencel, vs.frame_ix, 1);

/* set next screen to color 2 (a dark red) */
set_one_val(vb.pencel, vs.inks[0], vs.inks[2]);


/* set this screen to color 1 */
set_one_val(undof, vs.inks[0], vs.inks[1]);


/* superimpose future on present */
bluesome(vb.pencel, undof, (Pixel)vs.inks[2]);

/* make combination current screen */
pj_rcel_copy(undof, vb.pencel);
see_cmap();

/* and insert frame into FLI */
insert_frames(1, oix);

/* leave frame position one past original */
vs.frame_ix =oix;
advance_frame_ix();

/* Make sure we update FLI when leave this frame */
dirties();

rezoom();
}

/* stuff for remove guides */
static Errcode clean_t1(void *data, int ix, int intween, int scale, Autoarg *aa)
{
	UBYTE table[COLORS];
	int i;
	(void)data;
	(void)ix;
	(void)intween;
	(void)scale;
	(void)aa;

	for (i=0; i<COLORS; i++)
		table[i] = i;
	table[vs.inks[1]] = vs.inks[0];
	table[vs.inks[2]] = vs.inks[0];
	xlat_rast(vb.pencel, table, 1);

	return Success;
}

void clean_tween(void)
{
	uzauto(clean_t1, NULL);
}

static void cmask_line(UBYTE *line1, UBYTE *line2,
					   Bitmap *mask, SHORT y)
{
int width;
UBYTE *mbyte;
UBYTE mbit;
UBYTE mbits;

	mbyte = mask->bm.bp[0] + (y * mask->bm.bpr);
	mbit = 0x80;
	mbits = 0;

	width = mask->width;
	while(width-- > 0)
	{
		if(*line1++ != *line2++)
			mbits |= mbit;

		if((mbit >>= 1) == 0x00)
		{
			*mbyte++ = mbits;
			mbit = 0x80;
			mbits = 0;
		}
	}
}
static Errcode build_change_mask(Rcel *s1, Rcel *s2, Bitmap *mask)

/* loads mask with changes between s1 and s2 it is assumed that the
 * dimentions of s1,s2 and mask are the same If out of heap will make a 
 * zero change mask and return Ecode */
{
UBYTE *line1;
UBYTE *line2;
SHORT linesize;
SHORT liney;

	pj_clear_rast(mask);

	linesize = s1->width;

	if((line1 = pj_malloc(linesize * 2)) == NULL)
		return(Err_no_memory);

    line2 = line1 + linesize;
	for(liney = 0;liney < s1->height;++liney)
	{
		pj__get_hseg(s1,line1,0,liney,s1->width);
		pj__get_hseg(s2,line2,0,liney,s2->width);
		cmask_line(line1,line2,mask,liney);
	}
	pj_free(line1);
	return(0);
}

static void cupdate_line(UBYTE *newline,UBYTE *oldline,UBYTE *destline,
					     Bitmap *mask, SHORT y)
{
int width;
UBYTE *mbyte;
UBYTE mbit;
UBYTE mbits;

	mbyte = mask->bm.bp[0] + (y * mask->bm.bpr);
	mbit = 0;

	width = mask->width;
	while(width-- > 0)
	{
		if((mbit >>= 1) == 0x00)
		{
			mbits = *mbyte++;
			mbit = 0x80;
		}
		if(mbits & mbit)
		{
			*destline++ = *newline++;
			++oldline;
		}
		else
		{
			*destline++ = *oldline++;
			++newline;
		}
	}
}
static int update_changes(Rcel *new, Rcel *old, Rcel *dest,
						   Bitmap *mask)

/* this assumes new, old, dest, and mask are the same dimensions */
{
UBYTE *newline;
UBYTE *oldline;
UBYTE *destline;
SHORT linesize;
SHORT liney;

	linesize = dest->width;

	if((newline = pj_malloc(linesize * 3)) == NULL)
		return(Err_no_memory);

    oldline = newline + linesize;
	destline = oldline + linesize; 
	for(liney = 0;liney < dest->height;++liney)
	{
		pj__get_hseg(new,newline,0,liney,linesize);
		pj__get_hseg(old,oldline,0,liney,linesize);
		cupdate_line(newline,oldline,destline,mask,liney);
		pj__put_hseg(dest,destline,0,liney,linesize);
	}
	pj_free(newline);
	return(0);
}

static void restore_blue(Rcel *src,
						 Rcel *dest,
						 Pixel ink)

/*  Stuff for insert tween. src and dest must be same width */
{
UBYTE bink = ink;
UBYTE *spix;
UBYTE *dpix;
register UBYTE *s, *d;
SHORT y;
SHORT xcount;

	if((spix = begmem(src->width*2)) == NULL)
		return;

	dpix = spix + src->width;

	for(y=0;y < dest->height;++y)
	{
		xcount = src->width;
		pj__get_hseg(dest,dpix,0,y,xcount);
		pj__get_hseg(src,spix,0,y,xcount);

		s = spix;
		d = dpix;
		while(xcount-- > 0)
		{
			if (*d == bink)
				*d = *s;
			++s;
			++d;
		}
		pj__put_hseg(dest,dpix,0,y,src->width);
	}
	pj_free(spix);
}


static void next_bc(int blue) /* blue it or copy changes */
/* Either do repeat changes or next blue */
{
Bitmap *mask;
int err, oix;
Rcel_save opic;

	oix = vs.frame_ix;
	unzoom();
	maybe_push_most();
	fli_abs_tseek(undof,vs.frame_ix);
	if (blue)
		restore_blue(undof, vb.pencel, (Pixel)vs.inks[1]);

	if((alloc_mask(&mask,0,0)) >= 0)
	{
		if (report_temp_save_rcel(&opic, vb.pencel) >= 0)
		{
			build_change_mask(undof, vb.pencel, mask);
			scrub_cur_frame();
			advance_frame_ix();
			pj_rcel_copy(vb.pencel, undof);
			err = unfli(undof,vs.frame_ix,0);
			report_temp_restore_rcel(&opic, vb.pencel);
			if(err >= 0)
			{
				if(blue) /* make changes blue in render form */
				{
					pj_rcel_copy(undof, vb.pencel);
					 	pj_mask1blit(mask->bm.bp[0],mask->bm.bpr,0,0,vb.pencel,
								  0,0,mask->width,mask->height,vs.inks[1]);
				}
				else
				{
					update_changes(vb.pencel, undof, vb.pencel, mask);
					pj_cmap_copy(undof->cmap, vb.pencel->cmap);
				}
				dirties();
			}
			else
				vs.frame_ix = oix;
			free_mask(mask);
		}
	}
	maybe_pop_most();
	see_cmap();
	rezoom();
}

void qnext_changes(void)
{
	next_bc(0);
}

void qnext_blue(void)
{
	next_bc(1);
}

