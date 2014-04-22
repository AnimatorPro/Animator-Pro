#ifndef TFILE_H
#define TFILE_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

#define TDEV_MED '='
#define TDEV_LO '<'
#define TRD_CHAR '#'
#define TDEV_ID 27		/* one past MS-DOS */

extern Boolean is_tdrive(const char *dev);
extern Errcode set_temp_path(const char *new_path);
extern Errcode change_temp_path(const char *new_path);
extern Errcode trd_compact(long need_free);

#endif
