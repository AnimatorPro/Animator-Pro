
/* gif.c - High level gif routines.  Take care of the file packaging but
   not the compression/decompression.  */

#include <string.h>
#define REXLIB_INTERNALS
#include "errcodes.h"
#include "stdio.h"
#include "ffile.h"
#include "gif.h"
#include "memory.h"
#include "picdrive.h"

#ifndef isdigit
  #define isdigit(a) ( a >= '0' && a <= '9')
  #define islower(a) ( a >= 'a' && a <= 'z')
#endif

static struct gif_header gif;
static struct gif_image gim;

static char gifsig[] = "GIF87a";    /* signature for output files */

static int	gif_files_open = 0;
static int	gif_line;
FILE		*gif_save_file;
FILE		*gif_load_file;
static char iphase;
static int	iy;


int gif_out_line(UBYTE *pixels, int linelen, Raster *screen)
{
	int y;

	y = gif_line;
	if (gim.flags&ITLV_BIT)
	{
		y = iy;
		switch (iphase)
		{
			case 0:
			case 1:
				iy+=8;
				break;
			case 2:
				iy += 4;
				break;
			case 3:
				iy += 2;
				break;
		}
		if (iy >= gim.h)
		{
			switch (iphase)
			{
				case 0:
					iy = 4;
					break;
				case 1:
					iy = 2;
					break;
				case 2:
					iy = 1;
					break;
			}
			iphase++;
		}
	}
	gif_line++;
	pj_put_hseg(screen, pixels, 0, y, linelen);
	return(Success);
}


static Errcode read_gif_start(Gif_file *gf,
							  struct gif_header *ghdr,
							  struct gif_image *gimg,
							  Cmap *out_cmap, Anim_info *ainfo )

/* attempt non destructive verify and reading of info from a gif file */
{
Errcode err;
ULONG colors;
LONG oset;
int c;
FILE *f;

	f = gf->file;

	if(fread(ghdr, 1, sizeof(*ghdr), f) < sizeof(*ghdr))
		goto io_error;

	/*
	 * check the signature block at the start of the file.	we determine
	 * validity by pattern matching: the first 3 chars must be GIF, followed
	 * by 2 digits and a lowercase alpha character.  this should keep us
	 * compatible well into the future, since the GIF format is rigid enough
	 * that we can skip data we don't care about when new extension blocks
	 * are added in future versions.
	 */

	if ( (0 != memcmp(ghdr->giftype, gifsig, 3)) 
	|| (!(isdigit(ghdr->giftype[3]) 
	&& isdigit(ghdr->giftype[4]) && islower(ghdr->giftype[5])))
	   )
	{
		err = Err_bad_magic;
		goto error;
	}

	colors = (1<<((ghdr->colpix&PIXMASK)+1));

	if (ghdr->colpix&COLTAB)
	{
		if(out_cmap != NULL)
		{
			if (fread(out_cmap->ctab, 1, colors*3, f) < colors*3)
				goto io_error;
		}
		else if((oset = fseek(f,colors*3L,SEEK_CUR)) < 0)
		{
			err = oset;
			goto error;
		}
	}

	for (;;)	/* skip over extension blocks and other junk til get ',' */
	{
		if ((c = fgetc(f)) < 0)
			goto trunc_error;
		if (c == ',')
			break;
		if (c == ';')   /* semi-colon is end of piccie */
		{
			goto trunc_error;
		}
		if (c == '!')   /* extension block */
		{
			if ((c = fgetc(f)) < 0) /* skip extension type */
				goto trunc_error;
			for (;;)
			{
				if ((c = fgetc(f)) < 0)
					goto trunc_error;
				if (c == 0) /* zero 'count' means end of extension */
					break;
				if((oset = fseek(f,c,SEEK_CUR)) < 0)
				   {
					   err = oset;
					   goto error;
				   }
			}
		}
	}
	if (fread(gimg, 1, sizeof(*gimg), f) < sizeof(*gimg) )
		goto io_error;

	if((ainfo || out_cmap) && gimg->flags&COLTAB)
	{
		colors = (1<<((gimg->flags&PIXMASK)+1));
		if(out_cmap)
		{
			if (fread(out_cmap->ctab, 1, colors*3, f) < colors*3)
				goto io_error;
		}
	}

	if(ainfo)
	{
		memset(ainfo,0,sizeof(*ainfo));
		ainfo->width = gimg->w;
		ainfo->height = gimg->h;
		ainfo->num_frames = 1;
		ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;

		while(colors>>=1)
		{
			++ainfo->depth;
		}
	}

	return(Success);
io_error:
	if((err = pj_errno_errcode()) < Success)
		goto error;
trunc_error:
	err = Err_truncated;
error:
	return(err);
}

