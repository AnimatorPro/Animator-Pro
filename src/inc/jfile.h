#ifndef JFILE_H
#define JFILE_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef FILEPATH_H
	#include "filepath.h"
#endif

#ifndef XFILE_H
#include "xfile.h"
#endif

#ifndef FILEMODE_H

/* I'm copying this over from rexlib/inc/filemode.h for now;
 * I don't know how much of the pocorex stuff I'll be supporting
 * in the long run. */

typedef void *Jfile;
/* JFILE_INTERNALS */

#define JNONE NULL  /* if open fails this is it */

/* seek parameters */
#define JSEEK_START	0 /* defines for mode parameter to jseek */
#define JSEEK_REL	1
#define JSEEK_END	2

/* Normal MS-DOS open flags for mode parameter to jcreate() and jopen() */
#define JUNDEFINED -1
#define JREADONLY 0
#define JWRITEONLY 1
#define JREADWRITE 2

#endif

/** basic low level dos file calls found in the syslib **/

bool pj_exists(const char *path);
Errcode pj_ioerr(void);

void remove_path_name(char *path);
Errcode pj_delete(const char *path);
Errcode pj_rename(const char *oldname, const char *newname);

#ifdef PRIVATE_CODE

#define MAX_DEVICES 26
int pj_get_devices(UBYTE *devids);

/* device level calls */

Errcode get_dir(char *dir);		/* get current directory */
Errcode change_dir(const char *name);

long pj_file_size(const char *path);
Errcode pj_is_fixed(const char *device);
Errcode pj_pathdev_is_fixed(char *path);

Errcode pj_write_zeros(XFILE *xf, LONG oset, ULONG bytes);
Errcode copy_in_file(XFILE *xf,LONG bytes,LONG soff,LONG doff);
Errcode pj_copydata(XFILE *src, XFILE *dst, size_t size);

	/* Size of block of memory used during copy file */
#define PJ_COPY_FILE_BLOCK (32*1024L)

extern Errcode pj_copyfile(const char *src, const char *dst);

extern Errcode
pj_cpfile(const char *src, const char *dst, Errcode *opt_errfile);

extern Errcode
pj_copydata_oset(XFILE *src, XFILE *dst, LONG soset, LONG doset, size_t size);

Errcode pj_insert_space(XFILE *xf, LONG offset, LONG gapsize);

Errcode read_gulp(const char*name, void*buf, long size);
Errcode write_gulp(const char*name, void*buf, long size);

struct fndata;

/* set the 'DTA' area for directory search */
extern void pj_dset_dta(struct fndata *fn);
extern bool pj_dfirst(char *pattern, int attributes);
extern bool pj_dnext(void);

/* PRIVATE_CODE */ #endif

extern void init_stdfiles(void);
extern void cleanup_lfiles(void);

#endif
