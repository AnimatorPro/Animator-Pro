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

#endif /* __STACK_R32__ */


#ifdef  __STACK_R16__ 

/* C stack where every thing rounded to even byte size without 
 * zero extension turbo c 16 bit int mode */

#define sob_size(sz) ((sz+1)&~(1)) 

#endif /* __STACK_R16__ */

/**** sub-macros ****/

/* sizeof type as stack object */
#define ssizeof(type) sob_size(sizeof(type)) 

#endif /* CSTACK_H */

