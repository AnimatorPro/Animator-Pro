
/* rif.c - source code to the Amiga ANIM format
 * driver */

#define REXLIB_INTERNALS
#include "errcodes.h"
#include "stdtypes.h"
#include "stdio.h"
#include "picdrive.h"
#include "syslib.h"
#include "jiff.h"
#include "anim.h"


Boolean suffix_in(char *string, char *suff);
Errcode conv_bitmaps(UBYTE *planes[], int pcount, 
					 int bpr, int width, int height, Rcel *screen);
UBYTE *decode_vplane(UBYTE *comp, UBYTE *plane, int BytesPerRow);
UBYTE *decode_vkplane(UBYTE *comp, UBYTE *plane, 
					  int BytesPerRow, int *ytable);

#define ISUFFI ".ANI"

typedef struct planes
	{
	UBYTE *p[8];
	} Planes;

typedef struct ifile {
/** This structure starts with and Image_file, and has further data
 ** local to the PDR.  
 **/
	Image_file hdr;
	FILE *file;
	Boolean got_bufs;			/* True if have read in BitMapHeader &
								 * allocated buffers. */
	long anim_size;				/* Size of Anim file - header */
	struct BitMapHeader bh;
	AnimationHeader ah;
	int speed;
	int frame_count;
	int frame_ix;
	long fpos;					/* file position */
	UBYTE *sbuf;
	Planes p[2];
	UBYTE *s[2];
	int bpr;
	long psize;
	long bpsize;
	int *ytable;
} Ifile;



#define copy_bytes(source,dest,count) memcpy(dest,source,count)
#define clear_mem(dest,count) memset(dest,0,count)
#define clear_struct(s) clear_mem(s, sizeof(s))



intel_swap(void *pt)
/*****************************************************************************
 * Convert a 16 bit quantity between 68000 and intel format.
 ****************************************************************************/
{
#define x ((char *)pt)
char swap;

swap = x[0];
x[0] = x[1];
x[1] = swap;
#undef x
}

long_intel_swap(void *pt)
/*****************************************************************************
 * Convert a 32 bit quantity between 68000 and intel format.
 ****************************************************************************/
{
#define x ((SHORT *)pt)
SHORT swap;

swap = x[0];
x[0] = x[1];
x[1] = swap;
intel_swap((char *)x);
intel_swap((char *)(x+1));
#undef x
}


static Boolean spec_best_fit(Anim_info *ainfo)
/*****************************************************************************
 * Tell host that we can only write 8 bit-a-pixel images,
 * fixed width/height.
 ****************************************************************************/
{
Boolean nofit;

nofit = (ainfo->depth == 8);
ainfo->depth = 8;
return(nofit);	/* return whether fit was exact */
}


static Errcode make_ytable(int **pytable, int bpr, int height)
/*****************************************************************************
 * Allocate and fill in a table of starting addresses for each line of
 * bitplane.
 ****************************************************************************/
{
int *pt;
int acc;

if ((*pytable = pt = malloc(height * sizeof(*pt))) == NULL)
	return(Err_no_memory);
acc = 0;
while (--height >= 0)
	{
	*pt++ = acc;
	acc += bpr;
	}
return(Success);
}

static void init_planes(UBYTE *sbuf, Planes *p, int pcount, long psize)
/*****************************************************************************
 * Fill in an array of pointers with evenly spaced parts of a buffer.
 * (Represent a block of memory as a series of planes.)
 ****************************************************************************/
{
int i;

for (i=0; i<pcount; i++)
	{
	p->p[i] = sbuf;
	sbuf += psize;
	}
}

