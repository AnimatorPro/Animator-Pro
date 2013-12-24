/***************************************************************
Pict file pdr modules:

	Created by Peter Kennard.  Sept 29, 1991
		Implements non-pattern fill mode bitmap operations
		and transfer mode blit.  Parses and ignores all other
		opcodes.
****************************************************************/
#include "errcodes.h"
#include "pict.h"

extern Errcode pict_save_frame(Image_file *ifile, Rcel *screen, int num_frames,
							  Errcode (*seek_frame)(int ix,void *seek_data),
							  void *seek_data, Rcel *work_screen );

extern Errcode pict_rgb_readline(Image_file *ifile, Rgb3 *linebuf);

static void close_pict_file(Image_file **pif)
/* Close file if open and deallocate all buffers */
{
Pfile *pf;

	if((pf = *((Pfile **)pif)) == NULL)
		return;
	if(pf->file)
		fclose(pf->file);
	cleanup_buffers(pf);
	free(pf);
	*pif = NULL;
}
Errcode set_version(Pfile *pf, int version)
/* Setup a pfile for either version 1 or version 2 data streams.  Specificly
 * sets the opcode read function. */
{
extern Errcode read_v1_opcode(Pfile *pf);
extern Errcode read_v2_opcode(Pfile *pf);

	switch(version)
	{
		case 1:
			pf->read_opcode = read_v1_opcode;
			break;
		case 2:
			pf->read_opcode = read_v2_opcode;
			break;
		default:
			pf->lasterr = Err_version;
			break;
	}
	pf->version = version;
	return(pf->lasterr);
}
static Errcode pfile_open_sub(Pfile **ppf, char *path, char *fmode)
/* Allocate a pfile and open the file handle for the input fmode. */
{
Errcode err;

	if((*ppf = zalloc(sizeof(Pfile))) == NULL)
		return(Err_no_memory);

	if(((*ppf)->file = fopen(path, fmode)) == NULL)
		goto ioerror;

	return(Success);

ioerror:
	err = pj_errno_errcode();
	close_pict_file((Image_file **)ppf);
	return(err);
}
static Errcode open_pict_file(Pdr *pd, char *path, Image_file **pif,
							 Anim_info *ainfo )
/* Opens an existing pict file and querys for header info. */
{
Errcode err;
Pfile *pf;
Picthead ph;
int inbyte;
int version;

	if((err = pfile_open_sub((Pfile **)pif, path, "rb")) < Success)
		return(err);

	pf = *((Pfile **)pif);

	if(pf_read_oset(pf,&ph,sizeof(ph),512) < Success)
		goto pferror;

	/* Some picts have an indeterminate number of zeros after the 
	 * initial rectangle !!! */

	while((inbyte = fgetc(pf->file)) == 0); /* scan past zeros */

	if(inbyte == EOF)
		goto ioerror;
	if(inbyte != 0x11) 
		goto bad_magic;

	version = fgetc(pf->file);

	switch(version)
	{
		case EOF:
			goto ioerror;
		case 0:
		default:
			goto bad_magic;
		case 1:
			break;
		case 2:  /* Version 2 has an 0xFF after version byte */
			if((inbyte = fgetc(pf->file)) == EOF)
				goto ioerror;
			if(inbyte != 0x00FF)
				goto bad_magic;
			break;
	}

	if(set_version(pf,version) < Success)
		goto pferror;

	intel_swap_words(&ph.frame,sizeof(ph.frame)/2);
	pf->frame = ph.frame;

	/* We will load a default set of values and the file scanner may 
	 * alter them. */

	pf->ainfo.width = ph.frame.right - ph.frame.left;
	pf->ainfo.height = ph.frame.bot - ph.frame.top;
	pf->ainfo.num_frames = 1;
	pf->ainfo.aspect_dx = pf->ainfo.aspect_dy = 1;
	pf->ainfo.millisec_per_frame = DEFAULT_AINFO_SPEED;
	pf->ainfo.depth = 8;

#ifdef PRINTSTUFF

	printf("version %d\n", pf->version );

	printf("top %d left %d right %d bot %d\n", 
			ph.frame.top,
			ph.frame.left,
			ph.frame.right,
			ph.frame.bot );
#endif

	/* Save offset to first image related opcode */
	if((pf->opstart = ftell(pf->file)) < 0)
		goto ioerror;

	/* Scan the file for any explicit definition of pixel depth etc. */
	if((err = pict_scan_info(pf)) < Success)
		goto error;

	/* if info requested put in output */

	if(ainfo)
		*ainfo = pf->ainfo;

	cleanup_buffers(pf);

	return(Success);

bad_magic:
	err = Err_bad_magic;
	goto error;
ioerror:
	err = pj_errno_errcode();
	goto error;
pferror:
	err = pf->lasterr;
error:
	close_pict_file(pif);
	return(err);
}
static Errcode pict_scan_info(Pfile *pf)
/* After the pfile is determined to be a pict file, scan opcodes until 
 * the first pixel map is hit and return info about it in pf->ainfo. */
{
	pf->mode = PF_SCANINFO;
	pf->lasterr = Success;

	do_pictops(pf);

	if(pf->lasterr == RET_ENDSCAN) /* We hit a bitmap chunk. */
		return(Success);

	/* We should hit this error again when trying to read it and should
	 * indicate we found the right type of file even if the data is bad. 
	 * This so pj will indicate bad data instead of an unrecognized file
	 * type. */

	return(RET_EOPIC); 
}

