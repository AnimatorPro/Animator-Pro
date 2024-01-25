/***************************************************************
Pict file pdr modules:

	Created by Peter Kennard.  Sept 29, 1991
		Functions to read, parse and decompress pict file pixel map
		data chunks and transfer the pixels to a pj screen, or to 
		single line pixel buffers in true color mode.
****************************************************************/
#include "errcodes.h"
#include "memory.h"
#include "pict.h"
#include "picdrive.h"
#include "syslib.h"


void intel_swap_pixMap(pixMap *pm)
/* Intel swap integer fields in an apple pixel map header. */
{
static USHORT pmswaps[] = {
	SET_OFFSET(pixMap,rowBytes),
	SWAP_WORDS(7),
	SWAP_LONGS(1),
	SET_OFFSET(pixMap,pixelType),
	SWAP_WORDS(4),
	SWAP_LONGS(2),
	SWAP_WORDS(2),
	END_SWAPTAB
};
	intel_swap_struct(pm,pmswaps);
}
static Errcode read_pixMap(Pfile *pf, pixMap *pm)
/* Read a pixMap header from the file, and make allowance for version 1 or 2
 * types. Convert version 1 to version 2 format. */
{

	/* Note the baseAddr field is NOT in the file */

	if(pf_read(pf,&pm->PMAP_FSTART, PMAP_FSIZE) < Success)
		goto error;

	intel_swap_pixMap(pm);

	if(!(pm->rowBytes & 0x8000)) /* This is a BITmap */
	{
		/* Go back to end of bitMap in file and load pixMap values that 
		 * would be valid for a bitmap. */

		pf_seek_bytes(pf, sizeof(bitMap)-sizeof(pixMap));
		pm->packType = PACK_BRUN;
		pm->packSize = 0;
		pm->pixelType = 0;
		pm->cmpSize = 1;
		pm->pixelSize = 1;
		pm->cmpCount = 1;
		pm->planeBytes = 0;
		pm->myflags = PM_NOCTABLE;
	}
	else /* This is a pixel map.  Clear sign bit */
	{
		pm->rowBytes &= ~(0x8000);
		pm->myflags = 0;
		if(pm->pixelType == 16)
			pm->myflags |= PM_NOCTABLE;
	}

#ifdef PRINTSTUFF
#define PRTIT(f) printf(sizeof(MEMBER(pixMap,f)) > 2?"%s %ld\n":"%s %d\n",\
	#f, pm->f )

	PRTIT( rowBytes); 
	PRTIT(Bounds.top);    
	PRTIT(Bounds.left);    
	PRTIT(Bounds.right);    
	PRTIT(Bounds.bot);    
	PRTIT( version);
	PRTIT( packType);
	PRTIT( packSize);
	PRTIT( pixelType); 
	PRTIT( pixelSize); 
	PRTIT( cmpCount);
	PRTIT( cmpSize);
	PRTIT( planeBytes); 
#undef PRTIT
#endif /* PRINTSTUFF */

	return(Success);
error:
	return(pf->lasterr);
}
static Errcode read_cTable_head(Pfile *pf, colorTable *ct)
/* Read header for pixel map color table from file and intel swap it. */
{
static USHORT ctswaps[] = {
	SWAP_LONGS(1),
	SWAP_WORDS(2),
	END_SWAPTAB
};
	pf_read(pf,ct,sizeof(*ct));
	intel_swap_struct(ct,ctswaps);
	return(pf->lasterr);
}
static Errcode read_colorTable(Pfile *pf)
/* Read a color table header from file, alloc a buffer for pf->cTable and 
 * read in the color values from the file. */
{
ULONG bytesize;
ctEntry *entry;
colorTable ct;
USHORT use_indices;
int i;

	if(read_cTable_head(pf,&ct) < Success)
		goto error;

	bytesize = (ct.ctSize+1) * sizeof(ctEntry);

	freez(&pf->cTable);
	if((pf->cTable = malloc(bytesize + sizeof(colorTable))) == NULL)
		return(pf->lasterr = Err_no_memory);

	if(pf_read(pf,CT_ENTRIES(pf->cTable),bytesize) < Success)
		goto error;
	intel_swap_words(CT_ENTRIES(pf->cTable),bytesize>>1);

	*pf->cTable = ct;

#ifdef PRINTSTUFF
	#define PRTIT(f) printf(sizeof(ct.f) > 2?"%s %ld\n":"%s %d\n",\
		#f, pf->cTable->f )

		PRTIT(id);  
		PRTIT(ctFlags);    
		PRTIT(ctSize);    
	#undef PRTIT
#endif /* PRINTSTUFF */

	/* Check table entry order and set inorder flag if it is.  I would assume
	 * these tables may not be in order or the pixel_ix field wouldn't 
	 * exist, if they are not we need to make pen ids equal indices. Some
	 * pics I have gotten have all zeros in the index field and they display
	 * ok if the table is assumed to be in index order so I set it here */

	use_indices = 0;
	entry = CT_ENTRIES(pf->cTable);
	for(i = ct.ctSize;i >= 0;--i)
	{
		use_indices |= entry->pixel_ix;
		++entry;
	}
	/* Entry is at max now so we count down in loop below. */

	pf->flags |= PH_CTAB_INORDER; /* be optimistic */
	for(i = ct.ctSize;i >= 0;--i)
	{
		--entry;
		if(!use_indices)
			entry->pixel_ix = i;
		else
		{
			if((entry)->pixel_ix != i)
				pf->flags &= ~PH_CTAB_INORDER;
		}
	}
	
	return(Success);
error:
	return(pf->lasterr);
}
static Errcode skip_colorTable(Pfile *pf)
/* Read color table header from file and skip by the data.  Used for ignored 
 * chunks. */
{
colorTable ct;

	if(read_cTable_head(pf,&ct) >= Success)
		pf_seek_bytes(pf,(ct.ctSize+1) * sizeof(ctEntry));
	return(pf->lasterr);
}
static Errcode load_colorTable(Pfile *pf)
/* Load previously read in Apple style color table into a pj color map. */
{
Cmap *cmap;
Rgb3 *rgb;
ctEntry *cte;
int count;
USHORT max_ix;

	cmap = pf->screen->cmap;
	cte = CT_ENTRIES(pf->cTable);
	max_ix = cmap->num_colors;

	/* note ctSize is one less than actual size of table */
	for(count = pf->cTable->ctSize;count >= 0;--count)
	{
		/* must be within range of destination */
		if(cte->pixel_ix < max_ix)
		{
			rgb = &cmap->ctab[cte->pixel_ix];
			rgb->r = (cte->r >> 8);
			rgb->g = (cte->g >> 8);
			rgb->b = (cte->b >> 8);
		}
		++cte;
	}
	pj_cmap_load(pf->screen,cmap);
	return(Success);
}
static Errcode read_rowbytes_line(Pfile *pf,void *buf,unsigned maxsize)
/* Read line of implicit rowBytes size for an unpacked line. */
{
	return(pf_read(pf,buf,pf->pm.rowBytes));
}
static Errcode donothing_unpack(BYTE *packline, UBYTE *buf, int len)
/* Filler function: Does nothing. */
{
	return(Success);
}

