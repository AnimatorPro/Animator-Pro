/***************************************************************
Pict file pdr modules:

	Created by Peter Kennard.  Sept 29, 1991
		Implements non-pattern fill mode bitmap operations
		and transfer mode blit. Parses and ignores all other
		opcodes.
****************************************************************/
#include "errcodes.h"
#include "pict.h"

static Errcode write_pictstart(Pfile *pf)
/* Write initial header info for an 8 bit pict file */
{
struct pictstart {
	USHORT size16;  /* old size ignore this */
	pRect frame;    /* the bounding rect of whole pic */
	USHORT vop;
	USHORT version;
	USHORT hdrop;
	UBYTE hdrdat[24];
};
UBYTE buff[512];
Errcode err;

	/* first write out 512 bytes of zeros */

	memset(buff,0,sizeof(buff));

	if((err = pf_write(pf,buff,sizeof(buff))) < Success)
		return(err);

	/* then write out header data for a version 2 picture */

#define PS (*((struct pictstart *)buff))

	PS.size16 = 0;
	PS.frame.top = 0;
	PS.frame.left = 0;
	PS.frame.bot = pf->ainfo.height;
	PS.frame.right = pf->ainfo.width;
	PS.vop = 0x0011;
	PS.version = 0x02FF;  
	PS.hdrop = 0x0c00;

	/* this is for a 68000 based machine */
	intel_swap_words(&PS,(sizeof(PS)-sizeof(PS.hdrdat))/sizeof(short));

	/* 24 bytes of 0xFF for "header" data in version 2 pics */
	memset(PS.hdrdat,0xFF,sizeof(PS.hdrdat));

	return(pf_write(pf,&PS,sizeof(PS)));

#undef PS
}
static Errcode write_cmap(Pfile *pf)
/* Write a pict color map record for a pj color map. */
{
Errcode err;
colorTable *ct;
ctEntry *cte;
Rgb3 *rgb;
int i;

#define CTSIZE (sizeof(colorTable) + (256*sizeof(ctEntry)))

	if((ct = malloc(CTSIZE)) == NULL)
		return(Err_no_memory);

	ct->id = 0;
	ct->ctFlags = 0;
	ct->ctSize = 255;
	cte =  CT_ENTRIES(ct);
	rgb = pf->screen->cmap->ctab;

	for(i = 0;i < 256;++i)
	{
		cte->pixel_ix = i;
		cte->r = rgb->r << 8;
		cte->g = rgb->g << 8;
		cte->b = rgb->b << 8;
		++cte;
		++rgb;
	}

	intel_swap_words(ct,CTSIZE/sizeof(short));
	err = pf_write(pf,ct,CTSIZE);

error:
	free(ct);
	return(err);

#undef CTSIZE
}

