
/* lbm.c - source code to the Amiga style IFF FORM ILBM (Amiga&Dpaint)
 * and IFF FORM PBM (Dpaint II IBM) .PDR picture driver */


#define REXLIB_INTERNALS
#include "errcodes.h"
#include "stdio.h"
#include "picdrive.h"
#include "syslib.h"
#include "jiff.h"

typedef struct ifile {
/** This structure starts with and Image_file, and has further data
 ** local to the PDR.  
 **/
	Image_file hdr;
	FILE *file;
	struct BitMapHeader bh;
	Boolean is_pbm;
	long fclen;
} Ifile;

#define copy_bytes(source,dest,count) memcpy(dest,source,count)
#define clear_mem(dest,count) memset(dest,0,count)
#define clear_struct(s) clear_mem(s, sizeof(s))

Errcode unpic_line(FILE *f, UBYTE *buf, int len);

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
 * Tell host that we can only write 8 bit-a-pixel images,  and only one
 * frame, any width/height.
 ****************************************************************************/
{
Boolean nofit;

nofit = (ainfo->depth == 8
		 && ainfo->num_frames == 1);
ainfo->depth = 8;
ainfo->num_frames = 1;
ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;
return(nofit);	/* return whether fit was exact */
}

static Errcode find_anim_info(Ifile *gf, Anim_info *ainfo)
/*****************************************************************************
 * Search for the BMHD chunk, and fill in the ainfo from it.
 ****************************************************************************/
{
FILE *f = gf->file;
long size = gf->fclen;
long file_pos = sizeof(struct form_chunk);
Errcode err;
struct iff_chunk chunk;
struct BitMapHeader bh;

		/* fill in the rudiments of structure, and then go fishing for
		 * the resolution */
clear_struct(ainfo);
spec_best_fit(ainfo);
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
		if (fread(&bh, sizeof(bh), 1, f) != 1)
			{
			err = Err_truncated;
			goto ERROR;
			}
		intel_swap(&bh.w);
		intel_swap(&bh.h);
		intel_swap(&bh.x);
		intel_swap(&bh.y);
		ainfo->width = bh.w;
		ainfo->height = bh.h;
		ainfo->x = bh.x;
		ainfo->y = bh.y;
		if (bh.nPlanes > 8)
			return(Err_pdepth_not_avail);
		if (bh.compression > 1)
			return(Err_version);
		gf->bh = bh;
		return(Success);
		}
	if (chunk.iff_length & 1)
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


static Errcode open_file(Pdr *pd, char *path, Image_file **pif, 
						 Anim_info *ainfo )
