#ifndef STDIOLIB_H
#define STDIOLIB_H

/**** definition of stdiolib as provided by host to rexlib code 
 * note: definition stdio FILE etc must be included before this file 
 * since different hosts may have different definitions of standard io 
 * functions */

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef REXLIB_H
	#include "rexlib.h"
#endif

struct _f_data {
	char onebyte;
};

/* if EOF not -1 or not a 32 bit int or va_list is not a pointer we shouldn't
 * use this library */

struct _stdiolib_err_check_ {
	char xx[EOF == -1];
	char xx1[sizeof(size_t) == sizeof(unsigned int)];
	char xx2[sizeof(fpos_t) == sizeof(long)];
};


typedef struct stdiolib {
	Libhead hdr;
	struct _f_data *fdata;

	int *(*pj__get_pto_errno)(void);
	void (*clearerr)(FILE *fp);
	int (*feof)(FILE *fp);
	int (*ferror)(FILE *fp);
	Errcode (*pj_errno_errcode)(void);

	FILE *(*fopen)(const char *fname,const char *fmode);
	int (*fclose)(FILE *fp);

	long (*fseek)(FILE *fp,long offset,int whence);
	long (*ftell)(FILE *fp);
	int (*fflush)(FILE *fp);
	void (*rewind)(FILE *fp);

	size_t (*fread)(void *buf,size_t size,size_t n,FILE *fp);
	int (*fgetc)(FILE *fp);
	char *(*fgets)(char *s,int n,FILE *fp);
	int (*ungetc)(int c,FILE *fp);

	size_t (*fwrite)(const void *buf,size_t size,size_t n,FILE *fp);
	int (*fputc)(int c,FILE *fp);
	int (*fputs)(const char *s,FILE *fp);
	int (*fprintf)(FILE *fp,const char *fmt,...);

	int (*printf)(FILE *fp,const char *fmt,...);
	int (*sprintf)(char *buf,const char *fmt,...);


} Stdiolib;


#ifdef PRIVATE_CODE  

#ifdef NOTYET
	int (*vfprintf)(FILE *fp,const char *format,va_list arg);
	int (*fgetpos)(FILE *fp,fpos_t *pos);
	int (*fsetpos)(FILE *fp,const fpos_t *pos);
	int (*vfprintf)(FILE *fp,const char *format,va_list arg);
	int (*vprintf)(const char *format,va_list arg);
	int (*vsprintf)(char *s,const char *format,va_list arg);
#endif

#ifdef NOTYET
	int (*remove)(const char *filename);
	int (*rename)(const char *old,const char *new);
	FILE *(*freopen)(const char *fname,const char *mode,FILE *fp);
	int (*fscanf)(FILE*fp,const char *format,...);
	int (*getchar)(void);
	char *(*gets)(char *s);
	void (*perror)(const char *s);
	int (*putchar)(int c);
	int (*puts)(const char *s);
	int (*scanf)(const char *format,...);
	void (*setbuf)(FILE *fp,char *buf);
	int (*setvbuf)(FILE *fp,char *buf,int mode,size_t size);
	int (*sscanf)(const char *s,const char *format,...);
	FILE *(*tmpfile)(void);
	char *(*tmpnam)(char *s);
	int (*vfscanf)(FILE *fp,const char *format,va_list arg);
	int (*vscanf)(const char *format,va_list arg);
	int (*vsscanf)(const char *s,const char *format,va_list arg);
#endif /* NOTYET */

/* PRIVATE_CODE */ #endif

#endif /* STDIOLIB_H */
