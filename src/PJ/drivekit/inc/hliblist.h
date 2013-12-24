/*****************************************************************************
 * HLIBLIST.H	Create all the Hostlib structures needed by an application.
 *
 * ABSTRACT:
 *				This header file is #include'd by rex-loadable source code to
 *				automatically create the linked list of Hostlib structures
 *				the application needs.	This file should be #include'd by the
 *				'main' source module of the rex-loadable application (ie, the
 *				same source module that declares the rexlib_header structure).
 *
 *				Before including this header, the application #define's C
 *				macros indicating which host libraries are needed.	The
 *				macros are named 'HLIB_TYPE_n', where 'n' is 1 through 9.
 *				The library types are defined in REXLIB.H, which must be
 *				#include'd first.  For each of the library types defined,
 *				this header file will #include <uselib.h> to do the actual
 *				work of building the list.
 *
 *				For rex-loadable modules which do not need host-provided
 *				libraries, there is no need to include this header file, but
 *				there is no harm in doing so; it will not choke when there
 *				are no HLIB_TYPE_n macros defined.
 *
 * EXAMPLE:
 *				#include <rexlib.h>
 *				#define HLIB_TYPE_1 AA_STDIOLIB
 *				#define HLIB_TYPE_2 AA_POCOLIB
 *				#define HLIB_TYPE_3 AA_GFXLIB
 *				#include <hliblist.h>
 *
 * ENTRY:
 *				C macros HLIB_TYPE_1, HLIB_TYPE_2, etc, contain the types
 *				of the host libraries needed by the application.  It is OK
 *				if none of the macros are defined; no host libraries will
 *				be set up in this case.
 * EXIT:
 *				C macro HLIB_LIST contains the name of the Hostlib
 *				structure which heads the list of structures built.  The
 *				value in this macro is suitable for initializing the
 *				hostlib list field in the Rexlib header structure (it already
 *				includes the '&' to take the address of the structure, for
 *				initializing a pointer field.)	If no libraries were set up,
 *				this macro will substitute to the value 'NULL'.
 *
 * DEPENDENCIES:
 *				Header file REXLIB.H must be #include'd before this file.
 *
 * MAINTENANCE:
 *	11/24/90	Created.
 ****************************************************************************/

#ifdef HLIB_TYPE_1
  #define HLIB_TYPE HLIB_TYPE_1
  #include <usehlib.h>
#endif

#ifdef HLIB_TYPE_2
  #define HLIB_TYPE HLIB_TYPE_2
  #include <usehlib.h>
#endif

#ifdef HLIB_TYPE_3
  #define HLIB_TYPE HLIB_TYPE_3
  #include <usehlib.h>
#endif

#ifdef HLIB_TYPE_4
  #define HLIB_TYPE HLIB_TYPE_4
  #include <usehlib.h>
#endif

#ifdef HLIB_TYPE_5
  #define HLIB_TYPE HLIB_TYPE_5
  #include <usehlib.h>
#endif

#ifdef HLIB_TYPE_6
  #define HLIB_TYPE HLIB_TYPE_6
  #include <usehlib.h>
#endif

#ifdef HLIB_TYPE_7
  #define HLIB_TYPE HLIB_TYPE_7
  #include <usehlib.h>
#endif

#ifdef HLIB_TYPE_8
  #define HLIB_TYPE HLIB_TYPE_8
  #include <usehlib.h>
#endif

#ifdef HLIB_TYPE_9
  #define HLIB_TYPE HLIB_TYPE_9
  #include <usehlib.h>
#endif