static Rgb3 default_colors[] =
{
	0xFF,0xFF,0xFF,  /* white background */
	0x00,0x00,0x00,    /* black */
	0xFF,0x00,0x00,   /* red */
	0x00,0xFF,0x00,   /* green */
	0x00,0x00,0xFF,   /* blue */
	0x00,0xFF,0xFF,   /* cyan */
	0xFF,0xFF,0x00,   /* yellow */
	0xFF,0x00,0xFF,   /* magenta */
};

Errcode pict_read_first_frame(Image_file *ifile, Rcel *screen)
/* Now that we know the pict file has a depth <= 8 bits we draw it in the 
 * screen. */
{
Pfile *pf = (Pfile *)ifile;

	if(pf->lasterr < Success) /* Open and scan must have failed. */
		return(pf->lasterr);

	rewind(pf->file); /* Clear errors and re-init file. */

	if(fseek(pf->file,pf->opstart,SEEK_SET) < 0)
		return(pj_errno_errcode());

	pf->lasterr = Success;
	pf->mode = PF_FULLFRAME;
	pf->screen = screen;

	/* Clear color map to zero and load in default colors. */
	stuff_bytes(0,screen->cmap->ctab,
				screen->cmap->num_colors * sizeof(Rgb3));
	copy_bytes(default_colors,screen->cmap->ctab,sizeof(default_colors));
	if(pf->ainfo.depth == 1)
		pj_cmap_load(pf->screen,pf->screen->cmap);

	if(do_pictops(pf) >= Success)
		{
		if (pf->got_bitmap)
			pj_cmap_load(pf->screen,pf->screen->cmap);
		else
			pf->lasterr = Err_no_bitmap;
		}

	cleanup_buffers(pf);
	return(pf->lasterr);
}

static Errcode create_pict_file(Pdr *pd, char *path, Image_file **pif,
								 Anim_info *ainfo )
/* Create a new pict file and prepare it for writing data. */
{
Errcode err;

	if((err = pfile_open_sub((Pfile **)pif,path,"wb")) >= Success)
		(*(Pfile **)pif)->ainfo = *ainfo;

	return(err);
}
static Boolean pict_spec_best_fit(Anim_info *ainfo)
/* We can only write one frame 156 color pict files.  But, they can be any
 * size */
{
Boolean nofit;

	nofit = (ainfo->depth == 8
			 && ainfo->num_frames == 1);

	ainfo->depth = 8;
	ainfo->num_frames = 1;
	return(nofit);
}
static Errcode pict_rgb_seekstart(Image_file *ifile)
/* Seek to first line of RGB data in a > 8 bit depth pict file. */
{
Pfile *pf = (Pfile *)ifile;

	if(pf->first_pmap_oset <= 0)
		return(Err_uninit);

	rewind(pf->file); /* clear errors and re-init */

	/* seek to previously found pixMap header */
	if(fseek(pf->file,pf->first_pmap_oset,SEEK_SET) < 0)
		return(pj_errno_errcode());

	/* initialize to line -1 and read the pixMap header and set up
	 * things to read the lines in the PF_RGBLINES mode */

	pf->mode = PF_RGBLINES; 
	pf->last_rgbline = -1;  /* before start of pic */
	if(do_pixmap(pf) < Success)
		return(pf->lasterr);
	return(0);  /* we are in a top down file */
}

extern Errcode pict_rgb_readline(Image_file *ifile, Rgb3 *linebuf);

/**** driver header declaration ******/

#define HLIB_TYPE_1 AA_STDIOLIB
#define HLIB_TYPE_2 AA_GFXLIB
#define HLIB_TYPE_3 AA_SYSLIB
#include "hliblist.h"

static char pict_title_info[] = "Macintosh PICT and PICT2 formats.";

static char long_info[] = 
	"This module reads bitmap type records from macintosh PICT and PICT2 "
	"files.  Graphics drawing records in these files are ignored.  "
	"This module only writes files in the 8 bits per pixel, "
	"256 color PICT2 format.";


Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, HLIB_LIST },
	pict_title_info, 		/* title_info */
	long_info,                     /* long_info */
	".PCT",                 /* default_suffi */
	1,1,					/* max_write_frames, max_read_frames */
	pict_spec_best_fit,		/* (*spec_best_fit)() */
	create_pict_file,		/* (*create_image_file)() */
	open_pict_file,			/* (*open_image_file)() */
	close_pict_file, 		/* (*close_image_file)() */
	pict_read_first_frame,	/* (*read_first_frame)() */
	NOFUNC,					/* (*read_delta_next)() */
	pict_save_frame, 		/* (*save_frames)() */
	NULL,					/* pdr options */
	pict_rgb_seekstart,		/* (*rgb_seekstart)() */
	pict_rgb_readline,      /* (*rgb_readline)() */
};