/***********  bit to byte a pixel expander functions ********* /

/* All are: void expand_Xbit(UBYTE *bitbuf,UBYTE *pix, UBYTE *maxpix)
 * 
 * bitbuf is a buffer of X bit wide pixels, input
 * pix is the output buffer of byte wide pixels
 * maxpix it the loop controller or one beyond the last output pixel available
 */

typedef union bitbytes {
	struct {
		UBYTE low; 
		UBYTE hi; 
	} b;
	USHORT w; 
} Bitbytes;

static void dont_make_pixels(UBYTE *bitbuf,UBYTE *pix, UBYTE *maxpix, int bpr)
/* donothing place holder function for pixel cruncher */
{
	return;
}
static void expand_1bit(UBYTE *bitbuf,UBYTE *pix, UBYTE *maxpix, int bpr)
/* Expand buffer of 1 bit pixels to byte a pixel buffer. */
{
Bitbytes bits;
int count;

	bits.w = 0;
	count = 0;
	while(pix < maxpix)
	{
		if(--count <= 0)
		{
			count = 8;
			bits.b.low = *bitbuf++;
		}
		bits.w <<= 1;
		*pix++ = bits.b.hi & 1;
	}
}
static void expand_2bit(UBYTE *bitbuf,UBYTE *pix, UBYTE *maxpix, int bpr)
/* Expand buffer of 2 bit pixels to byte a pixel buffer. */
{
Bitbytes bits;
int count;

	bits.w = 0;
	count = 0;
	while(pix < maxpix)
	{
		if(--count <= 0)
		{
			count = 4;
			bits.b.low = *bitbuf++;
		}
		bits.w <<= 2;
		*pix++ = bits.b.hi & 0x3;
	}
}
static void expand_4bit(UBYTE *bitbuf,UBYTE *pix, UBYTE *maxpix, int bpr)
/* Expand buffer of 4 bit pixels to byte a pixel buffer. */
{
UBYTE b;

	for(;;)
	{
		if(pix >= maxpix)
			break;
		b = *bitbuf++;
		*pix++ = b >> 4;
		if(pix >= maxpix)
			break;
		*pix++ = b & 0xF;
	}
}
static void bits16_to_Rgb3(UBYTE *bitbuf, UBYTE *pix, UBYTE *maxpix, int bpr)
/* Expand 16 bit apple pixels to 24 bit r,g,b pixels */
{
USHORT *pix16 = (USHORT *)bitbuf;
Bitbytes bits;
#define RGB ((Rgb3 *)pix)

	/* layout of apple 16 bit pixel is 0rrrrrgggggbbbbb */

	while(pix < maxpix)
	{
		bits.w = *pix16++ << 1;
		RGB->r = (bits.b.hi & 0xF8) | ((bits.b.hi>>5) & 0x07);

		RGB->g = (bits.b.hi & 0x07);
		bits.w <<= 5;
		RGB->g |= (bits.b.hi & 0xF8);

		RGB->b = (bits.b.hi & 0x07);
		bits.w <<= 5;
		RGB->b |= (bits.b.hi & 0xF8);
		pix += sizeof(*RGB);
	}
#undef RGB
}
static void interleave_rgbs(UBYTE *bitbuf, UBYTE *pix, UBYTE *maxpix, int bpr)
/* Take 3 end to end [rrr][ggg][bbb] buffers and produce one buffer of 
 * ordered rgb triples  rgbrgbrgb. Bpr is 4x the length of the input buffer
 * used by each color component. The fourth color component is used as an
 * Alpha channel if present and is not used by the PDR. */
{
UBYTE *reds;
UBYTE *greens;
UBYTE *blues;
#define RGB ((Rgb3 *)pix)

	/* Layout of apple 32 bit pixels as decompressed by byte run compression
	 * is bpr/4 reds bpr/4 greens bpr/4 blues bpr/4 alphas */

	bpr >>= 2;
	reds = bitbuf;
	greens = reds + bpr;
	blues = greens + bpr;

	while(pix < maxpix)
	{
		RGB->r = *reds++;
		RGB->g = *greens++;
		RGB->b = *blues++;
		pix = OPTR(pix,sizeof(*RGB));
	}
#undef RGB
}
static void copy_pixel_bytes(UBYTE *bitbuf, UBYTE *pix, UBYTE *maxpix, int bpr)
/* this copies unpacked pixels just as they are */
{
	copy_bytes(bitbuf,pix,SIZE(pix,maxpix));
}
/*******************************************************************/
/*** transfer mode "blit" logic functions D must equal s + width !! 
 *** and is used for the loop test. Each function will logicly merge
 *** string s into d putting the result in d. */

