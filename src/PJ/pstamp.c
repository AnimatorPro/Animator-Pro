
/* pstamp.c - routine to read the first frame of a FLIC into a buffer
   1/5 the size of a screen.  (4 out of 5 lines are discarded here).
   Then do an x dimension shrink by 5 and put result on screen somewhere.
   */

#include "jimk.h"
#include "errcodes.h"
#include "fli.h"
#include "rastcomp.h"
#include "unchunk.h"

typedef struct GCC_PACKED frame_rec {
	Fli_frame ff;
	Pstamp_chunk ps;
} Frame_rec;
STATIC_ASSERT(pstamp, sizeof(Frame_rec) == 34);

Boolean pj_frame_has_pstamp(Fli_frame *frame)
{
	return(frame[1].type == FLI_PSTAMP);
}
static int shrink_uncompfli(Rcel *f,Chunk_id *chunk,int count,int sw,int sh,
							int dw, int dh, Boolean do_colors)

/* returns compression type of frame found and decompressed. does not do
 * anything for a FLI_COLOR_0 record */
{
int type = 0;

	while(count-- > 0)
	{
		switch (chunk->type)
		{
			case FLI_COLOR:
				if(do_colors)
					pj_fcuncomp64((const UBYTE *)(chunk+1), f->cmap->ctab);
				break;
			case FLI_COLOR256:
				if(do_colors)
					pj_fcuncomp((const UBYTE *)(chunk+1), f->cmap->ctab);
				break;
	/*		case FPS_BRUN: note value is FPS_BRUN */
			case FLI_BRUN:
			{
				type = chunk->type;
				pj_unbrun_scale_rect(f,chunk + 1, sw, sh, 0,0,dw,dh);
				break;
			}
	/*		case FPS_COPY: note value is FLI_COPY */
			case FLI_COPY:
			{
			Bytemap bm;
			Rasthdr spec;

				type = chunk->type;
				spec.width = sw;
				spec.height = sh;
				spec.pdepth = 8;

				pj_build_bytemap(&spec,(Raster *)&bm,(UBYTE *)(chunk + 1));
				pj_scale_blit(&bm,0,0,sw,sh,f,0,0,dw,dh,NULL);
				break;
			}
			case FLI_COLOR_0: /* note that this does nothing but set type */
				type = chunk->type;
			default:
				break;
		}
		chunk = (Chunk_id *)OPTR(chunk,chunk->size);
	}
	return(type);
}

static Errcode get_fli_pstamp(Rcel *pscel, Flifile *flif,
							  SHORT stampw, SHORT stamph)

/* reads in pstamp or fli first frame and puts inmage into "pscel" at
 * 0,0,stampw,stamph it will use the pscels cmap to read the cmap of the
 * fli into for making a translation table */
{
Errcode err;
UBYTE tctab[256];
UBYTE *xlat;
LONG size;
Boolean not_a_pstamp;
int ctype;

/* first chunck struct for fli frame record */
Frame_rec frec;
Frame_rec *psframe;

	psframe = &frec;

	if((size = flif->hdr.frame2_oset - flif->hdr.frame1_oset) > sizeof(frec))
		size = sizeof(frec);

	if((err = pj_readoset(flif->fd,&frec,
						  flif->hdr.frame1_oset, size)) < Success)
	{
		goto error;
	}

	if(frec.ff.type != FCID_FRAME)
	{
		err = Err_bad_magic;
		goto error;
	}

	/* figure out what size to read in */

	not_a_pstamp = frec.ps.type != FLI_PSTAMP;

	if(not_a_pstamp || frec.ps.data.type == FPS_XLAT256)
	{
		size = frec.ff.size;
	}
	else
	{
		size = sizeof(frec.ff) + frec.ps.size;
	}

	/* If bigger than what we read alloc a big buffer and read the thing */

	if(size > sizeof(frec))
	{
		if((psframe = pj_malloc(size)) == NULL)
		{
			err = Err_no_memory;
			goto error;
		}
		*psframe = frec;
		if((err = pj_read_ecode(flif->fd, psframe + 1,
								size - sizeof(*psframe))) < Success)
		{
			goto error;
		}
	}

	/* if not a pstamp record or just a translation table we need the actual
	 * first frame chunk */

	if(not_a_pstamp || frec.ps.data.type == FPS_XLAT256)
	{
		ctype = shrink_uncompfli(pscel,(Chunk_id *)&(psframe->ps),
								frec.ff.chunks,
								flif->hdr.width,flif->hdr.height,
								stampw, stamph, not_a_pstamp);

		if(not_a_pstamp) /* if we don't have a translation record, make one */
		{
			/* only one color needed for a one color frame */

			pj_make_pstamp_xlat(pscel->cmap->ctab, tctab,
								ctype == FLI_COLOR_0?1:COLORS );

			xlat = tctab;
		}
		else /* just set the pointer */
		{
			xlat = (UBYTE *)(psframe + 1);
		}

		/* color 0 needs only simple treatment */

		if(ctype == FLI_COLOR_0)
			pj_set_rast(pscel,xlat[0]);
		else
			xlat_rast(pscel,xlat,1);
	}
	else /* we have a pstamp record, no transltion needed only one chunk */
	{
		if((err = shrink_uncompfli(pscel,(Chunk_id *)&(psframe->ps.data),
								   1, frec.ps.width, frec.ps.height,
								   stampw, stamph, FALSE)) < Success)
		{
			goto error;
		}
	}


	err = Success;

error:

	if(psframe != &frec)
		pj_gentle_free(psframe);

	return(err);
}
static Errcode get_pic_pstamp(Rcel *smallcel,Jfile f,Pic_header *pic,
							  SHORT stampw, SHORT stamph)

