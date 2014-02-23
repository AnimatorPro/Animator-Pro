#include "errcodes.h"
#include "lfile.ih"

Errcode lfseek(LFILE *f, long offset, int whence)
{
Errcode err = Success;
static int mode_to_mode[] =  {JSEEK_START, JSEEK_REL, JSEEK_END};
UBYTE *newp;
long boff;			/* user buffered file position */

/* convert from C file seek mode to MS-DOS seek mode */
whence = mode_to_mode[whence];
/* remove eof flag */
f->flags &= ~(BFL_EOF);
/* Deal with write files (don't be very smart about seeking in buffer) */
if (f->is_dirty)
	{
	if ((err = _lf_wflush(f)) < Success)
		return(err);
	}
else if (f->can_buf_seek)
	{
	boff = f->fpos + f->pt - f->end;
	switch (whence)
		{
		case JSEEK_REL:
				/* convert offset to absolute offset */
			offset += boff;
			whence = JSEEK_START;
				/* Fall through to absolute case */
		case JSEEK_START:
			newp = f->pt + (offset - boff);
				/* See if new file position would be inside buffer */
			if (newp >= f->start && newp <=  f->end)
				{
				f->pt = newp;
				return(Success);
				}
			break;
		}
	}
if ((f->fpos = lseek(f->lfile, offset, whence)) < Success)
	{
	f->flags |= BFL_ERR;
	return(f->ferr = lerrno = pj_ioerr());
	}
f->pt = f->end = f->start;
return(err);
}
