#include "errcodes.h"
#include "ptrmacro.h"
#include "unchunk.h"

Errcode update_parsed_chunk(Chunkparse_data *pd, void *buf)

/* rewrites but does not alter size of parsed chunk must have same type and
 * size fields This depends on the version field following the size and type
 * fields in the Fat_chunk */
{
	if(((Fat_chunk *)buf)->size != pd->fchunk.size
		|| ((Fat_chunk *)buf)->type != pd->fchunk.type)
	{
		return(Err_bad_input);
	}
	if(pd->data_size <= 0)
		return(Success);
	return(pj_write_ecode(pd->fd,OPTR(buf,sizeof(Chunk_id)),pd->data_size));
}
