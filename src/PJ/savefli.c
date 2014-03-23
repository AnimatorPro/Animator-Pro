/* savefli.c - Make a FLIC file from temp file (temp.flx) on scratch
   device.  This thing just takes out the indexing.  Doesn't actually
   have to recompress any images.  (That's done in writefli.c). */

#include <string.h>
#include "jimk.h"
#include "animinfo.h"
#include "auto.h"
#include "commonst.h"
#include "errcodes.h"
#include "flx.h"
#include "ftextf.h"
#include "menus.h"
#include "picdrive.h"
#include "ptrmacro.h"
#include "softmenu.h"
#include "unchunk.h"

char dirty_file;
char dirty_frame;
long dirty_strokes;

extern char *get_flisave_pdr(char *pdr_path);

void dirties(void)
{
	dirty_file = dirty_frame = 1;
	dirty_strokes +=  1;
}

void cleans(void)
{
	dirty_file = 0;
	dirty_frame = 0;
	dirty_strokes = 0;
}

Boolean need_scrub_frame(void)
{
	return(dirty_frame);
}
Errcode scrub_frame_save_undo(void)
/* a scrub cur frame that insures undo is current index and saves it if not */
{
Errcode ret;

	if((ret = scrub_cur_frame()-1) != vs.frame_ix)
		save_undo();
	return(ret);
}
Errcode scrub_cur_frame(void)

/* scrub_cur_frame() - updates tflx with vf, reports errors.
 *
 * Returns index of frame left in undo + 1 or errcode 0 if undo is 
 * is left as it was, < 0 if error */
{
int frameix;

	if(dirty_frame)
	{
		if((frameix = sub_cur_frame()) < 0)
			return(softerr(frameix,"fli_insert"));
		dirty_frame = 0;
		return(frameix + 1); /* did one ok */
	}
	else
		return(0); /* didn't do it */
}
Errcode write_flx_frame(Flxfile *flx, int ix, Fli_frame *frame)
/* will re write frame record into current slot or another if needed */
{
LONG size;

	size = frame->size;
	if(size <= sizeof(*frame))
		size = 0;
	return(make_flx_record(flx, ix, frame,size,TRUE));
}

