
/* degas.c - source code to the Atari ST Degas format PJ .PDR picture 
 * driver */

#define REXLIB_INTERNALS
#include "errcodes.h"
#include "stdtypes.h"
#include "stdio.h"
#include "picdrive.h"
#include "syslib.h"
#include "ptrmacro.h"

Errcode unpic_line(FILE *f, UBYTE *buf, int len);

#define copy_bytes(source,dest,count) memcpy(dest,source,count)
#define clear_mem(dest,count) memset(dest,0,count)
#define clear_struct(s) clear_mem(s, sizeof(s))

typedef struct degas_head
{
	SHORT magic; 
	USHORT colormap[16];
} Degas_head;

typedef struct ifile {
/** This structure starts with an Image_file, and has further data
 ** local to the PDR.  
 **/
	Image_file hdr;
	FILE *file;
	Degas_head degas;
	int width;			/* x resolution */
	int height;			/* y resolution */
	int depth;			/* # of bits per pixel */
	int bpr;			/* bytes in a line in ST format */
	int wpl;			/* # of words in 1 line of single bitplane */
	int magic;			/* magic # at start of file */
	Boolean compressed;	/* is it compressed? */
	USHORT lbuf[80];	/* buffer for line in ST format */
	UBYTE bap_buf[640]; /* buffer for line in byte-a-pixel format */
} Ifile;


Boolean suffix_in(char *string, char *suff);

/** Tables with one entry for each DEGAS picture type */
char *dt_suffis[] = { ".PI1", ".PI2", ".PI3", ".PC1", ".PC2", ".PC3" };
int dt_width[] 	= { 320, 640, 640, 320, 640, 640 };
int dt_height[] = { 200, 200, 400, 200, 200, 400 };
int dt_depth[] 	= {   4,   2,   1,   4,   2,   1 };
int dt_bpr[]	= { 160, 160,  80, 160, 160,  80 };
int dt_wpl[]	= {  20,  40,  40,  20,  40,  40 };
Boolean dt_comp[] = { FALSE, FALSE, FALSE, TRUE, TRUE, TRUE };
SHORT dt_magic[] = { 0, 1, 2, 0x8000, 0x8001, 0x8002 };

Errcode set_dims_for_suffi(Ifile *gf, char *name)
/*****************************************************************************
 * Search for matching suffix to name, and set up various dimension 
 * storing variables in Ifile to match it.
 ****************************************************************************/
{
int ix;

for (ix = 0; ix < Array_els(dt_suffis); ++ix)
	{
	if (suffix_in(name, dt_suffis[ix]))
		{
		gf->width = dt_width[ix];
		gf->height = dt_height[ix];
		gf->depth = dt_depth[ix];
		gf->bpr = dt_bpr[ix];
		gf->wpl = dt_wpl[ix];
		gf->compressed = dt_comp[ix];
		gf->magic = dt_magic[ix];
		return(Success);
		}
	}
return(Err_suffix);
}


static void intel_swaps(void *p, int length)
/*****************************************************************************
 * Convert an array of 16 bit quantitys between 68000 and intel format.
 ****************************************************************************/
{
register char *x = p;
char swap;

while (--length >= 0)
	{
	swap = x[0];
	x[0] = x[1];
	x[1] = swap;
	x += 2;
	}
}


