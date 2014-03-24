
/* freem.c - 
	This module contains routines to pop you into and out of a state which
    maximizes free memory by swapping everything possible out to disk.
	These nest.  Only when the nesting count transitions between 0 and 1 is
	anything actually performed */

#include "jimk.h"
#include "errcodes.h"
#include "flicel.h"
#include "flx.h"
#include "inks.h"
#include "jfile.h"
#include "mask.h"
#include "memory.h"

static char pushed_cel;
static char pushed_alt;
static char pushed_screen;
static char pushed_mask;

Errcode fake_push(void)
{
	pushed_alt = pushed_cel = pushed_screen = pushed_mask = 1;
	return(0); /* allways successful */
}
void free_buffers()

/* frees all non transient buffers */
{
	release_uvfont();
	pj_rcel_free(vl.alt_cel);
	vl.alt_cel = NULL;
	free_the_cel();
	free_the_mask();
}

static Errcode push_alt_id(LONG id)
{
Errcode err;

	if(pushed_alt == 0)
	{
		if(vl.alt_cel)
		{
			if((err = save_pic(alt_name, vl.alt_cel,id,TRUE)) < 0)
				return(err);
			pj_rcel_free(vl.alt_cel);
			vl.alt_cel = NULL;
		}
	}
	pushed_alt++;
	return(0);
}

/***** screen push pop *****/

static Errcode push_screen_id(LONG timeid)
{
Errcode err;

	if (pushed_screen == 0)
	{
		if((err = save_pic(screen_name, vb.pencel,timeid,TRUE)) < 0)
			return(err);
	}
	pushed_screen++;
	return(0);
}

Errcode pop_screen_id(LONG check_id)
{
Errcode err;

	if(--pushed_screen == 0)
	{
		if (pj_exists(screen_name))
		{
			if((err = load_pic(screen_name, vb.pencel,check_id,TRUE)) < 0)
				return(err);
			pj_delete(screen_name);
			return(Success);
		}
	}
	return(1); /* not popped */
}

/******* cel push pop *******/

void fake_push_cel()
{
	++pushed_cel;
}
void fake_pop_cel()
{
	--pushed_cel;
}
Errcode push_cel(void)
{
Errcode err;

	if(pushed_cel == 0)
	{
		if (thecel != NULL && (err = save_fcel_temp(thecel)) < 0)
			return(err);
		free_fcel(&thecel);
	}
	++pushed_cel;
	return(0);
}
Errcode pop_cel(void)
{
Errcode err;

	if (--pushed_cel == 0)
	{
		if (!thecel && pj_exists(cel_name))
		{
			if((err = load_temp_fcel(cel_name,&thecel)) < 0)
			{
				pj_delete(cel_name); /* cel fli image file may still be there */
				return(err);
			}
		}
	}
	return(0);
}

static int push_mask(void)
{
Errcode err;

	if (pushed_mask == 0)
	{
		if(mask_rast)
		{
			if((err = save_the_mask(mask_name)) < Success)
				return(err);
			free_the_mask();
		}
	}
	pushed_mask++;
	return(0);
}

static Errcode pop_alt_id(LONG check_id)
{
Errcode err;

	if (--pushed_alt == 0)
	{
		if (!vl.alt_cel && pj_exists(alt_name) )
		{
			if(alloc_pencel(&vl.alt_cel) >= 0)
			{
				if((err = load_pic(alt_name, vl.alt_cel, check_id,TRUE)) < 0)
				{
					pj_rcel_free(vl.alt_cel);
					vl.alt_cel = NULL;
					return(err);
				}
				pj_delete(alt_name);
			}
		}
	}
	return(0);
}

Boolean mask_is_present(void)
{
	return(mask_rast != NULL || (pushed_mask < 0 && pj_exists(mask_name)));
}
static SHORT pop_mask(void)
{
SHORT err;

	if(--pushed_mask == 0)
	{
		if((mask_rast == NULL) && pj_exists(mask_name))
		{
			if((err = alloc_the_mask()) < Success)
				goto error;
			if((err = load_the_mask(mask_name)) < Success)
			{
				free_the_mask();
				goto error;
			}
			pj_delete(mask_name);
		}
	}
	return(Success);
error:
	return(err);
}

Errcode push_most_id(LONG id)
{
Errcode err;
	release_uvfont();
	if((err = push_alt_id(id)) < 0)
		goto error;
	if((err = push_cel()) < 0)
		goto error;
	return(push_mask());
error:
	return(err);
}
Errcode push_most(void)
{
Errcode ret;
	ret = push_most_id(0);
	return(ret);
}

