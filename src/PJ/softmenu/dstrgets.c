#include "dstring.h"
#include "errcodes.h"
#include "xfile.h"

#ifdef SLOW
Errcode dstring_get_line(Dstring *ds, XFILE *f)
{
int c;
Errcode err;

ds->blen = 0;
if ((c = xfgetc(f)) < Success)
	return(Err_eof);
for (;;)
	{
	if ((err = dstring_addc(ds,c)) < Success)
		return(err);
	if (c == '\n')
		break;
	if ((c = xfgetc(f)) < Success)
		break;
	}
return(dstring_addc(ds,0));
}
#endif /* SLOW */

Errcode dstring_get_line(Dstring *ds, XFILE *f)
{
int c;
Errcode err;
int blen = 0;
int bmax = ds->bmax;
char *buf = ds->buf;

ds->blen = 0;
if ((c = xfgetc(f)) < Success)
	return(Err_eof);
for (;;)
	{
	if (blen >= bmax)
		{
		ds->blen = blen;
		if ((err = dstring_newbuf(ds,bmax<<=1,TRUE)) < Success)
			return(err);
		buf = ds->buf;
		}
	buf[blen++] = c;
	if (c == '\n')
		break;
	if ((c = xfgetc(f)) < Success)
		break;
	}
if (blen >= bmax)
	{
	ds->blen = blen;
	if ((err = dstring_newbuf(ds,bmax<<=1,TRUE)) < Success)
		return(err);
	buf = ds->buf;
	}
buf[blen++] = 0;
ds->blen = blen;
return(Success);
}
