
/* seq.c - source code to the Atari ST Cyber Paint .SEQ format
 * driver */

#define REXLIB_INTERNALS
#include "errcodes.h"
#include "stdtypes.h"
#include "stdio.h"
#include "picdrive.h"
#include "syslib.h"
#include "seq.h"


Boolean suffix_in(char *string, char *suff);
void blit_box(int w, int h, int sx, int sy, unsigned char *s, int sbpr, 
			 int dx, int dy, unsigned char *d, int dbpr);
void xor_blit_box(int w, int h, int sx, int sy, unsigned char *s, int sbpr, 
			 int dx, int dy, unsigned char *d, int dbpr);
Errcode conv_bitmaps(UBYTE *planes[], int pcount, 
					 int bpr, int width, int height, Rcel *screen);

#define ISUFFI ".SEQ"

typedef struct ifile {
/** This structure starts with and Image_file, and has further data
 ** local to the PDR.  
 **/
	Image_file hdr;
	FILE *file;
	Seq_header seq;
	UBYTE stscreen[32000];		/* emulate an ST screen here */
} Ifile;


#define copy_bytes(source,dest,count) memcpy(dest,source,count)
#define clear_mem(dest,count) memset(dest,0,count)
#define clear_struct(s) clear_mem(s, sizeof(s))


	/* fixed width/height of a SEQ animation */
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

nofit = (ainfo->depth == 8
		 && ainfo->width == STW
		 && ainfo->height == STH);
ainfo->depth = 8;
ainfo->width = STW;
ainfo->height = STH;
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
if (fread(&gf->seq, sizeof(gf->seq), 1, f) != 1)
	{
	err = Err_truncated;
	goto ERROR;
	}
intel_swap(&gf->seq.magic);
intel_swap(&gf->seq.version);
long_intel_swap(&gf->seq.cel_count);
intel_swap(&gf->seq.speed);
if (gf->seq.magic != CYBER_MAGIC && gf->seq.magic != FLICKER_MAGIC)
	{
	err = Err_bad_magic;
	goto ERROR;
	}
if (gf->seq.version != 0 || gf->seq.resolution != 0)
	{
	err = Err_version;
	goto ERROR;
	}
