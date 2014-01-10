#include "errcodes.h"
#include "fli.h"

extern void pj_fcuncomp64(void *buf,Rgb3 *ctab);

Errcode fli_read_colors(Flifile *flif, Cmap *cmap)

/* returns Success if got it Err_no_record not or errcode (<0) */
{
Errcode err;
Fli_frame ff;
Chunk_id *chunk;
void (*uncomp)(void *buf,Rgb3 *ctab);
LONG oset;

	oset = flif->hdr.frame1_oset;

	if((err = pj_readoset(flif->fd,&ff,oset,sizeof(ff))) < 0)
		return(err);

	if((err=pj_fli_alloc_cbuf((void *)&chunk,16,16,cmap->num_colors))<0)
		return(err);

	oset += sizeof(Fli_frame);
	ff.size -= sizeof(Fli_frame);

	while(ff.chunks-- > 0)	
	{
		if(ff.size < sizeof(Chunk_id))
			goto corrupted;

		if((err = pj_readoset(flif->fd,chunk,oset,sizeof(Chunk_id))) < 0)
			goto out;
		if((ff.size -= chunk->size) < 0)
			goto corrupted;
		oset += chunk->size;

		switch(chunk->type)
		{
			case FLI_COLOR:
				uncomp = pj_fcuncomp64;
				goto readit;
			case FLI_COLOR256:
				uncomp = pj_fcuncomp;
			readit:
				if((err = pj_read_ecode(flif->fd,chunk,
									  chunk->size - sizeof(Chunk_id))) < 0)
				{
					goto out;
				}
				(*uncomp)(chunk,cmap->ctab);
				goto out;
			default:
				break;
		}
	}
	err = Err_no_record;
	goto out;

corrupted:
	err = Err_corrupted;
out:
	pj_free(chunk);
	return(err);
}