Errcode sub_cur_frame(void)
/* returns frame index left in undo buffer errcode if not possible */
{
Errcode err;
int pushed = 0;
int unzoomed = 0;
long cbufsz;
Fli_frame *cbuf;
Fli_frame *cbuf2;
void *alloc2;
SHORT undoix;
Boolean overwrite;
Flx ocurflx;

	if (!flix.fd)
		return(Err_bad_input);

	flx_clear_olays(); /* undraw cels cursors etc */

	if (vs.frame_ix == vs.bframe_ix)	/* mark buffer frame as no good */
		vs.bframe_ix = 0;
	/* grovel for memory... freeing up more and more ... */

	cbufsz = pj_fli_cbuf_size(vb.pencel->width,vb.pencel->height,
					   vb.pencel->cmap->num_colors);

	if ((cbuf = pj_malloc(cbufsz)) == NULL)
	{
		unzoomed = 1;
		unzoom();
		if ((cbuf = pj_malloc(cbufsz)) == NULL)
		{
			pushed = 1;
			push_most();
			if ((cbuf = begmem(cbufsz)) == NULL)
			{
				err = Err_reported;
				goto error;
			}
		}
	}

	/* put old frame 0 in undo buffer if not compressing the one and 
	 * only first frame */

	if(flix.hdr.frame_count != 1)
	{
		if ((err = gb_unfli(undof,0,0,cbuf)) < 0)
			goto error;
	}

	overwrite = TRUE; /* allow current record to be overwritten unless
					   * we find we can't allocate buffers etc */

	if(vs.frame_ix == 0)
	{
		undoix = 0;
		pj_fli_comp_frame1(cbuf,vb.pencel,flix.comp_type);
		if((err = write_flx_frame(&flix,0,cbuf))<0)
			goto error;
		if(flix.hdr.frame_count == 1)
		{
			save_undo();
			undoix = 0;
			goto done;
		}
	}
	else
	{
		/* seek to frame before and compress current delta */
		if(vs.frame_ix > 1)
		{
			if ((err = gb_fli_tseek(undof,0,vs.frame_ix-1, cbuf)) < 0)
				goto error;
		}
		ocurflx = flix.idx[vs.frame_ix];
		if(ocurflx.fsize == 0)
			ocurflx.fsize = sizeof(Fli_frame);

		pj_fli_comp_cel(cbuf,undof,vb.pencel,COMP_DELTA_FRAME,flix.comp_type);

		if((cbufsz - cbuf->size) >= ocurflx.fsize)
		{
			cbuf2 = OPTR(cbuf,cbuf->size);
			alloc2 = NULL;
		}
		else 
		{
			if((alloc2 = pj_malloc(ocurflx.fsize)) == NULL)
				overwrite = FALSE;
			cbuf2 = alloc2;
		}
		if(overwrite) /* unfli old frame via cbuf2 we may overwrite it */
		{
			err = gb_unfli(undof,vs.frame_ix,0,cbuf2);
			pj_freez(&alloc2); /* get rid of any alloc'd buffers */
			if(err < Success)
				goto error;
		}

		if(cbuf->size <= sizeof(Fli_frame))
			cbuf->size = 0;

		if((err = make_flx_record(&flix,vs.frame_ix,
								  cbuf,cbuf->size,overwrite)) < 0)
		{
			goto error;
		}

		/* re read old record from its old file slot */
		if(!overwrite && ocurflx.fsize > sizeof(Fli_frame))
		{
			if((err = pj_readoset(flix.fd,cbuf,ocurflx.foff,ocurflx.fsize))<0)
				goto error;
			if(cbuf->type != FCID_FRAME)
			{
				err = Err_bad_magic;
				goto error;
			}
			pj_fli_uncomp_frame(undof,cbuf,0);
		}
	}

	/* unfli to next frame */
	undoix = vs.frame_ix+1;
	if((err = gb_unfli(undof,undoix,0,cbuf)) < 0)
		goto error;

	pj_fli_comp_cel(cbuf,vb.pencel,undof,COMP_DELTA_FRAME,flix.comp_type);

	if((err = write_flx_frame(&flix,undoix,cbuf)) < 0)
		goto error;

	/* possibly have to update last loop frame if changing first frame 
	 * and more than one frame in fli */

	if(vs.frame_ix == 0)
	{
		/* advance undo to last frame in file */
		if((err = gb_fli_tseek(undof,undoix,flix.hdr.frame_count-1,cbuf))<0)
			goto error;
		undoix = flix.hdr.frame_count-1;
		pj_fli_comp_cel(cbuf,undof,vb.pencel,COMP_DELTA_FRAME,flix.comp_type);
		if((err = write_flx_frame(&flix,flix.hdr.frame_count,cbuf)) < 0)
			goto error;
	}

done:
	flush_tflx();
	err = wrap_frame(undoix);
error:
	pj_gentle_free(cbuf);
	if (pushed)
		pop_most();
	if (unzoomed)
		rezoom();
	flx_draw_olays(); /* restore cels and such */
	return(err);
}
static Errcode copy_flx_prefix(Flxfile *flx, Flifile *flif)

/* copys or makes the appropriate prefix chunks from the tempflx to the output
 * flifile and sets the frame1_oset in the output filifile and leaves
 * the output file position at the start of the first frame chunk */
{
Chunkparse_data pd;
Errcode err;

	init_chunkparse(&pd,flx->fd,FCID_PREFIX,sizeof(Flx_head),0,0);
	while(get_next_chunk(&pd))
	{
		switch(pd.type)
		{
			case FP_FLIPATH:  /* ignore these */
			case FP_FREE:
				break;
			case ROOT_CHUNK_TYPE: /* save old size */

				if((err = copy_parsed_chunk(&pd, flif->fd)) < Success)
					goto error;

				/* install a settings chunk following prefix chunk */

				if((err = write_fli_settings(flif->fd,
											FP_VSETTINGS)) < Success)
				{
					goto error;
				}
				break;
			default:
				if((err = copy_parsed_chunk(&pd, flif->fd)) < Success)
					goto error;
				break;
		}
	}

	if(pd.error < Success)
	{
		err = pd.error;
		goto error;
	}

	/* install prefix size write it and re-seek to end of prefix */

	pd.fchunk.size = pj_tell(flif->fd) - sizeof(Fli_head);
	pd.fchunk.type = FCID_PREFIX;

	if((err = pj_writeoset(flif->fd,&pd.fchunk,
						   sizeof(Fli_head),sizeof(Chunk_id))) < Success)
	{
		goto error;
	}

	/* back to end of file */
	err = pj_seek(flif->fd,pd.fchunk.size+sizeof(Fli_head),JSEEK_START);

error:
	return(err);
}
static Errcode save_fli_start(char *name, Flifile *flif)

