/***************************************************************
Autodesk Movie file pdr modules:

	Created by Peter Kennard.  Oct 5, 1991
		These modules implement a PDR for Autodesk movie file compressed
		EGA pixel animations.  Slide records, buttons looping and other
		functions of movie files are not implemented.
****************************************************************/
#include "movie.h"
#include "compress.h"

static void convert_ega_pixels(UBYTE *egabuf, UBYTE *pixbuf, int width)
/* Convert a buffer containing 3, 80 byte contiguous chunks of bitplane
 * bits, 1 bits, 2 bits and 4 bits, to a buffer of byte a pixel pixels. */
{
UBYTE *maxpix;
LONG allplanes;
int count;

	maxpix = pixbuf + width;
	count = 0;

	while(pixbuf < maxpix)
	{
		if(--count <= 0)
		{
			count = 8;
			allplanes = egabuf[0] | (egabuf[80] << 8) | (egabuf[160]<<16);
			++egabuf;
		}
		if(allplanes & 0x80)
			*pixbuf = 1;
		else
			*pixbuf = 0;

		if(allplanes & 0x8000)
			*pixbuf |= 0x2;

		if(allplanes & 0x800000)
			*pixbuf |= 0x4;

		++pixbuf;
		allplanes <<= 1;
	}
}

static Errcode decomp_lines(Mfile *mf,UBYTE *cbuf,UBYTE *pixbuf,int bsize,
							Coor x )
/* Decompress scan line records from a movie file compressed bitmap
 * record and install them in a byte a pixel screen. */
{
Coor y;
Coor width;
UBYTE egabuf[(3*80)+256]; /* 256 is in case of errant data */
UBYTE *egabyte;
UBYTE *egaend;
UBYTE *maxcbuf;
UBYTE c;
int count;

	y = *cbuf++;
	y += *cbuf++ << 8;
	if((Ucoor)x > XSIZE) /* be safe */
		return(Success);

	maxcbuf = cbuf + bsize;
	width = mf->ainfo.width - x; /* Adjust output width for x offset. */

	for(;;)
	{
		/* start a new line */

		egabyte = egabuf;
		egaend = egabyte + 80;
		stuff_bytes(0,egabuf,80*3); /* start with zeros */

		/* First decompress into ega format buffer. */

		for(;;)
		{
			if(cbuf >= maxcbuf)
				break;

			c = *cbuf++;

			if(c & QNZRUN)	/* Quick nonzero run */
			{
				c &= 0x7F;
				stuff_bytes(*cbuf++,egabyte,c); /* stuff em */
				egabyte += c;
			}
			else if(c & QZRUN)	/* Quick zero run */
			{
				egabyte += c & 0x3F;
			}
			else if(c & QNCOMP)	/* Quick uncompressed string */
			{
				c &= 0x1F;
				copy_bytes(cbuf,egabyte,c);
				egabyte += c;
				cbuf += c;
			}
			else switch(c) /* c is an opcode integer */
			{
				case ENDBUF:	/* End of buffer */
					return(Success);
				case ENDLINE:	/* End of line */
					goto ega_line_done;
				case ENDROW:	/* End of colour row (80 byte bitplane) */
					if((egabyte = egaend) > &egabuf[80*3])
						goto format_error;
					egaend += 80;
					continue;
				case ZERORUN:	/* Zero run, cleared above */
					egabyte += *cbuf++;
					break;
				case NZRUN:		/* Nonzero run value in next byte */
					count = *cbuf++;
					c = *cbuf++;
					stuff_bytes(c,egabyte,count);  /* stuff the run */
					egabyte += count;
					break;
				case UCDAT:		/* Uncompressed stream */
					count = *cbuf++;
					copy_bytes(cbuf,egabyte,count); /* copy em */
					cbuf += count;
					egabyte += count;
					break;
				case UCSING:	/* Single uncompressed byte */
					*egabyte++ = *cbuf++;
					break;
				default: /* invalid opcode */
					goto format_error;
			}

			/* check for over running expected bitplane length */

			if(egabyte > egaend)
				goto format_error;
		}

	ega_line_done:

		convert_ega_pixels(egabuf,pixbuf,width);
		pj_put_hseg(mf->screen,pixbuf,x,y++,width);

		if(cbuf >= maxcbuf)
			break;
	}

	return(Success);
format_error:	
	return(mf->lasterr = Err_format);
}