static Errcode bufs_for_bmhd(Ifile *gf)
/*****************************************************************************
 * Set up 2 amiga screen simulations and other initialization based on
 * gf->bh.
 ****************************************************************************/
{
Errcode err = Success;
UBYTE *sbuf;
long bpsize;
int bpr;
int psize;
int pcount = gf->bh.nPlanes;

if (!gf->got_bufs)	/* make sure we don't do this twice... */
	{
	gf->bpr = bpr = ((gf->bh.w+7)>>3);
	gf->psize = psize = bpr*gf->bh.h;
	gf->bpsize = bpsize = psize*pcount;
	if ((gf->sbuf = sbuf = malloc(2*bpsize)) == NULL)
		{
		err = Err_no_memory;
		goto ERROR;
		}
	init_planes(gf->s[0] = sbuf, &gf->p[0], pcount, psize);
	sbuf += bpsize;
	init_planes(gf->s[1] = sbuf, &gf->p[1], pcount, psize);
	if ((err = make_ytable(&gf->ytable, bpr, gf->bh.h)) < Success)
		goto ERROR;
	gf->got_bufs = TRUE;
	}
ERROR:
	return(err);
}

static Errcode read_bmhd(FILE *f, struct BitMapHeader *bm)
/*****************************************************************************
 * Read in BitMapHeader from current file position.  Do necessary
 * Intel/Motorola byte swapping.
 ****************************************************************************/
{
if (fread(bm, sizeof(*bm), 1, f) != 1)
	{
	return(Err_truncated);
	}
intel_swap(&bm->w);
intel_swap(&bm->h);
intel_swap(&bm->x);
intel_swap(&bm->y);
if (bm->nPlanes > 8)
	return(Err_pdepth_not_avail);
if (bm->compression > 1)
	return(Err_version);
#ifdef DEBUG
printf("bmhd: w %d h %d x %d y %d nPlanes %d compression %d masking %#x"
, bm->w, bm->h, bm->x, bm->y, bm->nPlanes, bm->compression, bm->masking);
#endif /* DEBUG */
return(Success);
}

static Errcode find_bmhd(FILE *f, struct BitMapHeader *bm, 
						 long size, long file_pos)
/*****************************************************************************
 * Search for the BMHD chunk, and fill it in.
 ****************************************************************************/
{
Errcode err;
struct iff_chunk chunk;

		/* Loop around looking for a BMHD chunk */
while (size > 0)
	{
	if ((err = fseek(f, file_pos, SEEK_SET)) < Success)
		goto ERROR;
	if (fread(&chunk, sizeof(chunk), 1, f) != 1)
		{
		err = Err_truncated;
		goto ERROR;
		}
	long_intel_swap(&chunk.iff_length);
	if (memcmp(chunk.iff_type.b4_name, "BMHD",4) == 0)
		{
		if ((err = read_bmhd(f, bm)) < Success)
			goto ERROR;
		return(Success);
		}
	if (chunk.iff_length & 1)	/* pad odd sizes */
		{
		chunk.iff_length += 1;
		}
	chunk.iff_length += sizeof(chunk);
	size -= chunk.iff_length;
	file_pos += chunk.iff_length;
	}
ERROR:
	return(Err_format);
}

static Boolean is_ilbm(struct form_chunk *c)
{
return(memcmp(c->fc_type.b4_name, "FORM", 4) == 0 &&
	   memcmp(c->fc_subtype.b4_name, "ILBM", 4) == 0);
}

static Errcode count_frames(Ifile *gf)
/*****************************************************************************
 * Scan through file counting each frame, and also setting up
 * buffers and reading the BMHD if necessary.
 ****************************************************************************/
{
FILE *f = gf->file;
long size = gf->anim_size;
long file_pos = sizeof(struct form_chunk);
long plen;		/* padded length */
Errcode err = Success;
struct form_chunk chunk;
int fcount = 0;

gf->speed = DEFAULT_AINFO_SPEED;
while (file_pos < size)
	{
	if ((err = fseek(f, file_pos, SEEK_SET)) < Success)
		goto ERROR;
	if (fread(&chunk, sizeof(chunk), 1, f) != 1)
		{
		err = Err_truncated;
		goto ERROR;
		}
	long_intel_swap(&chunk.fc_length);
	if (is_ilbm(&chunk))
		{
		if (!gf->got_bufs)
			{
			if ((err = find_bmhd(f, &gf->bh, 
							     chunk.fc_length-4,
								 file_pos+sizeof(chunk))) < Success)
				goto ERROR;
			if ((err = bufs_for_bmhd(gf)) < Success)
				goto ERROR;
			}
		++fcount;
		}
	plen = ((chunk.fc_length+1)&0xfffffffe);
	file_pos += plen + sizeof(struct iff_chunk);
	}
gf->frame_count = fcount - 2;		/* Anim files have 2 loop frames */
ERROR:
	return(err);
}

