#include "player.h"
#include "gfx.h"
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

static void true_cmap_fade(Cmap *scmap, Cmap *dcmap, Rgb3 *color, 
						   int p, int q)
{
ULONG vp;
ULONG q2;
int count;
UBYTE *s, *d;


	count = dcmap->num_colors;
	s = (UBYTE *)(scmap->ctab);
	d = (UBYTE *)(dcmap->ctab);

	vp = q - p;
	q2 = q/2;

	while (--count >= 0)
	{
		*d++ = ((*s++ * vp) + ((UBYTE)(color->r) * p) + q2)/q;
		*d++ = ((*s++ * vp) + ((UBYTE)(color->g) * p) + q2)/q;
		*d++ = ((*s++ * vp) + ((UBYTE)(color->b) * p) + q2)/q;
	}
}
static void sync_load_cmap(Cmap *dcmap)
{
	pj_wait_rast_vsync(vb.pencel);
	pj_set_colors(vb.pencel, 0, 128, (UBYTE *)(dcmap->ctab));
	pj_wait_rast_vsync(vb.pencel);
	pj_set_colors(vb.pencel, 128, 128, (UBYTE *)&(dcmap->ctab[128]));
}
static Errcode fade_cmap(Boolean fade_in)
{
extern ULONG pj_clock_1000();
Errcode err;
Cmap *scmap;
Cmap *dcmap;
ULONG start;
LONG elapsed, notdone;
Rgb3 *color;
LONG fade_time, *pp;
ULONG cmapsum;
ULONG newsum;

	if(fade_in)
	{
		color = &pcb.in_from;
		pp = &notdone;
		fade_time = pcb.fadein;
	}
	else
	{
		color = &pcb.out_to;
		pp = &elapsed;
		fade_time = pcb.fadeout;
	}

	/* get a clone of the souce color map for reference */

	dcmap = vb.pencel->cmap;
	if((scmap = clone_cmap(dcmap)) == NULL)
		return(Err_no_memory);

	start = pj_clock_1000();

	elapsed = 0;
	cmapsum = -1;

	while(elapsed < fade_time)
	{
		if((err = check_play_abort()) != Success)
			goto done;
		notdone = fade_time - elapsed;
		true_cmap_fade(scmap, dcmap, color, *pp, fade_time);
		if((newsum = cmap_crcsum(dcmap)) != cmapsum)
		{
			cmapsum = newsum;
			sync_load_cmap(dcmap);
		}	
		elapsed = pj_clock_1000() - start;
	}

	/* Make sure we have gone all the way and that it's accurate.
	 * Note that when fading out the colormap is altered to the fade out
	 * color */

	if(fade_in)
		pj_cmap_copy(scmap,dcmap);
	else /* fade out */
		stuff_cmap(dcmap,color);

	if((cmap_crcsum(dcmap)) != cmapsum)
		sync_load_cmap(dcmap);

	err = Success;
done:
	pj_cmap_free(scmap);
	return(err);
}
static Errcode wait_play_pause()

/* returns Err_abort if ESCEY (Success + 1) if other abort Success if 
 * no abort */
{
Errcode err;
ULONG time;


	time = pj_clock_1000() + pcb.pause;
	while((pcb.pause == 0) || time > pj_clock_1000())
	{
		if((err = check_play_abort()) != Success)
			return(err);
	}
	return(Success);
}
Errcode play_scripted_anim()

/* plays the previusly parsed scripted animation image.  This reports 
 * errors */
{
Errcode err;
Boolean do_fadein, singleframe;
Cmap *cmap;
int mouse_was_on;

	mouse_was_on = hide_mouse();
	cmap = vb.pencel->cmap;

	if((do_fadein = pcb.fadein > 0) != 0)
	{
		stuff_cmap(cmap,&pcb.in_from);
		sync_load_cmap(cmap);
	}

	/* These loaders will load the colors into the ram cmap but not into 
	 * the hardware. This is why we must re sync the color map if an error
	 * occurrs so the menu colors will be correct */

	singleframe = TRUE;

	if((pcb.loadpdr[0])
	    || (err = open_curfli(!do_fadein,FALSE)) < Success)
	{
	    if((err = open_curpic(!do_fadein)) < Success)
			goto error;
	}
	else /* fli opened ok */
	{
		singleframe = pcb.flif.hdr.frame_count == 1;
	}

	if(do_fadein)
	{
		if((err = fade_cmap(TRUE)) != Success)
			goto error;
	}

	if(singleframe)
	{
		if(pcb.pause < 0)
			pcb.pause = 5000; /* 5 seconds default */
	}
	else
	{
		if(pcb.sspeed < 0)
			pcb.speed = pcb.flif.hdr.speed;  /* default to file speed */
		else
			pcb.speed = pcb.sspeed;
	}

	for(;;)
	{
		if(singleframe)
			err = Success;
		else
			err = play_fli();

		/* unless error or a "soft" abort do the pause */

		if(err == Success && pcb.pause >= 0) 
			err = wait_play_pause();

		if(err < Success)
			goto error;

		if(err > Success && ((UBYTE)pcb.abort_key) == AKEY_REPLAY)
		{
			pcb.abort_key = 0;
			if((pla_seek_frame(0)) < Success)
				goto error;
			continue;
		}
		break;
	}

	if(pcb.fadeout > 0)
	{
		err = fade_cmap(FALSE);
		pj_clear_rast(vb.pencel);
	}

error:

	if(err < Success) /* color map may be out of sync here */
		pj_cmap_load(vb.pencel,vb.pencel->cmap);
	if(mouse_was_on)
		show_mouse();
	close_curfli();
	return(err);
}



