#include "lfile.ih"

static UBYTE stdoutb,stderrb;
#ifdef __TURBOC__
LFILE _lstdout = 
	{
	&stdoutb, &stdoutb, &stdoutb+1,
	1,			/* bsize */
	1,			/* lfile */
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
	2,			/* lfile */
	0,			/* fpos */
	Success,	/* ferr */
	BFL_TEXT|BFL_WRITE,	/* flags */
	FALSE,		/* is_dirty */
	FALSE,		/* can_buf_seek */
	};
#else /* __TURBOC__ */
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
#endif /* __TURBOC__ */
init_stdfiles()
{
#ifdef __TURBOC__
	return;
#else /* __TURBOC__ */
extern void *get_jstderr(), *get_jstdout();
	_lstderr.lfile = get_jstderr();
	_lstdout.lfile = get_jstdout();
	return;
#endif /* __TURBOC__ */
}
cleanup_lfiles()
{
	lfflush(lstdout);
	lfflush(lstderr);
}
