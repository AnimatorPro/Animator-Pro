#include "fli.h"

Errcode jwrite_chunk(Jfile f, /* file to write to */
					 void *data, /* chunk data */
					 LONG size, /* sizeof chunk data not including 
								* chunk header */
					 SHORT type) /* chunk_id type */

/* writes a "chunk" record to a file, returns error code */
{
Chunk_id chunk;

	chunk.size = size + sizeof(chunk);
	chunk.type = type;

	if(pj_write(f, &chunk, sizeof(chunk)) != sizeof(chunk)) 
		goto jio_error;

	if(pj_write(f, data, size) != size) 
		goto jio_error;

	return(Success);
jio_error:
	return(pj_ioerr());
}
