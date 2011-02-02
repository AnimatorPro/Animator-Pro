
/* summa.c - Fetch umouse_x, mouse_buttons etc from summa graphics
   12x12 Summa Sketch tablet.  Basically got to yammer out the
   serial port at it.  */

#include "errcodes.h"
#include "idriver.h"
#include "regs.h"
#include "ptrmacro.h"
#include "syslib.h"
#include "serial.h"


static LONG sum_min[2] = {100,100}, sum_max[2] = {5848-100,5848-100}, 
	sum_clip[2],
	sum_aspect[2] = {1,1}, sum_pos[2] = {5848/2, 5848/2};
static UBYTE sum_flags[2] = {0,0};


static Idr_option sum_opts[] =
{
	{ 
		RL_KEYTEXT("summa_opt0")
		"Summa input type\n" 
	  	"Stylus\n"
	  	"Puck\n"
	  	"Cancel" ,
		0x0003, /* enable either 0 or 1 (bits 0,1 on) */
		0,   	/* default mode */
	},
};

static Errcode sum_detect(Idriver *idr)
/* Sadly we can't really tell... */
{
	return(Success);
}

static Errcode sum_inquire(Idriver *idr)
{
	idr->button_count = 2;
	idr->channel_count = 2;
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
{
return(ser_write(port,&c,1));
}

int read_tablet(int port, char *buf, int size)
{
long end_time = pj_clock_1000() + 500 + size*10;	/* Calc timeout - 1/2 sec
											         * plus 1/100 for each char 													 * to read. */
int c;

while (size > 0)
	{
	if ((c = ser_get_char(port)) >= 0)
		{
		*buf++ = c;
		--size;
		}
	else
		{
		if ((c = ser_status(port)) < 0)
			return(c);	/* Line dropped or something */
		if (pj_clock_1000() > end_time)
			return(Err_timeout);
		}
	}
return(Success);
}


static void flush_comm(SHORT port)
{
char dummy;

	while(ser_status(port)>0)
	{
		if (read_tablet(port, &dummy, 1)<Success)
			return;
	}
}

static void wait_milli(ULONG t)
{
	t += pj_clock_1000();
	while(t > pj_clock_1000());
}

static char got_summa;

static void setup_summa(Idriver *idr)
{
SHORT port = idr->comm_port;

	to_tablet(port,0x20);	/* sync up autobaud */
	to_tablet(port,0);		/* reset */
	flush_comm(port);
	wait_milli(15);
	to_tablet(port, 0x44);	/* remote request mode */
	to_tablet(port, 0x62);    /* coordinates from upper left */
	to_tablet(port, 0x11);    /* let her rip */
	wait_milli(15);
}

static Errcode init_comm_port(int port)
/* initialization of port to 9600 baud odd parity 1 stop 8 bit data... */
{
ser_set_mode(port, BAUD_9600+PARITY_ODD+STOP_1+BITS_8);
ser_status(port);		/* To clear any overruns... */
ser_get_char(port);		/* ditto... */
return(ser_status(port));
}

static Errcode sum_open(Idriver *idr)
{
Errcode err;
char rbuf[6];
SHORT port = idr->comm_port;

	sum_inquire(idr);
	/* set things up to the right baud rate */
	if((err = init_comm_port(port)) < Success)
		return(err);
	setup_summa(idr);
	got_summa = TRUE;
	return(Success);
}

static Errcode sum_input(Idriver *idr)
{
int mb;
UBYTE rbuf[6];
SHORT port = idr->comm_port;
Errcode err;

		;	/* hey babe, where's the cursor? */
	if ((err = to_tablet(port, 0x50)) < Success)
		return(err);
	if ((err = read_tablet(port, rbuf, 5) ) >= Success)
	{
		if (rbuf[0] & 0x80)
		{
			mb = rbuf[0] & 0x3;
			if (sum_opts[0].mode == 1)	/* use puck??? reverse buttons. */
				mb = ((mb&1)<<1)+((mb&2)>>1);
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
		{
		if (err == Err_timeout)
			setup_summa(idr);
		else
			return(err);
		}
	return(Success);
}


static Errcode sum_close(Idriver *idr)
{
	if(got_summa)
	{
		to_tablet(idr->comm_port, 0x13);	/* SHADDUP already... */
		got_summa = FALSE;
	}
	return(Success);
}

static Errcode sum_setclip(Idriver *idr,long channel,long clipmax)
{
	if((USHORT)channel > idr->channel_count)
		return(Err_bad_input);

	if(((ULONG)clipmax) > sum_max[channel])
		clipmax = sum_max[channel];
	sum_clip[channel] = clipmax;
	return(0);
}

static Idr_library sum_lib = {
	sum_detect,
	sum_inquire,
	sum_input,
	sum_setclip,
	};


Hostlib _a_a_syslib	 = { NULL, AA_SYSLIB, AA_SYSLIB_VERSION};

Idriver rexlib_header = {
	{ REX_IDRIVER, IDR_VERSION, sum_open, sum_close, &_a_a_syslib },
	NULL,
	&sum_lib,				/* library */
	sum_opts,				/* options */
	Array_els(sum_opts),    /* num options */
	FALSE,					/* does keys */
};

