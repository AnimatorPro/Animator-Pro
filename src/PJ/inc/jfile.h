#ifndef JFILE_H
#define JFILE_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef FILEPATH_H
	#include "filepath.h"
#endif

#ifndef FILEMODE_H
	#include "filemode.h"
#endif

/** basic low level dos file calls found in the syslib **/

Boolean pj_exists(const char *path);
Jfile pj_create(const char *path, int mode);
Jfile pj_open(const char *path, int mode);
Errcode pj_close(Jfile f); 	/* this will check for JNONE (zeros) in f */
Errcode pj_closez(Jfile *jf); /* will check for and set JNONE */
Errcode pj_ioerr(void);

extern Boolean is_ram_file(Jfile fd);

long pj_seek(Jfile f,long offset,int mode);
long pj_tell(Jfile f);
long pj_read(Jfile f, void *buf, long size);
long pj_write(Jfile f, void *buf, long size);

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
Boolean is_directory(const char *path);

int get_jmode(Jfile fd);

Errcode pj_write_zeros(Jfile f, LONG oset, ULONG bytes);
Errcode copy_in_file(Jfile file,LONG bytes,LONG soff,LONG doff);
Errcode pj_copydata(Jfile src, Jfile dest, ULONG size);

/* pj_copyfile copies source to dest file and reports any error except
    for source not existing. */
Errcode pj_copyfile(char*source,char*dest);
	/* Size of block of memory used during copy file */
#define PJ_COPY_FILE_BLOCK (32*1024L)

/* pj_cpfile copies source to destination.  Does not report error
   but if there is one returns the error code and sets **perrfile
   to either source or dest (depending where error was). */
Errcode pj_cpfile(char *source, char *dest, char **perrfile);

extern Errcode
pj_copydata_oset(Jfile src, Jfile dest, LONG soset, LONG doset, ULONG size);

Errcode pj_insert_space(Jfile f,LONG offset, LONG gapsize);

Errcode pj_readoset(Jfile f,void *buf, LONG oset,LONG size);
Errcode pj_writeoset(Jfile f,void *buf, LONG oset,LONG size);

Errcode pj_read_ecode(Jfile f, void *buf, LONG size);
Errcode pj_write_ecode(Jfile f, void *buf, LONG size);

Errcode read_gulp(const char*name, void*buf, long size);
Errcode write_gulp(const char*name, void*buf, long size);

/* this is the data structure used by pj_dfirst() pj_dnext() in searching
   directories */
typedef struct fndata 
	{
	char reserved[21];
	char attribute;
	USHORT time, date;
	long size;
	char name[13];
	char fordos[128-43];
	} Fndata;

void pj_dset_dta(Fndata *fn); /* set the 'DTA' area for directory search */
Boolean pj_dfirst(char *pattern, int attributes);
/* defines for attributes parameters */
#define ATTR_DIR	16
#define ATTR_NORMAL  0
Boolean pj_dnext(void);

/* PRIVATE_CODE */ #endif

extern void init_stdfiles(void);
extern void cleanup_lfiles(void);
extern void *get_jstdout(void);
extern void *get_jstderr(void);

#endif /* JFILE_H */

