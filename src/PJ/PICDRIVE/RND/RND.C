/***************************************************************
RND file pdr modules:

	Created by Peter Kennard.  Oct 11, 1991
		These modules implement reading scan line and polygon data in
		256 color Autoshade render slide files and drawing the image into
		a screen.
****************************************************************/
#include "errcodes.h"
#include "rnd.h"


static char headstr[] = IDCHARS;



static void cleanup_buffers(Rfile *rf)
/* Free all dynamic work buffers. */
{
	freez(&rf->pktbuf);
	release_polypoints(&rf->pg,&rf->pts);
	free_pointlist(&rf->pts);
}

static void close_rnd_file(Image_file **pif)
/* Close file handle if open and deallocate all buffers */
{
Rfile *rf;

	if((rf = *((Rfile **)pif)) == NULL)
		return;
	if(rf->file)
		fclose(rf->file);
	cleanup_buffers(rf);
	free(rf);
	*pif = NULL;
}
static Errcode rfile_open_sub(Rfile **prf, char *path, char *fmode)
/* Allocate an rfile and open the file handle in fmode */
{
Errcode err;

	if((*prf = zalloc(sizeof(Rfile))) == NULL)
		return(Err_no_memory);

	if(((*prf)->file = fopen(path, fmode)) == NULL)
		goto ioerror;

	return(Success);

ioerror:
	err = pj_errno_errcode();
	close_rnd_file((Image_file **)prf);
	return(err);
}
static Errcode open_rnd_file(Pdr *pd, char *path, Image_file **pif,
							 Anim_info *ainfo )
/* Open existing rnd file and query for header info. */
{
Errcode err;
Rfile *rf;
Rndhdr rh;

	if((err = rfile_open_sub((Rfile **)pif, path, "rb")) < Success)
		return(err);

	rf = *((Rfile **)pif);

	if(rf_read(rf,&rh,sizeof(rh)) < Success)
		goto rferror;

	if(strncmp(headstr,rh.prs,IDCKSIZE) != 0)
		goto bad_magic;

	rf->hdr = rh.hdr;

	rf->ainfo.width = rf->hdr.xdots;
	rf->ainfo.height = rf->hdr.ydots;
	rf->ainfo.num_frames = 1;
	rf->ainfo.millisec_per_frame = DEFAULT_AINFO_SPEED;

	if(rh.hdr.ncolor <= 0) /* bad file header */
	{
		err = Err_format;
		goto error;
	}
	else 
	{
	ULONG ncolors;

		ncolors = rh.hdr.ncolor-1;
		while(ncolors)
		{
			++rf->ainfo.depth;
			ncolors >>= 1;
		}
	}
	
	if(ainfo)
		*ainfo = rf->ainfo;

	return(Success);

rferror:
	err = rf->lasterr;
	goto error;
bad_magic:
	err = Err_bad_magic;
error:
	close_rnd_file(pif);
	return(err);
}