static Errcode open_file(Pdr *pd, char *path, Image_file **pif, 
						 Anim_info *ainfo )
/*****************************************************************************
 * Open up the file, and if possible verify header. 
 ****************************************************************************/
{
Ifile *gf;
FILE *f;
Errcode err = Success;
struct form_chunk fc;

*pif = NULL;	/* for better error recovery */

if((gf = zalloc(sizeof(*gf))) == NULL)
	return(Err_no_memory);

if((f = gf->file = fopen(path, "rb")) == NULL)
	{
	err = pj_errno_errcode();
	goto ERROR;
	}
if (fread(&fc, sizeof(fc), 1, f) != 1)
	{
	err = Err_truncated;
	goto ERROR;
	}
if (memcmp(fc.fc_type.b4_name,"FORM",4) != 0 || 
	memcmp(fc.fc_subtype.b4_name,"ANIM",4) != 0)
	{
	err = Err_bad_magic;
	goto ERROR;
	}
long_intel_swap(&fc.fc_length);
gf->anim_size = fc.fc_length-4;
if (ainfo != NULL)		/* fill in ainfo structure if any */
	{
	if ((err = count_frames(gf)) < Success)
		goto ERROR;
	clear_struct(ainfo);
	ainfo->depth = 8;
	ainfo->width = gf->bh.w;
	ainfo->height = gf->bh.h;
	ainfo->x = gf->bh.x;
	ainfo->y = gf->bh.y;
	ainfo->num_frames = gf->frame_count;
	ainfo->millisec_per_frame = gf->speed;
	}
*pif = (Image_file *)gf;
return(Success);

ERROR:
close_file(&gf);
return(err);
}

static close_file(Ifile **pgf)
/*****************************************************************************
 * Clean up resources used by picture driver in loading (saving) a file.
 ****************************************************************************/
{
Ifile *gf;

if(pgf == NULL || (gf = *pgf) == NULL)
	return;
if(gf->file)
	fclose(gf->file);
if (gf->sbuf != NULL)
	free(gf->sbuf);
if (gf->ytable != NULL)
	free(gf->ytable);
free(gf);
*pgf = NULL;
}

static Errcode decode_dlta(Ifile *gf, long size, Planes *p, UBYTE *cbuf)
/*****************************************************************************
 * Decode a delta update from memory buffer into Planes.
 ****************************************************************************/
{
Errcode err = Success;
int d = gf->bh.nPlanes;
int bpr = gf->bpr;
int *ytable = gf->ytable;
UBYTE **ppt = p->p;
long *offsets;
long off;

offsets = (long *)cbuf;

while (--d >= 0)
	{
	long_intel_swap(offsets);
	off = *offsets++;
	if (off)
		decode_vkplane(cbuf+off, *ppt, bpr, ytable);
	++ppt;
	}
return(err);
}

Errcode decode_pline(UBYTE **pcbuf, UBYTE *p, int size)
{
char op, d;
int count;
UBYTE *cbuf = *pcbuf;

while (size > 0)
	{
	op = *cbuf++;
	if (op < 0)	/* it's a run */
		{
		d = *cbuf++;
		count = -op+1;
		size -= count;
		while (--count >= 0)
			*p++ = d;
		}
	else
		{
		count = op+1;
		size -= count;
		while (--count >= 0)
			*p++ = *cbuf++;
		}
	}
*pcbuf = cbuf;
return(size == 0 ? Success : Err_format);
}

