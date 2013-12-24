/* comprs.c - LZW compression code for GIF */

/*----------------------------------------------------------------------*/
/* Copyright (c) 1987													*/
/* by CompuServe Inc., Columbus, Ohio.	All Rights Reserved 			*/
/*----------------------------------------------------------------------*/

/*
 * ABSTRACT:
 *	The compression algorithm builds a string translation table that maps
 *	substrings from the input string into fixed-length codes.  These codes
 *	are used by the expansion algorithm to rebuild the compressor's table
 *	and reconstruct the original data stream.  In it's simplest form, the
 *	algorithm can be stated as:
 *
 *		"if <w>k is in the table, then <w> is in the table"
 *
 *	<w> is a code which represents a string in the table.  When a new
 *	character k is read in, the table is searched for <w>k.  If this
 *	combination is found, <w> is set to the code for that combination
 *	and the next character is read in.	Otherwise, this combination is
 *	added to the table, the code <w> is written to the output stream and
 *	<w> is set to k.
 *
 *	The expansion algorithm builds an identical table by parsing each
 *	received code into a prefix string and suffix character.  The suffix
 *	character is pushed onto the stack and the prefix string translated
 *	again until it is a single character.  This completes the expansion.
 *	The expanded code is then output by popping the stack and a new entry
 *	is made in the table.
 *
 *	The algorithm used here has one additional feature.  The output codes
 *	are variable length.  They start at a specified number of bits.  Once
 *	the number of codes exceeds the current code size, the number of bits
 *	in the code is incremented.  When the table is completely full, a
 *	clear code is transmitted for the expander and the table is reset.
 *	This program uses a maximum code size of 12 bits for a total of 4096
 *	codes.
 *
 *	The expander realizes that the code size is changing when it's table
 *	size reaches the maximum for the current code size.  At this point,
 *	the code size in increased.  Remember that the expander's table is
 *	identical to the compressor's table at any point in the original data
 *	stream.
 *
 *	The compressed data stream is structured as follows:
 *		first byte denoting the minimum code size
 *		one or more counted byte strings. The first byte contains the
 *		length of the string. A null string denotes "end of data"
 *
 *	This format permits a compressed data stream to be embedded within a
 *	non-compressed context.
 *
 * AUTHOR: Steve Wilhite
 *
 * REVISION HISTORY:
 *	 Speed tweaked a bit by Jim Kent 8/29/88
 *   Made into pj loadable rex code by Peter Kennard 8/16/90
 *   Converted from closed to open hashing in hopes of fixing an
 *   elusive bug.  This sped it up 2x.  Jim Kent 6/26/92
 *
 */

#include <stdio.h>
#include "errcodes.h"
#include "gif.h"

#define LARGEST_CODE	4095
#define TABLE_SIZE	(8*1024)

extern UBYTE gif_byte_buff[256+3];				 /* Current block */
extern FILE *gif_save_file;

extern int	 gif_get_pixel();


typedef struct lzchain
	{
	struct lzchain *next;
	short prior_code;
	short added_char;
	short code_id;
	} Lzchain;
Lzchain **hash_table;
Lzchain *chain_buf;			/* This holds preallocated lzchains. */
Lzchain *free_chain;		/* Next free lzchain. */

static int code_size;
static int clear_code;
static int eof_code;
static int bit_offset;
static int max_code;
static int free_code;

static Errcode err_status;

extern void *malloc(int size);
extern void free(void *mem);

static void init_table(int min_code_size)
/*****************************************************************************
 *
 *****************************************************************************/
{
	code_size = min_code_size + 1;
	clear_code = 1 << min_code_size;
	eof_code = clear_code + 1;
	free_code = clear_code + 2;
	max_code = 1 << code_size;

	free_chain = chain_buf;
	memset(hash_table,0,TABLE_SIZE*sizeof(*hash_table));
}

static Boolean flush(int n)
/*****************************************************************************
 * 0 if ok 1 if error in static global err_status
 *****************************************************************************/
{
	putc(n,gif_save_file);
	if(fwrite(gif_byte_buff, 1, n, gif_save_file) < n)
	{
		err_status = pj_errno_errcode();
		return(1);
	}
	return(0);
}

