#ifdef COMMENT
/******************* chunk parsing functions ********************/

/*
These functions are used to parse data files organized as sets of "chunks"
preceded by Chunk_id fields. These chunks may have optional "header data"
followed by subordinate chunks. It is most efficient when parsing "Fat_chunks"

as in:

struct file {

Fat_chunk file_id;	/* type for file size of whole file inclusive of id */
	head_data;	/* header data for this chunk immediately following id */
	Fat_chunk;	/* sub chunk */
		head_data;
		Fat_chunk;	/* sub chunk */
			chunk_data;
		Fat_chunk;	/* sub chunk */
			chunk_data;
	Fat_chunk;	/* sub chunk */
		chunk_data;
	Fat_chunk;	/* sub chunk */
		chunk_data;
EOF
*/

#endif /* BIG_COMMENT */



#include "errcodes.h"
#include "ptrmacro.h"
#include "unchunk.h"

#define Nextoset error




void init_chunkparse(Chunkparse_data *pd, Jfile fd, 
					 LONG root_type, LONG root_oset, 
					 LONG head_size, LONG root_size)
/*********************************************************************
 * Initializes a control structure for parsing chunks subordinate to
 * a Chunk_id containing sub chunks.
 *
 * Input args are:
 *
 * pd 			The cunkparse data to initialize
 * fd			The file to read chunks from
 * root_type  	The type expected, If set to -1 it will not read or verify
 *				the existence of the root chunk and will assume the root
 *				chunk is size provided.
 * root_oset	Offset to root chunk in file. If < 0 (DONT_SEEK_ROOT)
 				set offset to current position.
 * head_size    Size of any header data in the root chunk before the
 *				first subordinate chunk. inclusive of root sizeof(Chunk_id). 
 *				( head_size <= sizeof(Fat_chunk) means no head data).
 * root_size    The root chunk size provided if root_type is -1
 *
 ********************************************************************/
{
	pd->error = Success; /* good until fail */
	pd->fd = fd;

	if(head_size < sizeof(Chunk_id)) /* at least enough for size and id */
		head_size = sizeof(Chunk_id);

	if(root_type == DONT_READ_ROOT)
	{
		pd->chunk_left = root_size - head_size;
		pd->nextoset = head_size; /* start at end of header */
	}
	else
	{
		pd->nextoset = 0; /* start at head chunk */
		pd->chunk_left = -1; /* we read root chunk */
		pd->chunk_offset = head_size; /* passed to get_next_chunk in here */
	}

	if(root_oset < 0)
	{
		if((pd->nextoset = pj_tell(fd)) < 0)
			pd->error = pd->nextoset;
	}
	else
		pd->nextoset += root_oset;


	pd->type = root_type;

}
Boolean get_next_chunk(Chunkparse_data *pd)
/************************************************************************
 *     Returns TRUE if chunk is valid FALSE if error or end of root chunk. The 
 * error code for a FALSE return can be found in cp->error.  It will be 
 * Err_no_chunk if the root chunk was not found. Success if all parsed ok and 
 * it ended at the root chunks length. or other errors from file system or data 
 * is corrupted.
 * 
 *     If init_chunkparse() has other than DONT_READ_ROOT as the root_type 
 * get_next_chunk() will attempt to read the root chunk and verify that it has 
 * the type input on the first call after the init, and will place 
 * ROOT_CHUNK_TYPE (-1) in the pd->chunk_type field.
 * 
 *     If a valid chunk is found it leaves the file position at the first byte 
 * after the Fat_chunk of the chunk and has the Fat_chunk data in pd->fchunk.  
 * pd->chunk_offset is set to the offset of the Fat_chunk in the file 
 * copy_next_chunk() read_parsed_chunk() will only copy or read the head data.
 ************************************************************************/
{
	if(pd->chunk_left == 0 || pd->error < Success)
		return(FALSE); /* done or error */

	if((pd->error = pj_readoset(pd->fd,&pd->fchunk,
						        pd->nextoset,sizeof(Fat_chunk))) < Success)
	{
		goto error;
	}

	if(pd->chunk_left < 0) /* first time through if reading root chunk */
	{
		/* If the input root size < head size chunk_left will be < 0 and
		 * DONT_READ_ROOT will be true */

		if(pd->type == DONT_READ_ROOT)
		{
			pd->error = Err_corrupted;
			goto error;
		}

		if(pd->fchunk.type != pd->type)
		{
			pd->error = Err_no_chunk;
			goto error;
		}
		pd->chunk_left = pd->fchunk.size;

		/* set size to head_size with chunk in pd->chunk_offset */

		pd->data_size = pd->chunk_offset;
		pd->type = ROOT_CHUNK_TYPE; /* special type for root (-1) */
	}
	else
	{
		pd->data_size = pd->fchunk.size;
		pd->type = pd->fchunk.type;
	}

	if((pd->chunk_left -= pd->data_size) < 0)
	{
		pd->error = Err_corrupted;
		goto error;
	}

	pd->chunk_offset = pd->nextoset; /* current chunk offset */
	pd->nextoset += pd->data_size;  /* on to next by size of current */
	pd->data_size -= sizeof(Fat_chunk); 

	/* set pd->data_size to actual data size (may be negative) 
	 * if a Chunk_id only */

	return(TRUE);
error:
	return(FALSE);
}
