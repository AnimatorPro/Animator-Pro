#include "player.h"
#include "rastlib.h"
#include "picdrive.h"
#include <string.h>

extern ULONG pj_clock_1000();


void make_playcel(int width, int height)

/* open a backdrop window if the cel is smaller since a backdrop will be faster
 * in many cases when an menu is not obscuring the cel area */
{
Errcode err = Success;

	pcb.dcel = center_virtual_rcel(vb.pencel, &pcb.virtual, width, height);

	/* clear area not part of current cel if covered by last one */

	if(pcb.dcel->height < pcb.osize.height)
	{
		pj_set_rect(vb.pencel,0,pcb.osize.x,pcb.osize.y,
						  pcb.osize.width,pcb.dcel->y - pcb.osize.y);

		pj_set_rect(vb.pencel,0,pcb.osize.x,pcb.dcel->y + pcb.dcel->height,
						  pcb.osize.width,
						  (pcb.osize.y + pcb.osize.height) 
						  				- (pcb.dcel->y + pcb.dcel->height));
	}
	if(pcb.dcel->width < pcb.osize.width)
	{
	Coor y;

		height = Max(pcb.osize.height,pcb.dcel->height);
		y = Min(pcb.dcel->y,pcb.osize.y);

		pj_set_rect(vb.pencel,0,pcb.osize.x,y,
						  pcb.dcel->x - pcb.osize.x, height);

		pj_set_rect(vb.pencel,0,pcb.dcel->x + pcb.dcel->width,y,
						  pcb.osize.x + pcb.osize.width 
						  	- pcb.dcel->x + pcb.dcel->width, height);
	}
	copy_rectfields(pcb.dcel,&pcb.osize);
}
void free_ramfli_data(Ramfli *rf)
{
	free_slist((Slnode *)(rf->frames));
	rf->frames = NULL;
	pj_freez(&(rf->fhead));

}
void free_ramfli(Ramfli *rf)
{
	if(rf)
	{
		free_ramfli_data(rf);
		pj_free(rf);
	}
}
void free_ramflis()
{
Ramfli *rf;

	while((rf = pcb.ramflis) != NULL)
	{
		pcb.ramflis = rf->next;
		free_ramfli(rf);
	}
}
Errcode add_ramfli(char *name, Ramfli **prf)

