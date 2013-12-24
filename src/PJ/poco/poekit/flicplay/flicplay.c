/*****************************************************************************
 *
 ****************************************************************************/

#include "flicplay.h"

/*----------------------------------------------------------------------------
 * set up the host libraries we need...
 *--------------------------------------------------------------------------*/

#define HLIB_TYPE_1 AA_POCOLIB
#define HLIB_TYPE_2 AA_SYSLIB
#define HLIB_TYPE_3 AA_GFXLIB
#define HLIB_TYPE_4 AA_STDIOLIB

#include <hliblist.h>

/*----------------------------------------------------------------------------
 * your data and code goes here...
 *--------------------------------------------------------------------------*/

/*****************************************************************************
 * the following 3 functions are provided for the FLIC386P library;
 * some library functions call these, we just vector them through to the
 * appropriate hostlib routines.
 ****************************************************************************/

void *pj_malloc(unsigned amount)
{
	return malloc(amount);
}

void *pj_zalloc(unsigned amount)
{
	return zalloc(amount);
}

void pj_free(void *block)
{
	free(block);
}

static Errcode internal_error(Errcode err, char *str)
/*****************************************************************************
 * report a fatal internal error, and set builtin_err to Err_reported.
 ****************************************************************************/
{
	poeQerror(1, 4, err, str2ppt("Internal data integrity error in FLICPLAY.POE:\n%s"), str);
	return builtin_err = Err_reported;
}

static Errcode flic_integrity_check(Flic *pflic)
/*****************************************************************************
 * make sure the Flic* we got points to a valid Flic structure.
 * (ie, make sure we didn't get a recast pointer to some other datatype)
 ****************************************************************************/
{
	if (NULL == pflic)
		return builtin_err = Err_null_ref;

	if (IANS_FLIC_MAGIC != pflic->magic)
		return internal_error(Err_wrong_type,
			"Flic handle doesn't point to a valid Flic structure");

	if (NULL == pflic->flifile)
		return internal_error(Err_file_not_open,
			"Flifile structure not attached to Flic structure");

	if (NULL == pflic->root_raster)
		return internal_error(Err_null_ref, "NULL root_raster");

	if (NULL == pflic->playback_raster)
		return internal_error(Err_null_ref, "NULL playback_raster");

	if (NULL == pflic->framebuf)
		return internal_error(Err_null_ref, "NULL framebuf pointer");

	return Success;
}

static Errcode return_flic_info(Flic *pflic,
								Popot pwidth, Popot pheight,
								Popot pspeed, Popot pframes)
/*****************************************************************************
 * return each of the flicinfo values for which we got a non-NULL pointer.
 ****************************************************************************/
{
	Flifile *flifile;

	if (NULL == pflic || NULL == pflic->flifile)
		return internal_error(Err_null_ref,
			"Flic file not properly opened, cannot get info.");

	flifile = pflic->flifile;

	if (NULL != pwidth.pt) {
		if (Success != Popot_bufcheck(&pwidth, sizeof(int)))
			return builtin_err;
		*(int *)pwidth.pt = flifile->hdr.width;
	}

	if (NULL != pheight.pt) {
		if (Success != Popot_bufcheck(&pheight, sizeof(int)))
			return builtin_err;
		*(int *)pheight.pt = flifile->hdr.height;
	}

	if (NULL != pspeed.pt) {
		if (Success != Popot_bufcheck(&pspeed, sizeof(int)))
			return builtin_err;
		*(int *)pspeed.pt = flifile->hdr.speed;
	}

	if (NULL != pframes.pt) {
		if (Success != Popot_bufcheck(&pframes, sizeof(int)))
			return builtin_err;
		*(int *)pframes.pt = flifile->hdr.frame_count;
	}

	return Success;
}

static Popot flic_open(Popot path)
/*****************************************************************************
 * open and validate a flic file.,
 ****************************************************************************/
{
	Errcode err;
	Popot	ppflic = {NULL, NULL, NULL};
	Flic	*pflic;

	if (NULL == path.pt) {
		builtin_err = Err_null_ref;
		return ppflic;
	}

	if (Success > (err = do_flic_open(path.pt, &pflic))) {
		builtin_err = err;
		return ppflic;
	}

	ppflic = ptr2ppt(pflic, 0); /* size 0, creates read-only pointer */

	return ppflic;
}

static Errcode flic_info(Popot path, Popot width, Popot height, Popot speed, Popot frames)
/*****************************************************************************
 * return status and info about a flic file, close flic file before returning.
 * this is the *only* FLICPLAY function that does not abort the Poco program
 * if the flic file can't be opened.  instead, it returns an error status.
 ****************************************************************************/
{
	Errcode err;
	Flic	*pflic;
	Flifile *flifile;

	if (NULL == path.pt) {
		return builtin_err = Err_null_ref;
	}

	if (Success > (err = do_flic_open(path.pt, &pflic))) {
		return err;
	}

	flifile = pflic->flifile;

	err = return_flic_info(pflic, width, height, speed, frames);

	do_flic_close(pflic);

	return err;
}

