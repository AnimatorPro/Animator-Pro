#include "lfile.ih"

static UBYTE stdoutb,stderrb;
LFILE _lstdout = 
	{
	&stdoutb, &stdoutb, &stdoutb+1,
	1,			/* bsize */
	NULL,		/* lfile */
	0,			/* fpos */
	Success,	/* ferr */
	BFL_TEXT|BFL_WRITE,	/* flags */
	FALSE,		/* is_dirty */
	FALSE,		/* can_buf_seek */
	};
LFILE _lstderr = 
	{
	&stderrb, &stderrb, &stderrb+1,
	1,			/* bsize */
	NULL,		/* lfile */
	0,			/* fpos */
	Success,	/* ferr */
	BFL_TEXT|BFL_WRITE,	/* flags */
	FALSE,		/* is_dirty */
	FALSE,		/* can_buf_seek */
	};
init_stdfiles()
{
extern void *get_jstderr(), *get_jstdout();
	_lstderr.lfile = get_jstderr();
	_lstdout.lfile = get_jstdout();
	return;
}
cleanup_lfiles()
{
	lfflush(lstdout);
	lfflush(lstderr);
}
