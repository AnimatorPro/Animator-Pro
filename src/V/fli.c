
/* Fli.c - this file should probably be broken into a few sections.  It has
   the low level routines to uncompress and play-back a FLI flic.  It also
   has a few simple time-oriented utility routines.  Stuff for stepping
   back and forth a single frame.  */

#include "jimk.h"
#include "fli.h"
#include "fli.str"

/* error message - we decided it's not a good FLI file */
notafli(name)
char *name;
{
char *buf[3];

buf[0] = name;
buf[1] = fli_100 /* "Isn't a FLI file, sorry" */;
buf[2] = NULL;
continu_box(buf);
}

/* error message - single frame looks bad */
not_fli_frame()
{
continu_line(fli_101 /* "File corrupted." */);
}


/* Ok, now we've read in a frame of a FLI/FLX file ok.  Given a screen that
   has the last frame on it and the data for this frame, this function
   will switch through the chunks of the frame data updating the screen
   in the process.  The 'colors' parameter indicates whether we should
   update the hardware color map as well as the ram echo. */
uncompfli(f, frame, colors)
Vscreen *f;
struct fli_frame *frame;
int colors;	/* update color registers? */
{
int j;
char *c;
struct fli_chunk *chunk;

c = (char *)(frame+1);
if (colors)
	wait_vblank();
for (j=0;j<frame->chunks;j++)
	{
	chunk = (struct fli_chunk *)c;
	switch (chunk->type)
		{
		case FLI_WRUN:
			unrun(chunk+1, f->p);
			break;
		case FLI_SBSRSC: 
			unsbsrsccomp(chunk+1, f->p);
			break;
		case FLI_COLOR:
			if (colors)
				{
				cset_colors(chunk+1);
				}
			fcuncomp(chunk+1,f->cmap);
			break;
		case FLI_LC:
			unlccomp(chunk+1, f->p);
			break;
		case FLI_ICOLORS:
			copy_cmap(init_cmap, f->cmap);
			if (colors)
				see_cmap();
			break;
		case FLI_BLACK:
			clear_form(f);
			break;
		case FLI_BRUN:
			unbrun(chunk+1, f->p, f->h);
			break;
		case FLI_COPY:
			copy_words(chunk+1,f->p,32000);
			break;
		}
	c = norm_pointer(c + chunk->size);
	}
}


/* got buf unfli - have allocated the buffer ok already.  Read in the
   indicated frame from the FLX file, and then call upstairs to 
   uncompress it */
gb_unfli(f, ix, wait, frame)
Vscreen *f;		/* screen recieving update */
int ix;			/* which frame of file to read */
int wait;		/* wait for vblank (and update hardward color registers)? */
struct fli_frame *frame;
{
long size;

if (!jseek(tflx, cur_flx[ix].foff, 0) )
	return(0);
size = cur_flx[ix].fsize;
if (jread(tflx,frame,size) < size)
	{
	noroom();
	goto BAD;
	}
if (frame->type != FLIF_MAGIC)
	{
	not_fli_frame();
	goto BAD;
	}
uncompfli(f, frame, wait);
return(1);
BAD:
return(0);
}

/* allocate a buffer to read in a compressed delta frame from FLX (indexed
   FLI) file.  If can't allocate buffer go swap out everything we can 
   and try again.  Once got the buffer call above routine to read in 
   frame and eventually uncompress it. */
unfli(f, ix, wait)
Vscreen *f;	/* screen to update */
int ix;			/* which frame of file to read */
int wait;		/* wait for vblank (and update hardward color registers)? */
{
struct fli_frame *frame;	/* buffer area */
long size;
int ret = 0;
int pushed = 0;

size = cur_flx[ix].fsize;
if ((frame = laskmem(size)) == NULL)
	{
	pushed = 1;
	push_pics();
	if ((frame = lbegmem(size)) == NULL)
		goto OUT;
	}
ret =  gb_unfli(f, ix, wait, frame);
OUT:
gentle_freemem(frame);
if (pushed)
	pop_pics();
return(ret);
}

