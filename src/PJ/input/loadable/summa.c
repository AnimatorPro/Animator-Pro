
/* summa.c - Summa tablet driver (12x12") for Animator Pro. */

#include "errcodes.h"
#include "idriver.h"
#include "regs.h"
#include "ptrmacro.h"
#include "syslib.h"


	/* Current mouse x/y value in driver coordinates */
static LONG sum_pos[2] = {5848/2, 5848/2};
	/* Smallest mouse x/y value in driver coordinates */
static LONG sum_min[2] = {100,100};
	/* Largest mouse x/y value. */ 
static LONG sum_max[2] = {5848-100,5848-100};
	/* Space for AniPro to store it's clipping data. */
static LONG sum_clip[2];		
	/* Aspect ratio of driver coordinates. */
static LONG sum_aspect[2] = {1,1};
	/* Flags to tell AniPro about our channels (absolute). */
static UBYTE sum_flags[2] = {0,0};


static Idr_option sum_opts[] =
/*-----------------------------------------------------------------------
 * Text for menu to ask user about his configuration.  Also default
 * configuration info.
 *----------------------------------------------------------------------*/
{
	{
		RL_KEYTEXT("summa_opt0")
		"Summa input type\n"
		"Stylus\n"
		"Puck\n"
		"Cancel" ,
		0x0003, /* enable either 0 or 1 (bits 0,1 on) */
		0,	/* default mode */
	},
};

static void wait_milli(ULONG t)
/*-----------------------------------------------------------------------
 * Wait some milliseconds.
 *----------------------------------------------------------------------*/
{
	t += pj_clock_1000();
	while(t > pj_clock_1000());
}


static Errcode sum_detect(Idriver *idr)
/*-----------------------------------------------------------------------
 * Sadly we can't really tell if tablet is attatched since some of the
 * more brain-dead Summa models act the same if the pen is out of proximity
 * as they do if they are turned off.
 * So this routine basically just checks for serial port TIMEOUT.
 *----------------------------------------------------------------------*/
{
	SHORT port = idr->comm_port;


	/*  autobaud detection */
	if(!to_tablet(port, ' ')) {
	    return Err_timeout;
	}
	/*  reset */
	if (!to_tablet(port, '\0')) {
	    return Err_timeout;
	}
	/*  empty serial port */
	flush_comm(port);

	/*  wait for reset to finish */
	wait_milli(15);

	return Success;
}

static Errcode sum_inquire(Idriver *idr)
/*-----------------------------------------------------------------------
 * Return information about input device and driver.
 *----------------------------------------------------------------------*/
{
	idr->button_count = 2;
	idr->channel_count = 2;		/* 2 channels, x and y.  (No pressure). */
	idr->min = sum_min;
	idr->max = sum_max;
	idr->aspect = sum_aspect;
	idr->pos = sum_pos;
	sum_clip[0] = sum_max[0];
	sum_clip[1] = sum_max[1];
	idr->clipmax = sum_clip;
	idr->flags = sum_flags;
	return(Success);
}


static int to_tablet(SHORT port, UBYTE c)
/*-----------------------------------------------------------------------
 * Send a byte out serial port to tablet.
 *----------------------------------------------------------------------*/
{
union abcd_regs r;

	r.w.dx = port;
	r.b.ah = 1;
	r.b.al = c;
	jcomm(&r);
	if ((r.b.ah & 0x80) ==	0)
		return(1);
	else
		return(0);
}

static int read_tablet(SHORT port, UBYTE *c, int count)
/*-----------------------------------------------------------------------
 *----------------------------------------------------------------------*/
{
union abcd_regs r;

	while (--count >= 0)
	{
		r.w.dx = port;
		r.b.ah = 2;
		jcomm(&r);
		if ((r.b.ah & 0x80) !=	0)
			return(0);
		*c++ = r.b.al;
	}
	return(1);
}

static int comm_ready(SHORT port)
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

static void flush_comm(SHORT port)
/*-----------------------------------------------------------------------
 * Read any pending characters from comm port and throw them away.
 *----------------------------------------------------------------------*/
{
char dummy;

	while(comm_ready(port))
	{
		if (!read_tablet(port, &dummy, 1))
			return;
	}
}

static void setup_summa(Idriver *idr)
/*-----------------------------------------------------------------------
 * This routine sends codes to reset the tablet to a known state
 * and to go into a mode where it sends coordinates when we request them
 * (as opposed to when it feels like, er when the user moves the pen.)
 * It is called when the driver starts up,  and whenever the main input
 * routine decides that we've lost sync with the tablet.
 *----------------------------------------------------------------------*/
{
SHORT port = idr->comm_port;

	flush_comm(port);
	to_tablet(port,0x20);	/* sync up autobaud */
	to_tablet(port,0);		/* reset */
	flush_comm(port);
	wait_milli(15);
	to_tablet(port, 0x44);	/* remote request mode */
	to_tablet(port, 0x62);	  /* coordinates from upper left */
	to_tablet(port, 0x11);	  /* let her rip */
	wait_milli(15);
}

