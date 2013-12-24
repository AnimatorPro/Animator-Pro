/* wacom.c - Animator Pro driver for Wacom tablet.  This is an example of
 * a device with 3 channels, one of them being pressure on the pen. */

#include "ptrmacro.h"
#include "errcodes.h"
#include "idriver.h"
#include "regs.h"
#include "syslib.h"


#define WAC_PRESSURE	0x10
#define WAC_STYLUS	0x20
#define WAC_INSIDE	0x40
#define WAC_PSTYLUS	WAC_PRESSURE | WAC_STYLUS
#define WAC_BUTTONDOWN	0x20
#define WAC_BUTTONMASK	0x1f


	/* Maximum concievable pressure value. */
#define PMAX 256

	/* We store the minimum and maximum pressure values we see here,
	 * since the range of pressure the tablet transmits seems to
	 * vary with the weather.... */
int pmin = -10, pmax = 10;

	/* We wait for a pressure at least PTHRESH over pmin before
	 * signalling "left button down". */
#define PTHRESH 5

	/* Current mouse x/y value in driver coordinates */
LONG wac_pos[3] = {2048/2, 2048/2, 0};
	/* Smallest mouse x/y value in driver coordinates */
LONG wac_min[3] = {0,0,0};
	/* Largest mouse x/y value. */ 
LONG wac_max[3] = {2048,2048,PMAX};
	/* Space for AniPro to store it's clipping data. */
LONG wac_clip[3] = {2048, 2048, PMAX};
	/* Aspect ratio of driver coordinates. */
LONG wac_aspect[3] = {1,1,1};
	/* Flags to tell AniPro about our channels. */
UBYTE wac_flags[3] = {0,0,PRESSURE};


	/* What mode tablet is in?  In this case what pointing device is
	 * being used. */
#define PSTYLUS 0
#define PUCK 1
#define STYLUS 2

Idr_option wac_opts[] =
/*-----------------------------------------------------------------------
 * Text for menu to ask user about his configuration.  Also default
 * configuration info.
 *----------------------------------------------------------------------*/
{
	{
		/* mode qchoice text */
		RL_KEYTEXT("wacom_opt0")
		"Select device type\n"
		"Pressure sensitive stylus\n"
		"Side button stylus\n"
		"Puck\n"
		"Cancel",
		0x0007, 	      /* modes (bits) 0,1,2 enabled */
		PSTYLUS,	      /* default mode */
	},
};

int to_tablet(SHORT port, UBYTE c)
/*-----------------------------------------------------------------------
 * Send a byte out serial port to tablet.
 *----------------------------------------------------------------------*/
{
    union abcd_regs r;

    r.w.dx = port;
    r.b.ah = 1;
    r.b.al = c;
    jcomm(&r);
    if ((r.b.ah & 0x80) ==  0)
	return(1);
    else
	return(0);
}

int comm_ready(SHORT port)
/*-----------------------------------------------------------------------
 * See if comm port has any characters ready to read.
 *----------------------------------------------------------------------*/
{
    union abcd_regs r;

    r.w.dx = port;
    r.b.ah = 3;
    jcomm(&r);
    return(r.b.ah&1);
}


void flush_comm(SHORT port)
/*-----------------------------------------------------------------------
 * Read any pending characters from comm port and throw them away.
 *----------------------------------------------------------------------*/
{
    union abcd_regs r;

    while (comm_ready(port))
    {
	r.w.dx = port;
	r.b.ah = 2;
	jcomm(&r);
	if ((r.b.ah & 0x80) !=	0)
	    return;
    }
}


string_to_tablet(SHORT port, char *s)
/*-----------------------------------------------------------------------
 * Send zero terminated string out serial port.
 *----------------------------------------------------------------------*/
{
    while (*s != 0)
    {
	if (!to_tablet(port, *s++))
	    return(0);
    }
    return(1);
}

void wait_milli(ULONG t)
/*-----------------------------------------------------------------------
 * Wait some milliseconds.
 *----------------------------------------------------------------------*/
{
    t += pj_clock_1000();
    while(t > pj_clock_1000());
}

slow_to_wacom(SHORT port, char *s)
/*-----------------------------------------------------------------------
 * Send a string out serial port with a delay afterwards.
 *----------------------------------------------------------------------*/
{
    if (!string_to_tablet(port,s))
	return(0);
    wait_milli(150);
    return(1);
}

static UBYTE wac_initted = 0;	/* Have we initialized tablet? */


