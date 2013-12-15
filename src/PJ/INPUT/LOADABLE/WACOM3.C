

/* wacom.c - Fetch mouse position, mouse buttons etc from wacom 
   12x12 tablet.  Basically got to yammer out the serial port at it.  */

#include "ptrmacro.h"
#include "errcodes.h"
#include "idriver.h"
#include "regs.h"
#include "syslib.h"
#include "serial.h"

	/** Prototypes for interrupt driven serial functions **/
Errcode ser_init(int port, int ComParams);
void ser_cleanup(int port);
void ser_puts(int port, char *string);
void ser_write(int port, char *buf, int size);
int ser_read(int port, char *buf, int size);
int ser_write_left(int port);		
int ser_read_left(int port);
Errcode ser_status(int port);


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
Errcode err;
char c;

if (ser_read(port, &c, 1) == 1)	/* Fast return for ready char */
	return(c);
end_time = pj_clock_1000() + 500;	/* else set up 1/2 second time-out */
for (;;)
	{
	if (ser_read(port, &c, 1) == 1)
		return(c);
	else
		{
		if ((err = ser_status(port)) < 0)
			return(err);	/* Line dropped or something */
		if (pj_clock_1000() > end_time)
			return(Err_timeout);
		}
	}
}


static void flush_read(SHORT port)
/* Read any pending characters and throw 'em out. */
{
int err;
char c;

for (;;)
	{
	err = ser_status(port);
	if (err <= Success && err != Err_overflow)
		break;
	if (ser_read(port, &c, 1) != 1)
		break;
	}
}

static Errcode flush_write(SHORT port)
/* Read any pending characters and throw 'em out. */
{
long end_time;

end_time = pj_clock_1000() + 500;	/* else set up 1/2 second time-out */
for (;;)
	{
	if (ser_write_left(port) <= 0)
		return(Success);
	if (pj_clock_1000() > end_time)
		return(Err_timeout);
	}
}



void wait_milli(ULONG t)
/* Do nothing at all for some milliseconds */
{
	t += pj_clock_1000();
	while(t > pj_clock_1000());
}

Errcode slow_to_wacom(SHORT port, char *s)
/* send a string out serial port with a delay afterwards */
{
	ser_write(port,s,strlen(s));
	flush_write(port);
	wait_milli(150);
	return(ser_status(port));
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
if ((err = ser_init(port, BAUD_9600+PARITY_NONE+STOP_1+BITS_8)) < Success)
	return(err);

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


Errcode wac_detect_open(Idriver *idr)
/* Detect tablet and leave comm channel open (interrupts installed) */
{
Errcode err;

	if ((err = init_wacom(idr)) < Success)
		return(err);
	/* flush any garbage left at comm port from last time... */
	flush_read(idr->comm_port);
	return(err);
}

Errcode wac_detect(Idriver *idr)
{
return(Success);
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
ser_cleanup(idr->comm_port);
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
Errcode err;

wac_inquire(idr);
/* set things up to the right baud rate */
if ((err = wac_detect_open(idr)) < Success)
	return(err);
return(wac_input(idr));
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

ser_write(port, "RQ1\n", 4);
flush_write(port);
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
		flush_read(port);
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
