#include "ptrmacro.h"
#include "unchunk.h"

Errcode copy_parsed_chunk(Chunkparse_data *pd, Jfile dest)

/* copys to dest the parsed chunk including it's leading Chunk_id 
 * this prefers to seek. It might be better to do two writes */
{
LONG size;

	if(pd->data_size <= 0)
	{
		size = sizeof(Fat_chunk) + pd->data_size;
		if(pj_write(dest,&pd->fchunk,size) != size)
			return(pj_ioerr());
		return(Success);
	}
	else
	{
		if((size = pj_seek(pd->fd,pd->chunk_offset,JSEEK_START)) < 0)
			return(size);
		return(pj_copydata(pd->fd,dest,pd->data_size+sizeof(Fat_chunk)));
	}
}
