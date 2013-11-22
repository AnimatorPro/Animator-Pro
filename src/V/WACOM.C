

/* wacom.c - Fetch umouse_x, mouse_buttons etc from wacom 
   12x12 tablet.  Basically got to yammer out the serial port at it.  
   Use stuff in tablet.c. */

#include <stdio.h>
#include "jimk.h"

#ifdef WACOM
#ifdef OLD
extern WORD umouse_x;	/* x coordinate value 0 to (319*4) in active area*/
extern WORD umouse_y;	/* y coordinate value 0 to (199*4) in active area */
extern WORD mouse_button;	/* bit 0 for left bit 1 for right button */
extern WORD pressure_sensitive;	/* 1 if pressure sensitive device, 0 else */
extern WORD pressure;	/* pressure value scaled from 1 to 255 */
#endif OLD



/* send zero terminated string out serial port */
static
string_to_tablet(s)
char *s;
{
while (*s != 0)
	{
	if (!to_tablet(*s++))
		return(0);
	}
return(1);
}

static int wdelay = 10;		/* number of vblanks to wait for tablet */

static
slow_to_wacom(s)
char *s;
{
int i;

if (!string_to_tablet(s))
	return(0);
i = wdelay;
while (--i >= 0)
	wait_sync();
return(1);
}

#define PSTYLUS 0
#define PUCK 1
#define STYLUS 2


/* initialization of port to 9600 baud odd parity 1 stop 8 bit data... */
/* from Norton's Programmers Guide to IBM PC & PS2 p 228 */
/* 111			buad rate 9600
      00		no parity
	    0		1 stop bit
		 11		8 bit data
		    = 11100011 = 0xe3 */
init_wacom()
{
union regs r;

/* set things up to the right baud rate */
r.w.dx = vconfg.comm_port;
r.b.al = 0xe3;
r.b.ah = 0;
sysint(0x14, &r, &r);
if (!slow_to_wacom("$\r\n"))	/* Don't emulate anybody, be a Wacom 2 */
	return(0);
if (!slow_to_wacom("SR\r\n"))	/* In stream mode */
	return(0);
if (!slow_to_wacom("AS0\r\n"))	/* Ascii mode packets */
	return(0);
if (!slow_to_wacom("AL1\r\n"))	/* Send reply even if pen out of proximity */
	return(0);
if (vconfg.pucky4 == PSTYLUS)
	{
	if (!slow_to_wacom("PH1\r\n"))
		return(0);		/* Enable pressure sensitivity */
	}
else
	{
	if (!slow_to_wacom("PH0\r\n"))
		return(0);
	}
if (!slow_to_wacom("SC1400,1300\r\n"))	/* set up scale */
	return(0);
if (!slow_to_wacom("RQ1\r\n"))	/* and request 1 input */
	return(0);
return(wacom_get_input());
}

static char *
skip_past_comma(s)
char *s;
{
while (*s++ != ',')
	;
return(s);
}

static pmin = -10, pmax = 10;
#define PTHRESH 5

wacom_get_input()
{
static char wlinebuf[32];
char *s = wlinebuf;
int count = sizeof(wlinebuf)-2;
union regs r;
int stype;
int temp;
int ux,uy;
char c;

/* flush any garbage left at comm port from last time... */
flush_comm();
if (!string_to_tablet("RQ1\n"))
	return(0);
while (--count >= 0)
	{
	r.w.dx = vconfg.comm_port;
	r.b.ah = 2;
	sysint(0x14,&r,&r);
	if ((r.b.ah & 0x80) !=  0)
		{
		return(0);
		}
	if ((*s++ = r.b.al) == '\n')
		break;
	}
*s++ = 0;
s = wlinebuf;
switch (*s++)	/* see what first character of packet says it is */
	{
	case '!':
		stype = PSTYLUS;
		break;
	case '#':
		stype = STYLUS;
		break;
	case '*':
		stype = PUCK;		/* it's a puck */
		break;
	default:
		return(0);
	}
s = skip_past_comma(s);
ux = atoi(s) - 30;
s = skip_past_comma(s);
uy = atoi(s) - 40;
s = skip_past_comma(s);
temp = atoi(s);
pressure_sensitive = 0;
switch (stype)
	{
	case PSTYLUS:
		mouse_button = 0;
		pressure_sensitive = 1;
		if (temp > -99 && temp < 99)	/* filter to reasonable values */
			{
			umouse_x = ux;
			umouse_y = uy;
			if (temp < pmin)
				pmin = temp;
			if (temp > pmax)
				pmax = temp;
			if (temp >= pmin+PTHRESH)
				{
				mouse_button = 1;
				pressure = (temp-pmin-PTHRESH)*255/(pmax-pmin-PTHRESH);
				if (pressure > 255)
					pressure = 255;
				if (pressure < 1)
					pressure = 1;
				}
			}
		/* check for control key to interpret a right click 
			(Arr, no side button on pressure sensitive stylus!) */
		r.b.ah = 2;
		sysint(0x16,&r,&r);
		if (r.b.al & 0x4)	/* pendown on alt */
			mouse_button |= 0x2;
		break;
	case STYLUS:
		if (temp != 99)
			{
			umouse_x = ux;
			umouse_y = uy;
			mouse_button = temp;
			}
		break;
	case PUCK:
		mouse_button = 0;
		if (temp != 99)
			{
			umouse_x = ux;
			umouse_y = uy;
			if (temp&2)
				mouse_button |= 1;
			if (temp&4)
				mouse_button |= 2;
			}
		break;
		}
return(1);
}

#endif WACOM
