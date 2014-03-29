/* Fli.c - this file should probably be broken into a few sections.  It has
   the low level routines to uncompress and play-back a FLI flic.  It also
   has a few simple time-oriented utility routines.  Stuff for stepping
   back and forth a single frame.  */

#include "jimk.h"
#include "animinfo.h"
#include "auto.h"
#include "errcodes.h"
#include "flx.h"
#include "pentools.h"
#include "picdrive.h"
#include "picfile.h"
#include "timemenu.h"
#include "unchunk.h"

static Errcode load_fli(char *title);

Errcode read_flx_frame(Flxfile *flx, Fli_frame *frame, int ix)
{
Errcode err;

	if(flx->idx[ix].fsize <= sizeof(Fli_frame))
	{
		pj_i_get_empty_rec(frame);
		return(Success);
	}
	if((err = pj_readoset(flx->fd,frame,
						flx->idx[ix].foff,flx->idx[ix].fsize)) < Success)
	{
		return(softerr(err, "tflx_read1"));
	}
	if(frame->type != FCID_FRAME || frame->size != flx->idx[ix].fsize)
	{
		return(softerr(Err_bad_magic,"tflx_frame"));
	}
	return(Success);
}
Errcode gb_unfli_flx_frame(Flxfile *flx, Rcel *screen,int ix,
						   int wait,Fli_frame *frame)

/* got buf unfli - have allocated the buffer ok already.  Read in the
   indicated frame from the FLX file, and then call upstairs to 
   uncompress it, adds overlay image if present */
{
Errcode err;

	if((err = read_flx_frame(&flix,frame,ix)) < Success)
		return(err);
	pj_fli_uncomp_frame(screen,frame, wait);
	unfli_flx_overlay(flx,screen,ix);
	return(Success);
}

static Errcode
gb_flx_ringseek(Flxfile *flx, Rcel *screen, int curix, int ix, Fli_frame *frame)
/* seeks to frame from current position by going around through ring frame
 * and not re reading frame 0 returns actual index left in screen */
{
Errcode err;

	ix = fli_wrap_frame((Flifile *)flx,ix);
	curix = fli_wrap_frame((Flifile *)flx,curix);

	while(curix != ix)
	{
		++curix;
		if((err = gb_unfli_flx_frame(flx,screen,curix,0,frame)) < 0)
			return(softerr(err,"tflx_read2"));
		if(curix == flix.hdr.frame_count)
			curix = 0;
	}
	return(ix);
}


Errcode gb_unfli(Rcel *screen, /* screen recieving update */
			 	int ix,	 /* which frame of file to read */
			 	int wait,	/* wait for vblank 
			 			     * (and update hardware color registers)? */
			 	Fli_frame *frame)
{
	return(gb_unfli_flx_frame(&flix,screen,ix,wait,frame));
}
Errcode flx_ringseek(Rcel *screen, int curix, int ix)
{
Errcode err;
Fli_frame *frame;

	if((err = pj_fli_cel_alloc_cbuf(&frame,screen)) < 0)
		return(softerr(err,"tflx_seek"));
	err = gb_flx_ringseek(&flix, screen, curix, ix, frame);
	pj_free(frame);
	return(err);
}

Errcode unfli(Rcel *f, /* screen to update */
		 int ix,	/* which frame of file to read */
		 int wait)	/* wait for vblank (and update hardward color registers)? */

/* allocate a buffer to read in a compressed delta frame from FLX (indexed
   FLI) file.  If can't allocate buffer go swap out everything we can 
   and try again.  Once got the buffer call above routine to read in 
   frame and eventually uncompress it. */
{
struct fli_frame *frame;	/* buffer area */
Errcode err;
long size;
int pushed = 0;

	size = flix.idx[ix].fsize;
	if((frame = pj_malloc(Max(size,sizeof(Fli_frame)))) == NULL)
	{
		pushed = 1;
		push_most();
		if ((frame = begmem(size)) == NULL)
		{
			err = Err_reported;
			goto OUT;
		}
	}
	err = gb_unfli_flx_frame(&flix,f, ix, wait, frame);

OUT:
	pj_gentle_free(frame);
	if (pushed)
		pop_most();
	return(err);
}

