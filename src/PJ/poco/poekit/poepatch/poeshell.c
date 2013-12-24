/*****************************************************************************
 * poeshell.c - Shell to make patch10a module into a loadable poe module.
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * include the usual header files...
 *--------------------------------------------------------------------------*/

#include "pocorex.h"
#include "errcodes.h"
#include "patch10a.h"

/*----------------------------------------------------------------------------
 * set up the host libraries we need...
 *--------------------------------------------------------------------------*/

#define HLIB_TYPE_1 AA_POCOLIB	/* this one is always required in a POE */
#include <hliblist.h>

/*----------------------------------------------------------------------------
 * code and data here...
 *--------------------------------------------------------------------------*/

extern Lib_proto poe_calls[];	/* not really extern, just pre-declared */

static Errcode init_func(void)
/*****************************************************************************
 * rex-layer (loadtime) init function.
 *	 if the host version is < 179, we apply the patches and return.
 *	 if the host version is later, the patches are unneeded, they've been
 *	 compiled into the host already.  but, the poco program that loads us
 *	 doesn't know that, so instead of applying the patches (which would
 *	 fail miserably), we wipe out the entries in our lib_proto array.  this
 *	 prevents us from supplying the new functions; since they are alreday
 *	 present in the host's builtin libraries, it would generate an error if
 *	 we supplied them again.  the rules for a library proto say that the
 *	 proto string can't be empty, and can't contain C-style comments.  but,
 *	 they can contain preprocessor strings, as long as the associated func
 *	 pointers are null.  since the preprocessor will ignore (without warning)
 *	 any #pragma statement that doesn't have the word 'poco' after it, we
 *	 suppy "#pragma dummy" statements as a way of passing do-nothing comment
 *	 lines in place of our usual proto strings.
 ****************************************************************************/
{
	static char dummy_proto[] = "#pragma dummy";

	if (poePocoVersion() < 179) {
		init_patches_10a();
	} else {
		poe_calls[0].func  = NULL;
		poe_calls[1].func  = NULL;
		poe_calls[2].func  = NULL;
		poe_calls[0].proto = dummy_proto;
		poe_calls[1].proto = dummy_proto;
		poe_calls[2].proto = dummy_proto;
	}
	return Success;
}

/*----------------------------------------------------------------------------
 * Setup rexlib/pocorex interface structures...
 *--------------------------------------------------------------------------*/

static Lib_proto poe_calls[] = {
	{ po_get_boxbevel,	"int    GetBoxBevel(void);"},
	{ po_set_boxbevel,	"void   SetBoxBevel(int new_bevel);"},
	{ po_release_cel,	"void   CelRelease(void);"},
};

Setup_Pocorex(init_func, NOFUNC, "Patch POE v1.0a", poe_calls);
