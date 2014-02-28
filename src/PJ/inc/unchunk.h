#ifndef UNCHUNK_H
#define UNCHUNK_H

#ifndef FLI_H
	#include "fli.h"
#endif

/***** stuff for and chunk parser functions *****/

typedef struct chunkparse_data {

	USHORT type;	 
	LONG data_size;  /* values to test after call to get_next_chunk to see
					  * what kind of chunk was retrieved.
					  * type can have the value ROOT_CHUNK_TYPE and size will
					  * be the size of the chunk data. If it is the root chunk
					  * it will be the root head data size 
					  * This is so that read_parsed_chunk() will read the
					  * head data and copy_parsed_chunk() will copy it 
					  * the actual size in the chunk will be placed in 
					  * fchunk.size this should not be altered */

	LONG chunk_offset; /* offset to current chunk_id (not data) in 
						* source file */

	Fat_chunk fchunk;   /* actual chunk data read in This can be altered
						 * after reading by get next chunk */
	Jfile fd;           /* file pointer to source file loaded by init */
	LONG chunk_left;    /* how much of the root chunk is left to go */
	LONG nextoset;         /* offset to next chunk */
	Errcode error;	    /* if get_next_chunk() fails ( < Success) 
						 * will be Err_no_record if the root chunk is not
						 * found or other error if this is set by a the 
						 * user the next call to get_next_chunk() will return
						 * FALSE and leave error unchanged */
} Chunkparse_data;

void init_chunkparse(Chunkparse_data *pd, Jfile fd, 
					 LONG root_type, LONG root_oset, 
					 ULONG head_size, LONG root_size);

#define DONT_SEEK_ROOT -1   /* argument for root offest of init_chunkparse 
							 * that asks to start parse at current file
							 * position */

#define DONT_READ_ROOT -1   /* argument for root type of init_chunkparse 
							 * which will supress reading of root chunk and
							 * will go immediately to the first sub chunk 
							 * it will read the first chunk at the current
							 * position if DONT_SEEK_ROOT */

#define ROOT_CHUNK_TYPE -1  /* chunk type set by get_next_chunk() for root
							 * chunk if not DONT_READ_ROOT in init */

/* these items will read or copy the ammount of data specified by the
 * fchunk.size field. */

Boolean get_next_chunk(Chunkparse_data *pd);
/* if maxsize < 0 no checking */
Errcode read_parsed_chunk(Chunkparse_data *pd,void *buf,LONG maxsize);
Errcode copy_parsed_chunk(Chunkparse_data *pd,Jfile dest);
Errcode update_parsed_chunk(Chunkparse_data *pd, void *buf);

#endif /* UNCHUNK_H */