static Boolean gif_spec_best_fit(Anim_info *ainfo)
{
Boolean nofit;

	nofit = (ainfo->depth == 8
			 && ainfo->num_frames == 1);

	ainfo->depth = 8;
	ainfo->num_frames = 1;
	return(nofit);
}
static void close_gif_file(Image_file **pif)
{
Gif_file **gifile = (Gif_file **)pif;
Gif_file *gf;

	if(gifile == NULL || (gf = *gifile) == NULL)
		return;
	if(gf->file)
		fclose(gf->file);
	pj_free(gf);
	*gifile = NULL;
	gif_files_open = FALSE;
}
static Errcode gif_open_ifsub(Gif_file **gifile, char *path, char *rwmode)
{
Errcode err = Success;
Gif_file *gf;

	*gifile = NULL;

	if(gif_files_open)
		return(Err_too_many_files);

	if((gf = pj_zalloc(sizeof(Gif_file))) == NULL)
		return(Err_no_memory);

	/* gf->hdr.needs_work_cel = FALSE */

	if((gf->file = fopen(path, rwmode)) == NULL)
		err = pj_errno_errcode();

	gif_files_open = TRUE;
	*gifile = gf;
	return(err);
}
static Errcode open_gif_file(Pdr *pd, char *path, Image_file **pif,
							 Anim_info *ainfo )
{
Errcode err;
Gif_file **pgif;
struct gif_header ghdr;
struct gif_image gimg;
(void)pd;

	pgif = (Gif_file **)pif;

	if((err = gif_open_ifsub(pgif, path, "rb")) < Success)
		goto error;

	if((err = read_gif_start(*pgif,&ghdr,&gimg,
							  NULL,&((*pgif)->ainfo))) < Success)
	{
		goto error;
	}

	if(ainfo)
		*ainfo = (*pgif)->ainfo;
	return(Success);
error:
	close_gif_file(pif);
	return(err);
}
static Errcode create_gif_file(Pdr *pd, char *path, Image_file **pif,
							   Anim_info *ainfo )
{
Errcode err;
Gif_file **pgif;
(void)pd;

	pgif = (Gif_file **)pif;

	if((err = gif_open_ifsub(pgif, path, "wb")) < Success)
		goto error;

	(*pgif)->ainfo = *ainfo;
	return(Success);
error:
	close_gif_file(pif);
	return(err);
}
static Errcode gif_read_picframe(Image_file *ifile, Rcel *screen)
{
Errcode err;
Gif_file *gf;

	gif_line = 0;
	iphase = 0;
	iy = 0;
	gf = (Gif_file *)ifile;

	gif_load_file = gf->file;
	rewind(gif_load_file);

	if((err = read_gif_start(gf, &gif, &gim,screen->cmap,NULL)) < 0)
		return(err);

	pj_set_rast(screen, 0); 		   /* clear the screen */
	pj_cmap_load(screen,screen->cmap); /* update hardware cmap if needed */

	err = gif_decoder(gim.w,screen);
	return(err);
}
static Errcode gif_read_next(Image_file *ifile,Rcel *screen)
{
	(void)ifile;
	(void)screen;
	return(Success);
}



static Rcel *gifscreen;
static int pixy;
static Pixel *pixbuf, *pixb;
static int pixw;
static int pixleft;

int gif_get_pixel(void)
{
	if (--pixleft < 0)
	{
		pj_get_hseg(gifscreen, pixb = pixbuf, 0, ++pixy, pixw);
		pixleft = pixw-1;
	}
	return(*pixb++);
}

static Errcode gif_save_frame(Image_file *ifile, Rcel *screen, int num_frames,
							  Errcode (*seek_frame)(int ix,void *seek_data),
							  void *seek_data, Rcel *work_screen )
{
Gif_file *gf;
Errcode err;
UBYTE *cbyte;
long gif_wcount;
(void)num_frames;
(void)seek_frame;
(void)seek_data;
(void)work_screen;

	gf = (Gif_file *)ifile;

	if( (gf->ainfo.width) != screen->width
		|| gf->ainfo.height != screen->height)
	{
		return(Err_bad_input);
	}

	/* pixel stuff to use get dot */
	gifscreen = screen;
	pixy = 0;
	pixleft = pixw	= screen->width;
	if ((pixb = pixbuf = pj_malloc(pixw)) == NULL)
		return(Err_no_memory);
	pj_get_hseg(screen, pixbuf, 0, 0, pixw);

	gif_wcount = (long)screen->width*(long)screen->height;
	memset(&gif,0,sizeof(gif));
	gif_save_file = gf->file;

	strcpy(gif.giftype, gifsig);
	gif.w = gim.w = screen->width;
	gif.h = gim.h = screen->height;
	gim.x = gim.y = gim.flags = 0;
	gif.colpix = COLPIXVGA13;
	if (fwrite(&gif, 1, sizeof(gif), gif_save_file) < sizeof(gif))
		goto io_error;

	/* write global color map */
	cbyte = (UBYTE *)(screen->cmap->ctab);
	if (fwrite(cbyte, 1, COLORS*3, gif_save_file) < COLORS*3)
		goto io_error;
	if((err = fputc(',', gif_save_file)) < 0)   /* comma to start image */
		goto error;

	if (fwrite(&gim, 1, sizeof(gim), gif_save_file) < sizeof(gim))
		goto io_error;

	if((err = fputc(8, gif_save_file)) < 0)
		goto error;

	fflush(gif_save_file);
	if((err = gif_compress_data(8, gif_wcount)) < Success)
		goto error;
	fputc(';', gif_save_file);
	if ((err = fflush(gif_save_file)) < Success)
		goto error;
	goto done;

io_error:
	err = pj_errno_errcode();
error:
done:
	if (pixbuf != NULL)
		{
		pj_free(pixbuf);
		pixbuf = NULL;
		}
	return(err);
}
/**** driver header declaration ******/

static char s_gif_pdr_name[] = "GIF.PDR";
static char gif_title_info[] = "GIF standard picture format.";

static Pdr gif_pdr_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, NULL, NULL, NULL },
	gif_title_info, 		/* title_info */
	"",                     /* long_info */
	".GIF",                 /* default_suffi */
	1,1,					/* max_write_frames, max_read_frames */
	gif_spec_best_fit,		/* (*spec_best_fit)() */
	create_gif_file,		/* (*create_image_file)() */
	open_gif_file,			/* (*open_image_file)() */
	close_gif_file, 		/* (*close_image_file)() */
	gif_read_picframe,		/* (*read_first_frame)() */
	gif_read_next,			/* (*read_delta_next)() */
	gif_save_frame, 		/* (*save_frames)() */
};

Local_pdr gif_local_pdr = {
	NULL,
	s_gif_pdr_name,
	&gif_pdr_header,
};
