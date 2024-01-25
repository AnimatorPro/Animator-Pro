#ifndef STDIO_H
#define STDIO_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

/*** partial implementation of ansi standard io for use inside 
   * rex library code you should not link with other "C" librarys
   * when building pj compatable rex librarys ***/

struct _stdio_error_check_ {
	char c0[sizeof(int) == 4]; /* this is for a 32 bit int environment */
	char c1[sizeof(long) == sizeof(int)]; /* longs are 4 bytes too */
};

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define EOF (-1)

#define errno	 (*(*pj__get_pto_errno))())
#define getc(fp)	fgetc((fp))
#define putc(c,fp)	fputc((c),(fp))

#ifndef _SIZE_T_DEFINED_
	typedef unsigned int size_t;
#endif

typedef void **FILE;
typedef long fpos_t;

/* calls found in _a_a_stdiolib */

int *pj__get_pto_errno(void); /* internal item for getting pointer to errno */
void clearerr(FILE *fp);
int feof(FILE *fp);
int ferror(FILE *fp);

/* errno_errcode() returns animator standard error code for current 
 * value of errno see errcodes.h these codes are what is to be passed to 
 * an animator standard host */

Errcode pj_errno_errcode(void);

FILE *fopen(const char *fname,const char *fmode);
int fclose(FILE *fp);

long fseek(FILE *fp,long offset,int whence);
long ftell(FILE *fp);
int fflush(FILE *fp);
void rewind(FILE *fp);

size_t fread(void *buf,size_t size,size_t n,FILE *fp);
int fgetc(FILE *fp);
char *fgets(char *s,int n,FILE *fp);
int ungetc(int c,FILE *fp);

size_t fwrite(const void *buf,size_t size,size_t n,FILE *fp);
int fputc(int c,FILE *fp);
int fputs(const char *s,FILE *fp);
int fprintf(FILE *fp,const char *fmt,...);

int sprintf(char *buf,const char *fmt,...);

/*** printf is here only for debugging it should not be called in the finished
	 product or the screen will be trashed !!! ****/

int printf(const char *fmt,...);

#endif /* STDIO_H */
