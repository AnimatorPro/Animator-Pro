
/* tempflx.c - stuff that helps manage our main scratch file. */

#include "jimk.h"
#include "fli.h"
#include "flicmenu.h"

struct fli_head fhead;
Flx *cur_flx;
int tflx;	/* handle for temp file */
extern long write_tflx();

/* This module copies a single .FLI file to our TEMP.FLX file.
	A FLX file is just a FLI file with frame offsets and configuration
	settings at the start.

	This is sort of a private Vpaint file.  It's loaded on start-up if
	it exists already */



close_tflx()
{
vs.frame_ix = 0;
gentle_freemem(cur_flx);
cur_flx = NULL;
fhead.frames_in_table = 0;
if (tflx)
	{
	jclose(tflx);
	tflx = 0;
	}
}

#define IFRAMES 1	/* initial frames */
#define ITABLE 256 /* initial table */

empty_tempflx()
{
long sz, of;
Flx *fl;
struct fli_frame frame;
struct fli_chunk chunk;
long tsz;
int i;
WORD b2[2];

close_tflx();
if ((tflx = jcreate(tflxname)) == 0)
	{
	return(0);
	}
/* write out initial header with resolution and frame count info... */
zero_structure(&fhead, sizeof(fhead));
if (jwrite(tflx,&fhead,(long)sizeof(fhead) ) < sizeof(fhead) )
	{
	noroom();
	goto BAD;
	}
if (jwrite(tflx, &vs, (long)sizeof(vs) ) < sizeof(vs) )
	{
	noroom();
	goto BAD;
	}
/* write a blank index... */
tsz = ITABLE*sizeof(Flx);
if ((cur_flx = begmemc((unsigned)tsz))==NULL)
	goto BAD;
if (jwrite(tflx, cur_flx, tsz) < tsz)
	{
	noroom();
	goto BAD;
	}

/* Make up first frame of two chunks ... default color and clear screen */
fl = cur_flx;
of = sizeof(fhead) + sizeof(vs) + ITABLE*sizeof(Flx);
#define FULLCMSIZE (4+COLORS*3)
sz = sizeof(frame)+2*sizeof(chunk) + FULLCMSIZE;
zero_structure(&frame, sizeof(frame) );
frame.size = sz;
frame.type = FLIF_MAGIC;
frame.chunks = 2;
fl->foff = of;
fl->fsize = sz;
fl++;
if (jwrite(tflx, &frame, (long)sizeof(frame))  < sizeof(frame) )
	{
	noroom();
	goto BAD;
	}
/* make up initial color map chunk */
chunk.size = sizeof(chunk) + FULLCMSIZE;
chunk.type = FLI_COLOR;
if (jwrite(tflx, &chunk, (long)sizeof(chunk)) < sizeof(chunk) )
	{
	noroom();
	goto BAD;
	}
/* naughty type punning BS.  Write out that there's one op, and no
   skips, 256 (= 0)  colors */
b2[0] = 1;
b2[1] = 0;
if (jwrite(tflx, b2, (long)sizeof(b2))  < sizeof(b2))
	{
	noroom();
	goto BAD;
	}
if (jwrite(tflx, render_form->cmap, (long)COLORS*3) < COLORS*3)
	{
	noroom();
	goto BAD;
	}

/* initial pixel values - all 0 */
chunk.type = FLI_BLACK;
if (jwrite(tflx, &chunk, (long)sizeof(chunk)) < sizeof(chunk) )
	{
	noroom();
	goto BAD;
	}

/* now write out a couple of blank frames... */
of += sz;
frame.chunks = 0;
sz = frame.size = sizeof(frame);
i = IFRAMES+1;	/* one for the delta from last to first */
while (--i >= 0)	
	{
	if (jwrite(tflx, &frame, (long)sizeof(frame)) < sizeof(frame))
		{
		noroom();
		goto BAD;
		}
	fl->foff = of;
	fl->fsize = sz;
	of += sz;
	fl++;
	}

/* rewrite the header and index */
jseek(tflx, 0L, 0);
fhead.type = FLIX_MAGIC;
fhead.frame_count = IFRAMES;
fhead.width = XMAX;
fhead.height = YMAX;
fhead.bits_a_pixel = DEPTH;
fhead.speed = 5;
fhead.frames_in_table = ITABLE;
if (jwrite(tflx,&fhead,(long)sizeof(fhead) ) < sizeof(fhead) )
	{
	noroom();
	goto BAD;
	}
if (jwrite(tflx,&vs,(long)sizeof(vs) ) < sizeof(vs) )
	{
	noroom();
	goto BAD;
	}
if (jwrite(tflx, cur_flx, tsz) < tsz)
	{
	noroom();
	goto BAD;
	}
cleans();
return(1);

BAD:
	close_tflx();
	return(0);
}


otempflx()
{
long acc;

close_tflx();
if ((tflx = jopen(tflxname,2)) == 0)	/* open her up read/write... */
	{
	return(0);
	}
if (jread(tflx, &fhead, (long)sizeof(fhead)) < sizeof(fhead))
	{
	noroom();
	goto BADEND;
	}
if (fhead.type != FLIX_MAGIC)
	{
	notafli(tflxname);
	goto BADEND;
	}
if (jread(tflx, &vs, (long)sizeof(vs)) < sizeof(vs))
	{
	noroom();
	goto BADEND;
	}
acc = fhead.frames_in_table; 
acc *= sizeof(Flx);
if ((cur_flx = lbegmem(acc)) == NULL)
	goto BADEND;
if (jread(tflx, cur_flx, acc) < acc)
	{
	noroom();
	goto BADEND;
	}
return(1);

BADEND:
close_tflx();
return(0);
}