static Errcode decode_body(Ifile *gf, long size, Planes *p, UBYTE *cbuf)
{
Errcode err = Success;
int d = gf->bh.nPlanes;
int y = gf->bh.h;
int bpr = gf->bpr;
Planes lp = *p;
UBYTE *pt;
UBYTE **ppt;
int i;

while (--y >= 0)
	{
	ppt = lp.p;
	i = d;
	while (--i >= 0)
		{
		pt = *ppt;
		if ((err = decode_pline(&cbuf, pt, bpr)) < Success)
			goto ERROR;
		*ppt++ = pt+bpr;
		}
	}
ERROR:
	return(err);
}


static Errcode read_frame(Ifile *gf, long size, 
							Planes *p, Boolean is_dlta)
/*****************************************************************************
 * File is just past frame iff_chunk (in frame proper).
 * Read frame into p.
 ****************************************************************************/
{
FILE *f = gf->file;
Errcode err = Success;
UBYTE *cbuf = NULL;

if ((cbuf = malloc(size)) == NULL)
	goto ERROR;
if (fread(cbuf, size, 1, gf->file) != 1)
	{
	err = Err_truncated;
	goto ERROR;
	}
if (is_dlta)
	err = decode_dlta(gf, size, p, cbuf);
else
	err = decode_body(gf, size, p, cbuf);
ERROR:
if (cbuf != NULL)
	free(cbuf);
return(err);
}


static Errcode read_ilbm(Ifile *gf, Rcel *screen, Planes *p,
						 long size, long pos)
/*****************************************************************************
 * On entry file should be past the form_chunk of an FORM ILBM.
 * This guy parses the ILBM.
 ****************************************************************************/
{
FILE *f = gf->file;
long end = pos+size;
long plen;		/* padded length (to even) */
int csize;
Errcode err = Success;
struct iff_chunk chunk;

while (pos < end)
	{
	if ((err = fseek(f, pos, SEEK_SET)) < Success)
		goto ERROR;
	if (fread(&chunk, sizeof(chunk), 1, f) != 1)
		{
		err = Err_truncated;
		goto ERROR;
		}
	long_intel_swap(&chunk.iff_length);
	if (memcmp(chunk.iff_type.b4_name, "CMAP",4) == 0)
		{
		csize = (1<<gf->bh.nPlanes);
		if (fread(screen->cmap->ctab, 3, csize, f) != csize)
			{
			err = Err_truncated;
			goto ERROR;
			}
		pj_cmap_load(screen,screen->cmap);
		}
	else if (memcmp(chunk.iff_type.b4_name, "BODY", 4) == 0)
		{
		if (!gf->got_bufs)
			{
			err = Err_format;
			goto ERROR;
			}
		if ((err = read_frame(gf, chunk.iff_length, 
								p, FALSE)) < Success)
			goto ERROR;
		}
	else if (memcmp(chunk.iff_type.b4_name, "DLTA", 4) == 0)
		{
		if (!gf->got_bufs)
			{
			err = Err_format;
			goto ERROR;
			}
		if (gf->ah.operation != 5)
			{
			err = Err_version;
			goto ERROR;
			}
		if ((err = read_frame(gf, chunk.iff_length, 
								p, TRUE)) < Success)
			goto ERROR;
		}
	else if (memcmp(chunk.iff_type.b4_name, "ANHD", 4) == 0)
		{
		if (fread(&gf->ah, sizeof(gf->ah), 1, f) != 1)
			{
			err = Err_truncated;
			goto ERROR;
			}
		intel_swap(&gf->ah.w);
		intel_swap(&gf->ah.h);
		intel_swap(&gf->ah.x);
		intel_swap(&gf->ah.y);
		long_intel_swap(&gf->ah.abstime);
		long_intel_swap(&gf->ah.reltime);
		long_intel_swap(&gf->ah.bits);
		gf->speed = 1000*gf->ah.reltime/60;
#ifdef DEBUG
		printf(
		"Ah: operation %x, mask %x, w %d, h %d, x %d, y %d, "
		"interleave %d, bits %ld\n"
		, gf->ah.operation, gf->ah.mask, gf->ah.w, gf->ah.h
		, gf->ah.x, gf->ah.y, gf->ah.interleave, gf->ah.bits);
#endif /* DEBUG */
		}
	else if (memcmp(chunk.iff_type.b4_name, "BMHD", 4) == 0)
		{
		if (!gf->got_bufs)
			{
			if ((err = read_bmhd(f, &gf->bh)) < Success)
				goto ERROR;
			if ((err = bufs_for_bmhd(gf)) < Success)
				goto ERROR;
			}
		}
	plen = ((chunk.iff_length+1)&0xfffffffe);
	pos += plen + sizeof(struct iff_chunk);
	}
ERROR:
	return(err);
}


