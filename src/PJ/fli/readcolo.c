#include "cmap.h"
#include "errcodes.h"
#include "fli.h"
#include "memory.h"

Errcode fli_read_colors(Flifile *flif, Cmap *cmap)

/* returns Success if got it Err_no_record not or errcode (<0) */
{
Errcode err;
Fli_frame ff;
Chunk_id *chunk;
void (*uncomp)(const UBYTE *buf, Rgb3 *dst);
LONG oset;

	oset = flif->hdr.frame1_oset;

	err = xffreadoset(flif->xf, &ff, oset, sizeof(ff));
	if (err < Success)
		return err;

	if((err=pj_fli_alloc_cbuf((void *)&chunk,16,16,cmap->num_colors))<0)
		return(err);

	oset += sizeof(Fli_frame);
	ff.size -= sizeof(Fli_frame);

	while(ff.chunks-- > 0)	
	{
		if (ff.size < (long)sizeof(Chunk_id))
			goto corrupted;

		err = xffreadoset(flif->xf, chunk, oset, sizeof(Chunk_id));
		if (err < Success)
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
				err = xffread(flif->xf, chunk, chunk->size - sizeof(Chunk_id));
				if (err < Success)
					goto out;

				(*uncomp)((const UBYTE *)chunk, cmap->ctab);
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
