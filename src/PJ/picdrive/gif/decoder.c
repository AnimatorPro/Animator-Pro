
/* DECODE.C - An LZW decoder for GIF
 * Copyright (C) 1987, by Steven A. Bennett
 *
 * Permission is given by the author to freely redistribute and include
 * this code in any program as long as this credit is given where due.
 *
 * In accordance with the above, I want to credit Steve Wilhite who wrote
 * the code which this is heavily inspired by...
 *
 * GIF and 'Graphics Interchange Format' are trademarks (tm) of
 * Compuserve, Incorporated, an H&R Block Company.
 *
 * Release Notes: This file contains a decoder routine for GIF images
 * which is similar, structurally, to the original routine by Steve Wilhite.
 * It is, however, somewhat noticably faster in most cases.
 *
 * 12/20/90 (Ian)
 *	Speed tweaked, mainly by nuking all static variables (they are now local
 *	to the functions that use them, allowing optimization by the compiler),
 *	and by changing most all SHORT vars to int (again, allowing optimization).
 */

#include <stdio.h>
#include <string.h>
#include "errcodes.h"
#include "gif.h"
#include "memory.h"

#define gif_get_byte() getc(gif_load_file)

/* extern int gif_get_byte(void)
 *
 *	 - This external (machine specific) function is expected to return
 * either the next byte from the GIF file, or a negative number, as
 * defined in "errcodes.h"
 */

/* extern int gif_out_line(pixels, linelen, void *oline_data)
 *	   UBYTE pixels[];
 *	   int linelen;
 *
 *	 - This function takes a full line of pixels (one byte per pixel) and
 * displays them (or does whatever your program wants with them...).  It
 * should return zero, or negative if an error or some other event occurs
 * which would require aborting the decode process...  Note that the length
 * passed will almost always be equal to the line length passed to the
 * decoder function, with the sole exception occurring when an ending code
 * occurs in an odd place in the GIF file...  In any case, linelen will be
 * equal to the number of pixels passed...
 */

/* extern int bad_code_count;
 *
 * This value is the only other global required by the using program, and
 * is incremented each time an out of range code is read by the decoder.
 * When this value is non-zero after a decode, your GIF file is probably
 * corrupt in some way...
 */

static int bad_code_count;

#define MAX_CODES	4095

UBYTE  gif_byte_buff[256+3];	/* used by compress and decompress */

/* The following variables are used
 * for seperating out codes
 */
typedef struct codectl {
	int 	navail_bytes;
	int 	nbits_left;
	UBYTE	*pbytes;
	UBYTE	b1;
	} Codectl;

static long code_mask[13] = {
	 0,
	 0x0001, 0x0003,
	 0x0007, 0x000F,
	 0x001F, 0x003F,
	 0x007F, 0x00FF,
	 0x01FF, 0x03FF,
	 0x07FF, 0x0FFF
	 };


static int gif_get_block(UBYTE *buffer)
/*****************************************************************************
 * get the next data block from the image stream.
 *****************************************************************************/
{
	int count;

	if (0 >= (count = getc(gif_load_file)))
		{
		return count;
		}
	if (1 != fread(buffer, count, 1, gif_load_file))
		{
		return Err_truncated;
		}
	return count;
}

static int get_next_code(register Codectl *ctl, int curr_size)
/*****************************************************************************
 * gets the next code from the GIF file.  Returns the code, or else
 * a negative number in case of file errors...
 *****************************************************************************/
{
	unsigned long	 ret;

	ret = ctl->b1 >> (8 - ctl->nbits_left);
	while (curr_size > ctl->nbits_left)
		{
		if (ctl->navail_bytes <= 0)
			{
			ctl->pbytes = gif_byte_buff;
			if (0 >= (ctl->navail_bytes = gif_get_block(ctl->pbytes)))
				return Err_truncated;
			}
		ctl->b1 = *(ctl->pbytes);
		++ctl->pbytes;
		ret |= ctl->b1 << ctl->nbits_left;
		ctl->nbits_left += 8;
		--ctl->navail_bytes;
		}
	ctl->nbits_left -= curr_size;
	ret &= code_mask[curr_size];
	return ret;
}

