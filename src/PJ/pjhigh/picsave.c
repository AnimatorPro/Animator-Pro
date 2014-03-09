#include "errcodes.h"
#include "memory.h"
#include "palchunk.h"
#include "picfile.h"
#include "vmagics.h"

Errcode save_pic(char *name,Rcel *screen,LONG id, Boolean save_colors)
/* if save_colors is FALSE it may save a raster without a cmap */
{
Errcode err;
Jfile f;
SHORT y;
Pic_header pic;
Chunk_id chunk;
UBYTE *lbuf = NULL;
long lines;
long lct;
long bufsize;
long bpr;


	if((f = pj_create(name,JWRITEONLY)) == JNONE)
	{
		err = pj_ioerr();
		goto error;
	}

	/* load pic header data and write initial version ***/

	clear_mem(&pic, sizeof(pic) );
	pic.id.type = PIC_MAGIC;
	copy_rectfields(screen,&pic);
	pic.user_id = id;
	pic.depth = screen->pdepth;

	if((err = pj_write_ecode(f, &pic, (long)sizeof(pic))) < Success)
		goto error;

	if(save_colors)  /* if save colors is true write out a color map chunk */
	{
		if((err = pj_write_palchunk(f, screen->cmap, PIC_CMAP)) < Success)	
			goto error;
	}

	/***** write out a pixels chunk for the screen ******/
	/* get bytes per row to get plane size of pixels */

	if(pic.depth == 1 && screen->type == RT_BITMAP)
	{
		chunk.type = PIC_BITPIXELS;
		bpr = Bitmap_bpr(pic.width);
	}
	else if(pic.depth == 8)
	{
		chunk.type = PIC_BYTEPIXELS;
		bpr = Bytemap_bpr(pic.width);
	}
	else
	{
		err = Err_bad_input;
		goto error;
	}

	chunk.size = (bpr*pic.height) + sizeof(chunk);
	if((err = pj_write_ecode(f,&chunk,sizeof(chunk))) < Success)
		goto error;

	if((screen->type == RT_BYTEMAP || screen->type == RT_BITMAP)
		&& bpr == screen->hw.bm.bpr) 
	{
		if((err = pj_write_ecode(f, screen->hw.bm.bp[0], 
										bpr*pic.height)) < Success)
		{
			goto error;
		}
	}
	else /* note this doesnt handle writing low pixel depth items yet */
	{
		lines = screen->height;
		if((err = pj_get_rectbuf(screen->width, &lines, &lbuf)) < 0)
			goto error;
		y = 0;
		for (;;)
		{
			/* Compute distance to bottom of screen and bail if 0 or less */
			if ((lct = screen->height - y) <= 0)	
				break;
			/* If distance greater than rect height, set it to rect height */
			if (lct > lines)
				lct = lines;
			pj_get_rectpix(screen,lbuf,0,y,screen->width,lct);
			bufsize = lct * screen->width;
			if((err = pj_write_ecode(f, lbuf, bufsize)) < Success)
				goto error;
			y += lct;
		}
	}

	if((pic.id.size = pj_tell(f)) < 0)
	{
		err = pic.id.size;
		goto error;
	}
	/* flush final header */
	err = pj_writeoset(f,&pic,0,sizeof(pic));

error:
	pj_gentle_free(lbuf);
	pj_close(f);
	if(err < Success)
		pj_delete(name);
	return(err);
}