static char *dont_compress(void *pixels, void *cbuf, int width)
/* Filler compression function, does nothing. */
{
	return(cbuf);
}
static Errcode write_pmap(Pfile *pf, pixMap *inpm)
/* Write a pixel map chunk to a pict file. */
{
Errcode err;
pixMap pm = *inpm;
LONG pmoset;
LONG rects_oset;
LONG end_oset;
LONG chunk_written;  /* bytes of chunk written so far */
char *(*compress_line)(void *pixels, void *cbuf, int width);
Errcode (*write_line)(Pfile *pf, void *buf, int size);
int size_size; /* size of imbedded size */
void *pixbuf;  /* buffer for line of pixels */
char *cbuf;    /* compression buffer */
int width;     /* width of line */
int csize;     /* compressed size */

	width = pf->ainfo.width;

	/* We need a pixel buffer and a compression buffer. */
	if((pixbuf = malloc(width*3)) == NULL)
		return(Err_no_memory);

	/* Select compression type based on width of line per apple spec. */

	if(pf->ainfo.width < 8) /* no compression */
	{
		cbuf = pixbuf;  /* just write them out direct */
		compress_line = dont_compress;
		write_line = pf_write; /* just write em out, no imbedded size */
		size_size = 0;
	}
	else
	{
		cbuf = OPTR(pixbuf,width); /* need a separate buffer */
		compress_line = brun_pack_line;
		if(pf->ainfo.width <= 250) 
		{
			/* imbedded size is a byte */
			write_line = write_chunk8;
			size_size = 1;
		}
		else  
		{
			/* Imbedded size is a short int */
			write_line = write_chunk16;
			size_size = 2;
		}
	}


/******* Write out a pixel map header and color map chunk. *******
 ******  Due to poor planning at Apple each pmap chunk gets a copy of the 
 ******  color map, about 2k per copy.
 *****/

	/* Write out opcode for packed bits array. */

	if((err = pf_write_short(pf, 0x0098)) < Success)
		goto error;

	/* Save offset to pixMap header for rewrite later. */

	if((pmoset = ftell(pf->file)) < 0)
		goto ioerror;

	/* Allocate space in file for pixel map header. */

	if((err = pf_write(pf,&pm.PMAP_FSTART, PMAP_FSIZE)) < Success)
		goto error;

	/* Write out color table. */

	if((err = write_cmap(pf)) < Success)
		goto error;

	/* This is where the rectangles go later. */

	if((rects_oset = ftell(pf->file)) < 0)
		goto ioerror;

	/* Allocate space for two rectangles in file, pm is big enough */

	if((err = pf_write(pf,&pm,sizeof(pRect)*2)) < Success)
		goto error;

	/* Write out "transfer mode" field 0 for a straight copy. */

	if((err = pf_write_short(pf, 0)) < Success)
		goto error;

/***** Now write out the pixels using functions selected earlier ******/

	/* pixMap chunks are to be limited to about 30k sez apple.
	 * We are conservative by 1k here to cover header records and
	 * by 2 uncompressed lines better safe than sorry, with a closed
	 * architecture. */

	chunk_written = (rects_oset - pmoset) + pf->ainfo.width*2; 

	pm.Bounds.bot = pm.Bounds.top; /* Start empty having written no lines */

	/* While we have space and screen to go get,compress,and write lines. */

	while(chunk_written < 29000 && pm.Bounds.bot < pf->ainfo.height)
	{
		pj__get_hseg(pf->screen,pixbuf,0,pm.Bounds.bot,width);

		csize = compress_line(pixbuf,cbuf,width) - cbuf;

		if((err = write_line(pf,cbuf,csize)) < Success)
			goto error;
		
		chunk_written += csize + size_size;
		++pm.Bounds.bot;
	}

	/* We have successfully written all the pixels so we must re-write the 
	 * pixMap header and the rectangles to represent the now finished size
	 * of the actual pixel map pixels. */

	/* Save where we are to seek back when done. */

	if((end_oset = ftell(pf->file)) < 0)
		goto ioerror;

	*inpm = pm; /* Replace input pixMap header before swapping local copy. */

	intel_swap_pixMap(&pm); /* allright set to 68000 byte order */

	/* Re-write pixMap header with actual size Bounds rectangle. */

	if((err = pf_write_oset(pf,&pm.PMAP_FSTART,PMAP_FSIZE,pmoset)) < Success)
		goto error;

	/* Re-write two relative rectangles. Since they are all relative to screen
	 * they are all the same and we write Bounds. */

	if((err = pf_write_oset(pf,&pm.Bounds,sizeof(pRect),rects_oset)) < Success)
		goto error;

	if((err = pf_write(pf,&pm.Bounds,sizeof(pRect))) < Success)
		goto error;

	/** Seek back to where we were at end of pixels to continue. */

	if((fseek(pf->file,end_oset,SEEK_SET)) < 0)
		goto ioerror;

	if(end_oset & 1) /* Oops we must word align!, write a zero. */
	{
	static UBYTE zbyte = 0;

		if((err = pf_write(pf,&zbyte,1)) < Success)
			goto error;
	}

	/* Done, What a chore. */

	err = Success;
	goto done;
ioerror:
	err = pj_errno_errcode();
error:
done:
	freez(&pixbuf);
	return(err);
}
static Errcode write_pixelmaps(Pfile *pf)
/* Write out pixel map chunks until the whole screen is saved. */
{
Errcode err;
pixMap pm;

	memset(&pm,0,sizeof(pm)); /* start with zeros */

	/* This is what apple sez iz default. */
	pm.hRes = pm.vRes = 0x0048000;

	/* Byte a pixel mon, with smokin high bit. */
	pm.rowBytes = (0x8000 | pf->ainfo.width); 
	pm.pixelType = 0;
	pm.pixelSize = 8;
	pm.cmpSize = 8;
	pm.cmpCount = 1;

	/* Set right edge value to width for pmap. */
	pm.Bounds.right = pf->ainfo.width;

	/* Write em out until we got the whole pic is saved top to bottom. */

	while(pm.Bounds.top < pf->ainfo.height)
	{
		if((err = write_pmap(pf,&pm)) < Success)
			goto error;

		pm.Bounds.top = pm.Bounds.bot; /* We continue where the 
										* last one finished */
	}
	return(Success);
error:
	return(err);
}

Errcode pict_save_frame(Image_file *ifile, Rcel *screen, int num_frames,
							  Errcode (*seek_frame)(int ix,void *seek_data),
							  void *seek_data, Rcel *work_screen )
/* Save a screen to a newly created pict file. */
{
Pfile *pf;
Errcode err;

	pf = (Pfile *)ifile;

	if( (pf->ainfo.width) != screen->width
		|| pf->ainfo.height != screen->height)
	{
		return(Err_bad_input);
	}

	pf->screen = screen;

	if((err = write_pictstart(pf)) < Success)
		goto error;

	if((err = write_pixelmaps(pf)) < Success)
		goto error;

	/* Write end of picture opcode ***/

	return(pf_write_short(pf, 0x00FF));

error:
	return(err);
}
