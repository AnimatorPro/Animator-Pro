/*****************************************************************************
 * POCOREX.H - Header file defining structures used by POE loadable modules.
 ****************************************************************************/

#ifndef POCOREX_H
#define POCOREX_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef REXLIB_H
	#include "rexlib.h"
#endif

#ifndef POCOLIB_H
	#include "pocolib.h"
#endif

/*****************************************************************************
 * Poco loadable rex library data block
 ****************************************************************************/

#define POCOREX_VERSION 	0

typedef struct pocorex {
	Rexlib hdr; 			/* rexlib header see rexlib.h */
	Poco_lib lib;			/* Poco lib control struct, see pocolib.h */
} Pocorex;

/*****************************************************************************
 * The following macro provides the easiest way to set up your Pocorex data
 * block from within a POE source module.  After the prototypes have been
 * defined in the Lib_proto array, code a Setup_Pocorex macro.
 *
 * The following example assumes you want access to the SYSLIB host library,
 * you have two functions in your POE library that will be accessible to poco
 * programs, and you have init/cleanup functions to allocate/free a screen.
 *
 *	 Lib_proto lib_calls[] = {
 *		{libfunc1, "void func1(void);"},
 *		{libfunc2, "int *func2(int a);"},
 *	  };
 *
 *	 #define HLIB_TYPE_1 AA_SYSLIB
 *	 #include "hliblist.h"
 *
 *	 Setup_Pocorex(get_screen, free_screen, "My Poco Lib", lib_calls);
 *
 *****************************************************************************/

#define Setup_Pocorex(init, cleanup, libname, libprotos) \
 static char _l_name[] = libname;\
 Pocorex rexlib_header = { \
   {REX_POCO, POCOREX_VERSION, init, cleanup, HLIB_LIST, _l_name}, \
   {NULL, _l_name, libprotos, (sizeof(libprotos)/sizeof(libprotos[0]))} \
 };

#endif /* POCOREX_H */
