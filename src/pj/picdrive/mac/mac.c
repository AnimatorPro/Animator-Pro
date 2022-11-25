
/* mac.c - source code to the MacPaint format PJ .PDR picture driver */

#define REXLIB_INTERNALS
#include "errcodes.h"
#include "stdio.h"
#include "picdrive.h"
#include "syslib.h"

typedef struct ifile {
/** This structure starts with and Image_file, and has further data
 ** local to the PDR.  
 **/
	Image_file hdr;
	FILE *file;
	long hsize;		/* header size */
} Ifile;

#define copy_bytes(source,dest,count) memcpy(dest,source,count)
#define clear_mem(dest,count) memset(dest,0,count)
#define clear_struct(s) clear_mem(s, sizeof(s))

Errcode unpic_line(FILE *f, UBYTE *buf, int len);

	/* fixed width/height of a MacPaint pic */
#define MW 576
#define MH 720	
	/* bytes-per-row of MacPaint pic */
#define MBPR ((MW+7)/8)
	/* size of header */
#define MHDR_SIZE 512
	/* size of buffer, extra 128 bytes so don't crash on bad data */
#define MBUF_SIZE (MBPR+128)
	/* where ID bits might be in header */
#define ID_OFF 65
	/* This occurs sometimes, means header is 128 bytes bigger */
char new_id[8] = "PNTGMPNT";


static Boolean spec_best_fit(Anim_info *ainfo)
/*****************************************************************************
 * Tell host that we can only write 8 bit-a-pixel images,  and only one
 * frame, fixed width/height.
 ****************************************************************************/
{
Boolean nofit;

nofit = (ainfo->depth == 8
		 && ainfo->num_frames == 1
		 && ainfo->width == MW
		 && ainfo->height == MH);
ainfo->depth = 8;
ainfo->num_frames = 1;
ainfo->width = MW;
ainfo->height = MH;
ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;
return(nofit);	/* return whether fit was exact */
}

static Errcode open_file(Pdr *pd, char *path, Image_file **pif, 
						 Anim_info *ainfo )
/*****************************************************************************
 * Open up the file, and if possible verify header. 
 * In the case of a MacPaint pic, we just make sure file is at least
 * 512 bytes, and that the first couple of lines unpack ok.
 ****************************************************************************/
{
Ifile *gf;
Errcode err;
UBYTE buf[MBUF_SIZE];
int i;
FILE *f;

*pif = NULL;	/* for better error recovery */
if((gf = zalloc(sizeof(*gf))) == NULL)
	return(Err_no_memory);

if((f = gf->file = fopen(path, "rb")) == NULL)
	{
	err = pj_errno_errcode();
	goto ERROR;
	}
if (ainfo != NULL)		/* fill in ainfo structure if any */
	{
	clear_struct(ainfo);
	spec_best_fit(ainfo);
	}
						/* Look for id bits that might indicate size of
						 * header.  (I wish I had a _real_ spec for this
						 * format!)
						 */
fseek(f, (long)ID_OFF, SEEK_SET);
fread(buf, 1, 8, f);
if (memcmp(buf,new_id, 8) == 0)
	{
	gf->hsize = MHDR_SIZE+128;
	}
else
	{
	gf->hsize = MHDR_SIZE;
	}

							/* Seek past header */
if ((err = fseek(f, gf->hsize-1, SEEK_SET)) < Success)
	goto ERROR;
if (getc(f) < 0)			/* make sure actually have data here */
	{
	err = Err_truncated;
	goto ERROR;
	}
i = 100;
while (--i >= 0)			/* Check 1st 100 lines of file by unpacking. */
	{
	if ((err = unpic_line(f, buf, MBPR)) < Success)
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

	/* cmap for white background, black forground */
static UBYTE maccmap[] = {0xff,0xff,0xff,0,0,0};

static Errcode read_first(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * Seek to the beginning of an open  file, and then read in the
 * first frame of image into screen.  (In our case read in the only
 * frame of image.)
 ****************************************************************************/
{
Ifile *gf = (Ifile *)ifile;		/* There's a bit of data past ifile header */
FILE *f = gf->file;
int i;
UBYTE buf[MBUF_SIZE];
Errcode err;

if ((err = fseek(f, gf->hsize, SEEK_SET)) < Success)
	goto ERROR;
copy_bytes(maccmap,screen->cmap->ctab,sizeof(maccmap));
pj_cmap_load(screen,screen->cmap);
pj_clear_rast(screen);
for (i=0; i<MH; ++i)
	{
	if ((err = unpic_line(f, buf, MBPR)) < Success)
		goto ERROR;
	pj_mask1blit(buf, MBPR, 0, 0, screen, 0, i, MW, 1, 1);
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

static char title_info[] = "MacPaint black and white format.";

Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, &_a_a_syslib },
	title_info, 		/* title_info */
	"",  				/* long_info */
	".MAC",		 		/* default_suffi */
	0,1,		 		/* max_write_frames, max_read_frames */
	spec_best_fit,		/* (*spec_best_fit)() */
	NOFUNC,				/* (*create_image_file)() */
	open_file,			/* (*open_image_file)() */
	close_file,			/* (*close_image_file)() */
	read_first,			/* (*read_first_frame)() */
	read_next,			/* (*read_delta_next)() */
	NOFUNC,				/* (*save_frames)() */
};


