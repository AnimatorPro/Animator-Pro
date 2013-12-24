/* pocofunc.c - misc not part of a large family poco library routines */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "jimk.h"
#include "fli.h"
#include "pocoface.h"
#include "pocolib.h"
#include "errcodes.h"
#include "options.h"
#include "poly.h"

extern char    po_chainto_program_path[];  // in qpoco.c
extern Errcode builtin_err;
Errcode resize_default_temps(SHORT width, SHORT height);
void kill_seq(void);
void zoom_unundo(void);
void swap_undo(void);
void restore(void);

static int po_get_change_count(void)
/*****************************************************************************
 * int GetChangeCount(void)
 ****************************************************************************/
{
	return (dirty_file ? dirty_strokes : 0);
}

static void po_redo(void)
/*****************************************************************************
 * void Redo(void)
 ****************************************************************************/
{
_redo_draw(&vs.redo);
dirties();
}

static void po_exit(int err)
/*****************************************************************************
 * void exit(ErrCode err)
 ****************************************************************************/
{
if (err >= Success)
	err = Err_poco_exit; // magical errcode that runops.c xlates into Success
builtin_err = err;
return;
}

int po_rnd(int max)
/*****************************************************************************
 * int rnd(int max)
 ****************************************************************************/
{
if (max == 0)
	return(0);
if (max < 0)
	return builtin_err = Err_parameter_range;
return(rand()%max);
}

int po_version(void)
/*****************************************************************************
 * int PocoVersion(void)
 ****************************************************************************/
{
return(po_version_number);
}


static Errcode in_poco_reset(int width, int height)
{
	Errcode err;
	char pbuf[PATH_SIZE];

	free_render_cashes();					// Reset may redo ink types
	cleanup_toptext();						// It redoes the windows too.
	vset_get_path(POCO_PATH,pbuf);			// Save poco program name
	err = resize_default_temps(width,height);	// Reset!
	vset_get_path(POCO_PATH,pbuf);			// Restore poco name
	make_render_cashes();
	return err;
}

static Errcode po_resize_reset(int width, int height)
/*****************************************************************************
 * ErrCode ResizeReset(int width, int height)
 ****************************************************************************/
{
Errcode err;

if (width <= 0 || width > SHRT_MAX || height <= 0 || height > SHRT_MAX)
	return Err_parameter_range;
return in_poco_reset(width, height);
}

static Errcode po_reset(void)
/*****************************************************************************
 * ErrCode Reset(void)
 ****************************************************************************/
{
return(in_poco_reset(vb.pencel->width,vb.pencel->height));
}

static Boolean po_is_batch(void)
/*****************************************************************************
 * Boolean IsBatchRun(void) - Return is/isnot a run started from cmdline.
 ****************************************************************************/
{
	extern char *cl_poco_name;	/* in main.c */
	return (cl_poco_name != NULL);
}

static void po_chainto(Popot ppath)
/*****************************************************************************
 * void PocoChainTo(char *program_path)
 ****************************************************************************/
{
	if (ppath.pt == NULL)
		po_chainto_program_path[0] = '\0';
	else
		get_full_path(ppath.pt, po_chainto_program_path);
}

static Errcode po_system(Popot cmdline)
/*****************************************************************************
 * Errcode system(char *command_line)
 *	This behaves like ANSI system(), except if the command line string is
 *	empty ("") it invokes COMMAND.COM interactively.
 *
 *	The return value from text_mode_exec() will be non-Success if we couldn't
 *	find COMMAND.COM or didn't have enough DOS-area memory to load it into. In
 *	this case, we return the status to the caller without stopping the show.
 ****************************************************************************/
{
	Errcode err;

	if (cmdline.pt == NULL) {	/* ANSI behavior:  a NULL pointer is	*/
		return TRUE;			/* asking whether a command processor	*/
	} else {					/* is available.						*/
		return text_mode_exec(cmdline.pt);
	}
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

PolibMisc po_libmisc = {
po_exit,
	"void    exit(ErrCode err);",
kill_seq,
	"void    NewFlic(void);",
po_reset,
	"ErrCode Reset(void);",
po_resize_reset,
	"ErrCode ResizeReset(int width, int height);",
po_get_change_count,
	"int     GetChangeCount(void);",
po_version,
	"int     PocoVersion(void);",
po_redo,
	"void    Redo(void);",
restore,
	"void    Restore(void);",
po_rnd,
	"int     rnd(int max);",
rand,
	"int     rand(void);",
srand,
	"void    srand(int seed);",
clock,
	"long    clock(void);",
po_is_batch,
	"Boolean IsBatchRun(void);",
po_chainto,
	"void    PocoChainTo(char *program_path);",
po_system,
	"Errcode system(char *command_line);",
};

Poco_lib po_misc_lib = {
	NULL, "Misc. Functions",
	(Lib_proto *)&po_libmisc, POLIB_MISC_SIZE,
	};
