
/* summa.c - Fetch umouse_x, mouse_buttons etc from summa graphics
   12x12 Summa Sketch tablet.  Basically got to yammer out the
   serial port at it.  */

#include "jimk.h"



static char got_summa;

static
setup_summa()
{
if (!to_tablet(0x20))
	return(0);	/* sync up autobaud */
if (!to_tablet(0))
	return(0);		/* reset */
flush_comm();
wait_10milli();
if (!to_tablet(0x44))
	return(0);	/* remote request mode */
if (!to_tablet(0x62))
	return(0);    /* coordinates from upper left */
if (!to_tablet(0x11))
	return(0);    /* let her rip */
wait_10milli();
return(1);
}

/* Set up serial port to 9600 baud and see if tablet is there.  (The
   return value is unreliable, returning FALSE when pen is out of
   proximity, and is ignored by Animator.) */
init_summa()
{
union regs r;
UBYTE rbuf[2];

/* initialization of port to 9600 baud odd parity 1 stop 8 bit data... */
/* from Norton's Programmers Guide to IBM PC & PS2 p 228 */
/* 111			buad rate 9600
      01		odd parity
	    0		1 stop bit
		 11		8 bit data
		    = 11101011 = 0xeb */
r.w.dx = vconfg.comm_port;
r.b.al = 0xeb;
r.b.ah = 0;
sysint(0x14, &r, &r);
if (!setup_summa())
	return(0);
got_summa = 1;
return(1);
}

/* Poll the tablet. */
summa_get_input()
{
UBYTE rbuf[6];

if (usemacro)
	return;
to_tablet(0x50);	/* hey babe, where's the cursor? */
if (read_tablet(rbuf, 5) )
	{
	if (rbuf[0] & 0x80)
		{
		mouse_button = rbuf[0] & 0x3;
		if (vconfg.pucky4)	/* puck??? reverse buttons. */
			mouse_button = ((mouse_button&1)<<1)+((mouse_button&2)>>1);
		/* scale coordinates to be in the same range we get from mouse.
			IE 4 times 320x200 pixel coordinates */
		umouse_x = rbuf[1] + (rbuf[2]<<7);
		umouse_y = rbuf[3] + (rbuf[4]<<7);
		umouse_x >>= 2;
		umouse_y = sscale_by(umouse_y, 220, 1000);
		umouse_x -= 30;
		umouse_y -= 40;
		}
	else	/* out of phase */
		{
		setup_summa();
		}
	}
else
	setup_summa();
}


cleanup_summa()
{
if (got_summa)
	{
	to_tablet(0x13);	/* SHADDUP already... */
	got_summa = 0;
	}
}

