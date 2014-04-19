/* xfile.c */

#include <assert.h>
#include <stdarg.h>
#include "pjassert.h"
#include "errcodes.h"
#include "jfile.h"
#include "memory.h"
#include "tfile.h"
#include "xfile.h"

#ifdef USE_LFILE
	#include "lstdio.h"

	typedef LFILE REAL_FILE;
	extern void init_lstdfiles(void);

	#define real_stdout lstdout
	#define real_stderr lstderr

	#define real_fopen lfopen
	#define real_fclose lfclose
	#define real_fgetc lfgetc
	#define real_fputc lfputc
	#define real_ungetc lungetc
	#define real_fflush lfflush
	#define real_fread lfread
	#define real_fwrite lfwrite
	#define real_fseek lfseek
	#define real_rewind lrewind
	#define real_ftell lftell
	#define real_vfprintf lvfprintf
	#define real_fgets lfgets
	#define real_fputs lfputs
	#define real_ferror lferror
	#define real_errno lerrno
#else
	#include <stdio.h>
	#include <errno.h>

	typedef FILE REAL_FILE;

	#define real_stdout stdout
	#define real_stderr stderr

	#define real_fopen fopen
	#define real_fclose fclose
	#define real_fgetc fgetc
	#define real_fputc fputc
	#define real_ungetc ungetc
	#define real_fflush fflush
	#define real_fread fread
	#define real_fwrite fwrite
	#define real_fseek fseek
	#define real_rewind rewind
	#define real_ftell ftell
	#define real_vfprintf vfprintf
	#define real_fgets fgets
	#define real_fputs fputs
	#define real_ferror ferror
	#define real_errno errno
#endif /* USE_LFILE */

struct xfl
{
	REAL_FILE *rf;
};

static XFILE _xstdout;
static XFILE _xstderr;
XFILE *xstdout = &_xstdout;
XFILE *xstderr = &_xstderr;

void
init_stdfiles(void)
{
#ifdef USE_LFILE
	init_lstdfiles();
#endif

	_xstdout.rf = real_stdout;
	_xstderr.rf = real_stderr;
}

void
cleanup_lfiles(void)
{
	xfflush(xstdout);
	xfflush(xstderr);
}

XFILE *
xfopen(const char *path, const char *mode)
{
	XFILE *xf;

	xf = pj_malloc(sizeof(*xf));
	if (!xf)
		return NULL;

	/* temp file system. */
	if (path[0] == TDEV_MED && path[1] == DEV_DELIM) {
		path += 2;
	}

	xf->rf = real_fopen(path, mode);
	if (!xf->rf) {
		pj_free(xf);
		return NULL;
	}
	return xf;
}

int
xfclose(XFILE *xf)
{
	int ret;
	ret = real_fclose(xf->rf);
	pj_free(xf);

	return ret;
}

int
xfgetc(XFILE *xf)
{
	return real_fgetc(xf->rf);
}

int
xfputc(int c, XFILE *xf)
{
	return real_fputc(c, xf->rf);
}

int
xungetc(int c, XFILE *xf)
{
	return real_ungetc(c, xf->rf);
}

int
xfflush(XFILE *xf)
{
	return real_fflush(xf->rf);
}

size_t
xfread(void *ptr, size_t size, size_t nmemb, XFILE *xf)
{
	return real_fread(ptr, size, nmemb, xf->rf);
}

size_t
xfwrite(const void *ptr, size_t size, size_t nmemb, XFILE *xf)
{
	return real_fwrite(ptr, size, nmemb, xf->rf);
}

int
xfseek(XFILE *xf, long offset, enum XSeekWhence whence)
{
	int real_whence;

#ifdef USE_LFILE
	switch (whence) {
		case XSEEK_SET: real_whence = LSEEK_SET; break;
		case XSEEK_CUR: real_whence = LSEEK_CUR; break;
		case XSEEK_END: real_whence = LSEEK_END; break;
		default: real_errno = Err_bad_input; return -1;
	}
#else
	switch (whence) {
		case XSEEK_SET: real_whence = SEEK_SET; break;
		case XSEEK_CUR: real_whence = SEEK_CUR; break;
		case XSEEK_END: real_whence = SEEK_END; break;
		default: real_errno = EINVAL; return -1;
	}
#endif

	return real_fseek(xf->rf, offset, real_whence);
}

void
xrewind(XFILE *xf)
{
	real_rewind(xf->rf);
}

long
xftell(XFILE *xf)
{
	return real_ftell(xf->rf);
}

int
xfprintf(XFILE *xf, const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = real_vfprintf(xf->rf, format, ap);
	va_end(ap);
	return ret;
}

int
xprintf(const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = real_vfprintf(real_stdout, format, ap);
	va_end(ap);
	return ret;
}

int
xvfprintf(XFILE *xf, const char *format, va_list ap)
{
	return real_vfprintf(xf->rf, format, ap);
}

char *
xfgets(char *s, int size, XFILE *xf)
{
	return real_fgets(s, size, xf->rf);
}

int
xfputs(const char *s, XFILE *xf)
{
	return real_fputs(s, xf->rf);
}

int
xferror(XFILE *xf)
{
	return real_ferror(xf->rf);
}

int
xerrno(void)
{
	return real_errno;
}

/*--------------------------------------------------------------*/

Errcode
xffopen(const char *path, XFILE **pfp, const char *fmode)
{
	if ((*pfp = xfopen(path, fmode)) == NULL)
		return xffile_error();
	return Success;
}

void
xffclose(XFILE **pfp)
{
	if (*pfp)
		xfclose(*pfp);
	*pfp = NULL;
}

Errcode
xffread(XFILE *xf, void *buf, size_t size)
{
	if (real_fread(buf, 1, size, xf->rf) != size)
		return xffile_error();
	return Success;
}

Errcode
xffwrite(XFILE *xf, void *buf, size_t size)
{
	if (real_fwrite(buf, 1, size, xf->rf) != size)
		return xffile_error();
	return Success;
}

Errcode
xffwriteoset(XFILE *xf, void *buf, long offset, size_t size)
{
	if (xfseek(xf, offset, XSEEK_SET) != 0)
		goto error;
	if (real_fwrite(buf, 1, size, xf->rf) != size)
		goto error;
	return Success;
error:
	return xffile_error();
}

Errcode
xffseek(XFILE *xf, long offset, enum XSeekWhence whence)
{
	if (!pj_assert(xf != NULL)) return Err_bad_input;
	if (!pj_assert(XSEEK_SET <= whence && whence <= XSEEK_END)) return Err_bad_input;
	if (!pj_assert(xf->rf != NULL)) return Err_file_not_open;

	if (xfseek(xf, offset, whence))
		return xffile_error();
	return Success;
}

long
xfftell(XFILE *xf)
{
	const long offset = real_ftell(xf->rf);

	if (offset < 0)
		return xffile_error();
	return offset;
}

Errcode
xffile_error(void)
{
#ifdef USE_LFILE
	return lerrno;
#else
	return Err_stdio;
#endif
}