static Boolean write_code(int code)
/*****************************************************************************
 * returns 0 if ok 1 if error and error code is set in
 * static local variable err_status
 *****************************************************************************/
{
long temp;
int byte_offset;
int bits_left;

byte_offset = bit_offset >> 3;
bits_left = bit_offset & 7;
if (bits_left > 0)
	{
	temp = ((long) code << bits_left) | gif_byte_buff[byte_offset];
	gif_byte_buff[byte_offset] = temp;
	gif_byte_buff[byte_offset + 1] = temp >> 8;
	gif_byte_buff[byte_offset + 2] = temp >> 16;
	}
else
	{
	/* watcom 386 generates a bug unless these two lines are reversed
	   like this... */
	gif_byte_buff[byte_offset + 1] = code >> 8;
	gif_byte_buff[byte_offset] = code;
	}
bit_offset += code_size;
if (bit_offset >= 255*8)
	{
	if(flush(255))
		return(1);
	memcpy(gif_byte_buff, gif_byte_buff+255, 3);
	bit_offset -= 255*8;
	}
return(0);
}


static int compress_data(int min_code_size, long gif_wcount)
/*****************************************************************************
 *	Compress a stream of data bytes using the LZW algorithm.
 *
 * Inputs:
 *	min_code_size
 *		the field size of an input value.  Should be in the range from
 *		1 to 9.
 *
 * Returns:
 *	 0	normal completion
 *****************************************************************************/
{
short prefix_code;
int   d;
register int hx;
register int suffix_char;
Lzchain *chain;
Lzchain **pchain;

	if (min_code_size < 2 || min_code_size > 9)
	{
		if (min_code_size == 1)
			min_code_size = 2;
		else
			return Err_truncated;
	}
	bit_offset = 0;
	init_table(min_code_size);
	if(write_code(clear_code))
		goto bad_status;
	prefix_code = gif_get_pixel();
	gif_wcount -= 1;
	while (--gif_wcount >= 0)
	{
		suffix_char = gif_get_pixel();
		hx = prefix_code ^ suffix_char << 5;
		pchain = &hash_table[hx];
		chain = *pchain;
		for (;;)
		{
			if (chain == NULL)
			{
				if(write_code(prefix_code))
					goto bad_status;
				d = free_code;
				if (free_code <= LARGEST_CODE)
				{
					chain = free_chain++;
					chain->prior_code = prefix_code;
					chain->added_char = suffix_char;
					chain->code_id = free_code;
					chain->next = *pchain;
					*pchain = chain;
					free_code++;
				}
				if (d == max_code)
				{
					if (code_size < 12)
					{
						code_size++;
						max_code <<= 1;
					}
					else
					{
						if(write_code(clear_code))
							goto bad_status;
						init_table(min_code_size);
					}
				}

				prefix_code = suffix_char;
				break;
			}
			else if (chain->prior_code == prefix_code
			&&	chain->added_char == suffix_char)
			{
				prefix_code = chain->code_id;
				break;
			}
			else
			{
				chain = chain->next;
			}
		}
	}
	if(write_code(prefix_code)
		|| write_code(eof_code))
	{
		goto bad_status;
	}

	/* Make sure the code buffer is flushed */

	if (bit_offset > 0)
	{
		if(flush((bit_offset + 7)/8))
			goto bad_status;
	}

	if(flush(0)) /* end-of-data */
		goto bad_status;
	return(Success);
bad_status:
	return(err_status);
}

int gif_compress_data(int min_code_size, long pixel_count)
/*****************************************************************************
 *
 *****************************************************************************/
{
int ret;

	ret = Err_no_memory;	/* out of memory default */

	if ((hash_table = malloc(TABLE_SIZE * sizeof(*hash_table))) == NULL)
		goto no_hash;
	if ((chain_buf = malloc((1<<12) * sizeof(*chain_buf))) == NULL)
		goto no_chain;

	ret = compress_data(min_code_size, pixel_count);

	free(chain_buf);
no_chain:
	free(hash_table);
no_hash:
	return(ret);
}
