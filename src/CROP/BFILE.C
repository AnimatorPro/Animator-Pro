/* bfile.c - routines for buffered file io.  Functionally pretty equivalent
   to standard C fopen/fread/.../fclose set of routines, but with
   slightly different parameter conventions. */

#include "jimk.h"

bclose(bf)
Bfile *bf;
{
if (bf->writable)
	bflush(bf);
gentle_close(bf->fd);
gentle_freemem(bf->buf);
zero_structure(bf, sizeof(*bf) );
}

bgetbuf(bf)
Bfile *bf;
{
zero_structure(bf, sizeof(*bf) );
if ((bf->buf = askmem(BSIZE) ) == NULL)
	return(0);
return(1);
}

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


bseek(bf, offset, mode)
Bfile *bf;
long offset;
int mode;	/* 0 for from start, 2 for from end.  Won't work for relative */
{
switch (mode)
	{
	case SEEK_START:
	case SEEK_END:
		bflush(bf);
		bf->fpos = jseek(bf->fd,offset,mode);
		break;
	case SEEK_REL:
		if (offset >= 0 && offset < bf->left)
			{
			bf->fpos += offset;
			bf->left -= offset;
			bf->filept += offset;
			}
		else
			{
			bf->fpos += offset;
			bf->left = 0;
			jseek(bf->fd, bf->fpos, SEEK_START);
			}
		break;
	}
}

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
bf->fpos++;
return(1);
}


bwrite(bf, buf, count)
register Bfile *bf;
UBYTE *buf;
int count;
{
if (count <= 0)
	return(0);
if (bf->left >= count)
	{
	copy_bytes(buf, bf->filept, count);
	bf->filept += count;
	bf->fpos += count;
	bf->left -= count;
	return(count);
	}
else
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
bf->fpos++;
return(*bf->filept++);
}

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
	bf->fpos += count;
	return(count);
	}
else
#endif /* LATER */
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

