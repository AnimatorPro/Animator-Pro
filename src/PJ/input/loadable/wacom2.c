

/* wacom.c - Fetch mouse position, mouse buttons etc from wacom 
   12x12 tablet.  Basically got to yammer out the serial port at it.  */

#include "ptrmacro.h"
#include "errcodes.h"
#include "idriver.h"
#include "regs.h"
#include "syslib.h"
#include "serial.h"

#define PMAX 256

int pmin = -10, pmax = 10;
#define PTHRESH 5

LONG wac_min[3] = {0,0,0}, wac_max[3] = {2048,2048,PMAX}, 
	wac_clip[3] = {2048, 2048, PMAX},
	wac_aspect[3] = {1,1,1}, wac_pos[3] = {2048/2, 2048/2, 0};
UBYTE wac_flags[3] = {0,0,PRESSURE};

#define PSTYLUS 0
#define PUCK 1
#define STYLUS 2

Idr_option wac_opts[] =
{
	{ 
		/* mode qchoice text */
		RL_KEYTEXT("wacom_opt0")
		"Select device type\n" 
		"Pressure sensitive stylus\n"
		"Side button stylus\n"
		"Puck\n"
	  	"Cancel", 
		0x0007,    /* modes (bits) 0,1,2 enabled */
		PSTYLUS,   /* default mode */
	},
};

int timeout_get_char(int port)
{
long end_time;	/* Calc timeout - 1/2 sec */
int c;

	/* If character already pending do it fast */
if ((c = ser_get_char(port)) >= 0)	
	return(c);
end_time = pj_clock_1000() + 500;	/* else set up 1/2 second time-out */
for (;;)
	{
	if ((c = ser_get_char(port)) >= 0)
		{
		return(c);
		}
	else
		{
		if ((c = ser_status(port)) < 0)
			{
			return(c);	/* Line dropped or something */
			}
		if (pj_clock_1000() > end_time)
			{
			return(Err_timeout);
			}
		}
	}
}


static void flush_comm(SHORT port)
{
int c;

for (;;)
	{
	c = ser_status(port);
	if (c <= Success && c != Err_overflow)
		break;
	c = ser_get_char(port);
	}
}


void wait_milli(ULONG t)
{
	t += pj_clock_1000();
	while(t > pj_clock_1000());
}

Errcode slow_to_wacom(SHORT port, char *s)
/* send a string out serial port with a delay afterwards */
{
Errcode err;

	if ((err = ser_write(port,s,strlen(s))) < Success)
		return(err);
	wait_milli(150);
	return(Success);
}

char *skip_past_comma(char *s)
{
while (*s++ != ',')
	;
return(s);
}

UBYTE wac_initted = 0;

Errcode init_wacom(Idriver *idr)
{
SHORT port = idr->comm_port;
Errcode err;

if (wac_initted)
	return(Success);


/* set things up to the right baud rate */
ser_set_mode(port, BAUD_9600+PARITY_NONE+STOP_1+BITS_8);
err = ser_status(port);

			/* Don't emulate anybody, be a Wacom 2 */
if ((err = slow_to_wacom(port, "$\r\n")) < Success)
	return(err);
			/* Ascii mode packets */
if ((err = slow_to_wacom(port, "AS0\r\n"))	< Success)
	return(err);
			/* Send reply even if pen out of proximity */
if ((err = slow_to_wacom(port, "AL1\r\n"))	< Success)
	return(err);
if (idr->options[0].mode == PSTYLUS)
	{
	if ((err = slow_to_wacom(port,"PH1\r\n"))< Success)
		return(err);		/* Enable pressure sensitivity */
	}
else
	{
	if ((err = slow_to_wacom(port, "PH0\r\n"))< Success)
		return(err);
	}
/* Send scale signal.  Somehow tablet doesn't seem to get real margins,
   so have to make scale a little bigger for tablet than Animator */
											/* set up scale */
if ((err = slow_to_wacom(port, "SC2304,2304\r\n"))	< Success)
	return(err);
wac_initted = TRUE;
return(Success);
}


Errcode wac_detect(Idriver *idr)
{
Errcode err;

	if ((err = init_wacom(idr)) < Success)
		return(err);
	/* flush any garbage left at comm port from last time... */
	flush_comm(idr->comm_port);
	err = wac_input(idr);
	return(err);
}

Errcode wac_inquire(Idriver *idr)
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
{
wac_initted = FALSE;
return(Success);
}

Errcode wac_setclip(Idriver *idr,long channel,long clipmax)
{
	if((USHORT)channel > idr->channel_count)
		return(Err_bad_input);

	if(((ULONG)clipmax) > wac_max[channel])
		clipmax = wac_max[channel];
	wac_clip[channel] = clipmax;
	return(0);
}


Errcode wac_open(Idriver *idr)
{
wac_inquire(idr);
/* set things up to the right baud rate */
return(wac_detect(idr));
}


Errcode wac_input(Idriver *idr)
{
static char wlinebuf[32];
char *s = wlinebuf;
int count = sizeof(wlinebuf)-2;
int c;
int stype;
int temp;
int dp;
int ux,uy;
SHORT port = idr->comm_port;
Errcode err;

if ((err = ser_write(port, "RQ1\n", 4)) < Success)
	return(err);
while (--count >= 0)
	{
	if ((c = timeout_get_char(port)) < Success)
		{
		return(c);
		}
	if ((*s++ = c) == '\n')
		break;
	}
*s = 0;
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
		flush_comm(port);
		return(Err_version);
	}
s = skip_past_comma(s);
ux = atoi(s) - 120;
s = skip_past_comma(s);
uy = atoi(s) - 120;
s = skip_past_comma(s);
temp = atoi(s);
switch (stype)
	{
	case PSTYLUS:
		idr->buttons = 0; 
		wac_pos[2] = 0;
		if (temp > -99 && temp < 99)	/* filter to reasonable values */
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
		if (jkey_shift() & 0x4)	/* right button on control key */
			idr->buttons |= 0x2;
		break;
	case STYLUS:
		if (temp != 99)
			{
			wac_pos[0] = ux;
			wac_pos[1] = uy;
			idr->buttons = temp;
			}
		break;
	case PUCK:
		idr->buttons = 0;
		if (temp != 99)
			{
			wac_pos[0] = ux;
			wac_pos[1] = uy;
			if (temp&2)
				idr->buttons |= 1;
			if (temp&4)
				idr->buttons |= 2;
			}
		break;
		}
return(Success);
}

Idr_library wac_lib = {
	wac_detect,
	wac_inquire,
	wac_input,
	wac_setclip,
	};

Hostlib _a_a_syslib	 = { NULL, AA_SYSLIB, AA_SYSLIB_VERSION};

Idriver rexlib_header = {
	{ REX_IDRIVER, IDR_VERSION, wac_open, wac_close, &_a_a_syslib },
	NULL,
	&wac_lib,
	wac_opts, 			  /* options */
	Array_els(wac_opts),  /* num options */
};
