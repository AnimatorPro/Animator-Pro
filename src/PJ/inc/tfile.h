#ifndef TFILE_H
#define TFILE_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#define TDEV_MED '='
#define TDEV_LO '<'
#define TRD_CHAR '#'
#define TDEV_ID 27		/* one past MS-DOS */

#ifdef  TFILE_C
typedef struct tfl *Tfile;
#else
typedef void *Tfile;
#endif /* TFILE_C */

Boolean is_tdrive(char *dev);
int get_tdrive_id(char *tdrive);
char *get_temp_path(void);
Errcode set_temp_path(char *tp);
Errcode change_temp_path(char *new_path);
Tfile tcreate(const char *name, int mode);
Tfile topen(const char *name, int mode);
Errcode tclose(Tfile t);
long tread(Tfile t, void *buf, long size);
long twrite(Tfile t, void *buf, long size);
long tseek(Tfile t, long offset, int mode);
long ttell(Tfile t);
Errcode tdelete(const char *name);
Errcode terror(void);
Errcode trename(const char *old, const char *new);

Errcode trd_ram_to_files(void);
Errcode trd_up_to_ram(char *name);
Errcode trd_compact(long need_free);

#endif /* TFILE_H */