static init_comm_port(int port)
/*-----------------------------------------------------------------------
 * initialization of port to 9600 baud odd parity 1 stop 8 bit data... 
 * from Norton's Programmers Guide to IBM PC & PS2 p 228 
 * 111			buad rate 9600
 *    01		odd parity
 *	    0		1 stop bit
 *		 11		8 bit data
 *		    = 11101011 = 0xeb 
 *----------------------------------------------------------------------*/
{
union abcd_regs r;

	r.w.dx = port;
	r.b.al = 0xeb;
	r.b.ah = 0;
	jcomm(&r);
	return(r.b.ah != 0);
}

static char got_summa;		/* Flag variable used to insure driver isn't
							 * opened twice.  (Would be an AniPro bug,
							 * but you can never be too sure about the
							 * state of mind of your friendly host.)
							 */

static Errcode sum_open(Idriver *idr)
/*-----------------------------------------------------------------------
 * Called to start using input driver.  Return Success or negative
 * status code.
 *----------------------------------------------------------------------*/
{
	Errcode     err;

	sum_inquire(idr);

	/*  set things up to the right baud rate */
	if(!init_comm_port(idr->comm_port))
		return(Err_no_device);

	/*  check for possible timeout (cable broken, etc.) */
	if ((err = sum_detect(idr)) != Success) {
	    return err;
	}

	setup_summa(idr);
	got_summa = TRUE;
	return(Success);
}

static Errcode sum_close(Idriver *idr)
/*-----------------------------------------------------------------------
 * Called when through using input driver.
 *----------------------------------------------------------------------*/
{
	if(got_summa)
	{
		to_tablet(idr->comm_port, 0x13);	/* SHADDUP already... */
		got_summa = FALSE;
	}
	return(Success);
}

static Errcode sum_input(Idriver *idr)
/*-----------------------------------------------------------------------
 * AniPro calls this to ask for the current mouse/tablet position,
 * button state, etc.
 *----------------------------------------------------------------------*/
{
int mb;
UBYTE rbuf[6], *c;
SHORT port = idr->comm_port;
int i;


	to_tablet(port, 0x50);	/* hey babe, where's the cursor? */
	if (read_tablet(port, rbuf, 5) )
	{
		/*  expect header byte to have MSB set */
		if (rbuf[0] & 0x80)
		{
			/*  expect rest of bytes to have MSB clear */
			for (i = 0, c = &(rbuf[1]); i < 4; i++, c++) {
			    if (*c & 0x80) {
				/*  out of phase */
				setup_summa(idr);
				return Success;
			    }
			}
			if (sum_opts[0].mode == 1) {
				/* puck */
				mb = rbuf[0] & 0x07;
				if (!mb) {
				    mb = 0;
				} else if (mb > 2) {
				    mb = 2;
				} else {
				    mb = 1;
				}
			} else {
				/* stylus */
				mb = rbuf[0] & 0x03;
			}
			idr->buttons = mb;
			idr->pos[0] = rbuf[1] + (rbuf[2]<<7);
			idr->pos[1] = rbuf[3] + (rbuf[4]<<7);
		}
		else	/* out of phase */
		{
			setup_summa(idr);
		}
	}
	else
		setup_summa(idr);


	return(Success);
}


static Errcode sum_setclip(Idriver *idr,long channel,long clipmax)
/*-----------------------------------------------------------------------
 * AniPro calls this to inform input device the dimensions of the screen
 * in input device coordinates.
 *----------------------------------------------------------------------*/
{
	if((USHORT)channel > idr->channel_count)
		return(Err_bad_input);

	if(((ULONG)clipmax) > sum_max[channel])
		clipmax = sum_max[channel];
	sum_clip[channel] = clipmax;
	return(0);
}

static Idr_library sum_lib = {
/*-----------------------------------------------------------------------
 * Jump-table of functions device driver provides Ani Pro.
 *----------------------------------------------------------------------*/
	sum_detect,
	sum_inquire,
	sum_input,
	sum_setclip,
	};


	/* This tells the linker that we want to use some of the functions in
	 * syslib.h. */
Hostlib _a_a_syslib	 = { NULL, AA_SYSLIB, AA_SYSLIB_VERSION};

Idriver rexlib_header = {
/*-----------------------------------------------------------------------
 * This is AniPro's handle on the device.  
 *----------------------------------------------------------------------*/
	{ REX_IDRIVER, IDR_VERSION, sum_open, sum_close, &_a_a_syslib },
	NULL,
	&sum_lib,				/* Library. */
	sum_opts,				/* Options. */
	Array_els(sum_opts),	/* Number of options. */
	FALSE,					/* Checks keyboard too? */
};

