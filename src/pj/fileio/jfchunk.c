/* jfchunk.c
 *
 *  These functions are used to parse data files organized as sets of
 *  "chunks" preceded by Chunk_id fields. These chunks may have
 *  optional "header data" followed by subordinate chunks.  It is most
 *  efficient when parsing "Fat_chunks".
 */

#include "errcodes.h"
#include "fli.h"
#include "jfile.h"
#include "ptrmacro.h"
#include "unchunk.h"
#include "xfile.h"

/* Function: init_chunkparse
 *
 *  Initializes a control structure for parsing chunks subordinate to
 *  a Chunk_id containing sub chunks.
 *
 *  pd - The chunkparse data to initialize.
 *  xf - The file to read chunks from.
 *  root_type - The type expected.  If set to -1 it will not read or verify
 *              the existence of the root chunk and will assume the root
 *              chunk is size provided.
 *  root_oset - Offset to root chunk in file.   If < 0 (DONT_SEEK_ROOT)
 *              set offset to current position.
 *  head_size - Size of any header data in the root chunk before the
 *              first subordinate chunk. inclusive of root sizeof(Chunk_id).
 *              (head_size <= sizeof(Fat_chunk) means no head data).
 *  root_size - The root chunk size provided if root_type is -1.
 */
void
init_chunkparse(Chunkparse_data *pd, XFILE *xf,
		LONG root_type, LONG root_oset, ULONG head_size, LONG root_size)
{
	pd->error = Success; /* good until fail */
	pd->xf = xf;

	if (head_size < sizeof(Chunk_id)) /* at least enough for size and id */
		head_size = sizeof(Chunk_id);

	if (root_type == DONT_READ_ROOT) {
		pd->chunk_left = root_size - head_size;
		pd->nextoset = head_size; /* start at end of header */
	}
	else {
		pd->nextoset = 0; /* start at head chunk */
		pd->chunk_left = -1; /* we read root chunk */
		pd->chunk_offset = head_size; /* passed to get_next_chunk in here */
	}

	if (root_oset < 0) {
		pd->nextoset = xfftell(xf);
		if (pd->nextoset < 0)
			pd->error = pd->nextoset;
	}
	else {
		pd->nextoset += root_oset;
	}

	pd->type = root_type;
}

/* Function: get_next_chunk
 *
 *  Returns TRUE if chunk is valid FALSE if error or end of root
 *  chunk.  The error code for a FALSE return can be found in
 *  cp->error.  It will be Err_no_chunk if the root chunk was not
 *  found. Success if all parsed ok and it ended at the root chunks
 *  length. Or other errors from file system or data is corrupted.
 *
 *  If init_chunkparse() has other than DONT_READ_ROOT as the
 *  root_type get_next_chunk() will attempt to read the root chunk and
 *  verify that it has the type input on the first call after the
 *  init, and will place ROOT_CHUNK_TYPE (-1) in the pd->chunk_type
 *  field.
 *
 *  If a valid chunk is found it leaves the file position at the first
 *  byte after the Fat_chunk of the chunk and has the Fat_chunk data
 *  in pd->fchunk.  pd->chunk_offset is set to the offset of the
 *  Fat_chunk in the file copy_next_chunk() read_parsed_chunk() will
 *  only copy or read the head data.
 */
Boolean
get_next_chunk(Chunkparse_data *pd)
{
	if (pd->chunk_left == 0 || pd->error < Success)
		return FALSE; /* done or error */

	pd->error
		= xffreadoset(pd->xf, &pd->fchunk, pd->nextoset, sizeof(Fat_chunk));
	if (pd->error < Success)
		return FALSE;

	/* First time through if reading root chunk */
	if (pd->chunk_left < 0) {
		/* If the input root size < head size chunk_left will be < 0
		 * and DONT_READ_ROOT will be true.
		 */

		if (pd->type == (USHORT)DONT_READ_ROOT) {
			pd->error = Err_corrupted;
			return FALSE;
		}

		if(pd->fchunk.type != pd->type) {
			pd->error = Err_no_chunk;
			return FALSE;
		}
		pd->chunk_left = pd->fchunk.size;

		/* Set size to head_size with chunk in pd->chunk_offset. */

		pd->data_size = pd->chunk_offset;
		pd->type = ROOT_CHUNK_TYPE; /* special type for root (-1) */
	}
	else {
		pd->data_size = pd->fchunk.size;
		pd->type = pd->fchunk.type;
	}

	pd->chunk_left -= pd->data_size;
	if (pd->chunk_left < 0) {
		pd->error = Err_corrupted;
		return FALSE;
	}

	pd->chunk_offset = pd->nextoset; /* current chunk offset */
	pd->nextoset += pd->data_size; /* on to next by size of current */
	pd->data_size -= sizeof(Fat_chunk);

	/* Set pd->data_size to actual data size (may be negative)
	 * if a Chunk_id only.
	 */
	return TRUE;
}

/* Function: read_parsed_chunk
 *
 *  Reads in parsed chunk including it's leading Chunk_id.
 *  If maxsize < 0, it will read the entire chunk no matter what.
 *  If maxsize >= 0, it will only read that size in.
 *
 *  Note: you will always get at least a Fat_chunk worth of data.
 */
Errcode
read_parsed_chunk(Chunkparse_data *pd, void *buf, LONG maxsize)
{
	if (maxsize < 0 || (maxsize -= sizeof(Fat_chunk)) > pd->data_size)
		maxsize = pd->data_size;
	*(Fat_chunk *)buf = pd->fchunk;
	if (maxsize > 0)
		return xffread(pd->xf, OPTR(buf,sizeof(Fat_chunk)), maxsize);
	return Success;
}

/* Function: jwrite_chunk
 *
 *  Writes a "chunk" record to a file.
 *
 *  xf - file to write to.
 *  data - chunk data.
 *  size - size of chunk data not including chunk header.
 *  type - chunk_id type.
 */
Errcode
jwrite_chunk(XFILE *xf, void *data, LONG size, SHORT type)
{
	Errcode err;
	Chunk_id chunk;

	chunk.size = size + sizeof(chunk);
	chunk.type = type;

	err = xffwrite(xf, &chunk, sizeof(chunk));
	if (err < Success)
		return err;

	err = xffwrite(xf, data, size);
	if (err < Success)
		return err;

	return Success;
}

/* Function: copy_parsed_chunk
 *
 *  Copys to dest the parsed chunk including it's leading Chunk_id
 *  this prefers to seek. It might be better to do two writes.
 */
Errcode
copy_parsed_chunk(Chunkparse_data *pd, XFILE *dst)
{
	Errcode err;
	const size_t size = sizeof(Fat_chunk) + pd->data_size;

	if (pd->data_size <= 0) {
		return xffwrite(dst, &pd->fchunk, size);
	}
	else {
		err = xffseek(pd->xf, pd->chunk_offset, XSEEK_SET);
		if (err < Success)
			return err;

		return pj_copydata(pd->xf, dst, size);
	}
}

/* Function: update_parsed_chunk
 *
 *  Rewrites but does not alter size of parsed chunk.  Must have same
 *  type and size fields.  This depends on the version field following
 *  the size and type fields in the Fat_chunk.
 */
Errcode
update_parsed_chunk(Chunkparse_data *pd, void *buf)
{
	if (((Fat_chunk *)buf)->size != pd->fchunk.size
		|| ((Fat_chunk *)buf)->type != pd->fchunk.type) {
		return Err_bad_input;
	}

	if (pd->data_size <= 0)
		return Success;
	return xffwrite(pd->xf, OPTR(buf,sizeof(Chunk_id)), pd->data_size);
}