Errcode flisize_error(Errcode err, SHORT width,SHORT height) 
{
	if(width == vb.pencel->width && height == vb.pencel->height)
	{
		return(softerr(err, "!%d%d", "tflx_buffers",
							width, height));
	}
	else
	{
		return(softerr(err, "!%d%d", "fli_big",
							width, height ));
	}
}
Errcode resize_load_fli(char *flicname)
/* Make sure that a file-name refers to an existing FLI file 
 * or a valid pic format.
 * attempt reset of current fli window environment to 
 * the new image size. and attempt to load the images.
 * if a failure in resizing or loading you will end up with an
 * empty fli and an error reported */
{
Errcode err;
Anim_info ainfo;
char pdr_name[PATH_SIZE];

	hide_mp();

	if((err = find_pdr_loader(flicname, TRUE, &ainfo, 
							  pdr_name,vb.pencel)) < Success)
	{
		goto reshow_out; 
	}

	unzoom();
	push_most();
	close_tflx();

	if((err = set_penwndo_size(ainfo.width,ainfo.height)) < Success)
	{
		err = flisize_error(err,ainfo.width,ainfo.height);
		empty_tempflx(1);
		goto error;
	}
	else
	{
		if(is_fli_pdr_name(pdr_name))
		{
			err = load_fli(flicname);
			goto done;
		}

		if(ainfo.num_frames == 1)
		{
			empty_tempflx(1);
			if((err = pdr_load_picture(pdr_name,flicname,vb.pencel)) < Success)
			{
				goto error;
			}
			dirties();
			goto done;
		}
		/* try to load animation file using pdr */

		if(!soft_yes_no_box("!%d%s", "fliload_slow",
						    ainfo.num_frames, flicname ))
		{
			err = Err_abort;
			goto error;
		}

		vs.frame_ix = 0;
		if((err = make_pdr_tempflx(pdr_name,flicname,&ainfo)) >= Success
		     || err == Err_abort)
		{
			if((err = unfli(vb.pencel,0,1)) >= Success)
			{
				goto done;
			}
		}
		kill_seq();
		goto error;	
	}

done:
error:
	pop_most();
	rezoom();
reshow_out:
	show_mp();
	vs.bframe_ix = 0; /* back frame buffer no good now */
	return(softerr(err,"!%s", "fli_load", flicname));
}
Errcode open_default_flx(void)

/* resets settings to default values in fli (default_name) and creates 
 * an empty tflx for these settings at current fli window size */
{
Errcode err;
Vset_flidef fdef;

	if((err = load_default_settings(&fdef)) >= Success)
	{
		err = empty_tempflx(Max(fdef.frame_count,1));
		flix.hdr.speed = fdef.speed;
	}
	return(err);
}

#ifdef WITH_POCO
Errcode resize_default_temps(SHORT width, SHORT height)

/* called from poco and the like to reset to default settings */
{
Errcode err, ret;

	ret = Success;
	hide_mp();
	unzoom();
	push_most();
	close_tflx();

	if(width > 0 && height > 0)
	{
		if((ret = set_penwndo_size(width,height)) < 0)
			ret = flisize_error(ret,width,height);
	}

	if((err = open_default_flx()) < 0)
		return(err);

	pop_most();
	rezoom();
	show_mp();
	return(ret);
}
#endif

static Errcode load_fli(char *title)
/* Convert a FLI file into an indexed frame (FLX) file - into our 
   main temp file in fact.  Make first frame visible.  On failure
   generate an empty FLX file */
{
Errcode err;

	vs.frame_ix = 0;
	if((err = make_tempflx(title,1)) < 0)
		goto OUT;
	if((err = unfli(vb.pencel,0,1)) < 0)
		goto OUT;
	save_undo();
OUT:
	if(err < 0)
		kill_seq();
	zoom_it();
	return(err);
}

int wrap_frame(int frame)
/* Force a frame index to be inside the FLX. (between 0 and count - 1)  */
{
	if((frame = frame % flix.hdr.frame_count) < 0)
		frame += flix.hdr.frame_count;
	return(frame);
}


void check_loop(void)
/* If gone past the end go back to the beginning... */
{
	vs.frame_ix = wrap_frame(vs.frame_ix);
}

void advance_frame_ix(void)
/* Move frame counter forward one */
{
	vs.frame_ix++;
	check_loop();
}

SHORT flx_get_frameix(void *data)
{
	(void)data;
	return(vs.frame_ix);
}
SHORT flx_get_framecount(void *data)
{
	(void)data;
	return(flix.hdr.frame_count);
}

void last_frame(void *data)
/* Do what it takes to move to last frame of our temp file */
{
	(void)data;

	if (!flix.fd)
		return;
	flx_clear_olays();
	if (vs.frame_ix == flix.hdr.frame_count-1)
		return;	/* already there... */
	scrub_cur_frame();
	pj_rcel_copy(vb.pencel, undof);
	fli_tseek(undof,vs.frame_ix, flix.hdr.frame_count-1);
	pj_rcel_copy(undof,vb.pencel);
	see_cmap();
	zoom_it();
	vs.frame_ix = flix.hdr.frame_count-1;
	flx_draw_olays();
}