/*****************************************************************************
 * Open up the file, and if possible verify header. 
 ****************************************************************************/
{
Ifile *gf;
Errcode err;
FILE *f;
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
if (memcmp("FORM", fc.fc_type.b4_name, 4) != 0)
	{
	err = Err_bad_magic;
	goto ERROR;
	}
if (memcmp("PBM ", fc.fc_subtype.b4_name, 4) == 0)
	{
	gf->is_pbm = TRUE;
	}
else if (memcmp("ILBM", fc.fc_subtype.b4_name, 4) != 0)
	{
	err = Err_bad_magic;
	goto ERROR;
	}
long_intel_swap(&fc.fc_length);
gf->fclen = fc.fc_length - 4;	/* have skipped the "ILBM" */
if (ainfo != NULL)		/* fill in ainfo structure if any */
	{
	if ((err = find_anim_info(gf,ainfo)) < Success)
		goto ERROR;
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
free(gf);
*pgf = NULL;
}

static void bits_to_bytes(UBYTE *in, UBYTE *out, int w, UBYTE out_mask)
/*****************************************************************************
 * Subroutine to help convert from bit-plane to byte-a-pixel representation
 * or the out_mask into out byte-plane wherever a bit in in bit-plane is set.
 ****************************************************************************/
{
int k;
UBYTE imask;
UBYTE c;

while (w > 0)
	{
	imask = 0x80;
	k = 8;
	if (k > w)
		k = w;
	c = *in++;
	while (--k >= 0)
		{
		if (c&imask)
			*out |= out_mask;
		out += 1;
		imask >>= 1;
		}
	w -= 8;
	}
}


static Errcode decode_body(Ifile *gf, Rcel *screen)
/*****************************************************************************
 * Read in a BODY chunk into the screen
 ****************************************************************************/
{
Errcode err = Success;
FILE f = gf->file;
struct BitMapHeader bh = gf->bh;
UBYTE *pix_buf = NULL;
UBYTE *unc_buf = NULL;
int bpr;
int i;
int j;
UBYTE omask;

if ((pix_buf = malloc(bh.w)) == NULL)
	{
	err = Err_no_memory;
	goto ERROR;
	}
if (gf->is_pbm)
	{
	if (bh.nPlanes != 8)
		{
		err = Err_pdepth_not_avail;
		goto ERROR;
		}
	for (i=0; i<bh.h; ++i)
		{
		if ((err = unpic_line(f, pix_buf, bh.w)) < Success)
			goto ERROR;
		pj_put_hseg(screen, pix_buf, 0, i, bh.w);
		}
	}
else
	{
	bpr = ((bh.w+7)>>3);
	if ((unc_buf = malloc(bpr+128)) == NULL)	/* 128 extra for bad data */
		{
		err = Err_no_memory;
		goto ERROR;
		}
	for (i=0; i<bh.h; ++i)
		{
		omask = 1;
		j = bh.nPlanes;
		clear_mem(pix_buf, bh.w);
		while (--j >= 0)
			{
			if ((err = unpic_line(f, unc_buf, bpr)) < Success)
				goto ERROR;
			bits_to_bytes(unc_buf, pix_buf, bh.w, omask);
			omask <<= 1;
			}
		pj_put_hseg(screen, pix_buf, 0, i, bh.w);
		}
	}

ERROR:
if (pix_buf != NULL)
	free(pix_buf);
if (unc_buf != NULL)
	free(unc_buf);
return(err);
}

static Errcode read_first(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * Seek to the beginning of an open  file, and then read in the
 * first frame of image into screen.  (In our case read in the only
 * frame of image.)
 ****************************************************************************/
{
Ifile *gf = (Ifile *)ifile;		/* There's a bit of data past ifile header */
FILE *f = gf->file;
long size = gf->fclen;
long file_pos = sizeof(struct form_chunk);
Errcode err;
struct iff_chunk chunk;
int csize;

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
		if ((err = decode_body(gf, screen)) < Success)
			goto ERROR;
		}
	if (chunk.iff_length & 1)
		{
		chunk.iff_length += 1;
		}
	chunk.iff_length += sizeof(chunk);
	size -= chunk.iff_length;
	file_pos += chunk.iff_length;
	}
ERROR:
	return(err);
}

static Errcode read_next(Image_file *ifile,Rcel *screen)
/*****************************************************************************
 * Read in subsequent frames of image.  Since we only have one  this
 * routine is pretty trivial. 
 ****************************************************************************/
{
return(Success);
}

Hostlib _a_a_stdiolib = { NULL, AA_STDIOLIB, AA_STDIOLIB_VERSION };
Hostlib _a_a_gfxlib = { &_a_a_stdiolib, AA_GFXLIB, AA_GFXLIB_VERSION };
Hostlib _a_a_syslib = { &_a_a_gfxlib, AA_SYSLIB, AA_SYSLIB_VERSION };

static char title_info[] = "IFF ILBM (Amiga and Deluxe Paint).";

Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, &_a_a_syslib },
	title_info, 		/* title_info */
	"",  				/* long_info */
	".LBM",		 		/* default_suffi */
	0,1,		 		/* max_write_frames, max_read_frames */
	spec_best_fit,		/* (*spec_best_fit)() */
	NOFUNC,				/* (*create_image_file)() */
	open_file,			/* (*open_image_file)() */
	close_file,			/* (*close_image_file)() */
	read_first,			/* (*read_first_frame)() */
	read_next,			/* (*read_delta_next)() */
	NOFUNC,				/* (*save_frames)() */
};