static Errcode read_color_map(Rfile *rf)
/* Read all color entries scale them and load into screen until an end of 
 * color map record. */
{
Cmap *cmap;
Rgb3 *rgb;
USHORT maxintens;
struct {
	UBYTE opcode;
	Rd_cmap cm;
} cmrec;

	cmap = rf->screen->cmap;

	if((maxintens = rf->hdr.maxintens) == 0) /* assume unity if 0 input */
		maxintens = RGB_MAX - 1;

	for(;;)
	{
		if(rf_read(rf,&cmrec,sizeof(cmrec)) < Success)
			goto error;

		switch(cmrec.opcode)
		{
   			case RND_CMAP:   /* Set color map entry */
				if(((USHORT)cmrec.cm.ix) > 0xFF) /* out of range */
					continue;
				rgb = &cmap->ctab[cmrec.cm.ix];
				rgb->r = pj_uscale_by(cmrec.cm.r,(RGB_MAX - 1),maxintens);
				rgb->g = pj_uscale_by(cmrec.cm.g,(RGB_MAX - 1),maxintens);
				rgb->b = pj_uscale_by(cmrec.cm.b,(RGB_MAX - 1),maxintens);
				continue;
   			case RND_CMAPE:  /* End of color map */
				pj_cmap_load(rf->screen,cmap); /* load colors to screen */
				return(rf_seek(rf,1-sizeof(cmrec),SEEK_CUR)); 
			default:
				rf->lasterr = Err_format;
				goto error;
		}
	}
error:
	return(rf->lasterr);
}
static Errcode read_scanline(Rfile *rf)
/* Read scanline (hseg) record and load it into the screen. */
{
struct rd_sline sl;

	if(rf_read(rf,&sl,sizeof(sl)) < Success)
		goto error;

	if(((USHORT)sl.sdlen) > PKT_BUFSIZE)
		goto format_error;

	if(sl.sdlen == 0)
		return(Success);

	sl.y = rf->hdr.ydots-1 - sl.y;
	if(rf_read(rf,rf->pktbuf,sl.sdlen) < Success)
		goto error;

	while(sl.xrpt-- > 0)
	{
		pj_put_hseg(rf->screen,rf->pktbuf,sl.x,sl.y,sl.sdlen);
		sl.x += sl.sdlen;
	}

	return(Success);
format_error:
	rf->lasterr = Err_format;
error:
	return(rf->lasterr);
}
static Errcode read_256_color_frame(Rfile *rf)
/* Read and parse all the opcodes in a RND file. */
{
int opcode;
Rcel *screen = rf->screen;

	/* clear color map to zeros */
	stuff_bytes(0,screen->cmap->ctab,
				screen->cmap->num_colors * sizeof(Rgb3));

#ifdef DEBUG
	#define PRT(s) printf(s)
#else
	#define PRT(s)
#endif /* DEBUG */


	for(;;)
	{
		opcode = fgetc(rf->file);

		switch(opcode)
		{
			case EOF:
				return(Success);
   			case RND_CLEAR:   /* Clear entire display */
				pj_set_rast(screen,0);
				continue;
   			case RND_CMAPB:  /* Begin Color map */
				read_color_map(rf);
				break;
   			case RND_POLY:   /* Polygon */
				read_polygon(rf);
				break;
   			case RND_CRANGE:  /* Continous-color range */
				PRT("RND_CRANGE");
				goto bad_opcode;
   			case RND_CPOLY:   /* Continous-color polygon */
				PRT("RND_CPOLY");
				goto bad_opcode;
			case RND_WSLINE:  /* Output Scan line to driver */
				read_scanline(rf);
				break;
			case RND_RSLINE:  /* Input Scan line from driver */
				PRT("RND_RSLINE");
				goto bad_opcode;
			case RND_RCMAP:   /* Input color map rgb from driver */
				PRT("RND_RCMAP");
				goto bad_opcode;
			case RND_ETAIL:   /* i/o details for hard copy drivers */
				PRT("RND_ETAIL");
				goto bad_opcode;
			case RND_CFGREC:  /* execution time configuration record */
				PRT("RND_CFGREC");
				goto bad_opcode;
			case RND_NEWCFG:  /* new configuration record */
				PRT("RND_NEWCFG");
				goto bad_opcode;
			case RND_CHGCFG:  /* change configuration record */
				PRT("RND_CHGCFG");
				goto bad_opcode;
			case RND_SHOWCFG: /* show configuration record */
				PRT("RND_SHOWCFG");
				goto bad_opcode;
			case RND_FNAME:
				PRT("RND_FNAME");
				goto bad_opcode;
			default:
			bad_opcode:
#ifdef DEBUG
				printf("#%2d @ %06x", opcode, ftell(rf->file));
#endif /* DEBUG */
				rf->lasterr = Err_format;
				goto error;
		}
		if(rf->lasterr < Success)
			goto error;
	}
error:
	return(rf->lasterr);

#undef PRT
}
static Errcode rnd_read_first_frame(Image_file *ifile, Rcel *screen)
/* Now that we know the rnd file has a depth <= 8 bits draw it in the 
 * screen */
{
Rfile *rf = (Rfile *)ifile;

	if(rf->lasterr < Success) /* open and scan must have failed */
		return(rf->lasterr);

	cleanup_buffers(rf);
	if((rf->pktbuf = malloc(PKT_BUFSIZE+rf->ainfo.width)) == NULL)
		return(Err_no_memory);

	rf->pixbuf = OPTR(rf->pktbuf,PKT_BUFSIZE);

	rewind(rf->file); /* clear errors and re-init */

	rf->lasterr = Success;
	rf->screen = screen;


	if(rf_seek(rf,sizeof(Rndhdr),SEEK_SET) < Success)
		goto error;

	return(read_256_color_frame(rf));
error:
	return(rf->lasterr);
}


/**** driver header declaration ******/

#define HLIB_TYPE_1 AA_STDIOLIB
#define HLIB_TYPE_2 AA_GFXLIB
#define HLIB_TYPE_3 AA_SYSLIB
#include "hliblist.h"

static char rnd_title_info[] = "AutoShade .RND render slide format.";

static char long_info[] = "Reads Autoshade render slide files.  Does not "
						  "read files with more than 256 colors.";


Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, HLIB_LIST },
	rnd_title_info, 		/* title_info */
	long_info,              /* long_info */
	".RND",                 /* default_suffi */
	0,1,					/* max_write_frames, max_read_frames */
	NOFUNC,					/* (*spec_best_fit)() */
	NOFUNC,					/* (*create_image_file)() */
	open_rnd_file,			/* (*open_image_file)() */
	close_rnd_file, 		/* (*close_image_file)() */
	rnd_read_first_frame,	/* (*read_first_frame)() */
	NOFUNC,					/* (*read_delta_next)() */
	NOFUNC, 				/* (*save_frames)() */
	NULL,					/* pdr options */
	NOFUNC,					/* (*rgb_seekstart)() */
	NOFUNC,     			 /* (*rgb_readline)() */
};