Errcode init_wacom(Idriver *idr)
/*-----------------------------------------------------------------------
 * This routine sends codes to reset the tablet to a known state
 * and to go into a mode where it sends coordinates when we request them
 * (as opposed to when it feels like, er when the user moves the pen.)
 * It is called when the driver starts up,  and whenever the main input
 * routine decides that we've lost sync with the tablet.
 *----------------------------------------------------------------------*/
{
    union abcd_regs r;
    SHORT port = idr->comm_port;


    if (wac_initted)
		return(Success);


    /* Set things up to the right baud rate. */
	/* Initialization of port to 9600 baud odd parity 1 stop 8 bit data.... */
	/* from Norton's Programmers Guide to IBM PC & PS2 p 228 */
	/* 111			buad rate 9600
	      00		no parity
		    0		1 stop bit
			 11		8 bit data
			    = 11100011 = 0xe3 */
    r.w.dx = port;
    r.b.al = 0xe3;
    r.b.ah = 0;
    jcomm(&r);

    /*	don't emulate anybody, be a WACOM */
    if (!slow_to_wacom(port, "$\r\n"))
		return(Err_timeout);

    if (!slow_to_wacom(port, "SP\r\n"))     /* stop transmission */
		return(Err_timeout);

    if (!slow_to_wacom(port, "AS1\r\n"))    /* binary mode packets */
		return(Err_timeout);

    /* Send reply even if pen out of proximity */
    if (!slow_to_wacom(port, "AL1\r\n"))    /* always make data ready */
		return(Err_timeout);

    if (!slow_to_wacom(port, "OC1\r\n"))    /* origin at upper left */
		return(Err_timeout);

    if (!slow_to_wacom(port, "IC0\r\n"))    /* units in millimeters */
		return(Err_timeout);

    if (!slow_to_wacom(port, "SU0\r\n"))    /* set no increment distance */
		return(Err_timeout);

    /* enable pressure sensitivity or not */
    if (idr->options[0].mode == PSTYLUS)
    {
	if (!slow_to_wacom(port,"PH1\r\n"))
	    return(Err_timeout);	    /* enable pressure sense */
    }
    else
    {
	if (!slow_to_wacom(port, "PH0\r\n"))
	    return(Err_timeout);	    /* disable pressure sense */
    }

    /* Send scale signal.  Somehow tablet doesn't seem to get real margins,
       so have to make scale a little bigger for tablet than Animator */
    if (!slow_to_wacom(port, "SC2504,2154\r\n"))    /* set up scale */
		return(Err_timeout);
    if (!slow_to_wacom(port, "RQ1\r\n"))    /* and request 1 input */
		return(Err_timeout);
    wac_initted = TRUE;
    return(Success);
}


Errcode wac_detect(Idriver *idr)
/*-----------------------------------------------------------------------
 * See if the input device is attatched.   Return Success (0) if
 * so,  otherwise a negative error code.  (see errcodes.h).
 *----------------------------------------------------------------------*/
{
    Errcode err;

    if ((err = init_wacom(idr)) < Success)
	return(err);
    /* flush any garbage left at comm port from last time... */
    flush_comm(idr->comm_port);
    return(wac_input(idr));
}

Errcode wac_inquire(Idriver *idr)
/*-----------------------------------------------------------------------
 * Return information about input device and driver.
 *----------------------------------------------------------------------*/
{
    idr->button_count = 2;
    if (idr->options[0].mode == PSTYLUS)
	idr->channel_count = 3;
    else
	idr->channel_count = 2;
    idr->min = wac_min;
    idr->max = wac_max;
    idr->aspect = wac_aspect;
    idr->pos = wac_pos;
    idr->clipmax = wac_clip;
    idr->flags = wac_flags;
    return(Success);
}

Errcode wac_close(Idriver *idr)
/*-----------------------------------------------------------------------
 * Called when through using input driver.
 *----------------------------------------------------------------------*/
{
    wac_initted = FALSE;
    return(Success);
}

Errcode wac_setclip(Idriver *idr,long channel,long clipmax)
/*-----------------------------------------------------------------------
 * AniPro calls this to inform input device the dimensions of the screen
 * in input device coordinates.
 *----------------------------------------------------------------------*/
{
    if((USHORT)channel > idr->channel_count)
	return(Err_bad_input);

    if(((ULONG)clipmax) > wac_max[channel])
	clipmax = wac_max[channel];
    wac_clip[channel] = clipmax;
    return(0);
}


Errcode wac_open(Idriver *idr)
/*-----------------------------------------------------------------------
 * Called to start using input driver.  Return Success or negative
 * status code.
 *----------------------------------------------------------------------*/
{
    wac_inquire(idr);
/* set things up to the right baud rate */
    return(wac_detect(idr));
}


