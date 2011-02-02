#ifndef RECTANG_H
#define RECTANG_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif



/* Rectangle or Box used for all kinds of boxy things icons blits etc 
 * change this only under penalty of severe crash.  There are many instances
 * of (Rectangle *)&(struc->width) around. 
 *
 *	The order width,height,x,y is important in that if it is followed by
 *  MaxX and MaxY it becomes FullRect */


#define RECT_FIELDS \
	USHORT width, height;\
	SHORT x, y

typedef struct rectangle {
	RECT_FIELDS;
} Rectangle;

typedef struct srect {    /* signed rectangle */
	SHORT width, height;
	SHORT x, y;
} Srect;



#endif /* RECTANG_H */