open_tempflx()
{
vs.bframe_ix = 0; /* back frame buffer no good now */
if (otempflx())
	{
	rethink_settings();
	return(1);
	}
else
	return(0);
}

long
write_tflx_start(tablesize)
long tablesize;
{
long acc;

if ((tflx = jcreate(tflxname))==0)
	{
	cant_create(tflxname);
	goto BADEND;
	}
fhead.type = FLIX_MAGIC;
acc = fhead.frames_in_table = 
	((tablesize + 256) & 0xffffff00);
acc *= sizeof(Flx);
if ((cur_flx = lbegmem(acc)) == NULL)
	goto BADEND;
stuff_words(0, cur_flx, (unsigned)(acc/sizeof(WORD)));
if (jwrite(tflx, &fhead, (long)sizeof(fhead)) < sizeof(fhead))
	{
	noroom();
	goto BADEND;
	}
if (jwrite(tflx, &vs, (long)sizeof(vs) ) < sizeof(vs) )
	{
	noroom();
	goto BADEND;
	}
if (jwrite(tflx, cur_flx, acc) < acc)
	{
	noroom();
	goto BADEND;
	}
acc += sizeof(fhead) + sizeof(vs);
return(acc);
BADEND:
return(0);
}

long
frame_to_tflx(sfile, name, frame, acc, ix)
int sfile;
char *name;
struct fli_frame *frame;
long acc;
int ix;
{
long size;

if (jread(sfile, frame, (long)sizeof(*frame)) < sizeof(*frame))
	return(0);
if (frame->type != FLIF_MAGIC)
	{
	mangled(name);
	return(0);
	}
size =frame->size - sizeof(*frame);
if (jread(sfile, frame+1, size) < size)
	{
	truncated(name);
	return(0);
	}
size += sizeof(*frame);
if (jwrite(tflx, frame, size) < size)
	{
	noroom();
	return(-1);
	}
cur_flx[ix].foff = acc;
cur_flx[ix].fsize = size;
acc += size;
return(acc);
}

make_tempflx(name)
char *name;
{
int ok;

maybe_push_most();
ok = m_tempflx(name);
maybe_pop_most();
return(ok);
}

static
m_tempflx(name)
char *name;
{
int sfile;
long acc;
long size;
struct fli_frame *frame = NULL;	/* buffer area */
int i;

close_tflx();
if ((sfile = read_fli_head(name, &fhead)) == 0)
	goto BADEND;
if ((acc = write_tflx_start(fhead.frame_count+100L)) == 0)
	goto BADEND;
if ((frame = lbegmem(CBUF_SIZE)) == NULL)
	goto BADEND;
for (i=0;i <= fhead.frame_count; i++)
	{
#ifdef TOOSLOW
	if (check_abort(i, fhead.frame_count) )
		goto RINGEND;
#endif TOOSLOW
	if ((acc = frame_to_tflx(sfile, name, frame, acc, i)) <= 0)
		{
		if (acc < 0)
			goto BADEND;
		/* go try to ring frame */
		fhead.frame_count = i-1;
		while (fhead.frame_count > 0)
			{
			if (ring_tflx(frame))
				goto OKEND;
			fhead.frame_count -= 1;
			}
		goto BADEND;
		}
	}
freemem(frame);
frame = NULL;
if (!finish_tflx())
	goto BADEND;
jclose(sfile);
return(1);
RINGEND:
if ((fhead.frame_count = i) <= 0)	/* didn't get any good frames at all */
	goto BADEND;
if (!ring_tflx(frame))
	{
	noroom();
	goto BADEND;
	}
OKEND:
freemem(frame);
jclose(sfile);
return(1);
BADEND:
gentle_freemem(frame);
close_tflx();
jclose(sfile);
return(0);
}

finish_tflx()
{
long acc;

jseek(tflx, FLX_OFFSETS, 0);
acc = fhead.frames_in_table*sizeof(Flx);
if (jwrite(tflx, cur_flx, acc) < acc)
	{
	noroom();
	return(0);
	}
return(1);
}

/* Generate a delta between the last frame of FLIC and the 1st frame of
   FLIC (in case file truncated or load aborted... ) */
ring_tflx(cbuf)
char *cbuf;
{
long endoff;
int ix;
long size;

ix = fhead.frame_count;
vs.bframe_ix = 0;	/* invalidate back frame cashe */
fli_abs_tseek(&uf,ix - 1);
fli_abs_tseek(render_form, 0);
size = fli_comp_frame(cbuf,uf.p,uf.cmap,render_form->p,
	render_form->cmap,FLI_LC);
if ((endoff = write_tflx(cbuf, size) ) == 0)
	{
	return(0);
	}
cur_flx[ix].foff = endoff;
cur_flx[ix].fsize = size;
return(finish_tflx());
}


long
frame1_foff()
{
return(FLX_OFFSETS+(long)sizeof(Flx)*fhead.frames_in_table);
}



