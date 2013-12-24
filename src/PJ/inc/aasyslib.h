#ifndef AASYSLIB_H
#define AASYSLIB_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef REXLIB_H 
	#include "rexlib.h" 
#endif

#ifndef FILEMODE_H
	#include "filemode.h"
#endif

typedef struct syslib
	{
	Libhead hdr;
	void *(*malloc)(long);		/* uninitialized memory */
	void *(*zalloc)(long);		/* allocate cleared memory */
	void (*free)(void *pt);     /* free either */
	void *(*memset)(void *s,int c,int length); /* ansi memset function */
	void *(*memcpy)(void *dst,void *src,int length); /* ansi memcpy function */
	int (*memcmp)(void *s1,void *s2,int length); /* ansi memcmp function */
	char *(*strcpy)(char *s);  				/* ansi string copy */
	int (*strlen)(char *s);   				/* ansi strlen */
	int (*strcmp)(char *s1,char *s2); 		/* ansi string compare */
	char *(*pj_get_path_suffix)(char *path); 
	char *(*pj_get_path_name)(char *path);

/* Might as well let drivers load drivers... */
	Errcode (*pj_rex_load)(char *fname, void **entry, void **rlib_header);
	void (*pj_rex_free)(void **entry);
 	Errcode (*pj_rexlib_load)(char *name, USHORT type, 
						   Rexlib **prl,void **hostlibs,char *id_string);
	Errcode (*pj_rexlib_init)(Rexlib *rl,...);
	void (*pj_rexlib_free)(Rexlib **prl);

/* dos unbuffered file io interface */
	Errcode (*pj_ioerr)(void);	/* find out more about io errors */
	Jfile (*pj_open)(char *filename, int mode);
	Jfile (*pj_create)(char *filename, int mode);
	void (*pj_close)(Jfile f);
	long (*pj_read)(Jfile f, void *buf, long size);
	long (*pj_write)(Jfile f, void *buf, long size);
	long (*pj_seek)(Jfile f, long offset, int mode);
	long (*pj_tell)(Jfile f);
	Errcode (*pj_delete)(char *filename);
	Errcode (*pj_rename)(char *oldname, char *newname);
	Boolean (*pj_exists)(char *filename);

	long (*pj_clock_1000)(); /* millisecond clock 0 at program startup */

/* special debugging code */
	int (*boxf)(char *fmt,...); /* this is a printf for debugging */

} Syslib;

#endif /* AASYSLIB_H */
