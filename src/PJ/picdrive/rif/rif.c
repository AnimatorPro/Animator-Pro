
/* rif.c - source code to the Amiga Live! & Zoetrope .RIF format
 * driver */

#include <string.h>
#include "animinfo.h"
#include "errcodes.h"
#include "memory.h"
#include "picdrive.h"
#include "rif.h"
#include "xfile.h"

#define ISUFFI ".RIF"

typedef struct ifile {
/** This structure starts with and Image_file, and has further data
 ** local to the PDR.
 **/
	Image_file hdr;
	XFILE *file;
	struct riff_head rif;
	UBYTE *sbuf;
	UBYTE *planes[8];
	int bpr;
	long psize;
	int *ytable;
} Ifile;

static void close_file(Image_file **pif);

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

static void intel_swap(void *pt)
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

if ((*pytable = pt = pj_malloc(height * sizeof(*pt))) == NULL)
	return(Err_no_memory);
acc = 0;
while (--height >= 0)
	{
	*pt++ = acc;
	acc += bpr;
	}
return(Success);
}

static Errcode open_file(Pdr *pd, char *path, Image_file **pif,
						 Anim_info *ainfo )
/*****************************************************************************
 * Open up the file, and if possible verify header.
 ****************************************************************************/
{
Ifile *gf;
XFILE *f;
Errcode err = Success;
UBYTE *sbuf;
Riff_head rif;
int i;
(void)pd;

*pif = NULL;	/* for better error recovery */

if((gf = pj_zalloc(sizeof(*gf))) == NULL)
	return(Err_no_memory);

if ((f = gf->file = xfopen(path, "rb")) == NULL)
	{
	err = xerrno();
	goto ERROR;
	}
if (xfread(&rif, sizeof(rif), 1, f) != 1)
	{
	err = Err_truncated;
	goto ERROR;
	}
intel_swap(&rif.xoff);
intel_swap(&rif.yoff);
intel_swap(&rif.width);
intel_swap(&rif.height);
intel_swap(&rif.depth);
intel_swap(&rif.frame_count);
intel_swap(&rif.jiffies_frame);
intel_swap(&rif.frames_written);
if (rif.frames_written == 0)
	rif.frames_written = rif.frame_count;
gf->rif = rif;
if (memcmp(&rif.iff_type, "RIFF", 4) != 0)
	{
	err = Err_bad_magic;
	goto ERROR;
	}
if (rif.depth > 8 || rif.depth < 1)
	{
	err = Err_pdepth_not_avail;
	goto ERROR;
	}
gf->bpr = ((rif.width+7)>>3);
gf->psize = gf->bpr*rif.height;
						/** Allocate bitplanes for Amiga screen
						 ** simulation.
						 **/
if ((gf->sbuf = sbuf = pj_malloc(gf->psize*rif.depth)) == NULL)
	{
	err = Err_no_memory;
	goto ERROR;
	}
for (i=0; i<rif.depth; ++i)
	{
	gf->planes[i] = sbuf;
	sbuf += gf->psize;
	}
if ((err = make_ytable(&gf->ytable, gf->bpr, rif.height)) < Success)
	goto ERROR;
if (ainfo != NULL)		/* fill in ainfo structure if any */
	{
	clear_struct(ainfo);
	ainfo->depth = 8;
	ainfo->x = rif.xoff;
	ainfo->y = rif.yoff;
	ainfo->width = rif.width;
	ainfo->height = rif.height;
	ainfo->num_frames = rif.frames_written;
	ainfo->millisec_per_frame = 1000*rif.jiffies_frame/60;
	}
*pif = (Image_file *)gf;
return(Success);

ERROR:
close_file(pif);
return(err);
}

static void close_file(Image_file **pif)
/*****************************************************************************
 * Clean up resources used by picture driver in loading (saving) a file.
 ****************************************************************************/
{
Ifile **pgf = (Ifile **)pif;
Ifile *gf;

if(pgf == NULL || (gf = *pgf) == NULL)
	return;
if(gf->file)
	xfclose(gf->file);
if (gf->sbuf != NULL)
	pj_free(gf->sbuf);
if (gf->ytable != NULL)
	pj_free(gf->ytable);
pj_free(gf);
*pgf = NULL;
}

static void conv_amiga_cmap(USHORT *ami_cmap, UBYTE *pj_ctab, int size)
/*****************************************************************************
 * Convert color map from Amiga format (a nibble each of red green and blue
 * with the hi nibble clear,  16 bits total per color) to our one byte for
 * each RGB value format.
 ****************************************************************************/
{
USHORT cm;

while (--size >= 0)
	{
	cm = *ami_cmap++;
	*pj_ctab++ = ((cm&0xF00)>>4);
	*pj_ctab++ = ((cm&0x0F0));
	*pj_ctab++ = ((cm&0x00F)<<4);
	}
}

