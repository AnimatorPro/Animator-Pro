/*****************************************************************************
 * TGAREADF.C - Routines to read headers and data from a targa file.
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "ffile.h"
#include "memory.h"
#include "targa.h"

static Errcode read_targa_ctab(Targa_file *tf)
/*****************************************************************************
 * read the color palette from the targa file into our screen cmap.
 ****************************************************************************/
{
	Errcode err;
	Cmap	*pcmap;
	Rgb3	*ctab;
	int 	len = tf->tgahdr.tcmdata.mapcnt;

	pcmap = tf->screen_rcel->cmap;

	if (tf->is_grey)	/* greyscales are easy */
		{
		ctab = pcmap->ctab;
		for (len = 0; len < 256; ++len)
			{
			ctab->r = ctab->g = ctab->b = len;
			++ctab;
			}
		return(Success);
		}

	if (tf->tgahdr.tcmdata.mapbits != 24)
		{
		return Err_version;
		}

	if (len + tf->tgahdr.tcmdata.mapidx > COLORS) /* Make sure color map fits
												   * in our buffer */
		{
		return Err_version; 					   
		}

	memset(pcmap->ctab, 0, sizeof(pcmap->ctab));

	fseek(tf->file, tf->tgahdr.idlength+sizeof(tf->tgahdr), SEEK_SET);

	ctab = pcmap->ctab + tf->tgahdr.tcmdata.mapidx;
	while (--len >= 0)
		{
		ctab->b = getc(tf->file);
		ctab->g = getc(tf->file);
		ctab->r = getc(tf->file);
		++ctab;
		}

	pj_cmap_load(tf->screen_rcel, pcmap);

	err = Success;

	return err;
}

Errcode read_targa_header(Targa_file *tf)
/*****************************************************************************
 * Open file, read in and verify header.  Set up line buffers and precompute
 * some values for the control block.
 ****************************************************************************/
{
	Tgaheader	*th = &tf->tgahdr;
	Imgspec 	*im = &tf->tgahdr.imgdata;

	if (1 != fread(th, sizeof(*th), 1, tf->file))
		return pj_errno_errcode();

	/*------------------------------------------------------------------------
	 *
	 *----------------------------------------------------------------------*/

	switch (th->imgtype)
		{
		case MAPPED_IMAGE:
			tf->is_rgb = FALSE;
			tf->is_compressed = FALSE;
			tf->is_grey = FALSE;
			break;
		case RL_MAPPED_IMAGE:
			tf->is_rgb = FALSE;
			tf->is_compressed = TRUE;
			tf->is_grey = FALSE;
			break;
		case RGB_IMAGE:
			tf->is_rgb = TRUE;
			tf->is_compressed = FALSE;
			tf->is_grey = FALSE;
			break;
		case RL_RGB_IMAGE:
			tf->is_rgb = TRUE;
			tf->is_compressed = TRUE;
			tf->is_grey = FALSE;
			break;
		case BW_IMAGE:
			tf->is_rgb = FALSE;
			tf->is_compressed = FALSE;
			tf->is_grey = TRUE;
			break;
		case RL_BW_IMAGE:
			tf->is_rgb = FALSE;
			tf->is_compressed = TRUE;
			tf->is_grey = TRUE;
			break;
		case RDH_IMAGE:
		case RDHBLK_IMAGE:
			{
			return Err_version;
			}
		default:
			return Err_pic_unknown;
		}

	switch (th->maptype)
		{
		case MAP_NONE:
		case MAP_LUT:
			break;
		default:
			return Err_pic_unknown;
		}

	if (tf->is_rgb && im->pixsiz < 16)
		return Err_pic_unknown;
	else if (!tf->is_rgb && im->pixsiz > 8)
		return Err_pdepth_not_avail;

	switch (im->pixsiz)
		{
		case  8:
			tf->bpp = 1;
			break;
		case 16:
			tf->bpp = 2;
			break;
		case 24:
			tf->bpp = 3;
			break;
		case 32:
			tf->bpp = 4;
			break;
		default:
			return Err_pdepth_not_avail;
		}

	if ((im->imgdesc & INTLEVMASK) != 0)
		{
		return Err_version;
		}

	/*------------------------------------------------------------------------
	 *
	 *----------------------------------------------------------------------*/

	tf->width		= im->width;
	tf->height		= im->height;
	tf->pdepth		= im->pixsiz;
	tf->is_flipped	= ((im->imgdesc & SCRORG) == SCRORG) ? 0 : 1;
	tf->data_offset = th->idlength + sizeof(tf->tgahdr);
	if (th->maptype == MAP_LUT)
		tf->data_offset += ((th->tcmdata.mapbits+7)/8)*th->tcmdata.mapcnt;

	tf->bpr = tf->width * tf->bpp;
	if (NULL == (tf->lbuf = pj_malloc(tf->bpr+130*tf->bpp)))
		return Err_no_memory;

	tf->over = tf->lbuf + tf->bpr; /* extra at end to make decompression easier */

	return Success;
}