/* this is here to handle the old obsolete animator 1.0 cel type files */
{
Errcode err;
Rcel *pcel;
UBYTE xlat[256];

	if ((err = valloc_ramcel(&pcel, pic->width, pic->height)) < Success)
		goto error;

	if((err = pj_read_picbody(f,pic,(Raster *)pcel,pcel->cmap)) < Success)
		goto error;

	pj_scale_blit(pcel,0,0,pic->width,pic->height,
				  smallcel,0,0,stampw,stamph,NULL);

	pj_make_pstamp_xlat(pcel->cmap->ctab, xlat, COLORS);
	xlat_rast(smallcel,xlat,1);
	err = Success;
error:
	pj_rcel_free(pcel);
	return(err);
}
Errcode postage_stamp(Raster *r, char *name,
					  SHORT x,SHORT y,USHORT width, USHORT height,
					  Rectangle *actual)
{
Jfile jf;
union picfile {
	Flifile flif;
	Pic_header pic;
} pf;
Boolean isafli;
Errcode err;
SHORT stampw, stamph;
Rcel *ramcel = NULL;

	if((err = pj_fli_open(name, &pf.flif, JREADONLY)) >= Success)
	{
		isafli = TRUE;
		stampw = pf.flif.hdr.width;
		stamph = pf.flif.hdr.height;
	}
	else
	{
		isafli = FALSE;
		if ((jf = pj_open(name, JREADONLY)) == JNONE)
			return(pj_ioerr());
		/* note we return error for fli open above on failure */
		if(pj_read_pichead(jf,&pf.pic) < 0)
			goto error;
		stampw = pf.pic.width;
		stamph = pf.pic.height;
	}
	if(stampw <= 0 || stamph <= 0)
	{
		err = Err_corrupted;
		goto error;
	}

	pj_get_stampsize(width, height, stampw, stamph, &stampw, &stamph );

	if ((err = valloc_ramcel(&ramcel, stampw, stamph)) < Success)
		goto error;

	ramcel->x = x + ((width - stampw)>>1);
	ramcel->y = y + ((height - stamph)>>1);

	if(isafli)
		err = get_fli_pstamp(ramcel, &pf.flif, stampw, stamph);
	else
		err = get_pic_pstamp(ramcel,jf,&pf.pic,stampw,stamph);

	if(err < Success)
		goto error;

	pj_blitrect(ramcel, 0, 0, r, ramcel->x, ramcel->y, stampw, stamph);

	if(actual)
		copy_rectfields(ramcel,actual);

error:
	if(isafli)
		pj_fli_close(&pf.flif);
	else
		pj_close(jf);

	pj_rcel_free(ramcel);
	return(err);
}