/* read in the FLI header and verify that it has right magic number */
read_fli_head(title, flih)
char *title;
struct fli_head *flih;
{
int fd;

if ((fd = jopen(title, 0)) == 0)
	{
	cant_find(title);
	return(0);
	}
/* read in fli header and check it's magic number */
if (jread(fd,flih,sizeof(*flih)) < sizeof(*flih) )
	{
	truncated(title);
	jclose(fd);
	return(0);
	}
if (flih->type != FLIH_MAGIC)
	{
	notafli(title);
	jclose(fd);
	return(0);
	}
return(fd);
}


/* Make sure that a file-name refers to an existing FLI file */
check_fli(title)
char *title;
{
struct fli_head fh;
int f;

if ((f = read_fli_head(title, &fh)) == 0)
	return(0);
jclose(f);
return(1);
}

/* read in next frame from FLI (non-indexed) file */
gb_read_next_frame(fname,fd,fscreen,fliff,colors)
char *fname;	/* name of file to report errors */
int fd;			/* file handle */
Vscreen *fscreen;	/* screen to update */
struct fli_frame *fliff;	/* 64K buffer area (K not 1000) */
int colors;		/* wait for vblank and update hardware palette? */
{
long size_left;
int ret;

ret = 0;
if (jread(fd,fliff,sizeof(*fliff)) < sizeof(*fliff) )
	{
	truncated(fname);
	goto BADOUT;
	}
if (fliff->type != FLIF_MAGIC)
	{
	not_fli_frame();
	goto BADOUT;
	}
if (fliff->size >= CBUF_SIZE)
	{
	mangled(fname);
	goto BADOUT;
	}
size_left = fliff->size - sizeof(*fliff);
if (jread(fd,fliff+1,size_left) < size_left)
	{
	truncated(fname);
	goto BADOUT;
	}
uncompfli(fscreen, fliff, colors);
ret = 1;
BADOUT:
return(ret);
}


/* Allocate a buffer and then call above to read in next frame from a 
   FLI file */
read_next_frame(fname,fd,fscreen,colors)
char *fname;
int fd;
Vscreen *fscreen;
int colors;		/* update hw color map??? */
{
struct fli_frame *fliff;
int ret;

if ((fliff = lbegmem(CBUF_SIZE)) == NULL)
	return(0);
ret = gb_read_next_frame(fname, fd, fscreen, fliff, colors);
freemem(fliff);
return(ret);
}


/* Convert a FLI file into an indexed frame (FLX) file - into our 
   main temp file in fact.  Make first frame visible.  On failure
   generate an empty FLX file */
load_fli(title)
char *title;
{
int success = 0;

vs.frame_ix = 0;
if (!make_tempflx(title))
	goto OUT;
if (!unfli(render_form,0,1))
	goto OUT;
success = 1;
copy_form(render_form,&uf);
cleans();
fhead.session = 0;
check_dfree();
OUT:
if (!success)
	{
	kill_seq();
	}
zoom_it();
return(success);
}

/* Force a frame index to be inside the FLX.  */
wrap_frame(frame)
int frame;
{
while (frame < 0)
	frame += fhead.frame_count;
while (frame >= fhead.frame_count)
	frame -= fhead.frame_count;
return(frame);
}


/* If gone past the end go back to the beginning... */
check_loop()
{
vs.frame_ix = wrap_frame(vs.frame_ix);
}

/* Move frame counter forward one */
advance_frame_ix()
{
vs.frame_ix++;
check_loop();
}

/* Do what it takes to move to last frame of our temp file */
static
last_frame()
{
if (!tflx)
	return;
if (vs.frame_ix == fhead.frame_count-1)
	return;	/* already there... */
scrub_cur_frame();
copy_form(render_form, &uf);
fli_tseek(&uf,vs.frame_ix, fhead.frame_count-1);
copy_form(&uf,render_form);
see_cmap();
zoom_it();
vs.frame_ix = fhead.frame_count-1;
}