Errcode read_nextline(Image_file *ifile, Rgb3 *rgbbuf)
/*****************************************************************************
 * Read next line from file into tf->rgb_bufs[]  (where it'll be stored
 * as 8 bit color components.)
 ****************************************************************************/
{
	Targa_file *tf = (Targa_file *)ifile;
	int 	i, lpos;
	USHORT	*w;
	SHORT	ww;
	UBYTE	*p;
	UBYTE	pbuf[4];		  /* 4 is max possible bytes per pixel */
	int 	bpp = tf->bpp;
	int 	bpr = tf->bpr;

	if (tf->cury >= tf->height) 	/* oops, out of data   */
		return Err_truncated;		/* should never happen */

	/*------------------------------------------------------------------------
	 * first load (and if needed, decompress) a line of data...
	 *----------------------------------------------------------------------*/

	if (tf->is_compressed)
		{
		/* first deal with any overflow from last line */
		memcpy(tf->lbuf, tf->over, tf->over_count);
		lpos = bpr - tf->over_count;
		p = tf->lbuf + tf->over_count;
		while (lpos > 0)
			{
			if (1 != fread(&pbuf[0], sizeof(pbuf[0]), 1, tf->file))
				return Err_truncated;
			ww = pbuf[0];
			if (ww&0x0080)	  /* it's a run dude */
				{
				ww = (ww&0x007F)+1;   /* length of run - 1*/
				if (1 != fread(pbuf, bpp, 1, tf->file)) /* get data to repeat */
					return Err_truncated;
				while (--ww >= 0)
					{
					memcpy(p, pbuf, bpp);
					p += bpp;
					lpos -= bpp;
					}
				}
			else		   /* This bit is just raw data */
				{
				i = (ww+1)*bpp; /* length in bytes */
				if (1 != fread(p, i, 1, tf->file))
					return Err_truncated;
				p += i;
				lpos -= i;
				}
			}
		tf->over_count = -lpos;
		}
	else /* not compressed, just read in a line of data */
		{
		if (1 != fread(tf->lbuf, bpr, 1, tf->file))
			return Err_truncated;
		}

	/*------------------------------------------------------------------------
	 * now transform the targa color samples (stored in 16, 24, or 32 bit
	 * pixels, in bgrbgr... order) into the rgbrgb... format CONVERT eats.
	 *----------------------------------------------------------------------*/

	p = tf->lbuf;
	i = tf->width;
	switch (tf->pdepth)
		{
		case 32:
			while (--i >= 0)
				{
				rgbbuf->b = *p++;
				rgbbuf->g = *p++;
				rgbbuf->r = *p++;
				++p;
				++rgbbuf;
				}
			break;
		case 24:
			while (--i >= 0)
				{
				rgbbuf->b = *p++;
				rgbbuf->g = *p++;
				rgbbuf->r = *p++;
				++rgbbuf;
				}
			break;
		case 16:
			w = (USHORT *)p;
			while (--i >= 0)
				{
				ww = *w++;
				rgbbuf->r = ((ww&0x7c00)>>7);
				rgbbuf->g = ((ww&0x3e0)>>2);
				rgbbuf->b = ((ww&0x1f)<<3);
				++rgbbuf;
				}
			break;
		case 8:
			memcpy((Pixel *)rgbbuf, p, tf->width);
			break;
		default:
			return Err_driver_protocol;
		}

	++tf->cury;
	return Success;
}

Errcode read_seekstart(Image_file *ifile)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Targa_file *tf = (Targa_file *)ifile;

	tf->over_count = 0;
	tf->cury = 0;
	fseek(tf->file, tf->data_offset, SEEK_SET);
	return tf->is_flipped;
}

Errcode read_cmapped_image(Targa_file *tf)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;
	int 	y;
	int 	dy;
	int 	w = tf->width;
	int 	h = tf->height;

	if ((err = read_targa_ctab(tf)) < Success)
			return err;

	if (Success > (err = read_seekstart((Image_file *)tf)))
		{
		return err;
		}
	else if (0 == err)
		{
		y = 0;
		dy = 1;
		}
	else
		{
		y = h-1;
		dy = -1;
		}

	while (--h >= 0)
		{
		if (Success != (err = read_nextline((Image_file *)tf, (Rgb3 *)tf->lbuf)))
			return err;
		pj_put_hseg(tf->screen_rcel, tf->lbuf, 0, y, w);
		y += dy;
		}

	return Success;
}