/* saves up to first frame of new fli from flx */
{
Errcode err;

	flush_tflx(); /* so prefix chunk is updated */
	if((err = pj_fli_create(name,flif)) < 0)
		return(err);

	/* copy common fields from Flx_head of tflx */

	copy_fhead_common((Fli_head *)&flix.hdr, &flif->hdr);

	/* copy in aspect ratio from screen */
	flif->hdr.aspect_dx = vb.pencel->aspect_dx;
	flif->hdr.aspect_dy = vb.pencel->aspect_dy;

	/* copy prefix chunks from tflx to fli prefix chunk */

	return(copy_flx_prefix(&flix,flif));
}
Errcode sv_fli(char *name)

/* writes whole current tflx out to a fli file */
{
int i;
Fli_frame *cbuf;
Flifile flif;    /* output fli */
Errcode err;

	if(!flix.fd)
		return(Err_bad_input);
	clear_struct(&flif);

	scrub_cur_frame();
	if((err = pj_fli_cel_alloc_cbuf(&cbuf,vb.pencel)) < 0)
		goto error;

	if((err = save_fli_start(name, &flif)) < 0)
		goto error;

	/* write out all tflx frame chunks to fli file */

	for (i=0; i<=flix.hdr.frame_count; i++)
	{
		if((err = read_flx_frame(&flix,cbuf,i)) < 0)
			goto error;

		if(i == 0)
			err = pj_i_add_frame1_rec(name,&flif,cbuf);
		else if(i == flix.hdr.frame_count)
			err = pj_i_add_ring_rec(name,&flif,cbuf);
		else
			err = pj_i_add_next_rec(name,&flif,cbuf);

		if(err < Success)
			goto error;
	}
	cleans();

error:
	err = softerr(err,"!%s", "cant_save", name);
	pj_fli_close(&flif);
	pj_gentle_free(cbuf);
	return(err);
}

static Errcode save_fli(char *name)
/* save whole fli without altering records */
{
Errcode err;
int oix;

	unzoom();
	if((err = sv_fli(name)) >= 0)
	{
		oix = vs.frame_ix;
		make_tempflx(name,0);
		vs.frame_ix = oix;
	}
	else
		pj_delete(name);

	rezoom();
	return(err);
}
/******** stuff to save a segment of the flx to a fli file *********/

struct pdr_seek_dat {
	char *path;       /* path for abort check */
	int sstart;
	int send;
	int num_frames;
	int inc;   /* + or - 1 */
	Boolean undo_used;
	SHORT cur_frame;
	int cur_ix;
};