/* hide menus before jumping to last frame */
mlast_frame()
{
hide_mp();
last_frame();
draw_mp();
}

/* do what it takes to go to previous frame of our temp file.  If go before
   first frame then wrap back to last frame */
prev_frame()
{
int dest;

if (!tflx)
	return;
dest = wrap_frame(vs.frame_ix-1);
scrub_cur_frame();
fli_abs_tseek(&uf,dest);
vs.frame_ix = dest;
copy_form(&uf,render_form);
see_cmap();
zoom_it();
}

/* hide menus before stepping back one frame */
mprev_frame()
{
hide_mp();
prev_frame();
draw_mp();
}


/* Jump to first frame of temp file */
first_frame()
{
if (!tflx)
	return;
scrub_cur_frame();
if (unfli(render_form,0,1))
	vs.frame_ix = 0;
copy_form(render_form,&uf);
zoom_it();
}

/* Hide menus and jump to first frame of temp file */
mfirst_frame()
{
hide_mp();
first_frame();
draw_mp();
}

/* Jump to next frame of temp file, wrapping back to 1st frame if go past
   end... */
next_frame()
{
int oix;

oix = vs.frame_ix;
if (!tflx)
	return;
scrub_cur_frame();
vs.frame_ix++;
check_loop();
if (!unfli(render_form,vs.frame_ix,1))
	vs.frame_ix = oix;
copy_form(render_form,&uf);
zoom_it();
}

/* hide menus and go to next frame of temp file */
mnext_frame()
{
hide_mp();
next_frame();
draw_mp();
}


/* Ya, go play dem frames.  Replay temp file */
static
vp_playit(frames)
long frames;
{
long clock;

if (tflx != 0)
	{
	clock = get80hz();
	mouse_on = 0;
	for (;;)
		{
		if (frames == 0)
			break;
		--frames;
		clock += fhead.speed;
		if (!wait_til(clock))
			break;
		if (clock > get80hz())
			clock = get80hz();
		vs.frame_ix++;
		if (vs.frame_ix > fhead.frame_count)
			vs.frame_ix = 1;
		if (!unfli(render_form,vs.frame_ix,1))
			break;
		zoom_it();
		}
	mouse_on = 1;
	}
check_loop();	/* to go frame frame_count to 0 sometimes... */
}

/* Play temp file forever */
playit()
{
scrub_cur_frame();
vp_playit(-1L);
copy_form(render_form,&uf);
}

/* hide menus and then play temp file forever */
mplayit()
{
hide_mp();
playit();
draw_mp();
}


/* Play frames from start to stop of temp file */
static
pflip_thru(start, stop)
int start, stop;
{
int i;
Vscreen *tmp;
long count;

count = stop - start;
start = wrap_frame(start);
if ((tmp = alloc_screen()) == NULL)
	return;
copy_form(render_form, tmp);
fli_tseek(tmp,vs.frame_ix,start);
copy_form(tmp, render_form);
see_cmap();
zoom_it();
free_screen(tmp);
check_input();
if (RJSTDN || key_hit)
	return;
vs.frame_ix = start;
wait_a_jiffy(2*fhead.speed);
vp_playit(count);
wait_a_jiffy(2*fhead.speed);
}


/* some external variable that find_seg_range() sets up for us to tell
   us how many frames are in the time segment etc. */
extern int tr_r1,tr_r2;
extern int tr_rdir;
extern int tr_tix;
extern int tr_frames;

/* Flip through the time segment without destroying undo buffer or
   other-wise disturbing the 'paint context'.   Once parameter indicates
   whether we stop after doing one time or just keep going until user
   hits a key */
