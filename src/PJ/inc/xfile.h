#ifndef XFILE_H
#define XFILE_H

#include <stdarg.h>
#include <stddef.h>
#include "stdtypes.h"

#ifndef EOF
enum { EOF = -1 };
#else
STATIC_ASSERT(xfile, EOF == -1);
#endif

enum XSEEK_WHENCE {
	XSEEK_SET = 0,
	XSEEK_CUR = 1,
	XSEEK_END = 2
};

typedef struct xfl XFILE;

extern XFILE *xstdout;
extern XFILE *xstderr;

extern XFILE *xfopen(const char *path, const char *mode);
extern int xfclose(XFILE *xf);
extern int xfgetc(XFILE *xf);
extern int xfputc(int c, XFILE *xf);
extern int xungetc(int c, XFILE *xf);
extern int xfflush(XFILE *xf);
extern size_t xfread(void *ptr, size_t size, size_t nmemb, XFILE *xf);
extern size_t xfwrite(const void *ptr, size_t size, size_t nmemb, XFILE *xf);
extern int xfseek(XFILE *xf, long offset, enum XSEEK_WHENCE whence);
extern void xrewind(XFILE *xf);
extern long xftell(XFILE *xf);
extern int xfprintf(XFILE *xf, const char *format, ...);
extern int xprintf(const char *format, ...);
extern int xvfprintf(XFILE *xf, const char *format, va_list ap);
extern char *xfgets(char *s, int size, XFILE *xf);
extern int xfputs(const char *s, XFILE *xf);
extern int xferror(XFILE *xf);
extern int xerrno(void);

extern Errcode xffopen(const char *path, XFILE **pfp, const char *fmode);
extern void xffclose(XFILE **pfp);
extern Errcode xffread(XFILE *xf, void *buf, size_t size);
extern Errcode xffwrite(XFILE *xf, void *buf, size_t size);
extern Errcode xffwriteoset(XFILE *xf, void *buf, long offset, size_t size);
extern Errcode xffseek(XFILE *xf, long offset, enum XSEEK_WHENCE whence);
extern long xfftell(XFILE *xf);
extern Errcode xffile_error(void);

#endif