static int decoder(int	  linewidth, /* Pixels per line of image */
				   UBYTE  *buf, 	 /* for gif_line_out, where the pixels go */
				   UBYTE  *stack,	 /* Stack for storing pixels backwards */
				   UBYTE  *suffix,	 /* Suffix table */
				   USHORT *prefix,	 /* Prefix linked list */
				   void   *oline_data)
/*****************************************************************************
 * This function decodes an LZW image, according to the method used
 * in the GIF spec.  Every *linewidth* "characters" (ie. pixels) decoded
 * will generate a call to gif_out_line(), which is a user specific function
 * to display a line of pixels.  The function gets it's codes from
 * get_next_code() which is responsible for reading blocks of data and
 * seperating them into the proper size codes.	Finally, gif_get_byte() is
 * the global routine to read the next byte from the GIF file.
 *
 * It is generally a good idea to have linewidth correspond to the actual
 * width of a line (as specified in the Image header) to make your own
 * code a bit simpler, but it isn't absolutely necessary.
 *
 * Returns: 0 if successful, else negative.  (See ERRS.H)
 *
 *****************************************************************************/
{
	UBYTE	*sp, *bufptr;
	int 	code, fc, oc, bufcnt;
	int 	c, size, ret;
	int 	curr_size;					   /* The current code size */
	int 	clear;						   /* Value for a clear code */
	int 	ending; 					   /* Value for a ending code */
	int 	newcodes;					   /* First available code */
	int 	top_slot;					   /* Highest code for current size */
	int 	slot;						   /* Last read code */
	Codectl cctl;

   /* Initialize for decoding a new image...
	*/
   if ((size = gif_get_byte()) < 0)
	  return(size);
   if (size < 2 || size > 9)
	  return(Err_corrupted);

   curr_size = size + 1;
   top_slot  = 1 << curr_size;
   clear	 = 1 << size;
   ending	 = clear + 1;
   slot = newcodes = ending + 1;
   memset(&cctl, 0, sizeof(cctl));

   /* Initialize in case they forgot to put in a clear code.
	* (This shouldn't happen, but we'll try and decode it anyway...)
	*/
   oc = fc = 0;

   /* Set up the stack pointer and decode buffer pointer
	*/
   sp = stack;
   bufptr = buf;
   bufcnt = linewidth;

   /* This is the main loop.  For each code we get we pass through the
	* linked list of prefix codes, pushing the corresponding "character" for
	* each code onto the stack.  When the list reaches a single "character"
	* we push that on the stack too, and then start unstacking each
	* character for output in the correct order.  Special handling is
	* included for the clear code, and the whole thing ends when we get
	* an ending code.
	*/
   while ((c = get_next_code(&cctl,curr_size)) != ending)
	  {

	  /* If we had a file error, return without completing the decode
	   */
	  if (c < 0)
		 {
		 return(c);
		 }

	  /* If the code is a clear code, reinitialize all necessary items.
	   */
	  if (c == clear)
		 {
		 curr_size = size + 1;
		 slot = newcodes;
		 top_slot = 1 << curr_size;

		 /* Continue reading codes until we get a non-clear code
		  * (Another unlikely, but possible case...)
		  */
		 while ((c = get_next_code(&cctl,curr_size)) == clear)
			;

		 /* If we get an ending code immediately after a clear code
		  * (Yet another unlikely case), then break out of the loop.
		  */
		 if (c == ending)
			break;

		 /* Finally, if the code is beyond the range of already set codes,
		  * (This one had better NOT happen...	I have no idea what will
		  * result from this, but I doubt it will look good...) then set it
		  * to color zero.
		  */
		 if (c >= slot)
			c = 0;

		 oc = fc = c;

		 /* And let us not forget to put the char into the buffer... And
		  * if, on the off chance, we were exactly one pixel from the end
		  * of the line, we have to send the buffer to the gif_out_line(void)
		  * routine...
		  */
		 *bufptr++ = c;
		 if (--bufcnt == 0)
			{
			if ((ret = gif_out_line(buf, linewidth, oline_data)) < 0)
			   {
			   return(ret);
			   }
			bufptr = buf;
			bufcnt = linewidth;
			}
		 }
	  else
		 {

		 /* In this case, it's not a clear code or an ending code, so
		  * it must be a code code...  So we can now decode the code into
		  * a stack of character codes. (Clear as mud, right?)
		  */
		 code = c;

		 /* Here we go again with one of those off chances...  If, on the
		  * off chance, the code we got is beyond the range of those already
		  * set up (Another thing which had better NOT happen...) we trick
		  * the decoder into thinking it actually got the last code read.
		  * (Hmmn... I'm not sure why this works...  But it does...)
		  */
		 if (code >= slot)
			{
			if (code > slot)
			   ++bad_code_count;
			code = oc;
			*sp++ = fc;
			}

		 /* Here we scan back along the linked list of prefixes, pushing
		  * helpless characters (ie. suffixes) onto the stack as we do so.
		  */
		 while (code >= newcodes)
			{
			*sp++ = suffix[code];
			code = prefix[code];
			}

		 /* Push the last character on the stack, and set up the new
		  * prefix and suffix, and if the required slot number is greater
		  * than that allowed by the current bit size, increase the bit
		  * size.  (NOTE - If we are all full, we *don't* save the new
		  * suffix and prefix...  I'm not certain if this is correct...
		  * it might be more proper to overwrite the last code...
		  */
		 *sp++ = code;
		 if (slot < top_slot)
			{
			suffix[slot] = fc = code;
			prefix[slot++] = oc;
			oc = c;
			}
		 if (slot >= top_slot)
			if (curr_size < 12)
			   {
			   top_slot <<= 1;
			   ++curr_size;
			   }

		 /* Now that we've pushed the decoded string (in reverse order)
		  * onto the stack, lets pop it off and put it into our decode
		  * buffer...  And when the decode buffer is full, write another
		  * line...
		  */
		 while (sp > stack)
			{
			*bufptr++ = *(--sp);
			if (--bufcnt == 0)
			   {
			   if ((ret = gif_out_line(buf, linewidth,oline_data)) < 0)
				  {
				  return(ret);
				  }
			   bufptr = buf;
			   bufcnt = linewidth;
			   }
			}
		 }
	  }
   ret = 0;
   if (bufcnt != linewidth)
	  ret = gif_out_line(buf, (linewidth - bufcnt),oline_data);
   return(ret);
}


SHORT gif_decoder(int linewidth, void *oline_data)

/*****************************************************************************
 * basically just allocate memory for buffers and tables, and then
 * call Steve B.'s decoder
 *****************************************************************************/
{
UBYTE *buf;
UBYTE *stack = NULL;
UBYTE *suffix = NULL;
USHORT *prefix = NULL;
int   ret = Err_no_memory;

	if ((buf = (UBYTE *)pj_malloc(linewidth + 1)) == NULL)
		goto no_buff;
	if ((stack = (UBYTE *)pj_malloc(MAX_CODES+1)) == NULL)
		goto no_stack;
	if ((suffix = (UBYTE *)pj_malloc(MAX_CODES+1)) == NULL)
		goto no_suffix;
	if ((prefix = (USHORT *)pj_malloc((MAX_CODES+1)*sizeof(USHORT) )) == NULL)
		goto no_prefix;
	ret = decoder(linewidth,buf,stack,suffix,prefix,oline_data);

	pj_free(prefix);
no_prefix:
	pj_free(suffix);
no_suffix:
	pj_free(stack);
no_stack:
	pj_free(buf);
no_buff:
	return(ret);
}