void flx_seek_frame(SHORT frame)
/* scrub the cur-frame, and do a tseek */
{
	if(!flix.fd)
		return;
	flx_clear_olays();
	frame = wrap_frame(frame);
	if(frame != vs.frame_ix)
	{
		if(frame != (scrub_cur_frame()-1))
		{
			if (frame == vs.frame_ix+1)
			{
				/* optimization for just the next frame */
				pj_cmap_copy(vb.pencel->cmap,undof->cmap);
				fli_tseek(vb.pencel,vs.frame_ix,frame);
				if(!cmaps_same(vb.pencel->cmap,undof->cmap))
					see_cmap();
				zoom_it();
				save_undo();
			}
			else
			{
				save_undo();
				fli_tseek(undof,vs.frame_ix,frame);
				zoom_unundo();
			}
		}
		else
			zoom_unundo();

		vs.frame_ix = frame;
	}
	flx_draw_olays();
}

void flx_seek_frame_with_data(SHORT frame, void *data)
{
	(void)data;
	flx_seek_frame(frame);
}

void prev_frame(void *data)
/* do what it takes to go to previous frame of our temp file.  If go before
   first frame then wrap back to last frame */
{
	(void)data;
	flx_seek_frame(vs.frame_ix-1);
}

void first_frame(void *data)
/* Jump to first frame of temp file */
{
	(void)data;

	if (!flix.fd)
		return;
	flx_clear_olays();
	scrub_cur_frame();
	if(unfli(undof,0,1) >= 0)
		vs.frame_ix = 0;
	zoom_unundo();
	flx_draw_olays();
}

void next_frame(void *data)
/* Jump to next frame of temp file, wrapping back to 1st frame if go past
   end... */
{
	int oix;
	int undoix;
	(void)data;

	if (!flix.fd)
		return;
	flx_clear_olays();
	oix = vs.frame_ix;
	undoix = scrub_cur_frame()-1;
	++vs.frame_ix;
	check_loop();
	if(undoix != vs.frame_ix)
	{
		if (unfli(vb.pencel,vs.frame_ix,1) < 0)
			vs.frame_ix = oix;
		zoom_it();
		save_undo();
	}
	else
	{
		zoom_unundo(); /* undo has next frame left in it by sub_cur_frame() */
	}
	flx_draw_olays();
}

static Errcode vp_playit(LONG frames)
/* Ya, go play dem frames.  Replay temp file */
{
ULONG clock;
Errcode err;

	flx_clear_olays(); /* undraw cels cursors etc */
	if(flix.fd)
	{
		clock = pj_clock_1000();
		hide_mouse();
		for (;;)
		{
			if (frames == 0)
				{
				err = Success;
				break;
				}
			--frames;
			clock += flix.hdr.speed;
			if ((err = wait_til(clock)) != Err_timeout)
				break;
			if (clock > pj_clock_1000()) /* wrap */
				clock = pj_clock_1000();
			vs.frame_ix++;
			if (vs.frame_ix > flix.hdr.frame_count)
				vs.frame_ix = 1;
			if((err = unfli(vb.pencel,vs.frame_ix,1)) < 0)
				break;
			zoom_it();
		}
		show_mouse();
	}
	check_loop();	/* to go frame frame_count to 0 sometimes... */
	flx_draw_olays(); /* undraw cels cursors etc */
return(err);
}

void mplayit(void *data)
/* Play temp file forever */
{
	(void)data;

	hide_mp();
	scrub_cur_frame();
	vp_playit(-1L);
	pj_rcel_copy(vb.pencel,undof);
	show_mp();
}

static void pflip_thru(int start, int stop, int wait)
/* Play frames from start to stop of temp file */
{
Rcel *tmp;
long count;

count = stop - start;
start = wrap_frame(start);
if(alloc_pencel(&tmp) < 0)
	return;
pj_rcel_copy(vb.pencel, tmp);
fli_tseek(tmp,vs.frame_ix,start);
pj_rcel_copy(tmp, vb.pencel);
see_cmap();
zoom_it();
pj_rcel_free(tmp);
if(check_input(MBRIGHT|KEYHIT))
	return;
vs.frame_ix = start;
wait *= flix.hdr.speed;
wait_millis(wait);
if (vp_playit(count) >= Success)	/* don't wait if they aborted */
	wait_millis(wait);
}

static void fl_range(int once)
/* Flip through the time segment without destroying undo buffer or
   other-wise disturbing the 'paint context'.   Once parameter indicates
   whether we stop after doing one time or just keep going until user
   hits a key */
{
int oix;
Rcel_save opic;

	flx_clear_olays();
	oix = vs.frame_ix;
	find_seg_range();
	if (report_temp_save_rcel(&opic, vb.pencel) >= Success)
	{
		maybe_push_most();
		for (;;)
		{
			pflip_thru(tr_r1, tr_r2, (once ? 2 : 0));
			if (JSTHIT(MBRIGHT|KEYHIT) || once)
				break;
		}
		maybe_pop_most();
		report_temp_restore_rcel(&opic, vb.pencel);
	}
	zoom_it();
	vs.frame_ix = oix;
	flx_draw_olays();
}

