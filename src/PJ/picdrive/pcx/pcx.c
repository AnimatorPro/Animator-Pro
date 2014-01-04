
/* pcx.c - Source to PCX format PJ picture driver. */

#define REXLIB_INTERNALS
#include "errcodes.h"
#include "stdio.h"
#include "memory.h"
#include "pcx.h"
#include "picdrive.h"

#define PCX_MAX_RUN 63	/* longest run (since hi 2 bits are used elsewhere */
#define PCX_CMAP_MAGIC 12

/***** Uncompression and bit-plane to byte-a-pixel routines *******/


typedef struct unpcx_obj
/* This object allows us to decode a pcx file a line at a time. */
	{
	UBYTE *buf;
	UBYTE *over;
	int over_count;
	int bpl;
	FILE *file;
	} Unpcx_obj;

static Errcode unpcx_init(
	Unpcx_obj *upo, 	/* line unpacker object */
	int bpl, 			/* bytes-per-line */
	FILE *file)			/* source file */
/* Initialize object and allocate buffer big enough for bpl bytes plus
 * overflow.  */
{
clear_struct(upo);
if ((upo->buf = pj_malloc(bpl + PCX_MAX_RUN + 1)) ==  NULL)
	return(Err_no_memory);
upo->over = upo->buf + bpl;		/* Overflow area is just past buf proper */
upo->file = file;
upo->bpl = bpl;
return(Success);
}

static void unpcx_cleanup(Unpcx_obj *upo)
/* Cleanup resources associated with upo object. */
{
pj_freez(&upo->buf);
}


static Errcode unpack_pcx_line(Unpcx_obj *upo)
{
int bytes_left;
UBYTE *p;
register int count;
UBYTE data;
FILE *file = upo->file;

						/* first deal with any overflow from last line */
pj_copy_bytes(upo->over, upo->buf, upo->over_count);
bytes_left = upo->bpl - upo->over_count;
p = upo->buf + upo->over_count;
						/* loop while haven't decoded at least a line */
while (bytes_left > 0)
	{
	if ((count = getc(file)) < 0)		/* fetch next byte */
		return(Err_truncated);
	if ((count & 0xc0) == 0xc0)			/* if hi two bits set it's a run  */
		{
		count &= 0x3f;
		data = getc(file);
		bytes_left -= count;
		while (--count >= 0)
			*p++ = data;
		}
	else								/* it's just a normal pixel */
		{
		*p++ = count;
		bytes_left -= 1;
		}
	}
upo->over_count = -bytes_left;
return(Success);
}

static void bits_to_bytes(UBYTE *in, UBYTE *out, int w, UBYTE out_mask)
/* Subroutine to help convert from bit-plane to byte-a-pixel representation */
/* or the out_mask into out byte-plane wherever a bit in in bit-plane is set */
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

static bits2_to_bytes(UBYTE *in, UBYTE *out, int w)
/* Convert from CGA 2-bit-a-pixel format to byte-a-pixel format */
{
int k;
UBYTE imask;
UBYTE c, outc;

while (w > 0)
	{
	imask = 0x80;
	k = 4;
	if (k > w)
		k = w;
	c = *in++;
	while (--k >= 0)
		{
		if (c&imask)
			outc = 1;
		else
			outc = 0;
		imask >>= 1;
		if (c&imask)
			outc |= 2;
		*out++ = outc;
		imask >>= 1;
		}
	w -= 4;
	}
}