/* adds a new ram fli to the list for the flifile "flif" and the name given 
 * returns Errcode and pointer to ramfli added */
{
Ramfli *rf;

	if(NULL == (rf = pj_zalloc(sizeof(Ramfli)+strlen(name))))
		return(Err_no_memory);

	strcpy(rf->nbuf,name);
	rf->name = rf->nbuf;
	rf->next = pcb.ramflis;
	pcb.ramflis = rf;
	if(prf)
		*prf = rf;
	return(Success);
}
Ramfli *find_ramfli(char *local_path)
{
	return((Ramfli *)text_in_list(local_path,(Names *)pcb.ramflis));
}
static Errcode ask_ok_off_disk(Ramfli *rf)
{
	if(rf->flags & RF_ON_FLOPPY)
		return(FALSE);
	return(soft_yes_no_box("!%s","play_flimem", rf->name ));
}
Errcode load_ramfli(Ramfli *rfl, Flifile *flif)
{
Errcode err;
int frame_ix;
Ramframe frame;
Ramframe *rf;
Names first_one;
Ramframe *tail = (Ramframe *)&first_one;
LONG allocsize;
ULONG flags;
extern long mem_free;
long mem_needed;
Boolean ram_required = TRUE;

/* size to read in initially */
#define CHECK_SIZE  (sizeof(Ramframe) - OFFSET(Ramframe,frame))

	first_one.next = NULL;
	free_ramfli_data(rfl);

	/* load header from input one */

	if((rfl->fhead = pj_malloc(sizeof(Fli_head))) == NULL)
	{
		err = Err_no_memory;
		goto no_mem_error;
	}

	*(rfl->fhead) = flif->hdr;

	/* read in the frames */

	flags = rfl->flags;
	clear_struct(&frame);
	frame.frame.size = flif->hdr.frame1_oset;
	frame_ix = -1;

	mem_needed = flif->hdr.size; 

	while(++frame_ix <= (SHORT)(flif->hdr.frame_count))
	{
		frame.doff += frame.frame.size;

		/* the last frame if it is an empty frame will only be
		 * sizeof(Fli_frame) so we may have an Err_eof on the last 
		 * frame */

		if((err = pj_readoset(flif->fd,&frame.frame,
								  frame.doff,CHECK_SIZE)) < Success)
		{
			if(frame_ix != flif->hdr.frame_count)
				goto error;
			if((err = pj_readoset(flif->fd,&frame.frame,
							  frame.doff,sizeof(frame.frame))) < Success)
			{
				goto error;
			}
		}

		if( (!(flags & RF_LOAD_FIRST)) && frame_ix == 0)
			goto add_empty_frame;

		if( (!(flags & RF_LOAD_RING)) && frame_ix == flif->hdr.frame_count)
			goto load_empty_frame;

		/* simple memory test does not account for an un-needed ring frame */

		if((mem_needed - frame.doff) >= mem_free)
			goto no_mem_error;

		allocsize = OFFSET(Ramframe,frame)+frame.frame.size;

		if((rf = pj_malloc(allocsize)) == NULL)
		{
			if(ram_required)
				goto no_mem_error;
			goto add_empty_frame;
		}

		/* add next frame to list */
		rf->next = NULL;
		tail->next = rf;
		tail = rf;

		memcpy(rf,&frame,Min(allocsize,sizeof(frame)));

		if(allocsize < sizeof(frame))
			goto add_full_frame;

		/* read in data: It there is a pstamp chunk we don't need it so 
		 * bypass it and adjust frame data. Otherwise read whole chunk */

#ifdef DOESNT_WORK
		if(frame.first_chunk.type == FLI_PSTAMP)
		{
			rf->frame.size -= frame.first_chunk.size;
			rf->doff += frame.first_chunk.size;

			if(rf->frame.size > sizeof(Fli_frame)
				&& (err = pj_readoset(flif->fd,&(rf->first_chunk),
									  rf->doff + sizeof(Fli_frame),
								rf->frame.size - sizeof(Fli_frame))) < Success)
			{
				goto error;
			}
			--rf->frame.chunks; /* one less chunk */
		}
		else 
#endif	
		
		if(frame.frame.size > CHECK_SIZE 
				 && (err = pj_read_ecode(flif->fd,(rf + 1),
							frame.frame.size - CHECK_SIZE)) < Success)
		{
			goto error;
		}

	add_full_frame:

		rf->doff = -frame.doff; /* negative offset means we have a 
							   	 * good ram record */
		continue;

	add_empty_frame:

		if(rfl->fhead)
			pj_freez(&(rfl->fhead));

	load_empty_frame:

		/* this is a little truncated ramframe only up to doff which is 
		 * is positive in this case negative for a ram record */

		if((rf = pj_malloc(POSTOSET(Ramframe,doff))) == NULL)
			goto no_mem_error;

		rf->doff = frame.doff;
		rf->next = NULL;
		tail->next = rf;
		tail = rf;
	}
	rfl->frames = (Ramframe *)first_one.next;
	return(Success);

no_mem_error:

	/* if specified as required and not ok off disk ERROR out */

	free_slist(first_one.next); /* we need enough ram to have requestor */
	first_one.next = NULL;

	if(ram_required && !ask_ok_off_disk(rfl))
		err = Err_reported;
	else
		err = Success;

error:
	pj_freez(&(rfl->fhead));
	free_slist(first_one.next);
	return(err);
#undef CHECK_SIZE
}

void close_curfli()
{
	pj_freez(&pcb.cbuf);
	pj_fli_close(&pcb.flif); /* in case one is loaded */
	pcb.frame_ix = 0;
	pcb.speed = 0;
	pcb.flif.hdr.frame_count = 1;
	pcb.rfli = NULL;
	pcb.rframe = NULL;
}

static struct names check_pdrs[] = {
	{ NULL, gif_pdr_name },
	{ &check_pdrs[0], "pcx.pdr" },
	/* { &check_pdrs[1], "=pic.pdr" }, */
};


