#include "jimk.h"
#include "errcodes.h"
#include "pocoface.h"
#include "pocolib.h"

static Boolean po_alt_exists(void)
/*****************************************************************************
 * Boolean SwapExists(void);
 ****************************************************************************/
{
	return(vl.alt_cel != NULL);
}

static void po_alt_grab(void)
/*****************************************************************************
 * void SwapClip(void);
 *	The grab_alt() routine has non-optional softerr reporting and doesn't
 *	return an error status to us.  As a workaround, we check to make sure
 *	an alt cel got allocated, and if not, we set builtin_err to Err_reported.
 ****************************************************************************/
{
	grab_alt();
	if (vl.alt_cel == NULL)
		builtin_err = Err_no_memory;
}

static Errcode po_alt_swap(void)
/*****************************************************************************
 * ErrCode SwapTrade(void);
 ****************************************************************************/
{
	if (vl.alt_cel == NULL)
		return Err_not_found;
	swap_alt();
	return Success;
}

/*----------------------------------------------------------------------------
 * library protos...
 *
 * Maintenance notes:
 *	To add a function to this library, add the pointer and prototype string
 *	TO THE END of the following list.  Then go to POCOLIB.H and add a
 *	corresponding prototype to the library structure typedef therein which
 *	matches the name of the structure below.  When creating the prototype
 *	in POCOLIB.H, remember that all arguments prototyped below as pointers
 *	must be defined as Popot types in the prototype in pocolib.h; any number
 *	of stars in the Poco proto still equate to a Popot with no stars in the
 *	pocolib.h prototype.
 *
 *	DO NOT ADD NEW PROTOTYPES OTHER THAN AT THE END OF AN EXISTING STRUCTURE!
 *	DO NOT DELETE A PROTOTYPE EVER! (The best you could do would be to point
 *	the function to a no-op service routine).  Breaking these rules will
 *	require the recompilation of every POE module in the world.  These
 *	rules apply only to library functions which are visible to POE modules
 *	(ie, most of them).  If a specific typedef name exists in pocolib.h, the
 *	rules apply.  If the protos are coded as a generic array of Lib_proto
 *	structs without an explicit typedef in pocolib.h, the rules do not apply.
 *--------------------------------------------------------------------------*/

PolibSwap po_libswap = {
po_alt_exists,
	"Boolean SwapExists(void);",
po_alt_grab,
	"void    SwapClip(void);",
free_alt,
	"void    SwapRelease(void);",
po_alt_swap,
	"ErrCode SwapTrade(void);",
};

Poco_lib po_alt_lib = {
	NULL, "Swap Screen",
	(Lib_proto *)&po_libswap, POLIB_SWAP_SIZE,
	};