static Errcode unpack_pcx(Rcel *screen, Pcx_header *hdr, FILE *f)
{
Errcode err;
Unpcx_obj rupo;
int width, height;
int bpl;
int i;
UBYTE *uout_buf = NULL;
int depth, depth1;
UBYTE out_mask;

if ((err = unpcx_init(&rupo, hdr->bpl, f)) < Success)
	goto OUT;
width = hdr->x2 - hdr->x1 + 1;
height = hdr->y2 - hdr->y1 + 1;
if ((uout_buf = pj_malloc(width)) == NULL)
	{
	err = Err_no_memory;
	goto OUT;
	}
bpl = hdr->bpl;
if (hdr->nplanes == 1)		/* easy single plane case */
	{
	for (i=0; i<height; i++)
		{
		if ((err = unpack_pcx_line(&rupo)) < Success)
			goto OUT;
		switch (hdr->bitpx)
			{
			case 1:
				pj_set_hline(screen, 0, 0, i, width);
				pj_mask1blit(rupo.buf, bpl, 0, 0, screen, 0, i, width, 1, 1);
				break;
			case 2:
				bits2_to_bytes(rupo.buf, uout_buf, width);
				pj_put_hseg(screen, uout_buf,0,i,width);
				break;
			case 8:
				pj_put_hseg(screen, rupo.buf,0,i,width);
				break;
			default:
				err = Err_pdepth_not_avail;
				goto OUT;
			}
		}
	}
else if (hdr->bitpx == 1)
	{
	depth1 = hdr->nplanes;
	for (i=0; i<height; i++)
		{
		clear_mem(uout_buf, width);
		depth = depth1;
		out_mask = 1;
		while (--depth >= 0)
			{
			if ((err = unpack_pcx_line(&rupo)) < Success)
				goto OUT;
			bits_to_bytes(rupo.buf, uout_buf, width, out_mask);
			out_mask<<=1;
			}
		pj_put_hseg(screen, uout_buf,0,i,width);
		}
	}
else		/* some wierd currently unknown combination */
	{
	err = Err_pdepth_not_avail;
	}
OUT:
unpcx_cleanup(&rupo);
pj_freez(&uout_buf);
return(err);
}

/**** Compression code ****/

static void pcx_comp_buf(FILE *out, UBYTE *buf, int count)
/* Do PCX compression of buf into file out */
{
int same_count, lcount;
int c;

same_count = 0;
while ((count -= same_count) > 0)
	{
	if ((lcount = count) > PCX_MAX_RUN)
		lcount = PCX_MAX_RUN;
	if ((same_count = pj_bsame(buf,  lcount)) > 1)
		{
		putc(same_count|0xc0, out);
		putc(*buf, out);
		buf += same_count;
		}
	else
		{
		c = *buf;
		if ((c&0xc0) == 0xc0)
			{
			putc(0xc1, out);
			putc(c, out);
			buf += 1;
			}
		else
			{
			putc(c, out);
			buf += 1;
			}
		}
	}
}

static Errcode pcx_save_screen(FILE *out, Rcel *screen)
/* Save out header, pixel, and color map corresponding to screen.  Assumes
 * file open and at start of  file. */
{
Pcx_header rhdr;
UBYTE *buf = NULL;
Errcode err = Success;
int width = screen->width, height = screen->height, i;

												/* Set up header */
clear_struct(&rhdr);
rhdr.magic = 10;
rhdr.version = 5;
rhdr.encode = 1;
rhdr.bitpx = 8;
rhdr.x1 = 0;
rhdr.y1 = 0;
rhdr.x2 = screen->width-1;
rhdr.y2 = screen->height-1;
rhdr.cardw = screen->width;
rhdr.cardh = screen->height;
rhdr.nplanes = 1;
rhdr.bpl = screen->width;
if (fwrite(&rhdr, sizeof(rhdr), 1, out) < 1)	/* Write header */
	goto IOERR;
if ((buf = pj_malloc(screen->width)) == NULL)		/* Get line buffer */
	{
	err = Err_no_memory;
	goto OUT;
	}
for (i=0; i<height; i++)						/* Write out each line */
	{
	pj_get_hseg(screen, buf, 0, i, width);
	pcx_comp_buf(out, buf, width);
	if (ferror(out) != 0)
		goto IOERR;
	}
putc(PCX_CMAP_MAGIC,out);						/* Write out color map */
fwrite(screen->cmap->ctab,  sizeof(screen->cmap->ctab[0]),  
	screen->cmap->num_colors, out);
if (ferror(out) != 0)
	goto IOERR;
goto OUT;
IOERR:
	err = pj_errno_errcode();
	goto OUT;
OUT:
	pj_freez(&buf);
	return(err);
}








/**** Routines having more to do with PJ than PCX ******/

static pcx_files_open = 0;						/* lock data structures
											     * to prevent open without
											     * close */

static Boolean suffix_in(string, suff)
char *string, *suff;
{
string += strlen(string) - strlen(suff);
return( txtcmp(string, suff) == 0);
}

static Errcode decode_version(Pcx_header *hdr, Boolean *with_cmap)
/* Figure out whether a this version has a color map or not.  Also figure
 * out number number of planes in this version.  If version is unknown
 * return Errcode, else success. */
{
switch (hdr->version)
	{
	case 0:
		hdr->nplanes = 1;
		*with_cmap = FALSE;
		break;
	case 2:
		*with_cmap = TRUE;
		break;
	case 3:
		*with_cmap = FALSE;
		break;
	case 5:
		*with_cmap = TRUE;
		break;
	default:
		return(Err_version);
	}
return(Success);
}