static Errcode pdr_seek_seg_frame(int ix, void *data)
{
struct pdr_seek_dat *sd = data;
Errcode err;
LONG ocksum;
Rcel *tcel;
Errcode (*flx_seek)(Rcel *screen, int cur_ix, int ix);

	if(poll_abort() < Success)
	{
		if(soft_yes_no_box("!%s", "save_abort", pj_get_path_name(sd->path)))
			return(Err_abort);
	}

	if(ix == sd->cur_ix)
		return(Success);

	sd->cur_ix = ix;

	/* wrap frame index */

	if((ix = ix % sd->num_frames) < 0)
		ix += sd->num_frames;

	/* get index from sstart relative index */

	ix = sd->sstart + (ix * sd->inc);

	/* seek to frame */

	ocksum = cmap_crcsum(vb.pencel->cmap);

	if(ix < sd->cur_frame)
		flx_seek = fli_tseek;
	else
		flx_seek = flx_ringseek;

	if(ix != sd->cur_frame + 1)
	{
		/* If not seeking to next frame, and undo is unused seek in undo.
		 * Otherwise try to clone screen, and seek in clone and copy back
		 * if not enough mem seek on main screen. All this just to make
		 * things look good. */

		if(!sd->undo_used)
		{
			err = (*flx_seek)(undof,sd->cur_frame, ix );
			zoom_unundo();
			goto cmap_done;
		}

		if(NULL != (tcel = clone_any_rcel(vb.pencel)))
		{
			if((err = (*flx_seek)(tcel, sd->cur_frame, ix)) >= Success)
				pj_rcel_copy(tcel, vb.pencel);
			pj_rcel_free(tcel);
			if(err >= Success)
				goto check_cmap;
		}
	}
	err = (*flx_seek)(vb.pencel, sd->cur_frame, ix);

check_cmap:

	if(ocksum != cmap_crcsum(vb.pencel->cmap))
		pj_cmap_load(vb.pencel, vb.pencel->cmap);

cmap_done:

	sd->cur_frame = ix;
	return(err);
}
static Errcode pdr_save_flx_segment(char *pdr_name,char *flicname,
								    SHORT sstart, SHORT send)
{
Errcode err;
Pdr *pd;
Image_file *ifile = NULL;
Anim_info ainfo;
Anim_info spec;
struct pdr_seek_dat psd;
Rcel *work_screen;
Rcel *seek_screen;
Rcel virt_a;
Rcel virt_b;

	if((err = load_pdr(pdr_name, &pd)) < Success)
		return(cant_use_module(err,pdr_name));

	get_screen_ainfo(vb.pencel,&spec);
	if((spec.num_frames = send - sstart) < 0)
		spec.num_frames = -spec.num_frames;
	++spec.num_frames;

	spec.millisec_per_frame = flix.hdr.speed;

	ainfo = spec;

	if(!pdr_best_fit(pd, &spec))
	{
		if(    spec.depth < ainfo.depth
			|| spec.num_frames != ainfo.num_frames
			|| spec.width != ainfo.width
			|| spec.width != ainfo.height)
		{
			if(!soft_yes_no_box("!%d%d%d%d%d%d%d%d", "animsv_exact",
						   ainfo.width, ainfo.height,
						   0x0FF, 
						   ainfo.num_frames,
						   spec.width, spec.height,
						   (spec.depth < 8? 0x0FF>>(8 - spec.depth): 0x0FF),
						   spec.num_frames ))
			{
				err = Err_abort;
				goto done;
			}
		}
	}

	if((err = pdr_create_ifile(pd, flicname, &ifile, &spec )) < Success)
		goto error;

	/* load seek data */

	psd.path = flicname;
	psd.sstart = sstart;
	psd.send = send;
	psd.num_frames = spec.num_frames;
	psd.undo_used = ifile->needs_work_cel;
	psd.cur_frame = sstart;
	psd.cur_ix = 0;

	if(sstart <= send)
		psd.inc = 1;
	else
		psd.inc = -1;

	/* make sure we are on first frame */

	save_undo();
	if((err = fli_tseek(undof,vs.frame_ix,sstart)) < Success)
		goto error;
	zoom_unundo();

/****************
 *	Errcode (*save_frames)(Image_file *ifile, 
 *						   Rcel *screen, 
 *						   int num_frames, 
 *						   Errcode (*seek_frame)(int ix,void *seek_data),
 *						   void *seek_data,
 *						   Rcel *work_screen );
 ******************/


	start_abort_atom();

	seek_screen = center_virtual_rcel(vb.pencel, &virt_a, 
									  spec.width, spec.height);
	work_screen = center_virtual_rcel(undof, &virt_b, 
									  spec.width, spec.height);

	err = pdr_save_frames(ifile, seek_screen, spec.num_frames, 
						  pdr_seek_seg_frame, &psd, work_screen);


	if((err = errend_abort_atom(err)) < Success)
		goto error;
	goto done;

error:
	pdr_close_ifile(&ifile);
	pj_delete(flicname);
	goto out;
done:
	pdr_close_ifile(&ifile);
out:
	free_pdr(&pd);
	return(err);
}
static Errcode save_flx_segment(char *title,SHORT sstart, SHORT send)