static Boolean spec_best_fit(Anim_info *ainfo)
/*****************************************************************************
 * Tell host that we can only write 8 bit-a-pixel images,  and only one
 * frame, fixed width/height.
 * (I'm a bit unsure if this is even used for a read-only driver.
 * It's not really properly done here, as this driver does handle a
 * few resolutions other than 320x200.
 ****************************************************************************/
{
Boolean nofit;

nofit = (ainfo->depth == 8
		 && ainfo->num_frames == 1
		 && ainfo->width == 320
		 && ainfo->height == 200);
ainfo->depth = 8;
ainfo->num_frames = 1;
ainfo->width = 320;
ainfo->height = 200;
ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;
return(nofit);	/* return whether fit was exact */
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

*pif = NULL;	/* for better error recovery */

if((gf = zalloc(sizeof(*gf))) == NULL)
	return(Err_no_memory);

if ((err = set_dims_for_suffi(gf, path)) < Success)
	goto ERROR;

if((f = gf->file = fopen(path, "rb")) == NULL)
	{
	err = pj_errno_errcode();
	goto ERROR;
	}

if (fread(&gf->degas, sizeof(gf->degas), 1, f) != 1)
	{
	err = Err_truncated;
	goto ERROR;
	}
intel_swaps(&gf->degas.magic, 1);
if (gf->degas.magic != gf->magic)
	{
	err = Err_bad_magic;
	goto ERROR;
	}
intel_swaps(&gf->degas.colormap[0], 16);
if (ainfo != NULL)		/* fill in ainfo structure if any */
	{
	clear_struct(ainfo);
	ainfo->width = gf->width;
	ainfo->height = gf->height;
	ainfo->depth = 8;
	ainfo->num_frames = 1;
	ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;
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

conv_st_cmap(USHORT *st_cmap, UBYTE *pj_ctab)
{
int i;
USHORT cm;

for (i=0; i<16; i++)
	{
	cm = *st_cmap++;
	*pj_ctab++ = ((cm&0x700)>>3);
	*pj_ctab++ = ((cm&0x070)<<1);
	*pj_ctab++ = ((cm&0x007)<<5);
	}
}

static void conv_st_line(USHORT *st_source, UBYTE *bap_dest,
						 int st_d, int st_bpr, int st_wpl)
/*****************************************************************************
 * Convert from Atari ST interleaved bit-plane representation to 
 * byte-a-pixel.
 ****************************************************************************/
{
USHORT *nword, *n;
UBYTE c;
int i, j, k;
USHORT mask;
UBYTE omask;

intel_swaps(st_source, st_bpr/2);
nword = st_source;
i = st_wpl;
while (--i >= 0)
	{
	mask = 0x8000;
	j = 16;
	while (--j >= 0)
		{
		c = 0;
		k = st_d;
		omask = 1;
		n = nword;
		while (--k >= 0)
			{
			if (mask & *n++)
				c |= omask;
			omask <<= 1;
			}
		*bap_dest++ = c;
		mask >>= 1;
		}
	nword += st_d;
	}
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

static Errcode read_uncompressed(Ifile *gf, Rcel *screen)
/*****************************************************************************
 * Read in uncompressed DEGAS file body and convert it into byte-a-pixel
 * format on screen.
 ****************************************************************************/
{
FILE *f = gf->file;
int width = gf->width;
int height = gf->height;
int bpr = gf->bpr;
int depth = gf->depth;
int wpl = gf->wpl;
USHORT *lbuf = gf->lbuf;
UBYTE *bap_buf = gf->bap_buf;
int i;

for (i=0; i<height; ++i)
	{
	if (fread(lbuf, bpr, 1, f) != 1)
		{
		return(Err_truncated);
		}
	conv_st_line(lbuf, bap_buf, depth, bpr, wpl);
	pj_put_hseg(screen, bap_buf, 0, i, width);
	}
return(Success);
}

static Errcode read_compressed(Ifile *gf, Rcel *screen)
/*****************************************************************************
 * Read in compressed DEGAS file body and convert it into byte-a-pixel
 * format on screen.  A DEGAS compressed file is stored in line-interleaved
 * bitplanes (like Amiga IFF-ILBM) rather than word-interleaved like the
 * Atari-ST screen.
 ****************************************************************************/
{
FILE *f = gf->file;
int width = gf->width;
int height = gf->height;
int depth = gf->depth;
int bpr = gf->bpr/depth;
UBYTE *lbuf = (UBYTE *)(gf->lbuf);
UBYTE *bap_buf = gf->bap_buf;
int i,j;
int omask;
Errcode err;

for (i=0; i<height; ++i)
	{
	clear_mem(bap_buf, width);
	j = depth;
	omask = 1;
	while (--j >= 0)
		{
		if ((err = unpic_line(f, lbuf, bpr)) < Success)
			return(err);
		bits_to_bytes(lbuf, bap_buf, width, omask);
		omask <<= 1;
		}
	pj_put_hseg(screen, bap_buf, 0, i, width);
	}
return(Success);
}



	/* cmap for white background, black forground */
static UBYTE mono_cmap[] = {0xff,0xff,0xff,0,0,0};


static Errcode read_first(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * Seek to the beginning of an open  file, and then read in the
 * first frame of image into screen.  (In our case read in the only
 * frame of image.)
 ****************************************************************************/
{
Ifile *gf = (Ifile *)ifile;		/* There's a bit of data past ifile header */
FILE *f = gf->file;
Errcode err;

if (gf->depth == 1)
	copy_bytes(mono_cmap,screen->cmap->ctab,sizeof(mono_cmap));
else
	conv_st_cmap(gf->degas.colormap, (UBYTE *)(screen->cmap->ctab));
pj_cmap_load(screen,screen->cmap);
if ((err = fseek(f, sizeof(Degas_head), SEEK_SET)) < Success)
	goto ERROR;
if (gf->compressed)
	err = read_compressed(gf, screen);
else
	err = read_uncompressed(gf, screen);
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

static char title_info[] = "Atari ST DEGAS format.";

Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, &_a_a_syslib },
	title_info, 		/* title_info */
	"",  				/* long_info */
	".PC?;.PI?", 		/* default_suffi */
	0,1,		 		/* max_write_frames, max_read_frames */
	spec_best_fit,		/* (*spec_best_fit)() */
	NOFUNC,				/* (*create_image_file)() */
	open_file,			/* (*open_image_file)() */
	close_file,			/* (*close_image_file)() */
	read_first,			/* (*read_first_frame)() */
	read_next,			/* (*read_delta_next)() */
	NOFUNC,				/* (*save_frames)() */
};