static Errcode read_pcx_start(Pcx_file *gf,
							  struct pcx_header *hdr,
							  Anim_info *ainfo, Boolean *got_cmap)
/* Read in PCX - header,  and verify it is a good header.  
 *  Move appropriate fields from hdr to ainfo */
{
Errcode err;
FILE *f;

f = gf->file;

if(fread(hdr, 1, sizeof(*hdr), f) < sizeof(*hdr))
	goto io_error;

if (hdr->encode != 1)		/* all PCX files use compression type 1 */
	{
	err = Err_bad_magic;
	goto error;
	}

if ((err =  decode_version(hdr, got_cmap)) < Success)
	goto  error;

switch (hdr->bitpx)
	{
	case 1:
	case 2:
	case 8:
		break;
	default:
		err = Err_pdepth_not_avail;
		break;
	}

memset(ainfo,0,sizeof(*ainfo));
ainfo->width = hdr->x2 - hdr->x1 + 1;
ainfo->height = hdr->y2 - hdr->y1 + 1;
ainfo->num_frames = 1;
ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;
ainfo->depth = 8;		/* we'll always convert it to 8 bits */
return(Success);

io_error:
err = pj_errno_errcode();

error:
return(err);
}

static Boolean pcx_spec_best_fit(Anim_info *ainfo)
/* Tell host that we can only write 8 bit-a-pixel images,  and only one
 * frame.  No need to check width and height, since PCX format handles
 * any width/height. */
{
Boolean nofit;

nofit = (ainfo->depth == 8
		 && ainfo->num_frames == 1);
ainfo->depth = 8;
ainfo->num_frames = 1;
return(nofit);	/* return whether fit was exact */
}

static close_pcx_file(Pcx_file **pcxile)
/* Clean up resources used by PCX reader/writer */
{
Pcx_file *gf;

if(pcxile == NULL || (gf = *pcxile) == NULL)
	return;
if(gf->file)
	fclose(gf->file);
pj_free(gf);
*pcxile = NULL;
pcx_files_open = FALSE;
}

static Errcode pcx_open_ifsub(Pcx_file **pcxile, char *path, char *rwmode)
/* Check path suffix.  Allocate Pcx_file structure.  Open up a file.  
 * Return Errcode if any problems. */
{
Errcode err = Success;
Pcx_file *gf;

*pcxile = NULL;

if(pcx_files_open)
	return(Err_too_many_files);

if (!suffix_in(path, ".PCX"))
	return(Err_suffix);

if((gf = pj_zalloc(sizeof(Pcx_file))) == NULL)
	return(Err_no_memory);

if((gf->file = fopen(path, rwmode)) == NULL)
	err = pj_errno_errcode();

pcx_files_open = TRUE;
*pcxile = gf;
return(err);
}

static Errcode open_pcx_file(Pdr *pd, char *path, Image_file **pif, 
							 Anim_info *ainfo )
/* Check path for PCX suffix.  If there open it and read in header.
 * Return Errcode if header is bad or other failure. */
{
Errcode err;
Pcx_file **ppcx;
struct pcx_header hdr;
Boolean got_cmap;

ppcx = (Pcx_file **)pif;

if((err = pcx_open_ifsub(ppcx, path, "rb")) < Success)
	goto error;

if((err = read_pcx_start(*ppcx,&hdr,&((*ppcx)->ainfo),&got_cmap)) < Success)
	goto error;

if(ainfo)
	*ainfo = (*ppcx)->ainfo;
return(Success);

error:
close_pcx_file(ppcx);
return(err);
}

static Errcode create_pcx_file(Pdr *pd, char *path, Image_file **pif, 
							   Anim_info *ainfo )
/* Make sure path  has .PCX suffix. Create PCX file (but don't write 
 * anything to it yet).  Save ainfo where we can get to it later. */
{
Errcode err;
Pcx_file **ppcx;

ppcx = (Pcx_file **)pif;

if((err = pcx_open_ifsub(ppcx, path, "wb")) < Success)
	goto error;

(*ppcx)->ainfo = *ainfo;
return(Success);

error:
close_pcx_file(ppcx);
return(err);
}