static UBYTE auto_cmap[] =  /* only bother with first 8 colors */
{
	000,000,000, 255,000,000, 255,255,000, 000,255,000,
	000,255,255, 000,000,255, 255,000,255, 255,255,255,
};

#ifdef SLUFFED
252,252,252, 252,252,252, 252,0,0, 252,124,124,
160,0,0, 160,80,80, 124,0,0, 124,60,60,
72,0,0, 72,36,36, 36,0,0, 36,16,16,
252,60,0, 252,156,124, 160,40,0, 160,100,80,
124,28,0, 124,76,60, 72,16,0, 72,44,36,
36,8,0, 36,20,16, 252,124,0, 252,188,124,
160,80,0, 160,120,80, 124,60,0, 124,92,60,
72,36,0, 72,56,36, 36,16,0, 36,28,16,
252,188,0, 252,220,124, 160,120,0, 160,140,80,
124,92,0, 124,108,60, 72,56,0, 72,64,36,
36,28,0, 36,32,16, 252,252,0, 252,252,124,
160,160,0, 160,160,80, 124,124,0, 124,124,60,
72,72,0, 72,72,36, 36,36,0, 36,36,16,
188,252,0, 220,252,124, 120,160,0, 140,160,80,
92,124,0, 108,124,60, 56,72,0, 64,72,36,
28,36,0, 32,36,16, 124,252,0, 188,252,124,
80,160,0, 120,160,80, 60,124,0, 92,124,60,
36,72,0, 56,72,36, 16,36,0, 28,36,16,
60,252,0, 156,252,124, 40,160,0, 100,160,80,
28,124,0, 76,124,60, 16,72,0, 44,72,36,
8,36,0, 20,36,16, 0,252,0, 124,252,124,
0,160,0, 80,160,80, 0,124,0, 60,124,60,
0,72,0, 36,72,36, 0,36,0, 16,36,16,
0,252,60, 124,252,156, 0,160,40, 80,160,100,
0,124,28, 60,124,76, 0,72,16, 36,72,44,
0,36,8, 16,36,20, 0,252,124, 124,252,188,
0,160,80, 80,160,120, 0,124,60, 60,124,92,
0,72,36, 36,72,56, 0,36,16, 16,36,28,
0,252,188, 124,252,220, 0,160,120, 80,160,140,
0,124,92, 60,124,108, 0,72,56, 36,72,64,
0,36,28, 16,36,32, 0,252,252, 124,252,252,
0,160,160, 80,160,160, 0,124,124, 60,124,124,
0,72,72, 36,72,72, 0,36,36, 16,36,36,
0,188,252, 124,220,252, 0,120,160, 80,140,160,
0,92,124, 60,108,124, 0,56,72, 36,64,72,
0,28,36, 16,32,36, 0,124,252, 124,188,252,
0,80,160, 80,120,160, 0,60,124, 60,92,124,
0,36,72, 36,56,72, 0,16,36, 16,28,36,
0,60,252, 124,156,252, 0,40,160, 80,100,160,
0,28,124, 60,76,124, 0,16,72, 36,44,72,
0,8,36, 16,20,36, 0,0,252, 124,124,252,
0,0,160, 80,80,160, 0,0,124, 60,60,124,
0,0,72, 36,36,72, 0,0,36, 16,16,36,
60,0,252, 156,124,252, 40,0,160, 100,80,160,
28,0,124, 76,60,124, 16,0,72, 44,36,72,
8,0,36, 20,16,36, 124,0,252, 188,124,252,
80,0,160, 120,80,160, 60,0,124, 92,60,124,
36,0,72, 56,36,72, 16,0,36, 28,16,36,
188,0,252, 220,124,252, 120,0,160, 140,80,160,
92,0,124, 108,60,124, 56,0,72, 64,36,72,
28,0,36, 32,16,36, 252,0,252, 252,124,252,
160,0,160, 160,80,160, 124,0,124, 124,60,124,
72,0,72, 72,36,72, 36,0,36, 36,16,36,
252,0,188, 252,124,220, 160,0,120, 160,80,140,
124,0,92, 124,60,108, 72,0,56, 72,36,64,
36,0,28, 36,16,32, 252,0,124, 252,124,188,
160,0,80, 160,80,120, 124,0,60, 124,60,92,
72,0,36, 72,36,56, 36,0,16, 36,16,28,
252,0,60, 252,124,156, 160,0,40, 160,80,100,
124,0,28, 124,60,76, 72,0,16, 72,36,44,
36,0,8, 36,16,20, 80,80,80, 116,116,116,
148,148,148, 184,184,184, 216,216,216, 252,252,252,
};
#endif /* SLUFFED */

