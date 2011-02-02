#include "errcodes.h"
#include "picfile.h"
#include "unchunk.h"
#include "palchunk.h"

static Errcode read_pic_pixels(Jfile f,Pic_header *pic,Raster *cel,
							   SHORT chunk_type, LONG offset, LONG data_size)
{
Errcode err;
LONG bufsize;
Bytemap rr;
LONG bpr;
LONG toposet;
Boolean bufopen = 0;
SHORT x, y, ymax;
LONG lct;


	if(pic->width < cel->width || pic->height < cel->height)
		pj_clear_rast(cel); /* pic is smaller than cel */

	/* by the time we get here we must have the right type of chunk and
	 * the file must be positioned to the beginning of the data */

	switch(chunk_type)
	{
		case PIC_BITPIXELS:
			bpr = Bitmap_bpr(pic->width);
		    if(cel->type == RT_BITMAP && cel->pdepth == 1)
				goto check_readplane;
			break;
		case PIC_BYTEPIXELS:
			bpr = Bytemap_bpr(pic->width);
			if(cel->type == RT_BYTEMAP && cel->pdepth == 8)
				goto check_readplane;
			break;
		check_readplane:
			if(pic->depth != cel->pdepth
				|| data_size != cel->hw.bm.psize
		   		|| pic->width != cel->width 
		   		|| pic->height != cel->height)
			{
				break;
			}

			/* Note this is dependent on a Bytemap and Bitmap being the 
			 * same structure */

			return(pj_readoset(f,((Bytemap *)cel)->bm.bp[0],
								 offset, ((Bytemap *)cel)->bm.psize));
	}

	/* open a big buffer to read in pic */

	if((err = pj_open_temprast(&rr,pic->width,pic->height,pic->depth)) < 0)
		goto error;

	/* center pic */

	x = (cel->width - pic->width)/2;
	y = (cel->height - pic->height)/2;

	ymax = y + pic->height;
	if(ymax > cel->height)
		ymax = cel->height;

	/* seek to top of cel if part of pic above top of cel */

	if(y < 0) 
	{
		toposet = -y * bpr;
		data_size -= toposet;
		offset += toposet;
		y = 0;
	}
	if((offset = pj_seek(f,offset,JSEEK_START)) < 0)
	{
		err = offset;
		goto error;
	}

	err = Success;
	for (;;)
	{
		if ((lct = ymax - y) <= 0)	
			break;
		/* If distance greater than rect height, set it to rect height */
		if (lct > rr.height)
			lct = rr.height;
		bufsize = lct * bpr;
		if((data_size -= bufsize) < 0)
		{
			err = Err_corrupted;
			break;
		}
		if((err = pj_read_ecode(f, rr.bm.bp[0], bufsize)) < Success)
			break;
		pj_blitrect(&rr, 0, 0, cel, x, y, rr.width, lct); /* this clips!! */
		y += lct;
	}
error:
	pj_close_raster(&rr);
	return(err);
}
Errcode pj_read_picbody(Jfile f,Pic_header *pic,Raster *cel, Cmap *cmap)

/* from the file position just beyond the pic header will read a pic
 * into an Rcel it will truncate the pic if the cel is smaller and
 * center it otherwise */
{
Errcode err;
Chunkparse_data pd;

	if(pic->id.type == OPIC_MAGIC)
	{
		if(cmap)
		{
			if((err = pj_readoset(f,cmap->ctab,sizeof(Opic_header),
										  COLORS*3)) < Success)
			{
				goto error;
			}
			pj_shift_cmap(cmap->ctab,cmap->ctab,COLORS*3);
			pj_cmap_load(cel,cmap);
		}
		return(read_pic_pixels(f,pic,cel,PIC_BYTEPIXELS,
								   	sizeof(Opic_header)+(COLORS*3), 
									pic->width * pic->height ));
	}


	init_chunkparse(&pd,f,DONT_READ_ROOT,0,sizeof(Pic_header),pic->id.size);
	while(get_next_chunk(&pd))
	{
		switch(pd.type)
		{
			case PIC_CMAP:
				if(!cmap)
					break;
				if((err = pj_read_palchunk(f,&pd.fchunk,cmap)) < Success)
					goto error;
				pj_cmap_load(cel,cmap);
				cmap = NULL; /* flag done */
				break;
			case PIC_BITPIXELS:
			case PIC_BYTEPIXELS:
				if((err = read_pic_pixels(f,pic,cel,pd.type,
							  pd.chunk_offset + sizeof(Chunk_id),
							  pd.fchunk.size - sizeof(Chunk_id))) < Success)
				{
					goto error;
				}
				if(!cmap)
					goto done;
			default:
				break;
		}
	}
	if(pd.error < Success)
		goto error;
		
	if(cmap) /* no cmap chunk found and requested */
	{
		pj_get_default_cmap(cmap);
		pj_cmap_load(cel,cmap);
	}

done:
error:
	return(err);
}
Errcode load_pic(char *name,Rcel *rcel,LONG check_id, Boolean do_colors)

/* if do_colors is FALSE it may load a raster without a cmap */
{
Errcode err;
Jfile f;
Pic_header pic;

	if ((f = pj_open(name, JREADONLY)) == JNONE)
		return(pj_ioerr());
	if((err = pj_read_pichead(f,&pic)) < 0)
		goto error;
	if(check_id && check_id != pic.user_id)
	{
		err = Err_invalid_id;
		goto error;
	}
	if((err = pj_read_picbody(f,&pic,(Raster *)rcel,
							  do_colors?rcel->cmap:NULL)) < 0)
	{
		goto error;
	}
error:
	pj_close(f);
	return(err);
}