static
fl_range(once)
int once;
{
int oix;

oix = vs.frame_ix;
find_seg_range();
push_screen();
maybe_push_most();
for (;;)
	{
	pflip_thru(tr_r1, tr_r2);
	if (RJSTDN || key_hit || once)
		break;
	}
maybe_pop_most();
pop_screen();
zoom_it();
vs.frame_ix = oix;
}

/* flip through time segment once */
flip_range()
{
fl_range(1);
}

/* flip through time segment until key is pressed */
loop_range()
{
fl_range(0);
}


/* flip through last five frames */
flip5()
{
push_screen();
fhead.speed <<= 1;
maybe_push_most();
pflip_thru(vs.frame_ix-4, vs.frame_ix);
maybe_pop_most();
fhead.speed >>= 1;
pop_screen();
zoom_it();
}

/* Try to load screen from back-frame-buffer to avoid having to
   seek all the way from the 1st frame.  Back frame buffer is an
   uncompressed full screen pic file that tries to stay about 4 frames
   behind current frame. */
static
get_bscreen(screen, cur_ix, new_ix)
Vscreen *screen;
int cur_ix, new_ix;
{
int bix;

if ((bix = vs.bframe_ix) != 0)
	{
	if (bix <= new_ix)
		{
		if (bix > cur_ix)
			{
			if (jexists(bscreen_name) )
				{
				if (load_pic(bscreen_name, screen) )
					{
					return(bix);
					}
				}
			}
		}
	else
		{
		vs.bframe_ix = 0;
		}
	}
return(cur_ix);
}

/* Update the 'back frame buffer' to a new position. */
static
advance_bscreen(screen, ix, destix)
Vscreen *screen;
int ix, destix;
{
if (ix >= vs.bframe_ix+4 && ix > destix-4)
	{
	if (save_pic(bscreen_name, screen, 0) )
		vs.bframe_ix = ix;
	else
		vs.bframe_ix = 0;
	}
}

/* Move screen to a new frame assuming we've got the decompression buffer
   already... */
gb_fli_tseek(screen, cur_ix, new_ix, fbuf)
Vscreen *screen;
int cur_ix, new_ix;
char *fbuf;
{
int i;

cur_ix = get_bscreen(screen, cur_ix, new_ix);
if (new_ix == cur_ix)
	return(1);
if (new_ix > cur_ix && cur_ix != 0)
	{
	for (i=cur_ix+1; i<= new_ix; i++)
		{
		if (!gb_unfli(screen, i, 0, fbuf))
			return(0);
		advance_bscreen(screen, i, new_ix);
		}
	}
else
	{
	for (i=0; i<=new_ix; i++)
		{
		if (!gb_unfli(screen, i, 0, fbuf))
			return(0);
		advance_bscreen(screen, i, new_ix);
		}
	}
return(1);
}

/* move screen from one frame to another frame.  Screen must indeed
   contain the frame 'cur_ix'.  */
fli_tseek(screen, cur_ix, new_ix)
Vscreen *screen;
int cur_ix, new_ix;
{
int i;

cur_ix = get_bscreen(screen, cur_ix, new_ix);
if (new_ix == cur_ix)
	return(1);
if (new_ix > cur_ix)
	{
	for (i=cur_ix+1; i<= new_ix; i++)
		{
		if (!unfli(screen, i, 0))
			return(0);
		advance_bscreen(screen, i, new_ix);
		}
	}
else
	{
	for (i=0; i<=new_ix; i++)
		{
		if (!unfli(screen, i, 0))
			return(0);
		advance_bscreen(screen, i, new_ix);
		}
	}
return(1);
}

/* Force screen to a specific frame of temp file.  This will start from
   first frame of file, so the input frame need not contain anything in
   particular */
fli_abs_tseek(screen,new_ix)
Vscreen *screen;
int new_ix;
{
if (!unfli(screen, 0, 0))
	return(0);
return(fli_tseek(screen,0,new_ix) );
}