static Errcode read_next(Image_file *ifile,Rcel *screen)
/*****************************************************************************
 * Read in subsequent frames of image.  
 ****************************************************************************/
{
Ifile *gf = (Ifile *)ifile;		/* There's a bit of data past ifile header */
FILE *f = gf->file;
Errcode err = Success;
struct form_chunk chunk;
Planes *p = &gf->p[gf->frame_ix&1];
long plen;						/* padded (to even) length */

for (;;)						/* Search for 1st FORM ILBM */
	{
	if ((err = fseek(f, gf->fpos, SEEK_SET)) < Success)
		goto ERROR;
	if (fread(&chunk, sizeof(chunk), 1, f) != 1)
		{
		err = Err_truncated;
		goto ERROR;
		}
	long_intel_swap(&chunk.fc_length);
	if (chunk.fc_length < 0)	/* negative length chunks would hang us */
		{
		err = Err_format;
		goto ERROR;
		}
	plen = ((chunk.fc_length+1)&0xfffffffe);
	gf->fpos += plen + sizeof(struct iff_chunk);
	if (is_ilbm(&chunk))
		break;
	}
if (gf->frame_ix == 1)	/* frame 1 have to copy frame 0 into it first */
	memcpy(gf->s[1], gf->s[0], gf->bpsize);
if ((err = read_ilbm(gf, screen, p,
					 chunk.fc_length-4, ftell(f))) < Success)
	goto ERROR;
++gf->frame_ix;

CONV_SCREEN:
err = conv_bitmaps(p->p, gf->bh.nPlanes, 
				   gf->bpr, gf->bh.w, gf->bh.h, screen);

ERROR:
	return(err);
}



static Errcode read_first(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * Seek to the beginning of an open  file, and then read in the
 * first frame of image into screen.  
 ****************************************************************************/
{
Ifile *gf = (Ifile *)ifile;		/* There's a bit of data past ifile header */

				/* skip past header */
gf->fpos = sizeof(struct form_chunk);
gf->frame_ix = 0;
return(read_next(ifile, screen));
}

Hostlib _a_a_stdiolib = { NULL, AA_STDIOLIB, AA_STDIOLIB_VERSION };
Hostlib _a_a_gfxlib = { &_a_a_stdiolib, AA_GFXLIB, AA_GFXLIB_VERSION };
Hostlib _a_a_syslib = { &_a_a_gfxlib, AA_SYSLIB, AA_SYSLIB_VERSION };

static char title_info[] = "Amiga ANIM (DPaint III) format.";

Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, &_a_a_syslib },
	title_info, 		/* title_info */
	"",  				/* long_info */
	ISUFFI,		 		/* default_suffi */
	0,4000,		 		/* max_write_frames, max_read_frames */
	spec_best_fit,		/* (*spec_best_fit)() */
	NOFUNC,				/* (*create_image_file)() */
	open_file,			/* (*open_image_file)() */
	close_file,			/* (*close_image_file)() */
	read_first,			/* (*read_first_frame)() */
	read_next,			/* (*read_delta_next)() */
	NOFUNC,				/* (*save_frames)() */
};


