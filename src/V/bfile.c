
/* bfile.c - Jim Kent's very owned buffered file i/o.  Much like
   C fopen(), fclose(), etc in result.  Maybe faster, but certainly
   works better with my memory management routines.

   Some quirks:
   		bopen(char *name, Bfile *bf) 
			this always opens a file to read.  You pass it the Bfile
			cause, I donno... seems cleaner.
		bcreate(char *name, Bfile *bf)
			Creates a file to write. 
*/

#include "jimk.h"

/* flush and close a buffered file.  Free up memory used by buffer. */
bclose(bf)
Bfile *bf;
{
int ok = TRUE;

if (bf->writable)
	{
	ok = bflush(bf);
	}
gentle_close(bf->fd);
gentle_freemem(bf->buf);
zero_structure(bf, sizeof(*bf) );
return(ok);
}

static
bgetbuf(bf)
Bfile *bf;
{
zero_structure(bf, sizeof(*bf) );
if ((bf->buf = askmem(BSIZE) ) == NULL)
	return(0);
return(1);
}

/* Open up a buffered file to read */
bopen(name, bf)
char *name;
Bfile *bf;
{
if (!bgetbuf(bf))
	return(0);
if ((bf->fd = jopen(name, 0)) == 0)
	{
	bclose(bf);
	return(0);
	}
return(1);
}

/* Create a buffered file read/write */
bcreate(name, bf)
char *name;
Bfile *bf;
{
if (!bgetbuf(bf))
	return(0);
if ((bf->fd = jcreate(name)) == 0)
	{
	bclose(bf);
	return(0);
	}
bf->writable = 1;
bf->left = BSIZE;
bf->filept = bf->buf;
return(1);
}

/* Do any pending writes to buffered file.  Force next read to be
   straight from disk */
bflush(bf)
register Bfile *bf;
{
long size, wsize;

if (bf->writable)
	{
	size = BSIZE-bf->left;
	if (size > 0)
		{
		wsize = jwrite(bf->fd, bf->buf, size);
		bf->filept = bf->buf;
		bf->left = BSIZE;
		if (wsize < size)
			return(0);
		}
	}
else
	{
	bf->left = 0;
	}
return(1);
}

/* Seek to character position.  Mode parameter same as MS-DOS.  IE 
	mode = 0  for seek from beginning */
bseek(bf, offset, mode)
Bfile *bf;
long offset;
int mode;
{
bflush(bf);
jseek(bf->fd,offset,mode);
}

/* Write out a single byte to buffered file */
bputbyte(bf,c)
register Bfile *bf;
unsigned char c;
{
*bf->filept++ = c;
if (--bf->left <= 0)
	{
	if (!bflush(bf))
		return(-1);
	}
return(1);
}


/* Write out a block (32K or less) to buffered file */
bwrite(bf, buf, count)
register Bfile *bf;
UBYTE *buf;
int count;
{
if (count <= 0)
	return(0);
#ifdef WORKSOKBUTBIG
if (bf->left >= count)
	{
	copy_bytes(buf, bf->filept, count);
	bf->filept += count;
	bf->left -= count;
	return(count);
	}
else
#endif WORKSOKBUTBIG
	{
	register int i;

	for (i=0; i<count; i++)
		{
		if (bputbyte(bf, *buf++) < 0)
			return(i);
		}
	return(count);
	}
}


/* Read next byte of buffered file */
bgetbyte(bf)
register Bfile *bf;
{
if (--bf->left < 0)
	{
	bf->left = jread(bf->fd, bf->buf, BSIZE);
	if (bf->left <= 0)
		return(-1);
	--(bf->left);
	bf->filept = bf->buf;
	}
return(*bf->filept++);
}

/* Read a block (less than 32K) from buffered file */
bread(bf, buf, count)
register Bfile *bf;
UBYTE *buf;
int count;
{
int c,i;

if (count <= 0)
	return(0);
#ifdef LATER
if (bf->left >= count)
	{
	copy_bytes(bf->filept, buf, count);
	bf->filept += count;
	bf->left -= count;
	return(count);
	}
else
#endif LATER
	{
	for (i=0; i<count; i++)
		{
		if ((c = bgetbyte(bf)) < 0)
			return(i);
		*buf++ = c;
		}
	return(count);
	}
}

