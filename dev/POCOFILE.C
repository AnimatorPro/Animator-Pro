/* pocofuns.c - Misc poco library functions */
#include "jimk.h"
#include "errcodes.h"
#include "ptrmacros.h"
#include "pocoface.h"
#include "pocolib.h"
#include "palchunk.h"

extern Errcode builtin_err;

/* A bunch of load/save file functions */

static Errcode po_load_fli(Popot name)
/*****************************************************************************
 * ErrCode LoadFlic(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(resize_load_fli(name.pt));
}

static Errcode po_save_fli(Popot name)
/*****************************************************************************
 * ErrCode SaveFlic(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(save_fli(name.pt));
}

static Errcode po_load_pic(Popot title)
/*****************************************************************************
 * ErrCode LoadPic(char *name)
 ****************************************************************************/
{
if (title.pt == NULL)
	return(builtin_err = Err_null_ref);
dirties();
return(load_the_pic(title.pt) );
}

static Errcode po_save_pic(Popot title)
/*****************************************************************************
 * ErrCode SavePic(char *name)
 ****************************************************************************/
{
if (title.pt == NULL)
	return(builtin_err = Err_null_ref);
return(save_current_pictype(title.pt,vb.pencel));
}

static Errcode po_load_screen_pic(Popot screen, Popot title)
/*****************************************************************************
 * ErrCode LoadScreenPic(Screen *s, char *name)
 ****************************************************************************/
{
Rcel *s;
if (title.pt == NULL)
	return(builtin_err = Err_null_ref);
if (screen.pt == NULL)
	s = vb.pencel;
else
	s = screen.pt;
dirties();
return(load_any_picture(title.pt,s) );
}

static Errcode po_save_screen_pic(Popot screen, Popot title)
/*****************************************************************************
 * ErrCode SaveScreenPic(Screen *s, char *name)
 ****************************************************************************/
{
Rcel *s;
if (title.pt == NULL)
	return(builtin_err = Err_null_ref);
if (screen.pt == NULL)
	s = vb.pencel;
else
	s = screen.pt;
return(save_current_pictype(title.pt,s));
}

static Errcode po_load_cel(Popot name)
/*****************************************************************************
 * ErrCode LoadCel(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(load_the_cel(name.pt));
}

static Errcode po_save_cel(Popot name)
/*****************************************************************************
 * ErrCode SaveCel(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(save_the_cel(name.pt));
}

static Errcode po_load_path(Popot name)
/*****************************************************************************
 * ErrCode LoadPath(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(load_path(name.pt));
}

static Errcode po_save_path(Popot name)
/*****************************************************************************
 * ErrCode SavePath(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(save_path(name.pt));
}

static Errcode po_load_poly(Popot name)
/*****************************************************************************
 * ErrCode LoadPoly(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(load_polygon(name.pt));
}

static Errcode po_save_poly(Popot name)
/*****************************************************************************
 * ErrCode SavePoly(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(save_polygon(name.pt));
}

static Errcode po_load_colors(Popot name)
/*****************************************************************************
 * ErrCode LoadColors(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(load_palette(name.pt,1));
}

static Errcode po_save_colors(Popot name)
/*****************************************************************************
 * ErrCode SaveColors(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(pj_col_save(name.pt, vb.pencel->cmap));
}

static Errcode po_load_titles(Popot name)
/*****************************************************************************
 * ErrCode LoadTitles(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(load_titles(name.pt));
}

static Errcode po_save_titles(Popot name)
/*****************************************************************************
 * ErrCode SaveTitles(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(save_titles(name.pt));
}

static Errcode po_save_mask(Popot name)
/*****************************************************************************
 * ErrCode SaveMask(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(save_the_mask(name.pt));
}

static Errcode po_load_mask(Popot name)
/*****************************************************************************
 * ErrCode LoadMask(char *name)
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(load_the_mask(name.pt));
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

PolibAAFile po_libaafile = {
po_load_fli,	"ErrCode LoadFlic(char *name);",
po_save_fli,	"ErrCode SaveFlic(char *name);",
po_load_pic,	"ErrCode LoadPic(char *name);",
po_save_pic,	"ErrCode SavePic(char *name);",
po_load_cel,	"ErrCode LoadCel(char *name);",
po_save_cel,	"ErrCode SaveCel(char *name);",
po_load_path,	"ErrCode LoadPath(char *name);",
po_save_path,	"ErrCode SavePath(char *name);",
po_load_poly,	"ErrCode LoadPoly(char *name);",
po_save_poly,	"ErrCode SavePoly(char *name);",
po_load_colors, "ErrCode LoadColors(char *name);",
po_save_colors, "ErrCode SaveColors(char *name);",
po_load_titles, "ErrCode LoadTitles(char *name);",
po_save_titles, "ErrCode SaveTitles(char *name);",
po_load_mask,	"ErrCode LoadMask(char *name);",
po_save_mask,	"ErrCode SaveMask(char *name);",
po_save_screen_pic,
	"ErrCode SaveScreenPic(Screen *s, char *name);",
po_load_screen_pic,
	"ErrCode LoadScreenPic(Screen *s, char *name);",
};

Poco_lib po_load_save_lib =
	{
	NULL,
	"Autodesk Animator File",
	(Lib_proto *)&po_libaafile, POLIB_AAFILE_SIZE,
	};