/* returns ecode if cant do */
{
int i, last;
Flifile flif;
Fli_frame *cbuf;
Errcode err;
char pdr_name[PATH_SIZE];
Boolean fli_format;
SHORT oframe_ix;

	if(!flix.fd)
		return(Err_bad_input);

	if(sstart >= flix.hdr.frame_count)
		sstart = flix.hdr.frame_count-1;
	if(send >= flix.hdr.frame_count)
		send = flix.hdr.frame_count-1;

	oframe_ix = vs.frame_ix;
	clear_struct(&flif); /* for error out */
	cbuf = NULL;

	fli_format = (is_fli_pdr_name(get_flisave_pdr(pdr_name)));

	if(fli_format && sstart == 0 && send == flix.hdr.frame_count-1)
		return(save_fli(title));

	hide_mp();
	unzoom();
	push_most();
	scrub_cur_frame();

	if(!fli_format)
	{
		if((err = pdr_save_flx_segment(pdr_name,title,sstart,send)) < Success)
			goto error;
		goto done;
	}

	if((err = pj_fli_cel_alloc_cbuf(&cbuf,vb.pencel)) < 0)
		goto error;

	save_undo();
	gb_fli_tseek(undof,vs.frame_ix,sstart,cbuf);

	if((err = save_fli_start(title, &flif)) < 0)
		goto error;

	if((err = pj_fli_add_frame1(title, &flif, cbuf, undof)) < 0)
		goto error;

	if(sstart > send)
	{
		last = sstart;
		i = last - 1;
		while(i >= send)
		{
			zoom_unundo();
			gb_fli_tseek(undof,last,i,cbuf);
			last = i--;
			if((err = pj_fli_add_next(title, &flif, 
									   cbuf,vb.pencel,undof)) < 0)
			{
				goto error;
			}
		}
	}
	else /* forward, no recompression needed */
	{
		i = sstart+1;
		while(i <= send)
		{
			if((err = read_flx_frame(&flix,cbuf,i)) < 0)
				goto error;
			if((err = pj_i_add_next_rec(title, &flif, cbuf)) < 0)
				goto error;
			++i;
		}
		gb_fli_tseek(undof,sstart,send,cbuf);
	}

	zoom_unundo();
	gb_fli_tseek(undof,send,sstart,cbuf);
	if((err = pj_fli_add_ring(title,&flif,cbuf,vb.pencel,undof)) < 0)
		goto error;

	goto done;
error:
	pj_delete(title);
done:
	pj_gentle_free(cbuf);
	pj_fli_close(&flif);
	fli_abs_tseek(undof,(vs.frame_ix = oframe_ix));
	zoom_unundo();
	pop_most();
	rezoom();
	show_mp();
	return(err);
}
static Boolean save_as_fli(void)
{
char pdr_name[PATH_SIZE];
	return(is_fli_pdr_name(get_flisave_pdr(pdr_name)));
}
static void ask_qsave_seg(char *title_key, char *save_word,
						  SHORT sstart, SHORT send)
{
Errcode err;
char suffi[PDR_SUFFI_SIZE];
char titbuf[80];
char pdrinfo[40];
char sbuf[50];
char *flicname;
int num_frames;

	if((num_frames = send - sstart) < 0)
		num_frames = -num_frames;
	++num_frames;

	if((err = get_flisave_info(suffi, pdrinfo, sizeof(pdrinfo))) < Success)
		goto error;

	snftextf(titbuf,sizeof(titbuf),"!%d%d%s", stack_string(title_key,sbuf),
			 sstart+1, send+1, pdrinfo );

	if(!save_as_fli())
	{
		/* arrrgh take of trailing period from title */
		flicname = &pdrinfo[strlen(pdrinfo) - 1];	
		if(*flicname == '.')
			*flicname = 0;
		if(!soft_yes_no_box("!%d%s", "save_conv", num_frames, &pdrinfo ))
			return;
	}

	if ((flicname = vset_get_filename(titbuf, suffi, save_word,
				      		     	 FLI_PATH,NULL,1))!=NULL)
	{
		if(!overwrite_old(flicname))
			return;
		if((err = save_flx_segment(flicname,sstart,send)) < Success)
			goto error;
	}
error:
	softerr(err,"!%s", "fli_savef", flicname);
}

void qsave(void)
{
	ask_qsave_seg("save_fli", save_str, 0, flix.hdr.frame_count-1 );
}
void qsave_backwards(void)
/* Save out current FLIC with frames backwards. */
{
	ask_qsave_seg("save_fli_back", ok_str, flix.hdr.frame_count-1, 0 );
}
void qsave_segment(void)
/* Save out current segment of FLIC */
{
	clip_tseg();
	ask_qsave_seg("save_fli_seg", ok_str, vs.start_seg, vs.stop_seg );
}