if (ainfo != NULL)		/* fill in ainfo structure if any */
	{
	clear_struct(ainfo);
	ainfo->depth = 8;
	ainfo->width = STW;
	ainfo->height = STH;
	ainfo->num_frames = gf->seq.cel_count;
	ainfo->millisec_per_frame = 10*gf->seq.speed/60;
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


word_uncompress(SHORT *s, SHORT *d, int length)
/*****************************************************************************
 * Do word run-length decompression.  Note length is count in words, not
 * bytes.
 ****************************************************************************/
{
SHORT run_length;
SHORT run_data;

for (;;)
	{
	if (length <= 0)  /* check to see if out of data yet */
		break;
	run_length = *s++;
	length -= 1;	/* used up 1 SHORT of source */
	if (run_length & 0x8000)  /*  see if it's a compressed run or a literal */
		{
		run_length &= 0x7fff;	/* knock of the hi bit */
		length -= run_length;   /* used up lots more of source */
		while (--run_length >= 0)	/* go through loop run_length times */
			*d++ = *s++;
		}
	else	/* yeah, it's compressed a little */
		{	
		run_data = *s++;
		length -= 1;		/* used another SHORT of source */
		while (--run_length >= 0)
			*d++ = run_data;
		}
	}
}

columns_to_bitplanes(SHORT *s,SHORT *d,int wpl,int height)
/*****************************************************************************
 * Convert from column oriented bitplane to row oriented bitplane.
 * (In a compressed seq file,  after word-decompression the result
 * is such that the next word contains the 16 pixels underneath
 * the first word, not the 16 to the right.)
 ****************************************************************************/
{
int i, j, k;
SHORT *sp, *dp;
SHORT *sl, *dl;
unsigned plane_size;

plane_size = wpl*height;
/* step through planes */
for (i = 0; i< 4; i++)	
	{
	sp = s;
	dp = d;
	/* step through lines */
	for (j=0; j<height; j++)
		{
		sl = sp;
		dl = dp;
		/* step through words */
		for (k=0; k<wpl; k++)
			{
			*dl = *sl;
			dl += 1;
			sl += height;
			}
		sp += 1;
		dp += wpl;
		}
	d += plane_size;
	s += plane_size;
	}
}

de_interleave(SHORT *s,SHORT *d,int wpl,int height)
/*****************************************************************************
 * Convert from word-interleaved bitplanes (like ST screen) to normal
 * bitplanes.
 ****************************************************************************/
{
int i, j, k;
SHORT *sp, *dp;
SHORT *sl, *dl;
unsigned plane_size;

plane_size = wpl*height;
/* step through planes */
for (i = 0; i< 4; i++)	
	{
	sp = s;
	dp = d;
	/* step through lines */
	for (j=0; j<height; j++)
		{
		sl = sp;
		dl = dp;
		/* step through words */
		for (k=0; k<wpl; k++)
			{
			*dl = *sl;
			sl += 4;
			dl += 1;
			}
		sp += 4*wpl;
		dp += wpl;
		}
	s += 1;
	d += plane_size;
	}
}


static Errcode read_next(Image_file *ifile,Rcel *screen)
/*****************************************************************************
 * Read in subsequent frames of image.  
 ****************************************************************************/
{
Ifile *gf = (Ifile *)ifile;		/* There's a bit of data past ifile header */
FILE *f = gf->file;
Errcode err = Success;
Neo_head neo;
int sbpr;
unsigned plane_size;
unsigned buf_size;
SHORT *buf1 = NULL, *buf2 = NULL;
unsigned bsize;
UBYTE *planes;
UBYTE *stscreen = gf->stscreen;
int i;
UBYTE *stplanes[4];

			/** read in frame header,  swap around the fields that
			 ** need it from Intel to Motorola format,  and then
			 ** verify that the header is as it should be.
			 **/
if (fread(&neo, sizeof(neo), 1, f) != 1)
	{
	err = Err_truncated;
	goto ERROR;
	}
intel_swap(&neo.type);
intel_swap(&neo.resolution);
intel_swaps(neo.colormap, 16);
intel_swap(&neo.xoff);
intel_swap(&neo.yoff);
intel_swap(&neo.width);
intel_swap(&neo.height);
long_intel_swap(&neo.data_size);
sbpr = seq_Mask_line(neo.width);
plane_size = sbpr*neo.height;
buf_size = plane_size*4;	/* 4 bitplanes in ST display */
if (neo.type != -1)
	{
	err = Err_bad_record;
	goto ERROR;
	}
if (neo.compress == NEO_UNCOMPRESSED)
	{
	neo.data_size = buf_size;
	}
				/** Set the color map
				 **/
conv_st_cmap(neo.colormap, (UBYTE *)(screen->cmap->ctab));
pj_cmap_load(screen,screen->cmap);

				/** Check for zero length data.
				 ** Special (easy) case
				 **/
if (neo.data_size == 0)
	{
		/* if it's an xor don't have to do anything at all, cool breeze! */
		/* (otherwise it means clear the screen */
	if (neo.op != NEO_XOR)		
		clear_mem(stscreen, 32000);
	goto CONV_SCREEN;
	}
if (neo.data_size > 32000)	/* sanity check on data size */
	{
	err = Err_format;
	goto ERROR;
	}
				/** Allocate the buffers... 
				 **/
if ((buf1 = malloc(buf_size)) == NULL)
	{
	err = Err_no_memory;
	goto ERROR;
	}
if ((buf2 = malloc(buf_size)) == NULL)
	{
	err = Err_no_memory;
	goto ERROR;
	}

				/** Read in the possibly compressed and
				 ** generally in ST format data
				 **/
if (fread(buf1, neo.data_size, 1, f) != 1)
	{
	err = Err_truncated;
	goto ERROR;
	}
				/** Here we decompress data and convert it to
				 ** a bit-plane oriented cel.  planes points
				 ** to the beginning of the pixel data for this
				 ** cel.
				 **/
if (neo.compress == NEO_CCOLUMNS)
	{
	bsize = neo.data_size>>1;
	intel_swaps(buf1,bsize);	/* swap so counts are right */
	word_uncompress(buf1, buf2, bsize);
	intel_swaps(buf2,plane_size*2);	/* swap back so image right */
	columns_to_bitplanes(buf2,buf1,sbpr>>1,neo.height);
	planes = (UBYTE *)buf1;
	}
else
	{
	de_interleave(buf1,buf2,sbpr>>1,neo.height);
	planes = (UBYTE *)buf2;
	}
				/** Now we blit the cel onto the ST screen, doing
				 ** either a move-blit or an xor-blit.
				 **/
if (neo.op == NEO_XOR)
	{
	for (i=0; i<4; ++i)
		{
		xor_blit_box(neo.width, neo.height, 0, 0, planes, sbpr, 
			neo.xoff, neo.yoff, stscreen+8000*i, 40);
		planes += plane_size;
		}
	}
else
	{
	clear_mem(stscreen, 32000);
	for (i=0; i<4; ++i)
		{
		blit_box(neo.width, neo.height, 0, 0, planes, sbpr, 
			neo.xoff, neo.yoff, stscreen+8000*i, 40);
		planes += plane_size;
		}
	}
				/** Now convert stscreen to byte-a-pixel
				 **/
CONV_SCREEN:
for (i=0; i<4; ++i)
	{
	stplanes[i] = stscreen;
	stscreen += 8000;
	}
err = conv_bitmaps(stplanes, 4, 40, 320, 200, screen);

ERROR:
	if (buf1 != NULL)
		free(buf1);
	if (buf2 != NULL)
		free(buf2);
	return(err);
}



static Errcode read_first(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * Seek to the beginning of an open  file, and then read in the
 * first frame of image into screen.  
 ****************************************************************************/
{
Ifile *gf = (Ifile *)ifile;		/* There's a bit of data past ifile header */
Errcode err;

				/* skip past header & offset list */
if ((err = fseek(gf->file, 
				 gf->seq.cel_count * sizeof(long) + sizeof(Seq_header), 
				 SEEK_SET)) < Success)
	return(err);
return(read_next(ifile, screen));
}

Hostlib _a_a_stdiolib = { NULL, AA_STDIOLIB, AA_STDIOLIB_VERSION };
Hostlib _a_a_gfxlib = { &_a_a_stdiolib, AA_GFXLIB, AA_GFXLIB_VERSION };
Hostlib _a_a_syslib = { &_a_a_gfxlib, AA_SYSLIB, AA_SYSLIB_VERSION };

static char title_info[] = "Atari ST Cyber SEQ format.";

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


