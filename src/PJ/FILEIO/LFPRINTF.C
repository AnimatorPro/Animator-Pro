#include "lfile.ih"
#include "formatf.h"

#ifdef __TURBOC__
int lfprintf(LFILE *f, char *format, ...)
/* will break on large strings.  Not production code, just testing... */
{
va_list arg;
char buf[500];
int sz;
Errcode err;

va_start(arg, format);
sz = vsprintf(buf, format, arg);
err = lfputs(buf,f);
va_end(arg);
if (err < Success)
	return(err);
return(sz);
}
#else /* __TURBOC__ */
int lfprintf(LFILE *f, char *fmt, ...)
{
va_list varg;
Formatarg fa;
char c;

start_formatarg(fa,fmt);
while ((c = fa_getc(&fa)) != 0)
	if (lputc(c,f) == LEOF)
		return(Err_write);
end_formatarg(fa);
return(fa.count - 1);
}
#endif /* __TURBOC__ */