Errcode push_pics_id(LONG time_id)
{
Errcode err;
	if((err = push_most_id(time_id)) < 0)
		return(err);
	return(push_screen_id(time_id));
}
static void to_trd_maxmem()
/* make sure temp file system leaves enough for compression buffer and 
 * extra screen */
{
void *cbuf;
void *extra_screen;
long size;
long cbuf_size;
Jfile ofd;
long ooset;
Boolean got_bufs;

	if((ofd = flix.fd) != JNONE)
	{
		ooset = pj_tell(ofd);
		pj_close(flix.fd);
		flix.fd = JNONE;
	}

	/* maximum of fli or cel compression buffer */
	cbuf_size = pj_fli_cel_cbuf_size(vb.pencel);
	if(thecel)
	{
		if((size = pj_fli_cel_cbuf_size(thecel->rc)) > cbuf_size)
			cbuf_size = size;
	}
	/* make sure that buf is big enough for copyfile operation */
	if (cbuf_size < PJ_COPY_FILE_BLOCK)
		cbuf_size = PJ_COPY_FILE_BLOCK;
	cbuf = trd_askmem(cbuf_size);
	extra_screen = trd_askmem(pj_fli_cel_cbuf_size(vb.pencel));
	got_bufs = (cbuf != NULL && extra_screen != NULL);

	rdisk_set_max_to_avail(); /* set to max ram available */
	/* if all is currently popped we try to move tempflx back to ram */

	trd_freez(&cbuf);			/* free up memory for file copy */
	trd_freez(&extra_screen);

	if(got_bufs && pushed_mask <= 0 && pushed_alt <= 0 && pushed_cel <= 0)
	{
		trd_up_to_ram(tflxname); /* move file to ram if not there yet */
	}

	if(ofd != JNONE)
	{
		if((flix.fd = pj_open(tflxname,JREADWRITE)) == JNONE)
		{
			softerr(pj_ioerr(),"tflx_reopen");
			empty_tempflx(1);
		}
		else
			pj_seek(flix.fd,ooset,JSEEK_START);
	}
}
void set_trd_maxmem()
{
	rem_check_tflx_toram();
	to_trd_maxmem();
}

/*** checker "task" called from within input loop installed by doauto() ***/

static Waitask trdtask;

static int trdtask_func(Waitask *wt)
{
	(void)wt;

	if(pushed_mask > 0  	   /* wait till all is popped */
		|| pushed_alt > 0 
		|| pushed_cel > 0
		|| pushed_mask > 0
		|| flix.fd == JNONE    /* wait till file is open */
		|| cgroup_hidden(vb.screen) /* wait till menus are shown */
		|| zoom_hidden()) /* wait till rezoomed */
	{
		return(0);
	}

	if(!is_ram_file(flix.fd)) /* Only if it is no in ram already */
		to_trd_maxmem();

	return(TRUE); /* done with it */
}
void rem_check_tflx_toram()
{
	rem_waitask(&trdtask);
}
void add_check_tflx_toram()
{
	if(WT_ISATTACHED(&trdtask))
		return;
	if(is_ram_file(flix.fd)) /* it is in ram already, forget it */
		return;
	init_waitask(&trdtask,trdtask_func,NULL,WT_KILLCURSOR);
	add_waitask(&trdtask);
}
void pop_most(void)
{
	grab_uvfont();
	pop_alt_id(0);
	pop_cel();
	pop_mask();
	if(pushed_mask == 0 || pushed_alt == 0 || pushed_cel == 0) 
		set_trd_maxmem();
}
#ifdef SLUFFED
void pop_pics_id(LONG check_id)
{
	pop_screen_id(check_id);
	pop_most();
}
#endif /* SLUFFED */

static char pshd,dps;

void maybe_push_most(void)
{
char *pt;
long cbufsz;

	if (pshd == 0)
	{
		cbufsz = pj_fli_cbuf_size(vb.pencel->width,vb.pencel->height,
						   vb.pencel->cmap->num_colors);
		if ((pt = pj_malloc(cbufsz)) == NULL)
		{
			dps = 1;
			push_most();
		}
		else
		{
			dps = 0;
			pj_free(pt);
		}
	}
	pshd++;
}

void maybe_pop_most(void)
{
if (--pshd == 0)
	{
	if (dps)
		pop_most();
	dps = 0;
	}
}

void push_inks(void)
{
if(!(vl.ink && (vl.ink->needs & INK_NEEDS_ALT)))
	push_alt_id(0);
}

void ink_push_cel(void)
{
if (vs.ink_id != celt_INKID)
	push_cel();
}

void ink_pop_cel(void)
{
if (vs.ink_id != celt_INKID)
	pop_cel();
}

void pop_inks(void)
{
if(!(vl.ink && (vl.ink->needs & INK_NEEDS_ALT)))
	pop_alt_id(0);
}

