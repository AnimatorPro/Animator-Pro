/* findfree.c - Find space in the temp-file to put a frame.  Expand file
   if necessary.  Works on temp_flx - our basic indexed frame/state file. */

#include "jimk.h"
#include "errcodes.h"
#include "flx.h"
#include "linklist.h"

static int cmp_flx(void *f1, void *f2, void *data)
/* int value returned for sort_indarray() to compare two Flx */
{
	(void)data;
	return ((Flx *)f1)->foff > ((Flx *)f2)->foff ? -1 : 1;
}

#ifdef SLUFFED
LONG add_up_frames(void)
{
int i;
register Flx *cf;
LONG acc = 0;

i = flix.hdr.frame_count+1;
cf = flix.idx;
while (--i >= 0)
	{
	acc += cf->fsize;
	cf++;
	}
return(acc);
}
#endif /* SLUFFED */

LONG flx_file_hi(void) /* return last byte of flx file actually used... */
{
int i;
register Flx *cf;
LONG acc = 0;

	i = flix.hdr.frame_count+1;
	cf = flix.idx;
	while (--i >= 0)
	{
		if (cf->fsize && acc <= cf->foff + cf->fsize)
			acc = cf->foff + cf->fsize;
		cf++;
	}
	return(acc);
}

LONG ff_tflx(LONG size, int xcount, Flx *xpt)
{
Flx **sorted;
register Flx **s, *c;
int count,i;
long lastend, thisstart, gap, thissize;

	count = i = flix.hdr.frame_count+1;
	count += xcount;

	/* grab a buffer and stuff with pointers to cflx index so can sort it */
	if ((s = sorted = pj_malloc((unsigned)count*sizeof(Flx *))) == NULL)
		return(Err_no_memory);
	c = flix.idx;
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
	sort_indarray((void **)sorted, count, cmp_flx, NULL);

	/* Look for gaps in the list big enough to fit */
	s = sorted;
	i = count;
	/* This is first offset past index */
	lastend = flx_data_offset(&flix);
	while (--i >= 0)
	{
		c = *s++;
		thisstart = c->foff;
		thissize = c->fsize;
		if(thisstart != 0L && thissize != 0)
		{
			gap = thisstart - lastend;
			if (gap >= size)
				break;
			lastend = thisstart + thissize;
		}
	}
	/* if no gaps return past end of last chunk */
	pj_free(sorted);
	return(lastend);
}

#ifdef SLUFFED
LONG find_free_tflx(LONG size)
{
	return(ff_tflx(size, 0, NULL) );
}
#endif

Errcode make_flx_record(Flxfile *flx, ULONG recnum, void *newdata, LONG size,
						Boolean overwrite)

/* loads new offset and size of new record in index of flx if successful
 * leaves values unchanged if not, it will attempt to write out the new
 * record.  If this fails in a non destructive way it will leave the index
 * pointing to the old record (ie old record not overwritten) if the
 * old data has been overwritten it will leave index pointing to old (with 
 * partial new data) record area.  an Errcode is returned if fail */
{
Errcode err;
Flx oflx;
Flx *flxidx;
LONG oset, addsize;

/* find in flix a free chunk of size ... and attempts to write data in it 
 * will attempt to clean up the mess if failure */

	if(recnum >= flx->hdr.frames_in_table)
		return(Err_bad_input);

#ifdef LATER
	if(recnum > flx->hdr.frame_count) /* adding new record */
	{
		if(recnum != flx->hdr.frame_count+1)
			return(Err_bad_input);
		++flx->hdr.frame_count;
	}
#else
	if(recnum > flx->hdr.frame_count) /* no new records allowed at present */
		return(Err_bad_input);
#endif

	flxidx = flx->idx + recnum;

	if(size == 0) /* empty record */
	{
		flxidx->foff = 0;
		flxidx->fsize = 0;
		return(Success);
	}

	oflx = *flxidx;
	addsize = 0;

	if(!overwrite || size != oflx.fsize) /* not a simple so get a new one */
	{
		if(overwrite)
			flxidx->foff = flxidx->fsize = 0; /* declare this one "free" */

		oset = ff_tflx(size, 0, NULL);
		if (oset < 0) {
			err = oset;
			goto error; /* still recoverable */
		}

		/* not rewriting same area write whole new one */
		if(oset != oflx.foff)  
		{
			err = xffwriteoset(flx->xf, newdata, oset, size);
			if (err < Success)
				goto error; /* recoverable, old data not overwritten */

			oflx.foff = oset;
			oflx.fsize = size;
			goto done;
		}

		/* If bigger than old record but in same space try writing
		 * expanded area out first.
		 */
		if (size > oflx.fsize) {
			addsize = size - oflx.fsize;
			size = oflx.fsize;
			err = xffwriteoset(flx->xf, OPTR(newdata,size), oset+size, addsize);
			if (err < Success)
				goto error; /* still recoverable */
		}
	}
	else
	{
		oset = oflx.foff;
	}

	/* seek to start of old record */

	err = xffseek(flx->xf, oset, XSEEK_SET);
	if (err < Success)
		goto error; /* still recoverable we hope */

	/* write remainder of record out, NOT recoverable, so put length actually
	 * written in index */

	oflx.fsize = (long)xfwrite(newdata, 1, size, flx->xf);
	if (oflx.fsize != size) {
		err = xffile_error();
	}
	else {
		err = Success;
		oflx.fsize += addsize; /* add expanded size if any */
	}

done:
error:
	*flxidx = oflx; /* will have old data unless Non recoverable or success */
	return err;
}