void flip_range(void)
/* flip through time segment once */
{
	fl_range(1);
}

void loop_range(void)
/* flip through time segment until key is pressed */
{
	fl_range(0);
}


void flip5(void)
/* flip through last five frames */
{
Rcel_save opic;

	flx_clear_olays();
	if (report_temp_save_rcel(&opic, vb.pencel) >= Success)
	{
		flix.hdr.speed <<= 1;
		maybe_push_most();
		pflip_thru(vs.frame_ix-4, vs.frame_ix, 2);
		maybe_pop_most();
		flix.hdr.speed >>= 1;
		report_temp_restore_rcel(&opic, vb.pencel);
		zoom_it();
	}
	flx_draw_olays();
}

static int get_bscreen(Rcel *screen, int cur_ix, int new_ix)

/* Try to load screen from back-frame-buffer to avoid having to
   seek all the way from the 1st frame.  Back frame buffer is an
   uncompressed full screen pic file that tries to stay about 4 frames
   behind current frame. */
{
int bix;

if ((bix = vs.bframe_ix) != 0)
	{
	if (bix <= new_ix)
		{
		if (bix > cur_ix || cur_ix > new_ix)
			{
			if (pj_exists(bscreen_name) )
				{
				if(load_pic(bscreen_name, screen, bix,TRUE) >= 0) 
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

static void advance_bscreen(Rcel *screen, int ix, int destix)

/* Update the 'back frame buffer' to a new position. */
{
	if (ix >= vs.bframe_ix+4 && ix > destix-4)
	{
		if(save_pic(bscreen_name, screen,ix,TRUE) < 0 )
			vs.bframe_ix = 0;
		else
			vs.bframe_ix = ix;
	}
}



Errcode fli_tseek(Rcel *screen, int cur_ix, int new_ix)

/* move screen from one frame to another frame.  Screen must indeed
   contain the frame 'cur_ix'.  */
{
Errcode err;
int i;

	cur_ix = get_bscreen(screen, cur_ix, new_ix);
	if (new_ix == cur_ix)
		return(Success);
	if (new_ix > cur_ix)
	{
		for (i=cur_ix+1; i<= new_ix; i++)
		{
			if ((err = unfli(screen, i, 0)) < 0)
				return(err);
			advance_bscreen(screen, i, new_ix);
		}
	}
	else
	{
		for (i=0; i<=new_ix; i++)
		{
			if ((err = unfli(screen, i, 0)) < 0)
				return(err);
			advance_bscreen(screen, i, new_ix);
		}
	}
	return(Success);
}
Errcode gb_fli_tseek(Rcel *screen, 
	             int cur_ix, 
				 int new_ix, 
				 struct fli_frame *fbuf)

/* Move screen to a new frame assuming we've got the decompression buffer
   already... */
{
int i;
Errcode err;

	cur_ix = get_bscreen(screen, cur_ix, new_ix);
	if (new_ix == cur_ix)
		return(0);
	if (new_ix > cur_ix && cur_ix != 0)
	{
		for (i=cur_ix+1; i<= new_ix; i++)
		{
			if((err = gb_unfli(screen, i, 0, fbuf)) < 0)
				return(err);
			advance_bscreen(screen, i, new_ix);
		}
	}
	else
	{
		for (i=0; i<=new_ix; i++)
		{
			if ((err = gb_unfli(screen, i, 0, fbuf)) < 0)
				return(err);
			advance_bscreen(screen, i, new_ix);
		}
	}
	return(0);
}

Errcode gb_fli_abs_tseek(Rcel *screen, int new_ix, struct fli_frame *fbuf)
/* Force screen to a specific frame of temp file.  This will start from
   first frame of file, so the input frame need not contain anything in
   particular */
{
Errcode err;

	if (vs.bframe_ix == 0 || vs.bframe_ix > new_ix)
	{
		if ((err = gb_unfli(screen, 0, 0, fbuf)) < 0)
			return(err);
	}
	return(gb_fli_tseek(screen,0,new_ix,fbuf) );
}

Errcode fli_abs_tseek(Rcel *screen, int new_ix)

/* Force screen to a specific frame of temp file.  This will start from
   first frame of file, so the input frame need not contain anything in
   particular */
{
Errcode err;

	if (vs.bframe_ix == 0 || vs.bframe_ix > new_ix)
	{
		if ((err = unfli(screen, 0, 0)) < 0)
			return(err);
	}
	return(fli_tseek(screen,0,new_ix) );
}