static void or_pixels(UBYTE *s,UBYTE *d)
{
UBYTE *maxs;

	maxs = d;
	while(s < maxs)
		*d++ |= *s++;
}
static void xor_pixels(UBYTE *s,UBYTE *d)
{
UBYTE *maxs;

	maxs = d;
	while(s < maxs)
		*d++ ^= *s++;
}
static void andnot_pixels(UBYTE *s,UBYTE *d)
{
UBYTE *maxs;

	maxs = d;
	while(s < maxs)
		*d++ &= ~(*s++);
}
static void not_pixels(UBYTE *s,UBYTE *d)
{
UBYTE *maxs;

	maxs = d;
	while(s < maxs)
		*d++ = ~(*s++);
}
static void ornot_pixels(UBYTE *s,UBYTE *d)
{
UBYTE *maxs;

	maxs = d;
	while(s < maxs)
		*d++ |= ~(*s++);
}
static void xornot_pixels(UBYTE *s,UBYTE *d)
{
UBYTE *maxs;

	maxs = d;
	while(s < maxs)
		*d++ ^= ~(*s++);
}
static void and_pixels(UBYTE *s,UBYTE *d)
{
UBYTE *maxs;

	maxs = d;
	while(s < maxs)
		*d++ &= *s++;
}
static Errcode init_pixmap_transfer(Pfile *pf)
/* Sets up state variables in pictfile to start pixel transfer from file
 * to output raster of rgb lines from a pixel map record in the file. 
 * It sets the state to correspond to the description of the pixel data in 
 * the previously read pixel map header. */
{
LONG bsize;

	/* get width and height of output raster from this pixel map */

	pf->pixwidth = pf->pm.Bounds.right - pf->pm.Bounds.left;

	if(pf->mode == PF_RGBLINES)
		pf->pixwidth *= sizeof(Rgb3);

	pf->pmheight = pf->pm.Bounds.bot - pf->pm.Bounds.top;

	/* We potentially use 3 buffers one for pixels, one for packed
	 * bits, and one for unpacked bits. Maybe a little bigger than needed
	 * but surely enough.  Make sure it is at least 2*width for transfer
	 * mode functionality which needs an extra buffer for compositing.
	 * When the composite function is called, all buffer space other than
	 * pixbuf can be overwritten and used. */

	bsize = (pf->pm.rowBytes*3)+128;
	if(bsize < pf->pixwidth)
		bsize += pf->pixwidth;
	bsize += pf->pixwidth;

	/* re-allocate pixel buffer if too small */
	if(pf->pixbuf == NULL || pf->pbsize < bsize)
	{
		freez(&pf->pixbuf);
		if((pf->pixbuf = malloc(bsize)) == NULL)
			return(pf->lasterr = Err_no_memory);
		pf->pbsize = bsize;
	}

	/* We use one pixwidth line of pixels for the output buffer. */

	pf->maxpix = OPTR(pf->pixbuf,pf->pixwidth); 

	/* Select pixel expansion or crunching function based on pixel depth. */
	switch(pf->pm.pixelSize)
	{
		case 1:
			pf->make_pixels = expand_1bit;
			goto setup_bitbuf;
		case 2:
			pf->make_pixels = expand_2bit;
			goto setup_bitbuf;
		case 4:
			pf->make_pixels = expand_4bit;
			goto setup_bitbuf;
		case 16:
			pf->make_pixels = bits16_to_Rgb3;
			goto setup_bitbuf;
		case 32:
		{
		/* Note: In the unpacked case we copy the pixels.  This is because
		 * we are loading the pixels into an ouput line buffer and the 
		 * make_pixels function is used to load the pixels into the output 
		 * line.  Maybe an extra copy but code is simpler in rgb_readline() */

			switch(pf->pm.packType)
			{
				case PACK_BRUN_CMP:
					pf->make_pixels = interleave_rgbs;
					goto setup_bitbuf;
				case NOPACK_ODDSIZE: /* not packed, out RGBs are the same in
									  * this case */
					pf->make_pixels = copy_pixel_bytes; 
					goto setup_bitbuf;
			}
			goto set_dont_make;
		}
		default: /* If we don't support it, just do nothing. */
		case 8:
		set_dont_make:
		{
			pf->make_pixels = dont_make_pixels;
			pf->bitbuf = pf->pixbuf;  /* this needs no expansion so unpack 
							  		    * readbuf direct to pixbuf */
			break;
		}
		setup_bitbuf:
		{
			/* Set up a bitbuf as a separate area where decompressed bits 
			 * are put before being converted to pixels. note this is 
			 * long aligned */

			pf->bitbuf = OPTR(pf->maxpix,4-(pf->pixwidth & 3));   
			break;
		}
	}

	/* Select read function based on ranges of bytes per row as defined
	 * by apple docs. */

	if(pf->pm.rowBytes < 8) /* Data is unpacked and lines are bpr size. */
	{
		pf->read_line = read_rowbytes_line; 
		pf->pm.packType = NOPACK; /* set to select no packing packtype */
	}
	else if(pf->pm.rowBytes <= 250) 
	{
		/* imbedded size is a leading byte */
		pf->read_line = read_chunk8;
	}
	else  
	{
		/* imbedded size is a leading short int */
		pf->read_line = read_chunk16;
	}

	/* Select unpacker function based on packType field. */

	switch(pf->pm.packType)
	{
		case PACK_BRUN: /* byte run byte a pixel compression */
			pf->unpack_line = brun_unpack_line; 
			goto packed_line;
		case NOPACK: /* unpacked format */
		case NOPACK_ODDSIZE: 
			goto unpacked_line;
		case PACK_CHUNKRUN: /* word run compression only for 16 bit pixels */
		{
			if(pf->pm.pixelSize != 16)	
				goto unpacked_line;
			pf->unpack_line = wrun_unpack_line;
			goto packed_line;
		}
		case PACK_BRUN_CMP: /* Each byte size component byte run compressed */
		{
			if(pf->pm.pixelSize != 32)
				goto unpacked_line;
			if(pf->pm.cmpCount == 3)
				pf->unpack_line = brun_unpack_3compbytes;
			else
				pf->unpack_line = brun_unpack_line; 
			goto packed_line;
		}
		default:  /* Just make ugly screen of data. not supported */
		unpacked_line:
			pf->unpack_line = donothing_unpack; /* no compression */
			pf->readbuf = pf->bitbuf; /* no unpacking read to bitbuf */
			break;
		packed_line:
			/* Use a separate read area since we are unpacking */
			pf->readbuf = OPTR(pf->bitbuf,pf->pm.rowBytes); 
			break;
	}

	/* Set maxsize safety margin for reader so it won't overrun buffer. */
	pf->max_read_size = pf->pbsize - SIZE(pf->pixbuf,pf->readbuf) - 2;

	/* Select blit logic function for transfer mode field. */

	switch(pf->tmode)
	{
		default:
			pf->tmode = 0;
		case 0:  /* d = s NO compositing logic */
			break;
		case 1:  /* d |= s */
			pf->composite_pixels = or_pixels;
			break;
		case 2:  /* d ^= s */
			pf->composite_pixels = xor_pixels;
			break;
		case 3:  /* d &= ~s */
			pf->composite_pixels = andnot_pixels;
			break;
		case 4: /* d = ~s */
			pf->composite_pixels = not_pixels;
			break;
		case 5: /* d |= ~s */
			pf->composite_pixels = ornot_pixels;
			break;
		case 6: /* d ^= ~s */
			pf->composite_pixels = xornot_pixels;
			break;
		case 7: /* d &= s */
			pf->composite_pixels = and_pixels;
			break;
	}
	return(Success);
}
static Errcode pixmap_to_screen(Pfile *pf)
/* Using functions set by init_pixmap_transfer() read pixels and move
 * them to the screen via the transfer methods selected. */
{
int ret;
UBYTE *dstbuf;
SHORT height;
Coor x,y;

	height = pf->pmheight;
	dstbuf = pf->tmode == 0 ? pf->pixbuf:pf->maxpix;
	y = pf->dstRect.top;
	x = pf->dstRect.left;

	/** THE getit unpackit convertit blitit loop !!! ***/

	while(height-- > 0)
	{
		if(pf->read_line(pf,pf->readbuf,pf->max_read_size) < 0)
			goto error;

		if((ret = pf->unpack_line(pf->readbuf,pf->bitbuf,
								  pf->pm.rowBytes)) < Success)
		{
			goto reterr;
		}

		pf->make_pixels(pf->bitbuf,pf->pixbuf,pf->maxpix,pf->pm.rowBytes);

		if(pf->tmode)
		{
			/* note dstbuf and maxpix are and must be the same! */
			/* get source */
			pj_get_hseg(pf->screen,pf->maxpix,x,y,pf->pixwidth); 
			pf->composite_pixels(pf->pixbuf,pf->maxpix);
		}

		pj_put_hseg(pf->screen,dstbuf,x,y,pf->pixwidth);
		++y;
	}
	return(Success);
reterr:
	pf->lasterr = ret;
error:
	return(pf->lasterr);
}
static Errcode skip_bitsrect(Pfile *pf,pixMap *pm)
/* Seek by pixel data for an ignored pixMap. */
{
SHORT height;
Errcode  (*skipit)(Pfile *);

	/* Get width and height of pixel map. */

	height = pf->pm.Bounds.bot - pf->pm.Bounds.top;

	if(pf->pm.rowBytes < 8) /* data is unpacked and lines are bpr size */
		return(pf_seek_bytes(pf, height*pm->rowBytes));

	/* Data is in embedded size chunks */

	if(pf->pm.rowBytes <= 250) 
		skipit = skip_chunk8;
	else
		skipit = skip_chunk16;

	while(height-- > 0)
	{
		if(skipit(pf) < Success)
			break;
	}
	return(pf->lasterr);
}
Errcode do_pixmap(Pfile *pf)
/* Read in a pixel map record from the file and either get info,
 * transfer to screen, or prepare to read the first RGB line depending
 * on the pf->mode. */
{
	pf->got_bitmap = TRUE;
	if(read_pixMap(pf,&pf->pm) < Success)
		goto error;

	if(pf->mode == PF_SCANINFO)
	{
	  /* Set offset to first pixel map if we are in an RGB true color file. */
		if((pf->ainfo.depth = pf->pm.pixelSize) > 8)
		{
			if((pf->first_pmap_oset = ftell(pf->file)) < 0)
				return(pf->lasterr = pj_errno_errcode());	
			pf->first_pmap_oset -= PMAP_FSIZE; /* we already read it */
		}
		return(pf->lasterr = RET_ENDSCAN);
	}
	else if(pf->mode == PF_FULLFRAME)  
	{
		/* Ignore true color data for full frame mode. */

		if((pf->ainfo.depth = pf->pm.pixelSize) > 8)
			return(skip_pmap_data(pf,&pf->pm));
	}

	/* If there is one, read the color map and load it into the screen. */

	if(!(pf->pm.myflags & PM_NOCTABLE))
	{
		if(read_colorTable(pf) < Success)
			goto error;
		if(pf->mode == PF_FULLFRAME)
			load_colorTable(pf);
	}

	/* Read in relative positioning rectangles. */

	if(read_pRect(pf,&pf->srcRect) < Success)
		goto error;

	if(read_pRect(pf,&pf->dstRect) < Success)
		goto error;

	/* Set destination rect as relative to picture frame. */
	set_rect_relto(&pf->dstRect, &pf->frame);

	/* Read transfer mode field. */

	if(read_short(pf,&pf->tmode) < Success)
		goto error;

#ifdef PRINTSTUFF

	printf("srect t %d l %d b %d r %d\n" 
	       "drect t %d l %d b %d r %d\n" 
	       "tmode %d\n", 
			pf->srcRect.top,
			pf->srcRect.left,
			pf->srcRect.bot,
			pf->srcRect.right,

			pf->dstRect.top,
			pf->dstRect.left,
			pf->dstRect.bot,
			pf->dstRect.right,

		    pf->tmode );

#endif /* PRINTSTUFF */

	if(pf->op & 1) /* ops 0x90 and 0x98 and 0x9a have no clipping region 
					* ops 0x91 and 0x99 and 0x9b do */
	{
		skip_region(pf); /* we don't use these now */
	}

	/* Set up state environment for pixel transfer. */

	if(init_pixmap_transfer(pf) < Success)
		goto error;

	/* If we are doing full screens transfer the whole pixMap to the screen */

	if(pf->mode == PF_FULLFRAME)
	{
		if(pf->pm.pixelSize > 8)
			return(pf->lasterr = Err_unimpl);
		return(pixmap_to_screen(pf));
	}

	/* else, We are doing RGB lines and just initalized the next pixMap. */

	return(pf->lasterr = RET_PMAPSTART);
error:
	return(pf->lasterr);
}
static Errcode skip_pmap_data(Pfile *pf, pixMap *pm)
/* Seek by all data for an ignored pixMap */
{
	if(!(pm->myflags & PM_NOCTABLE))
	{
		if(skip_colorTable(pf) < Success)
			goto error;
	}
	skip_bitsrect(pf,pm);
error:
	return(pf->lasterr);
}
Errcode skip_pattern(Pfile *pf)
/* Seek over and ignore a fill patern description. */
{
SHORT patType;
pixMap pm;

	if(read_short(pf,&patType) < Success)
		goto error;

#ifdef PRINTSTUFF
	printf("pat%d ", patType );
#endif /* PRINTSTUFF */

	pf_seek_bytes(pf,8);  /* skip over pattern words */

	switch(patType)
	{
		case 1:
		{
			if(read_pixMap(pf,&pm) < Success)
				goto error;
			skip_pmap_data(pf,&pm);
			break;
		}
		case 2: /* dither pattern */
			pf_seek_bytes(pf,sizeof(RGBColor));
			break;
		default:
			pf->lasterr = Err_format;
			break;
	}

error:
	return(pf->lasterr);
}
Errcode do_truecolor(Pfile *pf)
/* Read the pixel map header info and prepare to read the next RGB line
 * from that pixMap or return info if scanning. */
{
	/* Seek past a dummy size of 0 and an end of file opcode */
	if(pf_seek_bytes(pf,4) < Success)
		goto error;

#ifdef  PRINTSTUFF
	printf("True color!!\n");
#endif

	do_pixmap(pf);

error:
	return(pf->lasterr);
}
Errcode pict_rgb_readline(Pfile *pf, Rgb3 *linebuf)
/* Read next available RGB line in the file. */
{
int ret;

	++pf->last_rgbline; /* next please */

#ifdef PRINTSTUFF
	printf("readline %d", pf->last_rgbline );
#endif

	for(;;)
	{
		if(pf->lasterr < Success)
			return(pf->lasterr);

		if(pf->lasterr == RET_EOPIC) /* Alas, no more pixels. */
			goto stuff_white;

		if(pf->last_rgbline >= pf->ainfo.height) /* bigger than pic size ! */
			return(Err_eof);

		if(pf->last_rgbline < pf->dstRect.top) /* gap in pixel maps! */
			goto stuff_white;

		/* Scan through opcodes to next pixMap record or to EOPIC if
		 * the line we want is beyond the last one in the current pixMap. */

		if(pf->last_rgbline >= pf->dstRect.bot)
			do_pictops(pf);
		else
			break;
	}

	/* Read compressed line of pixels. */
	if(pf->read_line(pf,pf->readbuf,pf->max_read_size) < 0)
		goto error;

	/* Clip if somehow outside of line. */
	if(((USHORT)pf->dstRect.left) >= pf->ainfo.width)
		goto stuff_white;

	/* Unpack line of pixels. */
	if((ret = pf->unpack_line(pf->readbuf,pf->bitbuf,
								  pf->pm.rowBytes)) < Success)
	{
		pf->lasterr = ret;
		goto error;
	}

	if(pf->dstRect.left > 0 || pf->dstRect.right > pf->ainfo.width)
	{
		/* If we are somehow offset from output line put in what we got
		 * and leave the rest white. This is really an error. */

		stuff_bytes(0xFF,linebuf,pf->pixwidth);
		pf->make_pixels(pf->bitbuf,pf->pixbuf,pf->maxpix,pf->pm.rowBytes);
		copy_bytes( pf->pixbuf, linebuf + pf->dstRect.left,
				(pf->ainfo.width - pf->dstRect.left)*sizeof(Rgb3) );
	}
	else 
		pf->make_pixels(pf->bitbuf,(UBYTE *)linebuf,
						OPTR(linebuf,pf->pixwidth),pf->pm.rowBytes);

	/* We did this one ok, next line please. */
	++pf->dstRect.top;  
	return(Success);

stuff_white:
	stuff_bytes(0xFF,linebuf,pf->pixwidth);
	return(Success);

error:
	printf("error");
	return(pf->lasterr);
}