static UBYTE default_pcx_cmap[] = 
		  {
	      0x00, 0x00, 0x00,       
	      0x00, 0x00, 0xaa,
	      0x00, 0xaa, 0x00,
	      0x00, 0xaa, 0xaa,
	      0xaa, 0x00, 0x00,
	      0xaa, 0x00, 0xaa,
	      0xaa, 0xaa, 0x00,
	      0xaa, 0xaa, 0xaa,
	      0x55, 0x55, 0x55,
	      0x55, 0x55, 0xff,
	      0x55, 0xff, 0x55,
	      0x55, 0xff, 0xff,
	      0xff, 0x55, 0x55,
	      0xff, 0x55, 0xff,
	      0xff, 0xff, 0x55,
	      0xff, 0xff, 0xff        
		  };

static UBYTE bwcmap[] = {0,0,0,0xff,0xff,0xff,};

static Errcode pcx_read_picframe(Image_file *ifile, Rcel *screen)
/* Seek to the beginning of an open  PCX file, and then read in the
 * first frame of image into screen.  (In our case read in the only
 * frame of image.) */
{
Errcode err;
Pcx_file *gf;
FILE *pcx_load_file;
Anim_info info;
Pcx_header hdr;
Boolean got_cmap;
Cmap  *cmap = screen->cmap;
Rgb3  *ctab  = cmap->ctab;

	gf = (Pcx_file *)ifile;		/* ifile has more data past the Image_file */
	pcx_load_file = gf->file; 	/* Grab the FILE handle */
	rewind(pcx_load_file);		/* Go back to beginning of file */

								/* Load and verify header. */
	if((err = read_pcx_start(gf, &hdr, &info, &got_cmap)) < Success)
		return(err);

	if (got_cmap)
		pj_copy_bytes(hdr.palette, ctab, 16*sizeof(ctab[0]));
	else
		{
		if (hdr.nplanes*hdr.bitpx == 1)    /* one bit plane no cmap, use b&w */
			pj_copy_bytes(bwcmap,ctab,2*sizeof(ctab[0]));
		else							   /* use standard vga thingie */
			pj_copy_bytes(default_pcx_cmap, ctab, 16*sizeof(ctab[0]));
		}
	pj_cmap_load(screen,cmap); /* update hardware cmap if needed */

	if ((err = unpack_pcx(screen, &hdr, pcx_load_file)) < Success)
		return(err);

	if (hdr.bitpx == 8 && got_cmap)
		{
		if (getc(pcx_load_file) != PCX_CMAP_MAGIC)  
			return(Err_bad_magic);
		fread(ctab,  sizeof(ctab[0]),  cmap->num_colors, pcx_load_file);
		pj_cmap_load(screen,cmap); /* update hardware cmap if needed */
		}
	return(err);
}

static Errcode pcx_read_next(Image_file *ifile,Rcel *screen)
/* Read in subsequent frames of image.  Since we only have one  this
 * routine is pretty trivial. */
{
	return(Success);
}


static Errcode pcx_save_frame(Image_file *ifile, Rcel *screen, int num_frames,
						      Errcode (*seek_frame)(int ix,void *seek_data),
						      void *seek_data, Rcel *work_screen ) 
{
Pcx_file *gf;

gf = (Pcx_file *)ifile;

if( gf->ainfo.width != screen->width
	|| gf->ainfo.height != screen->height)
	{
	return(Err_bad_input);
	}
return(pcx_save_screen(gf->file, screen));
}

/**** driver header declaration ******/

static char pcx_pdr_name[] = "PCX.PDR";
static char pcx_title_info[] = "PCX standard picture format.";

static Pdr pcx_pdr_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, NULL, NULL, NULL },
	pcx_title_info,  		/* title_info */
	"",  					/* long_info */
	".PCX",			 		/* default_suffi */
	1,1,			 		/* max_write_frames, max_read_frames */
	pcx_spec_best_fit,		/* (*spec_best_fit)() */
	create_pcx_file,		/* (*create_image_file)() */
	open_pcx_file,			/* (*open_image_file)() */
	close_pcx_file,			/* (*close_image_file)() */
	pcx_read_picframe,		/* (*read_first_frame)() */
	pcx_read_next,			/* (*read_delta_next)() */
	pcx_save_frame,			/* (*save_frames)() */
};

Local_pdr pcx_local_pdr = {
	NULL,
	pcx_pdr_name,
	&pcx_pdr_header
};