static Errcode draw_frame(Mfile *mf, int ix)
/* Decompress a movie file bit image record into the 8 bit screen. */
{
LONG fsize;
void *pixbuf;
void *cbuf;
Coor x;


	/* Allocate iobuffer and one line of 8 bit pixels. */

	if((cbuf = malloc(IOBLEN + mf->ainfo.width)) == NULL)
		return(mf->lasterr = Err_no_memory);

	pixbuf = OPTR(cbuf,IOBLEN);

	/* Seek to start of frame record. */
	if(mf_seek(mf, mf->hframea[ix], SEEK_SET) < Success)
		goto error;

	fsize = mf->hflen[ix]; /* Get size from index. */

	if(mf->hframe[ix].ftype & FTBBINC)  /* For now seek past "buttons" */
	{
	SHORT nbuttons;

		if(mf_read(mf,&nbuttons,sizeof(nbuttons)) < Success)
			goto error;
		if(mf_seek(mf,nbuttons * sizeof(struct bdesc), SEEK_CUR) < Success)
			goto error;
		fsize -= sizeof(nbuttons) + (nbuttons * sizeof(struct bdesc));
	}

	pj_set_rast(mf->screen,0); /* Start cleared, This is not a delta format. */
	/* Load in 8 color palette */
	copy_bytes(auto_cmap,mf->screen->cmap->ctab,sizeof(auto_cmap));
	pj_cmap_load(mf->screen,mf->screen->cmap);

	x = mf->hfllim[ix] * 8; /* get x offset of lines from index. */

	/* While we have record left, read and decompress it. */

	while(fsize > 0) 
	{
	LONG sz;

		sz = Min(fsize,IOBLEN);
		if(mf_read(mf,cbuf,sz) < Success)
			goto error;
		fsize -= sz;
		if(decomp_lines(mf,cbuf,pixbuf,sz,x) < Success)
			goto error;
	 }

error:
	freez(&cbuf);
	return(mf->lasterr);
}
Errcode draw_next_frame(Mfile *mf)
/* Read and decompress next available frame in a movie file. */
{
USHORT ftype;

	/* "Seek" forward to next visible frame */
	for(;;)
	{
    	if(++mf->cur_frame >= mf->hdr.hframes) /* out of frames */
			return(mf->lasterr = Err_eof);
    	ftype = mf->hframe[mf->cur_frame].ftype;

	    if(ftype & (FTBNOTIMG |FTBTEXT|FTBSLIDE|FTBMOVIE))
			continue;

#ifdef TESTING
		/* is it not a bit image? */
	    if(ftype & (FTBNOTIMG))
			continue;

	    if(ftype & FTBTEXT)
		{
			printf("text!");
			continue;
		}
	    if(ftype & FTBSLIDE)
		{
			printf("slide!");
			continue;
		}
	    if(ftype & FTBMOVIE)
		{
			printf("movie!");
			continue;
		}
		if(ftype & FTBSAMEAS)
			printf("Beta version please note: FTBSAMEAS record.");

#endif /* TESTING */

		break;
	}
	return(draw_frame(mf,mf->cur_frame));
}

