#include <string.h>
#define LFILE_C
#include "lfile.ih"
#include "stdtypes.h"
#include "memory.h"
#include "ptrmacro.h"
#include "formatf.h"


Errcode lerrno;

static LFILE *_lfopen(char *filename, char *mode, LFILE *f)
/*
 *	Attempt to open <filename> in the given
 *	<mode> and attach it to the stream <f>
 *  Return LFILE * on success, NULL on error.  On error 
 *  further info is stored in lerrno;
 */
{
char *p = NULL;
int i, flags;
Boolean append = FALSE,  creat = FALSE;
int jmode;
Lfile h;

flags = BFL_TEXT;		/* by default open in text mode */
while(*mode)
	{
	switch(*mode++)
		{
		case 'r':
			flags |= BFL_READ;
			break;
		case 'w':
			flags |= BFL_WRITE;
			creat = TRUE;
			break;
		case 'a':
			flags |= BFL_WRITE;
			append = TRUE;
			break;
		case '+':
			flags |= (BFL_READ | BFL_WRITE);
			break;
		case 'b':
			flags &= ~BFL_TEXT;
			break;
		case 't':
			break;	/* we're in text mode already... */
		default:
			lerrno = Err_bad_input;
			return(NULL);
		}
	}
if((i = (flags & (BFL_READ | BFL_WRITE))) == 0)
	{
	lerrno = Err_bad_input;
	return(NULL);
	}
else if(i == BFL_READ)
	{
	jmode = JREADONLY;
	}
else if(i == BFL_WRITE)
	{
	jmode = JWRITEONLY;
	}
else
	{
	jmode  =  JREADWRITE;
	}
if ((p = pj_malloc(LBUF_SIZE)) == NULL)
	{
	lerrno = Err_no_memory;
	return(NULL);
	}
if (creat)
	h = lcreate(filename, jmode);
else
	{
	if ((h = lopen(filename, jmode)) == JNONE)
		if (flags & BFL_WRITE)
			h = lcreate(filename, jmode);
	}
if (h == JNONE)
	{
	lerrno = pj_ioerr();
	pj_free(p);
	return(NULL);
	}
if (append)
	{
	if ((f->fpos = lseek(h, 0L, JSEEK_END)) < Success)
		{
		lerrno = f->fpos;
		lclose(f->lfile);
		pj_free(p);
		return(NULL);
		}
	}
/* seeking will be slower on write or text files */
if ((flags&BFL_TEXT) || (flags&BFL_WRITE))
	f->can_buf_seek = FALSE;
else
	f->can_buf_seek =  TRUE;
f->pt = f->end = f->start = p;
f->bsize = LBUF_SIZE;
f->flags = flags;
f->lfile = h;
return(f);
}

LFILE *lfopen(char *name, char *mode)
{
LFILE *f;

if ((f = pj_zalloc(sizeof(*f))) == NULL)
	{
	lerrno =  Err_no_memory;
	return(NULL);
	}
if (_lfopen(name, mode, f) == NULL)
	{
	pj_free(f);
	return(NULL);
	}
return(f);
}

static long untext(LFILE *f, char *buf, long count)
/* Translate text from ms-dos to unix representation.  If encounter
   control-z make file appear at end-of-file.  Filter out '\r' chars. */
{
#define CONT_Z 0x1a
char *out;
char *in;
char c;

	out = in = buf;

	while (--count >= 0)
	{
		switch(c = *in++)
		{
			case CONT_Z:
				f->flags |= BFL_CONTZ;
				goto OUT;
			case '\r':
				break;
			default:
				*out++ = c;
				break;
		}
	}
OUT:
	return(out - buf);
#undef CONT_Z
}

static void retext(char *in, char *out, long count)
/* move count bytes of in to out, expanding <lf>'s in in to
   <cr><lf>'s in out */
{
char c;

	while (--count >= 0)
	{
		if ((c = *in++) == '\n')
			*out++ = '\r';
		*out++ = c;
	}
}

static long _lf_uread(LFILE *f, char *buf, long count)
/* Read in count.   If in text mode do <cr/lf>  translation.
 * Beware that return may be less than count even before end of
 * file because of stripped <cr>'s */
{
long ur1;

	if (f->flags&BFL_TEXT)
	{
		if(f->flags&BFL_CONTZ)	/* If have gotten a control Z previously stop */
			return(0);
		ur1 = lread(f->lfile, buf, count);
		if (ur1 <= 0)
			return(ur1);
		return(untext(f, buf, ur1));
	}
	else	/* untranslated case */
	{
		return(lread(f->lfile, buf, count));
	}
}

static long count_lf(char *buf, long count)
/* cont the number of <lf>'s in buf */
{
long ccount = 0;

while (--count >= 0)
	if (*buf++ == '\n')
		++ccount;
return(ccount);
}

static long _lf_uwrite(LFILE *f, char *buf, long count)
/* write out count.   If in text mode do <cr/lf>  translation */
{
char cr = '\r';
long i;
Lfile lf = f->lfile;
char *trbuf;
long trcount, wcount, cct;

if (f->flags&BFL_TEXT)
	{
	trcount = count + (cct = count_lf(buf, count));
	if ((trbuf = pj_malloc(trcount)) != NULL)
		{
		retext(buf, trbuf, count);
		if ((wcount = lwrite(lf, trbuf, trcount)) < trcount)
			{
			pj_free(trbuf);
			return(wcount-cct);
			}
		pj_free(trbuf);
		return(count);
		}
	else
		{
		for (i=0; i<count; i++)	 /* translated writes the  _slow_  way */
			{
			switch (*buf)
				{
				case '\n':
					if (lwrite(lf, &cr, 1L) < 1)
						goto OUT;
				default:
					if (lwrite(lf, buf, 1L) < 1)
						goto OUT;
				}
			buf += 1;
			}
	OUT:
		return(i);
		}
	}
else	/* untranslated case */
	{
	return(lwrite(lf, buf, count));
	}
}


Errcode _lf_wflush(LFILE *f)
/* do pending writes. */
{
long size;
long wrsize;
Errcode err = Success;

if (f->is_dirty)
	{
	size = f->pt -  f->start;
	if ((wrsize = _lf_uwrite(f, f->start, size)) < size)
		{
		err = f->ferr = lerrno = pj_ioerr();
		f->flags |= BFL_ERR;
		}
	f->fpos += wrsize;
	f->is_dirty = FALSE;
	}
f->pt = f->start;
f->end = f->start + f->bsize;
return(err);
}

Errcode _lf_rflush(LFILE *f)
{
long bsize;
long rsize;
Errcode err = Success;

if (f->flags & (BFL_EOF|BFL_ERR))
	return(f->ferr);
if (f->is_dirty)
	_lf_wflush(f);
bsize = f->bsize;
if ((rsize = _lf_uread(f, f->start, bsize)) <= 0)
	{
	f->flags |= BFL_EOF;
	if (rsize == 0)		/* at end of FILE */
		return(f->ferr = Err_eof);
	else
		return(f->ferr = rsize);
	}
f->fpos += rsize;
f->pt = f->start;
f->end = f->start + rsize;
return(err);
}

Errcode lfclose(LFILE *f)
{
Errcode err = Success;

if (f == NULL)
	return(lerrno = Err_null_ref);
if (f->is_dirty)
	err = _lf_wflush(f);
lclose(f->lfile);
pj_free(f->start);
memset(f,0,sizeof(*f));		/* to discourage people from still using this... */
pj_free(f);
return(err);
}

