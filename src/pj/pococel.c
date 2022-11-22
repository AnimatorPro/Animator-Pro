
#include "errcodes.h"
#include "ptrmacro.h"
#include "flicel.h"
#include "pocoface.h"
#include "pocolib.h"

extern Flicel *thecel;
extern Errcode cel_from_rect(Rectangle *rect, Boolean render_only);
extern Errcode clip_cel(void);

/* Cel oriented stuff */
static void po_ink_paste(void)
/*****************************************************************************
 * void CelPaste(void);
 ****************************************************************************/
{
free_render_cashes();		/* annoying but necessary */
render_thecel();
make_render_cashes();
dirties();
}

Errcode po_get_cel(int x, int y, int width, int height)
/*****************************************************************************
 * ErrCode CelGet(int x, int y, int width, int height);
 *****************************************************************************/
{
Rectangle rect;

rect.x = x;
rect.y = y;
rect.width = width;
rect.height = height;
return(cel_from_rect(&rect,TRUE));
}

static void po_move_cel(int dx, int dy)
/*****************************************************************************
 * void CelMove(int dx, int dy);
 *****************************************************************************/
{
if (!thecel)
	return;
translate_flicel(thecel,dx,dy);
maybe_ref_flicel_pos(thecel);
}


static void po_move_cel_to(int x, int y)
/*****************************************************************************
 * void CelMoveTo(int x, int y);
 ****************************************************************************/
{
if (!thecel)
	return;
thecel->cd.cent.x = x;
thecel->cd.cent.y = y;
maybe_ref_flicel_pos(thecel);
}

static void po_rotate_cel(double angle)
/*****************************************************************************
 * void CelTurn(double angle);
 ****************************************************************************/
{
if (!thecel)
	return;
thecel->cd.rotang.z += angle*1024/360;
maybe_ref_flicel_pos(thecel);
}

static void po_rotate_cel_to(double angle)
/*****************************************************************************
 * void CelTurnTo(double angle);
 ****************************************************************************/
{
if (!thecel)
	return;
thecel->cd.rotang.z = angle*1024/360;
maybe_ref_flicel_pos(thecel);
}

static Errcode po_cel_seek(int abspos, int relpos)
/*****************************************************************************
 * if abspos < 0 then use relpos.  Return cel position
 ****************************************************************************/
{
int err;
int cpos;
int npos;

if (thecel != NULL)
	{
	if((err = reopen_fcelio(thecel,JREADONLY)) < Success)
		goto OUT;
	cpos = thecel->cd.cur_frame;
	if (abspos < 0)
		npos = cpos + relpos;
	else
		npos = abspos;
	if (npos != cpos)
		err = seek_fcel_frame(thecel, npos);
	if (err >= Success)
		err = npos;
	close_fcelio(thecel);
	}
else
	err = Err_not_found;
OUT:
	return(err);
}

static Boolean po_cel_exists(void)
/*****************************************************************************
 * Boolean CelExists(void);
 ****************************************************************************/
{
return(thecel != NULL);
}

static Errcode po_cel_next_frame(void)
/*****************************************************************************
 * ErrCode CelNextFrame(void);
 *****************************************************************************/
{
return(po_cel_seek(-1,1));
}

static Errcode po_cel_back_frame(void)
/*****************************************************************************
 * ErrCode CelBackFrame(void);
 ****************************************************************************/
{
return(po_cel_seek(-1,-1));
}

static Errcode po_cel_get_frame(void)
/*****************************************************************************
 * int CelGetFrame(void);
 ****************************************************************************/
{
return(po_cel_seek(-1,0));
}

static Errcode po_cel_set_frame(int ix)
/*****************************************************************************
 * ErrCode CelSetFrame(int frame);
 ****************************************************************************/
{
return(po_cel_seek(ix,0));
}

static int po_cel_frame_count(void)
/*****************************************************************************
 * int CelFrameCount(void);
 ****************************************************************************/
{
if (thecel == NULL)
	return(Err_not_found);
return(thecel->flif.hdr.frame_count);
}

static Errcode po_cel_where(Popot px, Popot py, Popot pangle)
/*****************************************************************************
 * ErrCode CelWhere(int *x, int *y, double *angle);
 * returns x,y position of cel, and angle.
 ****************************************************************************/
{

if (px.pt == NULL || py.pt == NULL || pangle.pt == NULL)
	return(builtin_err = Err_null_ref);
if (thecel == NULL)
	return(Err_not_found);
vass(px.pt,int) = thecel->cd.cent.x;
vass(py.pt,int) = thecel->cd.cent.y;
vass(pangle.pt,double) = thecel->cd.rotang.z*360.0/1024.0;
return Success;
}

static Errcode po_clip_cel(void)
/*****************************************************************************
 * ErrCode CelClip(void);
 *	 In addition to the types of errors the internal clip_cel() routine can
 *	 return to us (no memory, etc), we check to make sure that something
 *	 actually got clipped.	If thecel comes up NULL, it means that the
 *	 picscreen was totally blank; ie, there was nothing to clip.  We check
 *	 for this specifically because the internal clip_cel() routine doesn't.
 *	 (It would just leave Cel menu items disabled).  A program that gets
 *	 Success back from this function might reasonably assume that a cel then
 *	 exists and that other cel functions will work.
 ****************************************************************************/
{
	Errcode err;

	if (Success > (err = clip_cel()))
		return err;
	if (thecel == NULL)
		return Err_not_found;
	return Success;

}

static Errcode po_clip_changes(void)
/*****************************************************************************
 * ErrCode CelClipChanges(void);
 *	 In addition to the types of errors the internal clip_cel() routine can
 *	 return to us (no memory, etc), we check to make sure that something
 *	 actually got clipped.	
 ****************************************************************************/
{
	qget_changes();
	if (thecel == NULL)
		return Err_not_found;
	return Success;
}

static void po_release_cel(void)
/*****************************************************************************
 * release thecel, if it exists.
 ****************************************************************************/
{
	if (thecel != NULL)
		noask_delete_the_cel();
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

PolibCel po_libcel = {
po_cel_exists,
	"Boolean CelExists(void);",
po_ink_paste,
	"void    CelPaste(void);",
po_move_cel,
	"void    CelMove(int dx, int dy);",
po_move_cel_to,
	"void    CelMoveTo(int x, int y);",
po_rotate_cel,
	"void    CelTurn(double angle);",
po_rotate_cel_to,
	"void    CelTurnTo(double angle);",
po_cel_next_frame,
	"ErrCode CelNextFrame(void);",
po_cel_back_frame,
	"ErrCode CelBackFrame(void);",
po_cel_set_frame,
	"ErrCode CelSetFrame(int frame);",
po_cel_get_frame,
	"int     CelGetFrame(void);",
po_cel_frame_count,
	"int     CelFrameCount(void);",
po_cel_where,
	"ErrCode CelWhere(int *x, int *y, double *angle);",
po_get_cel,
	"ErrCode CelGet(int x, int y, int width, int height);",
po_clip_cel,
	"ErrCode CelClip(void);",
po_release_cel,
	"void    CelRelease(void);",
po_clip_changes,
	"ErrCode CelClipChanges(void);", 
};

Poco_lib po_cel_lib =
	{
	NULL, "Cel",
	(Lib_proto *)&po_libcel, POLIB_CEL_SIZE,
	};


