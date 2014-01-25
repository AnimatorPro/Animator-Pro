#include "dstring.h"
#include "errcodes.h"
#include "memory.h"
#include "util.h"

void dstring_init(Dstring *ds)
{
ds->is_dynamic = FALSE;
ds->buf = ds->sbuf;
ds->sbuf[0] = 0;
ds->bmax = sizeof(ds->sbuf);
ds->blen = 0;
}

void dstring_cleanup(Dstring *ds)
{
if (ds->is_dynamic)
	pj_freez(&ds->buf);
}

Errcode dstring_newbuf(Dstring *ds,int newsize, Boolean copy_old)
/* Allocate dynamic string buffer for Stok. Re-allocs even if size 
 * desired is equal to current buffer size */
{
char *newbuf;
int isd;

if ((isd = (newsize > sizeof(ds->sbuf))) != FALSE)
	{
	if ((newbuf = pj_malloc(newsize)) == NULL)
		return(Err_no_memory);
	}
else
	{
	newbuf = ds->sbuf;
	newsize = sizeof(ds->sbuf);
	}
if (copy_old)
	memcpy(newbuf,ds->buf,ds->blen);
dstring_cleanup(ds);
ds->is_dynamic = isd;
ds->buf = newbuf;
ds->bmax = newsize;
return(Success);
}
Errcode dstring_get_clone(Dstring *ds,char **ptext)
/* gets a clone of a dstring possibly at the expense of the dstring by
 * simply assigning it's dynamic buffer to the *ptext and re-initializing 
 * if failure *ptext will be set to NULL */
{
	if(ds->is_dynamic && ds->blen == ds->bmax)
	{
		*ptext = ds->buf;
		dstring_init(ds);
	}
	else
	{
		if((*ptext = pj_malloc(ds->blen)) == NULL)
			return(Err_no_memory);
		memcpy(*ptext,ds->buf,ds->blen);
	}
	return(Success);
}
#ifdef SLUFFED
Errcode dstring_addc(Dstring *ds, char c)
/* Append a character to Dstring */
{
if (ds->blen >= ds->bmax)
	{
	Errcode err;
	if ((err = dstring_newbuf(ds,ds->bmax+1,TRUE)) < Success)
		return(err);
	}
ds->buf[ds->blen++] = c;
return(Success);
}
#endif /* SLUFFED */

Errcode dstring_memcpy(Dstring *ds, char *s, int len)
/* save a memory buffer into Dstring */
{
if (ds->bmax < len)
	{
	Errcode err;
	if ((err = dstring_newbuf(ds,len,FALSE)) < Success)
		return(err);
	}
memcpy(ds->buf,s,len);
ds->blen = len;
return(Success);
}

#ifdef SLUFFED
Errcode dstring_strcpy(Dstring *ds, char *s)
/* save a string into Dstring */
{
return(dstring_memcpy(ds,s,strlen(s)+1));
}
#endif /* SLUFFED */

Errcode dstring_memcat(Dstring *ds, Dstring *ss)
{
int len = ds->blen + ss->blen;

if (ds->bmax < len)
	{
	Errcode err;
	if ((err = dstring_newbuf(ds,len,TRUE)) < Success)
		return(err);
	}
memcpy(ds->buf+ds->blen,ss->buf,ss->blen);
ds->blen = len;
return(Success);
}

Errcode dstring_strcat(Dstring *ds, Dstring *ss)
{
if (ds->blen > 0)		/* get rid of terminating 0 if any */
	{
	if (ds->buf[ds->blen-1] == 0)
		ds->blen -= 1;
	}
return(dstring_memcat(ds,ss));
}

