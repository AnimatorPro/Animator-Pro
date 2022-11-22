/*****************************************************************************
 * OTDEMO.C - A POE module to demonstrate the use of OverTime.
 *
 *	Major POE items/features demonstrated herein:
 *
 *		- Using OverTime from a POE module.
 *		- Workaround for SetColorMap() bug.
 *		- Simple floating point math in a POE.
 *
 *	This demo is also applicable to the OverAll() and OverSeg() functions.
 *	Either of those could be substituted for OverTime() in the code below
 *	without changing anything else.
 *
 *	The POE version of the OverTime function is rather sparsely documented.
 *	Its parameters are different than those of the OverTime function as
 *	called from a poco program.  Rather than receiving a single value that
 *	varies between 0 and 1 on each call, the POE version uses three values:
 *
 *		ix		- The current frame number (zero-based).
 *		total	- The number of frames to be processed.
 *		scale	- A value that varies from 0 to SCALE_ONE (2^14).
 *
 *	Note that the ix and total parameters are NOT absolute frame numbers,
 *	but rather are relative to the user's requested extents.  If the user
 *	has requested 'To Segment' and the starting frame number of the segment
 *	is 9, the ix value will still be 0 for the first frame, 1 for the next
 *	frame, and so on.
 *
 *	The render_circle() routine, below, demonstrates visually the
 *	relationship between the ix, total, and scale parameters.  It draws a
 *	pair of circles (in different colors) on each frame.  One circle is
 *	drawn based upon the scale parameter, and the other based upon the
 *	ix and total parameters.  If, in the Time dialog, the user has turned
 *	off all the options for varying the time scale (PingPong, In Slow,
 *	Out Slow, etc), the two circles will be drawn on top of each other.
 *	IE, the scale value changes in a linear fashion.  When the user has
 *	selected an option such as In Slow, the scale value changes in a non-
 *	linear fashion, causing the circles to be drawn at different sizes in
 *	each frame, showing the non-linearity.
 *
 *	Most of the time, it is best to use the scale parameter when moving
 *	or sizing objects in an OverTime rendering function.  The user will
 *	expect your rendering to work properly with the In Slow and related
 *	options.
 *
 * NOTES:
 *
 *		All functions for this POE exist in this one source code module.
 *		The functions herein are defined as static if they are called
 *		only from within this module, and as global if they are exported
 *		to the Poco program.
 *
 * MAINTENANCE:
 *
 *	10/25/91	Ian Lepore
 *				Created.
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * include the usual header files...
 *--------------------------------------------------------------------------*/

#include "pocorex.h"    /* required header file, also includes pocolib.h */
#include "errcodes.h"   /* most POE programs will need error codes info  */

/*----------------------------------------------------------------------------
 * set up the host libraries we need...
 *--------------------------------------------------------------------------*/

#define HLIB_TYPE_1 AA_POCOLIB	/* this one is always required in a POE */
#include <hliblist.h>

/*----------------------------------------------------------------------------
 * your data and code goes here...
 *--------------------------------------------------------------------------*/

#define GREEN_IX 2
#define BLUE_IX  3

typedef struct screen_data {
	int size;
	int xcenter;
	int ycenter;
	} ScreenData;

static void render_circles(void *data, int ix, int total, int scale)
/*****************************************************************************
 * for each frame, draw a pair of circles.
 *
 *	one of the circles is sized based on the 'scale' parameter, and the
 *	other is drawn calcing our own scaling based on the current ix and
 *	the total number of frames.  this is done to demonstrate the fact
 *	that it's important to use the scale parameter in such operations,
 *	because it will properly reflect the user's choices in terms of
 *	Ping Pong mode, and so on.
 *
 ****************************************************************************/
{
	double		scaled_radius;
	double		unscaled_radius;
	ScreenData	*sd = data;

	scaled_radius	= 1.0 + ((sd->size * (scale / (double)SCALE_ONE)) / 2.0);
	unscaled_radius = 1.0 + ((sd->size * (ix	/ (double)total))	  / 2.0);

	poeSetColor(GREEN_IX);
	poeCircle(sd->xcenter, sd->ycenter, (int)scaled_radius);

	poeSetColor(BLUE_IX);
	poeCircle(sd->xcenter, sd->ycenter, (int)unscaled_radius);

//	poeQtext(3, 3*sizeof(int),
//		str2ppt("In render_circles, ix=%d, total=%d, scale=%d\n"),
//		ix, total, scale);

}

void make_circle_flic(void)
/*****************************************************************************
 * this is the entry point from poco - it sets up and calls the OverTime.
 *
 * the only tricky thing here is the used of poePicDirtied() to work around
 * a bug in the builtin libs SetColorMap() function.  Changing the color map
 * doesn't signal to the internal routines that the screen/palette changed,
 * so when you change colors in a frame but no pixels, the palette changes
 * "don't take".  By making an explicit PicDirtied() call, we force Ani Pro
 * to see the changes to the palette in the first frame, so that the
 * SetFrameCount() call will copy the modified palette into all the frames
 * it creates.
 ****************************************************************************/

{
	int 	   changes;
	int 	   width;
	int 	   height;
	ScreenData thedata;

	changes = poeGetChangeCount();
	if (changes != 0) {
		if (!poeQquestion(1,sizeof(int),
				str2ppt("You have %d unsaved changes.\n\n"
						"Okay to discard changes and create new flic?"),
				changes)
			)
			return;  // user said no, just punt.
	}

	poeReset();
	poeSetColorMap(GREEN_IX, 0, 255, 0);
	poeSetColorMap(BLUE_IX, 0, 0,	255);
	poePicDirtied();
	poeSetFrameCount(10);

	poeSetInk(str2ppt("OPAQUE"));
	poeSetFilled(FALSE);
	poeSetBrushSize(0);

	poeGetSize(var2ppt(width), var2ppt(height));

	if (builtin_err < Success)	// check status of all the preceding
		return; 				// function calls before continuing.

	thedata.size = (width < height) ? width-5 : height-5;
	thedata.xcenter = width / 2;
	thedata.ycenter = height / 2;

	OverTime(render_circles, &thedata);
}

/*----------------------------------------------------------------------------
 * Setup rexlib/pocorex interface structures...
 *--------------------------------------------------------------------------*/

static Lib_proto poe_calls[] = {
	{ make_circle_flic, "void MakeCircleFlic(void);" },
};

Setup_Pocorex(NOFUNC, NOFUNC, "OverTime Demo POE", poe_calls);