static Errcode unrif_comp(Ifile *gf, UBYTE *plane, struct comp_size *comp)
/*****************************************************************************
 * Uncompress one bit plane of RIFF.
 ****************************************************************************/
{
UBYTE *comp_buf = NULL;
XFILE *f = gf->file;
unsigned size;
short type;
Errcode err = Success;

if ((size = comp->size) != 0)
	{
	if ((comp_buf = pj_malloc(size)) == NULL)
		{
		err = Err_no_memory;
		goto ERROR;
		}
							/** If it's uncompressed read it
							 ** straight into bitplane.
							 **/
	if ((type = comp->comp) == VCOMP_NONE)
		{
		if (xfread(plane, size, 1, f) != 1)
			{
			err = Err_truncated;
			goto ERROR;
			}
		}
	else
		{
							/** Else read in the compressed data
							 ** and unpack it.
							 **/
		if (xfread(comp_buf, size, 1, f) != 1)
			{
			err = Err_truncated;
			goto ERROR;
			}
		switch (type)
			{
			case VCOMP_VRUN:
				decode_vplane(comp_buf, plane, gf->bpr);
				break;
			case VCOMP_SKIP:
				decode_vkplane(comp_buf, plane, gf->bpr, gf->ytable);
				break;
			}
		}
	}
ERROR:
if (comp_buf != NULL)
	pj_free(comp_buf);
return(err);
}

static Errcode read_next(Image_file *ifile,Rcel *screen)
/*****************************************************************************
 * Read in subsequent frames of image.
 ****************************************************************************/
{
Ifile *gf = (Ifile *)ifile; 	/* There's a bit of data past ifile header */
XFILE *f = gf->file;
struct vcomp_iff vc_h;
Errcode err = Success;
int i;


						/** Read in frame header, do any
						 ** necessary Intel/Motorola swapping,
						 ** and verify the header id bits.
						 **/
if (xfread(&vc_h, sizeof(vc_h), 1, f) != 1)
	{
	err = Err_truncated;
	goto ERROR;
	}
intel_swaps(vc_h.comps, 16);
intel_swaps(vc_h.cmap, 32);
if (memcmp(vc_h.iff_type, "VRUN", 4) != 0)
	{
	err = Err_bad_record;
	goto ERROR;
	}
						/** Read in and uncompress the bitplanes
						 ** one by one.
						 **/
for (i=0; i<gf->rif.depth; ++i)
	{
	if ((err = unrif_comp(gf, gf->planes[i], &vc_h.comps[i])) < Success)
		goto ERROR;
	}

conv_amiga_cmap(vc_h.cmap,(UBYTE *)(screen->cmap->ctab),32);
pj_cmap_load(screen,screen->cmap);
err = conv_bitmaps(gf->planes, gf->rif.depth,
				   gf->bpr, gf->rif.width, gf->rif.height, screen);

ERROR:
	return(err);
}



static Errcode read_first(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * Seek to the beginning of an open  file, and then read in the
 * first frame of image into screen.
 ****************************************************************************/
{
Ifile *gf = (Ifile *)ifile; 	/* There's a bit of data past ifile header */
Errcode err;

				/* skip past header & offset list */
if ((err = xfseek(gf->file,
				 gf->rif.frame_count * sizeof(long) + sizeof(Riff_head),
				 XSEEK_SET)) < Success)
	return(err);
return(read_next(ifile, screen));
}


/*****************************************************************************
 * Rex/Pdr interface stuff...
 ****************************************************************************/

static char rif_pdr_name[] = "RIF.PDR";
static char title_info[] = "Amiga Zoetrope & Live! format.";

static Pdr rif_pdr_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, NULL, NULL, NULL },
	title_info, 		/* title_info */
	"",                 /* long_info */
	ISUFFI, 			/* default_suffi */
	0,4000, 			/* max_write_frames, max_read_frames */
	spec_best_fit,		/* (*spec_best_fit)() */
	NOFUNC, 			/* (*create_image_file)() */
	open_file,			/* (*open_image_file)() */
	close_file, 		/* (*close_image_file)() */
	read_first, 		/* (*read_first_frame)() */
	read_next,			/* (*read_delta_next)() */
	NOFUNC, 			/* (*save_frames)() */
};

Local_pdr rif_local_pdr = {
	NULL,
	rif_pdr_name,
	&rif_pdr_header
};
