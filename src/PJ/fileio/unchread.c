#include "ptrmacro.h"
#include "unchunk.h"

Errcode read_parsed_chunk(Chunkparse_data *pd,void *buf,LONG maxsize)

/* reads in parsed chunk including it's leading Chunk_id if maxsize < 0
 * it will read the entire chunk no matter what. if >= 0 it will only read 
 * that size in NOTE you will always get at least a Fat_chunk worth of 
 * data */
{
	if(maxsize < 0 || (maxsize-=sizeof(Fat_chunk)) > pd->data_size)
		maxsize = pd->data_size;
	*(Fat_chunk *)buf = pd->fchunk;
	if(maxsize > 0)
		return(pj_read_ecode(pd->fd,OPTR(buf,sizeof(Fat_chunk)),maxsize));
	return(Success);
}