Errcode open_curpic(Boolean load_colors)
{
Errcode err;
Names single_pdr;
Names *check_name;

	if(pcb.loadpdr[0])
	{
		check_name = &single_pdr;
		single_pdr.name = pcb.loadpdr;
	}
	else
	{
		check_name = &check_pdrs[Array_els(check_pdrs)-1];
	}


	while(check_name)
	{
		if((err = load_picture(check_name->name,pcb.fliname,
							   load_colors)) >= Success)
		{
			break;
		}
		if(err == Err_no_file)
			break;
		check_name = check_name->next;
	}
	return(err);
}
static void uncomp_ramframe(Rcel *screen, Ramframe *rf,
							SHORT width, SHORT height, Boolean do_colors)
{
	pj_fli_uncomp_frame(screen,&rf->frame,do_colors);
}
Errcode open_curfli(Boolean load_colors,Boolean force_ram)
{
Errcode err;
Boolean open_fli = TRUE;
Ramframe *frame1;

	close_curfli();
	if((pcb.rfli = find_ramfli(pcb.fliname)) != NULL
			&& pcb.rfli->frames != NULL )
	{
		if(pcb.rfli->fhead) /* get header from ramfli file not needed! */
		{
			pcb.flif.hdr = *(pcb.rfli->fhead);
			open_fli = FALSE;
		}
		force_ram = FALSE;
	}
	else
	{
		pcb.rfli = NULL;
	}

	/* if we need a disk file we need the file open and a de-compression 
	 * buffer for the file records */

	if(open_fli)
	{
		if((err = pj_fli_open(pcb.fliname,&pcb.flif,JREADONLY)) < Success)
			return(err);
		if((err = pj_fli_alloc_cbuf(&pcb.cbuf, pcb.flif.hdr.width,
			    pcb.flif.hdr.height, vb.pencel->cmap->num_colors)) < Success)
		{
			goto error;
		}
	}

	pcb.speed = pcb.flif.hdr.speed;
	make_playcel(pcb.flif.hdr.width, pcb.flif.hdr.height);

	if(force_ram) /* if requested and not there already load fli into ram */
	{
		if((err = add_ramfli(pcb.fliname,&pcb.rfli)) < Success)
			goto error;
		pcb.rfli->flags |= (RF_LOAD_RING|RF_LOAD_FIRST);
		if((err = load_ramfli(pcb.rfli, &pcb.flif)) < Success)
			goto error;
		if(pcb.rfli->frames == NULL) /* no frames loaded */
			pcb.rfli = NULL;
	}

	if(pcb.rfli)
	{
		frame1 = pcb.rfli->frames;
		pcb.rframe = frame1->next;
		if(frame1->doff < 0)
		{
			uncomp_ramframe(pcb.dcel,frame1,pcb.flif.hdr.width,
									 		pcb.flif.hdr.height,
									 		load_colors);
			goto done;
		}
	}

	if((err = pj_fli_seek_first(&pcb.flif)) < Success)
		goto error;
	if((err = pj_fli_read_uncomp(NULL, &pcb.flif, pcb.dcel,
					   			 pcb.cbuf, load_colors)) < Success)
	{
		goto error;
	}

done:
	pcb.next_frame_oset = pcb.flif.hdr.frame2_oset;
	return(Success);
error:
	pj_clear_rast(vb.pencel);
	close_curfli();
	return(err);
}
static Errcode pla_read_first()
{
	pcb.frame_ix = pcb.flif.hdr.frame_count; /* force read of first frame */
	return(pla_seek_frame(0));
}
Errcode pla_seek_frame(int frame)
{
int i;
Errcode err;
LONG frame_oset;
Fli_frame *cbuf = pcb.cbuf;
Boolean seek_one;
LONG cmap_cksum;
Rcel *seekcel;

	if(pcb.flif.fd == JNONE && pcb.rfli == NULL)
		return(Success);

	/* wrap frame */
	frame = fli_wrap_frame(&pcb.flif,frame);

	if(frame > pcb.frame_ix)
	{
		frame_oset = pcb.next_frame_oset;
		i = pcb.frame_ix;
		seek_one = (i == frame - 1);
	}
	else if(frame < pcb.frame_ix) 
	{
		if(pcb.frame_ix == pcb.flif.hdr.frame_count - 1) /* do ring frame */
		{
			frame_oset = pcb.next_frame_oset;
			i = pcb.frame_ix;
		}
		else /* re seek from start */
		{
			frame_oset = pcb.flif.hdr.frame1_oset;
			i = -1;
		}
		seek_one = (frame == 0);
	}
	else 	/* same frame as current */
		goto done;

	if(seek_one)
	{
		seekcel = pcb.dcel; 
	}
	else /* more than one frame */
	{
		/* let's try to seek in background if there is memory */
		if((seekcel = clone_rcel(pcb.dcel)) == NULL)
			seekcel = pcb.dcel; 
		cmap_cksum = cmap_crcsum(seekcel->cmap);	
	}

	/* if the fli is in ram get frame from ram if there */
	if(pcb.rfli)
	{

		if(frame_oset == pcb.flif.hdr.frame1_oset)
			pcb.rframe = pcb.rfli->frames;

		while(i++ != frame)
		{
			if(pcb.rframe->doff < 0)
			{
				uncomp_ramframe(seekcel,pcb.rframe,
										pcb.flif.hdr.width,
										pcb.flif.hdr.height,
										seek_one);
			}
			else
			{
				if((frame_oset = pj_seek(pcb.flif.fd,pcb.rframe->doff,
										 JSEEK_START)) < Success)
				{
					return(frame_oset);
				}
				if((err = pj_fli_read_uncomp(pcb.script_mode?NULL:pcb.fliname,
								&pcb.flif,seekcel,cbuf,seek_one)) < Success)
				{
					goto error;
				}
			}

			/* last one is ring frame if present. Some plays may not have a
			 * ring frame */

			if((pcb.rframe = pcb.rframe->next) == NULL)
			{
				pcb.rframe = pcb.rfli->frames->next;
				i = 0;
			}
		}

		if((frame_oset = pcb.rframe->doff) < 0)
			frame_oset = -frame_oset;

		goto seek_done;
	}

	if((frame_oset = pj_seek(pcb.flif.fd,frame_oset,JSEEK_START)) < Success)
	{
		err = frame_oset;
		goto error;
	}

	while(i++ != frame)
	{
		if((err = pj_fli_read_uncomp(pcb.script_mode?NULL:pcb.fliname,
								&pcb.flif,seekcel,cbuf,seek_one)) < Success)
		{
			goto error;
		}

		if(i >= pcb.flif.hdr.frame_count)
		{
			if((frame_oset = pj_seek(pcb.flif.fd,
						     pcb.flif.hdr.frame2_oset,JSEEK_START)) < Success)
			{
				err = frame_oset;
				goto error;
			}
			i = 0;
		}
		else
		{
			frame_oset += cbuf->size; /* add size of frame read in */
		}
	}


seek_done:

	if(!seek_one)
	{
		if(seekcel != pcb.dcel)
		{
			pj_blitrect(seekcel,0,0,pcb.dcel,0,0,
						pcb.dcel->width,pcb.dcel->height);
			pj_cmap_copy(seekcel->cmap,pcb.dcel->cmap);
			pj_rcel_free(seekcel);
		}
		if(cmap_cksum != cmap_crcsum(pcb.dcel->cmap))
			pj_cmap_load(pcb.dcel, pcb.dcel->cmap);
	}

	if(frame == 0)
		pcb.next_frame_oset = pcb.flif.hdr.frame2_oset;
	else
		pcb.next_frame_oset = frame_oset;

done:
	pcb.frame_ix = frame;
	err = Success;
error:
	return(err);
}
void pla_seek_first(void *dat)
{
	if(pcb.frame_ix != 0)
		pla_read_first();
}
void pla_seek_last(void *dat)
{
	pla_seek_frame(pcb.flif.hdr.frame_count - 1);
}
static Boolean is_abort_key()
{
	if(pcb.script_mode 
		&& !pcb.lock_key
		&& (icb.inkey == CTRL_C || ((UBYTE)icb.inkey) == ESCKEY))
	{
		pcb.abort_key = toupper_inkey();
		return(TRUE);
	}
	return(FALSE);
}
Errcode check_play_abort()
{
int hitkey;

	if(!check_input(KEYHIT|MBRIGHT|MBPEN))
		return(Success);

	/* here we check for input to callchoice() or linkchoice() 
	 * this will only be true during script mode */

	if(pcb.choices != NULL)
	{
		if(!JSTHIT(KEYHIT))
			return(Success);

		if(is_abort_key() && (UBYTE)icb.inkey != ESCKEY)
			return(Err_abort);

		if(pcb.choice) /* if another key is hit continue aborting */
			return(Success + 1);

		hitkey = toupper((UBYTE)icb.inkey);

		if(hitkey && strchr(pcb.choices,hitkey) != NULL)
		{
			pcb.choice = hitkey;
			return(Success + 1);
		}
		check_script_keys(); 
		pcb.choice = 0;
		return(Success);
	}

	if(pcb.lock_key)
	{
		if(JSTHIT(KEYHIT))
			check_script_keys();
		return(Success);
	}

	if(!JSTHIT(KEYHIT))
	{
		if(JSTHIT(MBPEN))
		{
			if(!pcb.script_mode) /* no pen aborts in menu mode */
				return(Success);
			pcb.abort_key = AKEY_REPLAY;
		}
		goto abort_ok;
	}

	if(is_abort_key())
		return(Err_abort);

	if(check_script_keys())
		return(Success);

	pcb.abort_key = toupper_inkey();
	
abort_ok:

	if(pcb.script_mode)
		return(Success + 1);
	pcb.abort_key = ESCKEY;
	return(Err_abort);
}
Errcode play_fli()

