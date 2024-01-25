
/* neo.c - source code to the Atari ST Neochrome format PJ .PDR picture 
 * driver */

#define REXLIB_INTERNALS
#include "errcodes.h"
#include "stdtypes.h"
#include "stdio.h"
#include "picdrive.h"
#include "syslib.h"


Boolean suffix_in(char *string, char *suff);

#define ISUFFI ".NEO"

typedef struct neo_head
{
	SHORT type;  /* 0 for neo, 1 for programmed pictures, 2 for cels? */
	SHORT resolution; /*0 lores, 1 medium, 2 hires*/
	USHORT colormap[16];
	char filename[8+1+3];
	SHORT ramp_seg; /*hibit active, bits 0-3 left arrow, 4-7 right arrow*/
	char ramp_active;  /*hi bit set if actively cycled*/
	char ramp_speed;  /*60hz ticks between cycles*/
	SHORT slide_time;  /*60hz ticks until next picture*/
	SHORT xoff, yoff;  /*upper left corner of cel*/
	SHORT width, height; /*dimensions of cel*/
	char	op;		/* xor this one, copy it? */
	char	compress;	/* compressed? */
	long	data_size;		/* size of data */
	char reserved[30];	/* Please leave this zero, I may expand later */
	char pad[30]; /* You can put some extra info here if you like */
} Neo_head;

typedef struct ifile {
/** This structure starts with and Image_file, and has further data
 ** local to the PDR.  
 **/
	Image_file hdr;
	FILE *file;
	Neo_head neo;
	USHORT lbuf[80];
	UBYTE bap_buf[320];
} Ifile;


#define copy_bytes(source,dest,count) memcpy(dest,source,count)
#define clear_mem(dest,count) memset(dest,0,count)
#define clear_struct(s) clear_mem(s, sizeof(s))


	/* fixed width/height of a NEO pic */
#define STW 320
#define STH 200	

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
 ****************************************************************************/
{
Boolean nofit;

nofit = (ainfo->depth == 8
		 && ainfo->num_frames == 1
		 && ainfo->width == STW
		 && ainfo->height == STH);
ainfo->depth = 8;
ainfo->num_frames = 1;
ainfo->width = STW;
ainfo->height = STH;
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
if (!suffix_in(path, ISUFFI))
	return(Err_suffix);

if((gf = zalloc(sizeof(*gf))) == NULL)
	return(Err_no_memory);

if((f = gf->file = fopen(path, "rb")) == NULL)
	{
	err = pj_errno_errcode();
	goto ERROR;
	}
if (fread(&gf->neo, sizeof(gf->neo), 1, f) != 1)
	{
	err = Err_truncated;
	goto ERROR;
	}
if (gf->neo.type != 0)
	{
	err = Err_bad_magic;
	goto ERROR;
	}
if (gf->neo.resolution != 0)
	{
	err = Err_version;
	goto ERROR;
	}
intel_swaps(&gf->neo.colormap[0], 16);
if (ainfo != NULL)		/* fill in ainfo structure if any */
	{
	clear_struct(ainfo);
	spec_best_fit(ainfo);
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
/*****************************************************************************
 * Convert color map from 0000 0RRR 0GGG 0BBB (16 bit) ST format to 
 * PJ 24 bit format.
 ****************************************************************************/
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
int i;

conv_st_cmap(gf->neo.colormap, (UBYTE *)(screen->cmap->ctab));
pj_cmap_load(screen,screen->cmap);
if ((err = fseek(f, sizeof(Neo_head), SEEK_SET)) < Success)
	goto ERROR;
for (i=0; i<200; ++i)
	{
	if (fread(gf->lbuf, 160, 1, f) != 1)
		{
		err = Err_truncated;
		goto ERROR;
		}
	conv_st_line(gf->lbuf, gf->bap_buf, 4, 160, 20);
	pj_put_hseg(screen, gf->bap_buf, 0, i, 320);
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

static char title_info[] = "Atari ST NeoChrome format.";

Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, &_a_a_syslib },
	title_info, 		/* title_info */
	"",  				/* long_info */
	ISUFFI,		 		/* default_suffi */
	0,1,		 		/* max_write_frames, max_read_frames */
	spec_best_fit,		/* (*spec_best_fit)() */
	NOFUNC,				/* (*create_image_file)() */
	open_file,			/* (*open_image_file)() */
	close_file,			/* (*close_image_file)() */
	read_first,			/* (*read_first_frame)() */
	read_next,			/* (*read_delta_next)() */
	NOFUNC,				/* (*save_frames)() */
};


