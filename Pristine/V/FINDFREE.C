
/* findfree.c - Find space in the temp-file to put a frame.  Expand file
   if necessary.  Works on temp_flx - our basic indexed frame/state file. */
#include "jimk.h"
#include "fli.h"

extern long frame1_foff();

/* int value returned for sort_array() to compare two Flx */
static
cmp_flx(f1,f2)
Flx *f1, *f2;
{
return(f1->foff>f2->foff ? -1 : 1);
}

long
add_up_frames()
{
int i;
register Flx *cf;
long acc = 0;

i = fhead.frame_count+1;
cf = cur_flx;
while (--i >= 0)
	{
	acc += cf->fsize;
	cf++;
	}
return(acc);
}

long
flx_file_hi()	/* return last byte of flx file actually used... */
{
int i;
register Flx *cf;
long acc = 0;

i = fhead.frame_count+1;
cf = cur_flx;
while (--i >= 0)
	{
	if (acc <= cf->foff + cf->fsize)
		acc = cf->foff + cf->fsize;
	cf++;
	}
return(acc);
}


/* find in tflx a free chunk of size ... */
long
ff_tflx(size, xcount, xpt)
long size;
int xcount;
Flx *xpt;
{
Flx **sorted;
register Flx **s, *c;
int count,i;
long csize;
long lastend, thisstart, gap;

count = i = fhead.frame_count+1;
count += xcount;

/* grab a buffer and stuff with pointers to cflx index so can sort it */
if ((s = sorted = begmem((unsigned)count*sizeof(Flx *))) == NULL)
	return(0);
c = cur_flx;
while (--i>=0)
	{
	*s++ = c++;
	}
i = xcount;
c = xpt;
while (--i >= 0)
	{
	*s++ = c++;
	}
sort_array(sorted, count, cmp_flx);


/* Look for gaps in the list big enough to fit */
s = sorted;
i = count;
/* This is first offset past index */
lastend = frame1_foff();
while (--i >= 0)
	{
	c = *s++;
	thisstart = c->foff;
	if (thisstart != 0L)
		{
		gap = thisstart - lastend;
		if (gap >= size)
			{
			break;
			}
		lastend = thisstart + c->fsize;
		}
	}
/* if no gaps return past end of last chunk */
freemem(sorted);
return(lastend);
}

long
find_free_tflx(size)
long size;
{
return(ff_tflx(size, 0, NULL) );
}
