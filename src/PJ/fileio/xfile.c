/* xfile.c
 *
 * XFILE routines (xfopen, etc) simply redirect to stdio equivalents.
 *
 * XFFILE routines (xffopen, etc.) return an error code and are
 * generally safer.
 */

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

/*--------------------------------------------------------------*/

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

/*--------------------------------------------------------------*/
/* XFILE routines.                                              */
/*--------------------------------------------------------------*/

XFILE *
xfopen(const char *path, enum XReadWriteMode mode)
{
	XFILE *xf;
	const char *str;

	switch (mode) {
		case XREADONLY: str = "rb"; break;
		case XWRITEONLY: str = "wb"; break;
		case XREADONLY_TEXT: str = "r"; break;
		case XWRITEONLY_TEXT: str = "w"; break;
		case XREADWRITE_OPEN: str = "rb+"; break;
		case XREADWRITE_CLOBBER: str = "wb+"; break;
		default: return NULL;
	}

	xf = pj_malloc(sizeof(*xf));
	if (!xf)
		return NULL;

	if (is_tdrive(path)) {
		path += 2;
	}

	xf->rf = real_fopen(path, str);
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
	pj_free(xf); /* TODO: really free it? */

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
/* XFFILE routines.                                             */
/*--------------------------------------------------------------*/

Errcode
xffopen(const char *path, XFILE **pxf, enum XReadWriteMode mode)
{
	if (!pj_assert(path != NULL)) return Err_bad_input;
	if (!pj_assert(pxf != NULL)) return Err_bad_input;
	if (!pj_assert(XREADONLY <= mode && mode <= XREADWRITE_CLOBBER)) return Err_bad_input;

	*pxf = xfopen(path, mode);
	if (*pxf == NULL)
		return xffile_error();
	return Success;
}

Errcode
xffclose(XFILE **pfp)
{
	int ret;

	if (!pj_assert(pfp != NULL)) return Err_bad_input;
	if (!pj_assert((*pfp) != NULL)) return Err_file_not_open;
	if (!pj_assert((*pfp)->rf != NULL)) return Err_file_not_open;

	ret = xfclose(*pfp);
	*pfp = NULL; /* TODO: really free it? */

	if (ret != 0)
		return xffile_error();
	return Success;
}

Errcode
xffread(XFILE *xf, void *buf, size_t size)
{
	if (!pj_assert(xf != NULL)) return Err_bad_input;
	if (!pj_assert(buf != NULL)) return Err_bad_input;
	if (!pj_assert(xf->rf != NULL)) return Err_file_not_open;

	if (real_fread(buf, 1, size, xf->rf) != size)
		return xffile_error();
	return Success;
}

Errcode
xffwrite(XFILE *xf, void *buf, size_t size)
{
	if (!pj_assert(xf != NULL)) return Err_bad_input;
	if (!pj_assert(buf != NULL)) return Err_bad_input;
	if (!pj_assert(xf->rf != NULL)) return Err_file_not_open;

	if (real_fwrite(buf, 1, size, xf->rf) != size)
		return xffile_error();
	return Success;
}

Errcode
xffreadoset(XFILE *xf, void *buf, long offset, size_t size)
{
	if (!pj_assert(xf != NULL)) return Err_bad_input;
	if (!pj_assert(buf != NULL)) return Err_bad_input;
	if (!pj_assert(xf->rf != NULL)) return Err_file_not_open;

	if (xfseek(xf, offset, XSEEK_SET) != 0)
		return xffile_error();
	return xffread(xf, buf, size);
}

Errcode
xffwriteoset(XFILE *xf, void *buf, long offset, size_t size)
{
	if (!pj_assert(xf != NULL)) return Err_bad_input;
	if (!pj_assert(buf != NULL)) return Err_bad_input;
	if (!pj_assert(xf->rf != NULL)) return Err_file_not_open;

	if (xfseek(xf, offset, XSEEK_SET) != 0)
		return xffile_error();
	return xffwrite(xf, buf, size);
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
	long offset;

	if (!pj_assert(xf != NULL)) return Err_bad_input;
	if (!pj_assert(xf->rf != NULL)) return Err_file_not_open;

	offset = real_ftell(xf->rf);
	if (offset < 0)
		return xffile_error();
	return offset;
}

long
xffseek_tell(XFILE *xf, long offset, enum XSeekWhence whence)
{
	Errcode err;

	err = xffseek(xf, offset, whence);
	if (err != Success)
		return err;
	return xfftell(xf);
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
