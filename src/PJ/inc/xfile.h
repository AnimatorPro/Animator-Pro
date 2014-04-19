#ifndef XFILE_H
#define XFILE_H

#ifndef _STDARG_H_INCLUDED
#include <stdarg.h>
#endif

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

#ifndef EOF
enum { EOF = -1 };
#else
STATIC_ASSERT(xfile, EOF == -1);
#endif

enum XReadWriteMode {
	XUNDEFINED = -1,
	XREADONLY = 0,      /* rb */
	XWRITEONLY,         /* wb */
	XREADONLY_TEXT,     /* r - can we remove this? */
	XWRITEONLY_TEXT,    /* w - can we remove this? */
	XREADWRITE_OPEN,    /* rb+ - open existing. */
	XREADWRITE_CLOBBER  /* wb+ - create or clobber. */
};

enum XSeekWhence {
	XSEEK_SET = 0,
	XSEEK_CUR = 1,
	XSEEK_END = 2
};

typedef struct xfl XFILE;

extern XFILE *xstdout;
extern XFILE *xstderr;

extern XFILE *xfopen(const char *path, enum XReadWriteMode mode);
extern int xfclose(XFILE *xf);
extern int xfgetc(XFILE *xf);
extern int xfputc(int c, XFILE *xf);
extern int xungetc(int c, XFILE *xf);
extern int xfflush(XFILE *xf);
extern size_t xfread(void *ptr, size_t size, size_t nmemb, XFILE *xf);
extern size_t xfwrite(const void *ptr, size_t size, size_t nmemb, XFILE *xf);
extern int xfseek(XFILE *xf, long offset, enum XSeekWhence whence);
extern void xrewind(XFILE *xf);
extern long xftell(XFILE *xf);
extern int xfprintf(XFILE *xf, const char *format, ...);
extern int xprintf(const char *format, ...);
extern int xvfprintf(XFILE *xf, const char *format, va_list ap);
extern char *xfgets(char *s, int size, XFILE *xf);
extern int xfputs(const char *s, XFILE *xf);
extern int xferror(XFILE *xf);
extern int xerrno(void);

extern Errcode xffopen(const char *path, XFILE **pxf, enum XReadWriteMode mode);
extern void xffclose(XFILE **pfp);
extern Errcode xffread(XFILE *xf, void *buf, size_t size);
extern Errcode xffwrite(XFILE *xf, void *buf, size_t size);
extern Errcode xffwriteoset(XFILE *xf, void *buf, long offset, size_t size);
extern Errcode xffseek(XFILE *xf, long offset, enum XSeekWhence whence);
extern long xfftell(XFILE *xf);
extern Errcode xffile_error(void);

/* Check inclusion of deprecated file wrappers. */
#ifdef FFILE_H
#error "xfile and ffile"
#endif

#ifdef LSTDIO_H
#error "xfile and lstdio"
#endif

#endif