Errcode wac_input(Idriver *idr)
/*-----------------------------------------------------------------------
 * AniPro calls this to ask for the current mouse/tablet position,
 * button state, etc.
 *----------------------------------------------------------------------*/
{
    static char wlinebuf[7];
    char *s;
    union abcd_regs r;
    int stype;
    int dp;
    static int ux = 1024, uy = 512, temp = 0;
    static int ux_save = 1024, uy_save = 512, temp_save = 0;
    SHORT port = idr->comm_port;
    int i;


request_input:

    s = wlinebuf;

    /*	request 7-byte input */
    if (!string_to_tablet(port, "RQ1\n"))
    {
	return(Err_timeout);
    }

    /*	attempt to receive the header byte (it has it's MSB set) */
    do {

	/*  receive bytes */
	r.w.dx = port;
	r.b.ah = 2;
	jcomm(&r);
	if ((r.b.ah & 0x80) !=	0) {
	    return(Err_timeout);
	}

    /* until a header byte is received */
    } while (!(r.b.al & 0x80));

    /*	point 's' to the sample buffer */
    s = wlinebuf;

    /*	save header byte */
    *s++ = r.b.al;

    /*	get rest of bytes */
    for (i = 0; i < 6; i++) {

	/*  receive bytes */
	r.w.dx = port;
	r.b.ah = 2;
	jcomm(&r);
	if ((r.b.ah & 0x80) !=	0) {
	    return(Err_timeout);
	}

	/*  if a header byte is detected, then go back and request again */
	if (r.b.al & 0x80) {
	    goto request_input;
	}

	/*  put the in the sample buffer */
	*s++ = r.b.al;
    }

    s = wlinebuf;

    switch (*s & WAC_PSTYLUS) {
    case 0:
	stype = PUCK;
	break;
    case WAC_STYLUS:
	stype = STYLUS;
	break;
    case WAC_PSTYLUS:
	stype = PSTYLUS;
	break;
    default:
	flush_comm(port);
	return(Err_version);
    }


    /*	if pointing device is inside the tablet */
    if (*s & WAC_INSIDE) {

	/*  compute x-value and scale and clamp it */

	ux = ((wlinebuf[0] & 3) << 14) + (wlinebuf[1] << 7) + wlinebuf[2];
	ux -= 120;

	uy = ((wlinebuf[3] & 3) << 14) + (wlinebuf[4] << 7) + wlinebuf[5];
	uy -= 120;

	/*  get pressure/button field */

	temp = wlinebuf[6];

	ux_save = ux;
	uy_save = uy;
	temp_save = temp;

    } else {

	/*  pointing device is not inside the tablet, use last good value */

	ux = ux_save;
	uy = uy_save;
	temp = temp_save;

    }



    switch (stype)
    {
    case PSTYLUS:

	/*  temp is a 7-bit signed pressure value */
	if (temp & 0x40) {		/* extend sign bit if necessary */
	    temp |= 0xffffff80;
	}

	idr->buttons = 0;
	wac_pos[2] = 0;

	if (temp > -99 && temp < 99)  /* filter to reasonable values */
	{
	    wac_pos[0] = ux;
	    wac_pos[1] = uy;
	    if (temp < pmin)
		pmin = temp;
	    if (temp > pmax)
		pmax = temp;
	    dp = pmax-(pmin+PTHRESH);
	    temp -= pmin+PTHRESH;
	    if (temp > 0)
	    {
		wac_pos[2] = temp*PMAX/dp;
		idr->buttons = 1;
	    }
	}

	/* check for control key to interpret a right click
		(Arr, no side button on pressure sensitive stylus!) */
	if (jkey_shift() & 0x4)       /* right button on control key */
	    idr->buttons |= 0x2;
	break;

    case STYLUS:

	wac_pos[0] = ux;
	wac_pos[1] = uy;
	idr->buttons = temp & 0x03;
	break;

    case PUCK:
	wac_pos[0] = ux;
	wac_pos[1] = uy;
	/* left and top buttons are left mouse button, others are right
	   mouse button */
	idr->buttons = 0;
	temp &= WAC_BUTTONMASK;
	if (temp == 1 || temp == 2) {
	    idr->buttons |= 1;
	} else if (temp == 3 || temp == 4) {
	    idr->buttons |= 2;
	}
	break;
    }
    return(Success);
}

Idr_library wac_lib = {
/*-----------------------------------------------------------------------
 * Jump-table of functions device driver provides Ani Pro.
 *----------------------------------------------------------------------*/
	wac_detect,
	wac_inquire,
	wac_input,
	wac_setclip,
    };

	/* This tells the linker that we want to use some of the functions in
	 * syslib.h. */
Hostlib _a_a_syslib	 = { NULL, AA_SYSLIB, AA_SYSLIB_VERSION};

Idriver rexlib_header = {
/*-----------------------------------------------------------------------
 * This is AniPro's handle on the device.  
 *----------------------------------------------------------------------*/
	{ REX_IDRIVER, IDR_VERSION, wac_open, wac_close, &_a_a_syslib },
	NULL,
	&wac_lib,
	wac_opts,		      /* options */
	Array_els(wac_opts),	      /* num options */
       };
