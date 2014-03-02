
#include "errcodes.h"
#include "idriver.h"
#include "regs.h"

extern Boolean jgot_mouse(void);
extern void jmousey(struct wabcd_regs *mouse_regs);

static LONG mou_min[2], mou_max[2] = {2048,2048}, 
	mou_clip[2] = {2048,2048},
	mou_aspect[2] = {1,1}, mou_pos[2] = {1024, 1024};
static UBYTE mou_flags[2] = {RELATIVE,RELATIVE};

static Errcode mou_detect(Idriver *idr)
{
(void)idr;

if (jgot_mouse())
	return(Success);
else
	return(Err_no_device);
}

static Errcode mou_inquire(Idriver *idr)
{
idr->button_count = 2;
idr->channel_count = 2;
idr->min = mou_min;
idr->max = mou_max;
mou_clip[0] = mou_max[0];
mou_clip[1] = mou_max[1];
idr->clipmax = mou_clip;
idr->aspect = mou_aspect;
idr->pos = mou_pos;
idr->flags = mou_flags;
return(Success);
}

static Errcode mou_open(Idriver *idr)
{
Errcode err;

if ((err = mou_detect(idr)) < Success)
	return(err);
mou_inquire(idr);
return(Success);
}

static Errcode mou_close(Idriver *idr)
{
	(void)idr;
	return(Success);
}

static Errcode mou_input(Idriver *idr)
{
static struct wabcd_regs mrg;

	mrg.ax = 3;
	jmousey(&mrg);
	/* Extract the button bits */
	idr->buttons = mrg.bx & ((1 << 2) - 1); 
	mrg.ax = 11;
	jmousey(&mrg);
	mou_pos[0] += (SHORT)mrg.cx;
	mou_pos[1] += (SHORT)mrg.dx;
	return(Success);
}

static Errcode mou_setclip(Idriver *idr,SHORT channel,long clipmax)
{
	if((USHORT)channel > idr->channel_count)
		return(Err_bad_input);

	if(((ULONG)clipmax) > mou_max[channel])
		clipmax = mou_max[channel];
	mou_clip[channel] = clipmax;
	return(0);
}

static Idr_library mou_lib = {
	mou_detect,
	mou_inquire,
	mou_input,
	mou_setclip,
	};

Errcode init_mouse_idriver(Idriver *idr)
{
	idr->hdr.init = mou_open;
	idr->hdr.cleanup = mou_close;
	idr->lib = &mou_lib;
	idr->options = NULL;
	return(Success);
}


