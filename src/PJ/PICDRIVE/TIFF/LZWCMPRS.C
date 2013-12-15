/* lzwcmprs.c - LZW compression code for TIFF */

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
 * AUTHOR: Steve Wilhite
 *
 * REVISION HISTORY:
 *	 08/29/88	Speed tweaked a bit by Jim Kent
 *	 08/16/90	Made into pj loadable rex code by Peter Kennard
 *	 03/14/91	Converted GIF LZW code to TIFF LZW. (Ian)
 *
 */

#include "tiff.h"

#define LARGEST_CODE	4095
#define TABLE_SIZE		(8*1024)

typedef unsigned int UINT;

static USHORT *prior_codes = NULL;
static USHORT *code_ids    = NULL;
static UBYTE  *added_chars = NULL;

static int	 output_maxlen;
static UBYTE *output_buffer;
static int	 bit_offset;

static int code_size;
static int clear_code;
static int eof_code;
static int max_code;
static int next_code;

static void gfreez(void **ptr)
/*****************************************************************************
 *
 ****************************************************************************/
{
	if (ptr != NULL && *ptr != NULL)
		{
		free(*ptr);
		*ptr = NULL;
		}
}

static void init_table()
/*****************************************************************************
 *
 *****************************************************************************/
{
	int min_code_size = 8;

	code_size  = min_code_size + 1;
	clear_code = 1 << min_code_size;
	eof_code   = clear_code + 1;
	next_code  = clear_code + 2;
	max_code   = (1 << code_size) - 1;

	memset(code_ids,0,TABLE_SIZE*sizeof(*code_ids));
}

static Errcode output_a_code(unsigned int code)
/*****************************************************************************
 * bit-pack a code into the output buffer.
 *****************************************************************************/
{
	ULONG	temp;
	UBYTE	*buf;
	int 	byte_offset;
	int 	bits_used;
	int 	bits_left;

	byte_offset = bit_offset >> 3;
	bits_used = bit_offset & 7;
	bits_left = 8 - bits_used;

	if (byte_offset >= output_maxlen)
		return Err_overflow;

	buf = &output_buffer[byte_offset];

	/*
	 * resist the temptation to 'optimize' the ">> bits_left << code_size"
	 * in the following into a single shift!  the current logic ensures that
	 * existing buffer cruft is cleaned out of the destination byte by the
	 * ">> bits_left" operation when bits_left equals 8.
	 */

	temp = (((*buf >> bits_left) << code_size) | code) << (24 - (bits_used + code_size));

	*buf++ = temp >> 16;
	*buf++ = temp >> 8;
	*buf++ = temp;

	bit_offset += code_size;
	return Success;
}

Errcode lzw_compress(UBYTE *inbuf, UBYTE *outbuf, int count)
/*****************************************************************************
 *	Compress a line of data bytes using the LZW algorithm.
 *****************************************************************************/
{
	Errcode err;
	USHORT	prefix_code;
	UINT	d;
	UINT	hx;
	UINT	suffix_char;

	output_buffer = outbuf;
	bit_offset = 0;
	init_table();
	output_a_code(clear_code);				/* must be after init_table()! */

	suffix_char = *inbuf++;
	prefix_code = suffix_char;
	while (--count)
		{
		suffix_char = *inbuf++;
		hx = prefix_code ^ suffix_char << 5;
		d = 1;
		for (;;)
			{
			if (code_ids[hx] == 0)
				{
				if (Success != (err = output_a_code(prefix_code)))
					return err;
				d = next_code;
				if (next_code < LARGEST_CODE)
					{
					prior_codes[hx] = prefix_code;
					added_chars[hx] = suffix_char;
					code_ids[hx] = next_code;
					next_code++;
					}
				if (d == LARGEST_CODE-1)
					{
					if (Success != (err = output_a_code(clear_code)))
						return err;
					init_table();
					}
				if (d == max_code)
					{
					if (code_size < 12)
						{
						code_size++;
						max_code = (1<<code_size)-1; //max_code <<= 1;
						}
					}

				prefix_code = suffix_char;
				break;
				}
			if (prior_codes[hx] == prefix_code &&
				added_chars[hx] == suffix_char)
				{
				prefix_code = code_ids[hx];
				break;
				}
			hx += d;
			d += 2;
			if (hx >= TABLE_SIZE)
				hx -= TABLE_SIZE;
			}
		}

	if (Success != (err = output_a_code(prefix_code)))
		return err;

	/*------------------------------------------------------------------------
	 * endcase city...if we just output the max_code, we need to bump up
	 * the code size before writing the eof code.
	 *----------------------------------------------------------------------*/

	if (next_code == max_code) {
		if (code_size < 12) {
			++code_size;
			max_code = (1<<code_size)-1;
		} else {
			if (Success != (err = output_a_code(clear_code)))
				return err;
			init_table();
		}
	}

	if (Success != (err = output_a_code(eof_code)))
		return err;

	return (bit_offset >> 3) + ((bit_offset & 7) ? 1 : 0);
}

void lzw_cleanup(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	gfreez(&code_ids);
	gfreez(&added_chars);
	gfreez(&prior_codes);
}

Errcode lzw_init(int bufmaxlen)
/*****************************************************************************
 *
 ****************************************************************************/
{

	output_maxlen = bufmaxlen;

	if (NULL == (prior_codes = malloc(TABLE_SIZE*sizeof(*prior_codes))))
		goto ERROR_EXIT;
	if (NULL == (code_ids = malloc(TABLE_SIZE*sizeof(*code_ids))))
		goto ERROR_EXIT;
	if (NULL == (added_chars = malloc(TABLE_SIZE)))
		goto ERROR_EXIT;

	return Success;

ERROR_EXIT:

	lzw_cleanup();
	return Err_no_memory;
}

