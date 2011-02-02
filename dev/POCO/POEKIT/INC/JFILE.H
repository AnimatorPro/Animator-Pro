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

Boolean pj_exists(char *path);
Jfile pj_create(char *path,int mode);
Jfile pj_open(char *path,int mode);
Errcode pj_close(Jfile f); 	/* this will check for JNONE (zeros) in f */
Errcode pj_closez(Jfile *jf); /* will check for and set JNONE */
Errcode pj_ioerr(void);

long pj_seek(Jfile f,long offset,int mode);
long pj_tell(Jfile f);
long pj_read(Jfile f, void *buf, long size);
long pj_write(Jfile f, void *buf, long size);

Errcode pj_delete(char *path);
Errcode pj_rename(char *oldname, char *newname);



#endif /* JFILE_H */