static Popot flic_open_info(Popot path, Popot width, Popot height, Popot speed, Popot frames)
/*****************************************************************************
 * return info about a flic file, and leave the file open for further work.
 ****************************************************************************/
{
	Popot	ppflic;

	ppflic = flic_open(path);
	if (Success <= builtin_err) {
		return_flic_info(ppflic.pt, width, height, speed, frames);
	}
	return ppflic;
}

static void flic_close(Popot theflic)
/*****************************************************************************
 * close a previously-opened flic file.
 ****************************************************************************/
{
	if (Success > flic_integrity_check(theflic.pt))
		return;
	do_flic_close(theflic.pt);
}

static void flic_rewind(Popot theflic)
/*****************************************************************************
 * rewind a flic file to first frame.
 ****************************************************************************/
{
	if (Success > flic_integrity_check(theflic.pt))
		return;
	do_rewind(theflic.pt);
}

static void flic_seek_frame(Popot theflic, int theframe)
/*****************************************************************************
 * seek the flic to the specified frame.
 ****************************************************************************/
{
	Flic *pflic = theflic.pt;

	if (Success > flic_integrity_check(pflic))
		return;

	if (theframe < 0 || theframe > pflic->num_frames-1) {
		builtin_err = Err_parameter_range;
		return;
	}

	do_seek_frame(theflic.pt, theframe);
}

static void flic_play_options(Popot theflic,
					   int speed, int keyhit, Popot screen, int x, int y)
/*****************************************************************************
 * override current playback options with new values.
 ****************************************************************************/
{
	if (Success > flic_integrity_check(theflic.pt))
		return;

	builtin_err = do_flic_options(theflic.pt, speed, keyhit, screen.pt, x, y);
}

static void flic_play(Popot theflic)
/*****************************************************************************
 * play a flic until the user hits a key or mouse button.
 ****************************************************************************/
{
	if (Success > flic_integrity_check(theflic.pt))
		return;

	builtin_err = do_play(theflic.pt);
}

static void flic_play_once(Popot theflic)
/*****************************************************************************
 * play a flic start-to-finish, once, then stop.
 ****************************************************************************/
{
	if (Success > flic_integrity_check(theflic.pt))
		return;

	builtin_err = do_play_once(theflic.pt);
}

static void flic_play_timed(Popot theflic, int milliseconds)
/*****************************************************************************
 * play the flic for the specified amount of time.
 ****************************************************************************/
{
	if (Success > flic_integrity_check(theflic.pt))
		return;

	builtin_err = do_play_timed(theflic.pt, milliseconds);
}

static void flic_play_count(Popot theflic, int frame_count)
/*****************************************************************************
 * play the specified number of frames.
 ****************************************************************************/
{
	if (Success > flic_integrity_check(theflic.pt))
		return;

	builtin_err = do_play_count(theflic.pt, frame_count);
}

/*----------------------------------------------------------------------------
 * Setup rexlib/pocorex interface structures...
 *--------------------------------------------------------------------------*/

static Lib_proto poe_calls[] = {

	{NULL,				"typedef struct __flic_handle__ Flic;"},

	{flic_info, 		"Errcode FlicInfo(char *path, int *width, "
									"int *height, int *speed, int *frames);"},
	{flic_open_info,	"Flic    *FlicOpenInfo(char *path, int *width, "
									"int *height, int *speed, int *frames);"},
	{flic_open, 		"Flic    *FlicOpen(char *path);"},
	{flic_close,		"void    FlicClose(Flic *theflic);"},
	{flic_rewind,		"void    FlicRewind(Flic *theflic);"},
	{flic_seek_frame,	"void    FlicSeekFrame(Flic *theflic, int theframe);"},
	{flic_play_options, "void    FlicOptions(Flic *theflic, "
									"int speed, int keyhit_stops_playback, "
									"Screen *playback_screen, "
									"int xoffset, int yoffset);"},
	{flic_play, 		"void    FlicPlay(Flic *theflic);"},
	{flic_play_once,	"void    FlicPlayOnce(Flic *theflic);"},
	{flic_play_timed,	"void    FlicPlayTimed(Flic *theflic, int milliseconds);"},
	{flic_play_count,	"void    FlicPlayCount(Flic *theflic, int frame_count);"},

};

Setup_Pocorex(NOFUNC, do_flic_close_all, "FlicPlay Library v1.0", poe_calls);

