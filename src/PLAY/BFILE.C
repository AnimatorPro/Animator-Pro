
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


bgetbuf(bf)
Bfile *bf;
{
zero_structure(bf, sizeof(*bf) );
if ((bf->buf = askmem(BSIZE) ) == NULL)
	return(0);
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


#ifdef REALLY

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


bseek(bf, offset, mode)
Bfile *bf;
long offset;
int mode;
{
bflush(bf);
jseek(bf->fd,offset,mode);
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


#endif REALLY