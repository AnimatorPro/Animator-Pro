#ifndef GRID_H
#define GRID_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

extern USHORT constrain_angle(SHORT angle);
extern void qgrid_keep_undo(void);
extern void qgrid(void);
extern void grid_flixy(SHORT *flix, SHORT *fliy);

#endif
