/* Mmouse.c - source for Autodesk Animator Pro alternative
 * Microsoft compatible mouse driver.
 *
 */

#include "errcodes.h"
#include "idriver.h"
#include "regs.h"
#include "ptrmacro.h"


static LONG mou_pos[2];		/* Current mouse x/y value in driver coordinates */
static LONG	mou_min[2];		/* Smallest mouse x/y value in driver coordinates */
static LONG mou_max[2];		/* Largest mouse x/y value. */
static LONG mou_clip[2];	/* Space for AniPro to store it's clipping data. */
static LONG mou_aspect[2] = {1,1};	/* Aspect ratio of driver coordinates. */
static UBYTE mou_flags[2] = {RELATIVE,RELATIVE}; /* It's a relative device. */

static scale_mouse(UBYTE mode)
/*-----------------------------------------------------------------------
 * Set up how mouse units correspond to pixel units.   
 * This takes a fairly indirect route to scaling the size of mouse input.
 * The driver always reports the same mouse-coordinates no matter
 * what the scale is (slow, medium, or fast),  but tells AniPro
 * the range of mouse values is greater for a slow mouse.
 *----------------------------------------------------------------------*/
{
static SHORT scaler[] = {4,2,1};	/* Correspond to slow, medium, fast. */
SHORT scale = scaler[mode];

mou_clip[0] = mou_clip[1] = mou_max[0] = mou_max[1] = 1024*scale;
mou_pos[0] = mou_pos[1] = 512*scale;	/* Mouse starts in middle of screen. */
}

static Errcode mou_detect(Idriver *idr)
/*-----------------------------------------------------------------------
 * See if the input device is attatched.   Return Success (0) if
 * so,  otherwise a negative error code.  (see errcodes.h).
 *----------------------------------------------------------------------*/
{
if (jgot_mouse())
	return(Success);
else
	return(Err_no_device);
}

static Idr_option mou_opts[] =
/*-----------------------------------------------------------------------
 * Configuration options.  There may be up to four of these,  each
 * of which brings up a "numbered choice" requestor in AniPro.
 *----------------------------------------------------------------------*/
{
	{
		/* mode qchoice text */
		RL_KEYTEXT("mmouse_opt0")		/* Optional index into aa.mu */
		"Select mouse speed:\n" 		/* First line - header of menu. */
	  	"Slow\n"						/* 	mode 0 choice. */
	  	"Medium\n"						/* 	mode 1 choice. */
		"Fast\n"						/*  mode 2 choice. */
	  	"Cancel", 
		0x0007,   /* bits (modes) 0,1,2 enabled */
		1,   /* default mode (medium) */
	},
#define MOU_LEFTY (mou_opts[1].mode)
	{ 
		/* mode qchoice text */
		RL_KEYTEXT("mmouse_opt1")
		"Swap buttons for lefty?\n"
	  	"No\n"
	  	"Yes\n"
	  	"Cancel", 
		0x0003,   /* bits (modes) 0,1,enabled */
		0,   /* default mode (No) */
	},
};

static Errcode mou_inquire(Idriver *idr)
/*-----------------------------------------------------------------------
 * Return information about input device and driver.
 *----------------------------------------------------------------------*/
{
	scale_mouse(mou_opts[0].mode);
	idr->button_count = 2;
	idr->channel_count = 2;			/* Would be 3 if pressure sensitive. */
	idr->min = mou_min;				/* Array of min values, one for each 
									 * channel. */
	idr->max = mou_max;				/* A max value for each channel. */
	mou_clip[0] = mou_max[0];
	mou_clip[1] = mou_max[1];
	idr->clipmax = mou_clip;		/* Clip values (AniPro sets these
									 * via calls to mou_setclip,  since
									 * the digitizing tablet may be
									 * square but the screen a rectangle. */
	idr->aspect = mou_aspect;		/* Aspect ratio of device coordinates. */
	idr->pos = mou_pos;				/* Current position. */
	idr->flags = mou_flags;			/* Relative/Absolute channel? */
	return(Success);
}

static Errcode mou_open(Idriver *idr)
/*-----------------------------------------------------------------------
 * Called to start using input driver.  Return Success or negative
 * status code.
 *----------------------------------------------------------------------*/
{
Errcode err;

	if ((err = mou_detect(idr)) < Success)
		return(err);
	return(mou_inquire(idr));
}

static Errcode mou_close(Idriver *idr)
/*-----------------------------------------------------------------------
 * Called when through using input driver.
 *----------------------------------------------------------------------*/
{
	return(Success);
}

static Errcode mou_input(Idriver *idr)
/*-----------------------------------------------------------------------
 * AniPro calls this to ask for the current mouse/tablet position,
 * button state, etc.
 *----------------------------------------------------------------------*/
{
static struct wabcd_regs mrg;

	mrg.ax = 3;
	jmousey(&mrg);
	/* Extract the button bits */
	if(MOU_LEFTY == TRUE)
	{
		if(mrg.bx & 0x0001)
			idr->buttons = 0x0002;
		else
			idr->buttons = 0;

		if(mrg.bx & 0x0002)
			idr->buttons = 0x0001; 
	}
	else
	{
		idr->buttons = mrg.bx & (0x0003); 
	}
	mrg.ax = 11;
	jmousey(&mrg);
	mou_pos[0] += (SHORT)(mrg.cx);
	mou_pos[1] += (SHORT)(mrg.dx);
	return(Success);
}

static Errcode mou_setclip(Idriver *idr,SHORT channel,long clipmax)
/*-----------------------------------------------------------------------
 * AniPro calls this to inform input device the dimensions of the screen
 * in input device coordinates.
 *----------------------------------------------------------------------*/
{
	if((USHORT)channel > idr->channel_count)
		return(Err_bad_input);

	if(((ULONG)clipmax) > mou_max[channel])
		clipmax = mou_max[channel];
	mou_clip[channel] = clipmax;
	return(0);
}

static Idr_library mou_lib = {
/*-----------------------------------------------------------------------
 * Jump-table of functions device driver provides Ani Pro.
 *----------------------------------------------------------------------*/
	mou_detect,
	mou_inquire,
	mou_input,
	mou_setclip,
	};

Idriver rexlib_header = {
/*-----------------------------------------------------------------------
 * This is AniPro's handle on the device.  
 *----------------------------------------------------------------------*/
	{ REX_IDRIVER, IDR_VERSION, mou_open, mou_close, NULL },
	NULL,
	&mou_lib,				/* Library. */
	mou_opts,				/* Options. */
	Array_els(mou_opts),    /* Number of options. */
	FALSE,					/* Checks keyboard too? */
};

