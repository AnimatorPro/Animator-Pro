#ifndef CSTACK_H
#define CSTACK_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef PTRMACRO_H
	#include "ptrmacro.h"
#endif


/**** macros for C stack manipulation ******/

typedef void *C_stack;

#ifdef __STACK_R32__ 

/**** for stack items rounded to 32 bit boundaries with zero extension
 **** on integer types watcom c 32 bit int extended mode ****/

/* size of a stack object */

#define sob_size(sz) ((sz+sizeof(int)-1)&~(sizeof(int)-1)) 

/* assign an object to it's stack object representation */

#define assign_c_stack(cs,type,pt) { \
	(sizeof(type)==1)?*((ULONG *)cs)=*((UBYTE *)pt):\
	(sizeof(type)==2)?*((ULONG *)cs)=*((USHORT *)pt):\
	(sizeof(type)==3)?*((ULONG *)cs)=*((ULONG *)pt),((UBYTE *)cs)[3]=0:\
	(sizeof(type)==4)?*((ULONG *)cs)=*((ULONG *)pt):\
	*((type *)cs)=*((type *)pt);\
}

#endif /* __STACK_R32__ */


#ifdef  __STACK_R16__ 

/* C stack where every thing rounded to even byte size without 
 * zero extension turbo c 16 bit int mode */

#define sob_size(sz) ((sz+1)&~(1)) 

#define assign_c_stack(cs,type,pt) {*((type *)cs)=*((type *)(pt));}

#endif /* __STACK_R16__ */

/**** sub-macros ****/

/* sizeof type as stack object */
#define ssizeof(type) sob_size(sizeof(type)) 

#define add_c_stack(cs,type,pt) \
	{assign_c_stack(cs,type,pt);cs=OPTR(cs,ssizeof(type));}

#define push_c_stack(cs,type,pt) \
	{cs=OPTR(cs,-ssizeof(type));assign_c_stack(cs,type,pt);}


#endif /* CSTACK_H */