/* returns Err_abort if aborted by control c key (Success + 1)
 * if by another key */
{
Errcode err;
int mouse_was_on;
int loops;

	mouse_was_on = hide_mouse();
	pcb.cktime = pj_clock_1000();
	loops = pcb.loops;

	for(;;)
	{
		if( pcb.script_mode 
			&& (pcb.frame_ix + 1) == pcb.flif.hdr.frame_count
			&& --loops == 0 )
		{
			err = Success;
			break;
		}

		for(;;)
		{
			if((err = check_play_abort()) != Success)
				goto loop_aborted;
				
			if((pcb.cktime + pcb.speed) < pj_clock_1000())
				break;
		}
		pcb.cktime += pcb.speed;

		if((pla_seek_frame(pcb.frame_ix + 1)) < Success)
			goto error;

		if(pcb.cktime > pj_clock_1000())	 /* wrap */
			pcb.cktime = pj_clock_1000();

	}

loop_aborted:

	pcb.frame_ix = fli_wrap_frame(&pcb.flif,pcb.frame_ix);

error:
	if(mouse_was_on)
		show_mouse();
	return(err);
}
Errcode load_picture(char *pdr_name,char *picname, Boolean load_colors)
{
Errcode err;
char pdr_path[PATH_SIZE];
Pdr *pd;
Image_file *ifile;
Anim_info ainfo;
void *libfunc;
Rastlib *lib;

	close_curfli();
	make_resource_name(pdr_name,pdr_path);

	if((err = load_pdr(pdr_path, &pd)) < Success)
	{
		cant_use_module(err,pdr_path);
		if(pcb.script_mode)
			return(err);
		return(Err_reported);
	}

	get_screen_ainfo(vb.pencel,&ainfo);
	if((err = pdr_open_ifile(pd, picname, &ifile, &ainfo)) < Success)
		goto error;

	make_playcel(ainfo.width, ainfo.height);

	/* this is a bit of a fudge to prevent the picture driver from loading the
	 * colors into the screen by setting the library function to do 
	 * nothing */

	lib = vb.screen->viscel->lib;
	libfunc = lib->set_colors;

	if(!load_colors)
		lib->set_colors = (rl_type_set_colors)pj_vdo_nutin;

	err = pdr_read_first(ifile,pcb.dcel);
	lib->set_colors = libfunc;

	pdr_close_ifile(&ifile);

error:
	free_pdr(&pd);
	return(err);
}
static SHORT pla_get_frameix(void *data)
{
	return(pcb.frame_ix);
}
static SHORT pla_get_framect(void *data)
{
	return(Max(pcb.flif.hdr.frame_count,1));
}
static void pla_play_fli()
{
	hide_mp();
	play_fli();
	show_mp();
}

Minitime_data playfli_data = {
	pla_seek_first,  /* first frame */
	NULL,   		 /* prev */
	NULL, 			 /* ix */
	NULL,      		 /* next */
	pla_play_fli,	  		 /* play it */
	pla_seek_last,   /* last frame */
	NULL, /* opt_all */
	NULL, /* opt_tsl_first */
	pla_get_frameix, 	/* get_frame_ix */
	pla_get_framect,	/* get_frame_count */
	NULL, /* clear_overlays used to clean up frame before seeking etc */
	NULL, /* draw_overlays used to restore overlays after seeking etc */
	pla_seek_frame, /* (*seek_frame)(SHORT ix, void *data); */
	0,	  /* start with a clear stack */
	NULL, /* data */
};
