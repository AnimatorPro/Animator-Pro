/*****************************************************************************
 * poeskel.c - A skeleton poe module, from which POE projects can be cloned.
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * include the usual header files...
 *--------------------------------------------------------------------------*/

#include "pocorex.h"    /* required header file, also includes pocolib.h */
#include "errcodes.h"   /* most POE programs will need error codes info  */

//#include "syslib.h"   /* include this if you use AA_SYSLIB   */
//#include "gfx.h"      /* include this if you use AA_GFXLIB   */
//#include "math.h"     /* include this if you use AA_MATHLIB  */
//#include "stdio.h"    /* include this if you use AA_STDIOLIB */

/*----------------------------------------------------------------------------
 * set up the host libraries we need...
 *	Add HLIB_TYPE_2 thru n defines as needed below, with each define naming
 *	one of the AA_xxxLIB types listed above.  Don't include libraries you
 *	don't need, it only slows down the loading of your POE module at runtime.
 *--------------------------------------------------------------------------*/

#define HLIB_TYPE_1 AA_POCOLIB	/* this one is always required in a POE */
								/* add other defines here, as needed	*/
#include <hliblist.h>

/*----------------------------------------------------------------------------
 * your data and code goes here...
 *--------------------------------------------------------------------------*/

static void skeleton_func(void)
{
	return;
}

/*----------------------------------------------------------------------------
 * Setup rexlib/pocorex interface structures...
 *--------------------------------------------------------------------------*/

static Lib_proto poe_calls[] = {
	{ skeleton_func, "void SkeletonFunc(void);" },
};

Setup_Pocorex(NOFUNC, NOFUNC, "Skeleton POE", poe_calls);
