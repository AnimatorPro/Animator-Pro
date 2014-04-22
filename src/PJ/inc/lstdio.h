#ifndef LSTDIO_H
#define	LSTDIO_H

#ifndef _STDARG_H_INCLUDED
#include <stdarg.h>
#endif

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif 

#ifndef  ERRCODES_H
	#include "errcodes.h"
#endif 

#ifndef JFILE_H
#include "jfile.h"
#endif

#define Lfile Jfile
#define lopen pj_open
#define lcreate pj_create
#define lclose pj_close
#define lseek pj_seek
#define lread pj_read
#define lwrite pj_write

/* LFILE->flags structure flags */
#define	BFL_READ		0x0001		/* file may be read from */
#define	BFL_WRITE		0x0002		/* file may be written to */
#define	BFL_TEXT		0x0004		/* file is in <cr/lf> translation mode */
#define	BFL_EOF			0x0008		/* EOF has been reached */
#define	BFL_ERR			0x0010		/* an error has occured */
#define BFL_CONTZ		0x0020		/* have gotten a control z */
#define BFL_RW			(BFL_READ|BFL_WRITE)

typedef struct lfile	/* LFILE structure */
	{
	UBYTE *pt;			/* next character pointer */
	UBYTE *start;		/* start address of buffer */
	UBYTE *end;			/* end address of buffer + 1 */
	int bsize;			/* size of buffer (==bend-bstart) */
	Lfile lfile;		/* lower level file handle */
	long  fpos;			/* lower level file position */
	Errcode ferr;		/* file error status */
	USHORT flags;		/* open mode, etc. */
	UBYTE is_dirty;		/* has buffer been written to? */
	UBYTE can_buf_seek;	/* can we seek in buffer? */
	} LFILE;

/* Default buffer size */
#define LBUF_SIZE 4096

/* stream macros */
#define lclearerr(fp)	((fp)->flags &= ~(BFL_ERR|BFL_EOF))
#define lfeof(fp)	((fp)->flags & BFL_EOF)
#define lferror(fp)	((fp)->flags & BFL_ERR)

#define lgetc(f) \
	(((f)->pt < (f)->end) ? *(f)->pt++ : lfgetc(f))

#define lputc(c,f) \
 (((f)->pt<(f)->end)?(*(f)->pt++=(c),(f)->is_dirty=TRUE):lfputc(c,f))


LFILE *lfopen(const char *name, const char *mode);
Errcode lfclose(LFILE *f);
Errcode lfgetc(LFILE *f);
Errcode lfputc(int c, LFILE *f);
Errcode lungetc(int c, LFILE *f);
Errcode lfflush(LFILE *f);
unsigned lfread(void *buf, unsigned size, unsigned count, LFILE *f);
unsigned lfwrite(const void *buf, unsigned size, unsigned count, LFILE *f);
Errcode lfseek(LFILE *f, long offset, int whence);
void lrewind(LFILE *f);
long lftell(LFILE *f);
char *lfgets(char *s,int max, LFILE *f);
Errcode lfputs(const char *s, LFILE *f);
int lprintf(const char *fmt, ...);
int lfprintf(LFILE *f, const char *fmt, ...);
int lvfprintf(LFILE *f, const char *fmt, va_list args);

/* defines for whence parameter to lfseek */
#define LSEEK_SET 0
#define LSEEK_CUR 1
#define LSEEK_END 2

#define LEOF (-1)

extern LFILE _lstdout, _lstderr;
extern Errcode lerrno;

#define lstdout (&_lstdout)
#define lstderr (&_lstderr)

#endif /* LSTDIO_H */
