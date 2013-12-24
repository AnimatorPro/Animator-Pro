
/* summa.c - Fetch umouse_x, mouse_buttons etc from summa graphics
   12x12 Summa Sketch tablet.  Basically got to yammer out the
   serial port at it.  */

#include "errcodes.h"
#include "idriver.h"
#include "regs.h"
#include "ptrmacro.h"


static LONG sum_min[2] = {100,100}, sum_max[2] = {5848-100,5848-100}, 
	sum_clip[2],
	sum_aspect[2] = {1,1}, sum_pos[2] = {5848/2, 5848/2};
static UBYTE sum_flags[2] = {0,0};


static Idr_option sum_opts[] =
{
	{ 
		RL_KEYTEXT("summapj_opt0"),
		0x0003, /* enable either 0 or 1 (bits 0,1 on) */
		0,   	/* default mode */
	},
};

static Errcode sum_detect(Idriver *idr)
/* Sadly we can't really tell... */
{
	return(1);		/* 1 for maybe... */
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

static int read_tablet(SHORT port, UBYTE *c, int count)
{
union abcd_regs r;

while (--count >= 0)
	{
	r.w.dx = port;
	r.b.ah = 2;
	jcomm(&r);
	if ((r.b.ah & 0x80) !=  0)
		return(0);
	*c++ = r.b.al;
	}
return(1);
}

static int comm_ready(SHORT port)
{
union abcd_regs r;

r.w.dx = port;
r.b.ah = 3;
jcomm(&r);
return(r.b.ah&1);
}

static void flush_comm(SHORT port)
{
char dummy;

while (comm_ready(port))
	{
	if (!read_tablet(port, &dummy, 1))
		return;
	}
}

static void wait_10milli(void)
{
wait_sync();
wait_sync();
}

static char got_summa;

static void setup_summa(Idriver *idr)
{
SHORT port = idr->comm_port;

to_tablet(port,0x20);	/* sync up autobaud */
to_tablet(port,0);		/* reset */
flush_comm(port);
wait_10milli();
to_tablet(port, 0x44);	/* remote request mode */
to_tablet(port, 0x62);    /* coordinates from upper left */
to_tablet(port, 0x11);    /* let her rip */
wait_10milli();
}

init_comm_port(int port)
/* initialization of port to 9600 baud odd parity 1 stop 8 bit data... */
/* from Norton's Programmers Guide to IBM PC & PS2 p 228 */
/* 111			buad rate 9600
      01		odd parity
	    0		1 stop bit
		 11		8 bit data
		    = 11101011 = 0xeb */
{
union abcd_regs r;

r.w.dx = port;
r.b.al = 0xeb;
r.b.ah = 0;
jcomm(&r);
return(r.b.ah != 0);
}

static Errcode sum_open(Idriver *idr)
{
sum_inquire(idr);
/* set things up to the right baud rate */
if (!init_comm_port(idr->comm_port))
	return(Err_no_device);
setup_summa(idr);
got_summa = TRUE;
return(Success);
}

static Errcode sum_input(Idriver *idr)
{
int mb;
UBYTE rbuf[6];
SHORT port = idr->comm_port;

to_tablet(port, 0x50);	/* hey babe, where's the cursor? */
if (read_tablet(port, rbuf, 5) )
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
	setup_summa(idr);
return(Success);
}


static Errcode sum_close(Idriver *idr)
{
if (got_summa)
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


Errcode init_summa_idriver(Idriver *idr)
{
	idr->lib = &sum_lib;
	idr->hdr.init = sum_open;
	idr->hdr.cleanup = sum_close;
	idr->options = sum_opts;
	idr->num_options = Array_els(sum_opts);
	return(Success);
}


