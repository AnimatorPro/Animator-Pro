#include <string.h>
#include "jimk.h"
#include "animinfo.h"
#include "commonst.h"
#include "errcodes.h"
#include "flicel.h"
#include "flipath.h"
#include "menus.h"
#include "picdrive.h"
#include "picfile.h"
#include "softmenu.h"
#include "unchunk.h"
#include "util.h"
#include "zoom.h"

Errcode save_fcel_temp(Flicel *fc)

/* (re)creates and writes out a .tmp file for a flicel if cel 
 * has a temp name a cel with FCEL_RAMONLY flag set cnat have a temp file
 * saved for it */
{
Flifile flif;
Errcode err;
Chunk_id fchunk;
char *path;

	if(!fc || !fc->cpath || NULL == (path = fc->tpath))
		return(Err_bad_input);

	fc->cpath->fid = fc->flif.hdr.id; /* update cpath id from cel fli header */

	if((err = pj_fli_create(path,&flif)) < Success)
		goto error;

	flif.hdr.width = fc->flif.hdr.width;
	flif.hdr.height = fc->flif.hdr.height;
	flif.hdr.bits_a_pixel = fc->flif.hdr.bits_a_pixel;
	/* frame_count = 0 */

	fchunk.size = sizeof(Celdata) + fc->cpath->id.size;
	fchunk.type = FCID_PREFIX;

	if((err = pj_write_ecode(flif.fd,&fchunk,sizeof(fchunk))) < Success)
		goto error;

	if((err = pj_write_ecode(flif.fd,&fc->cd,sizeof(Celdata))) < Success)
		goto error;

	if((err = pj_write_ecode(flif.fd,fc->cpath,
							fc->cpath->id.size)) < Success)
	{
		goto error;
	}

	if((err = pj_i_flush_head(&flif)) < Success)
		goto error;

	pj_fli_close(&flif);
	return(Success);
error:
	pj_fli_close(&flif);
	pj_delete(path);
	return(err);
}
Errcode create_celfli_start(char *tempname, char *fliname, 
							Flicel **pfcel, Rcel *rc)
{
Errcode err;
Flicel *fc;

	if((err = alloc_fcel(pfcel)) < Success)
		return(err);

	fc = *pfcel;
	fc->rc = rc;
	fc->flags |= FCEL_OWNS_RAST;

	if(tempname)
	{
		if(NULL == (fc->tpath = clone_string(tempname)))
		{
			err = Err_no_memory;
			goto error;
		}
	}

	if(fliname)
	{
		if((err = pj_fli_create(fliname,&fc->flif)) < Success)
			goto error;
	}

	fc->flif.hdr.width = rc->width;
	fc->flif.hdr.height = rc->height;
	fc->flif.hdr.bits_a_pixel = 8; /* rc->pdepth; */
	fc->flif.hdr.speed = 71;

	/* fudge for aspect ratio */
	fc->flif.hdr.aspect_dx = vb.pencel->aspect_dx;
	fc->flif.hdr.aspect_dy = vb.pencel->aspect_dy;

	set_fcel_center(fc,rc->x + (rc->width>>1),rc->y + (rc->height>>1));

	if(fliname)
	{
		if((err = alloc_flipath(fliname,&fc->flif,&fc->cpath)) < Success)
			goto error;
	}

	return(Success);
error:
	fc->rc = NULL;
	fc->flags &= ~FCEL_OWNS_RAST;
	free_fcel(pfcel);
	if(fliname)
		pj_delete(fliname);
	return(err);
}
Errcode make1_flicel(char *tempname, char *fliname, Flicel **pfcel, Rcel *rc)

/* makes a one frame flicel from a pic in an rcel it attaches the rcel to it
 * A celinfo.tmp file is created and a file that has the cel as a 1 frame fli
 * if the file names are non NULL. If either file name is NULL neither file
 * is created, and the flag FCEL_RAMONLY is set.
 * It takes posession of the rcel and will take care of freeing it in
 * free_fcel() note this is only called with a pointer to thecel for
 * now and does a jdelete(cel_name) on error */
{
Errcode err;
Flicel *fc;

	if(tempname == NULL || fliname == NULL)
		tempname = fliname = NULL;

	if((err = create_celfli_start(tempname,fliname,pfcel,rc)) < Success)
		goto error;

	fc = *pfcel;

	set_flicel_tcolor(fc,vs.inks[0]); /* set to current tcolor */

	if(fliname == NULL)
	{
		fc->flags |= FCEL_RAMONLY;
		fc->flif.hdr.frame_count = 1;
	}
	else
	{
		/* create temp cel fli file records with one frame */
		if((err = pj_write_one_frame_fli(fliname,&fc->flif,rc)) < Success)
			goto error;
		pj_fli_close(&fc->flif);

		/* write an info file for this cel's fli */
		if((err = save_fcel_temp(fc)) < Success)
			goto error;
	}

	return(Success);
error:
	fc->rc = NULL;
	free_fcel(pfcel);
	if(tempname)
	{
		pj_delete(tempname);
		pj_delete(fliname);
	}
	return(err);
}
Errcode load_fli_fcel(char *flipath,char *tempname,char *celfli_name,
					  Flicel **pfc)

/* loads a fli file as a newly allocated flicel, if tempname is non-null
 * builds a temp fli from the fli or points a tempname file to the fli 
 * if the tempname is NULL it will not build a temp file and will point the 
 * ram cel to the fli even if the fli is on a removable device */
{
Errcode err;
Flifile flif;
Flicel *fc;
LONG chunksize;
Chunkparse_data pd;
Boolean make_flicopy; 
Boolean found_celdata;
char device[DEV_NAME_LEN];

	if((err = alloc_fcel(pfc)) < Success)
		return(err);
	fc = *pfc;

	if(tempname)
	{
		if(NULL == (fc->tpath = clone_string(tempname)))
		{
			err = Err_no_memory;
			goto error;
		}
	}

	clear_struct(&flif);

	if((err = get_path_device(flipath,device)) < Success) 
		goto error;

	/* if allowed make sure cel fli file is not on removable drive */
	make_flicopy = (tempname != NULL   
					&& celfli_name != NULL  
					&& !pj_is_fixed(device));

	/* attempth to open fli requested as cel */

	if(make_flicopy)
	{
		if((err = pj_fli_open(flipath,&flif,JREADONLY)) < Success)
			goto error;
	}
	else
	{
		if((err = pj_fli_open(flipath,&flif,JREADWRITE)) < Success)
			goto error;

		/* we've got to have a valid update time ! */
		if(flif.hdr.id.update_time == 0)
		{
			if((err = pj_i_flush_head(&flif)) < Success)
				goto error;
		}
	}

	found_celdata = FALSE;
	init_chunkparse(&pd,flif.fd,FCID_PREFIX,sizeof(Fli_head),0,0);
	while(get_next_chunk(&pd))
	{
		if(pd.type == FP_CELDATA)
		{
			if(pd.fchunk.size == sizeof(Celdata))
			{
				/* try to read it */
				pd.error = read_parsed_chunk(&pd,&fc->cd,-1); 
				found_celdata = TRUE;
			}
			break;
		}
	}

	if(pd.error < Success && pd.error != Err_no_chunk)
	{
		err = pd.error;
		goto error;
	}

	if(!found_celdata)
	{
		/* No positioning chunk found. Just put cel in upper left corner */
		fc->cd.cent.x = flif.hdr.width>>1;
		fc->cd.cent.y = flif.hdr.height>>1;
	}


	/* load fli dimensions and alloc raster for fli frames */

	fc->flif.hdr.width = flif.hdr.width;
	fc->flif.hdr.height = flif.hdr.height;
	if ((err = alloc_fcel_raster(fc)) < Success)
		goto error;

	refresh_flicel_pos(fc);

	if(make_flicopy)
	{
		if((err = pj_fli_create(cel_fli_name,&fc->flif)) < Success)
			goto error;

		fc->flif.hdr = flif.hdr;

		fc->flif.hdr.frame1_oset = sizeof(Fli_head);
		fc->flif.hdr.frame2_oset = sizeof(Fli_head) 
						+ (flif.hdr.frame2_oset - flif.hdr.frame1_oset);

		chunksize = flif.hdr.size - flif.hdr.frame1_oset;

		if((err = pj_copydata_oset(flif.fd, fc->flif.fd,
					   flif.hdr.frame1_oset, 
					   fc->flif.hdr.frame1_oset, chunksize )) < Success)
		{
			softerr(err,"first_ok");
			err = 0;
		}
		if((fc->flif.hdr.size = pj_tell(fc->flif.fd)) < Success)
		{
			err = fc->flif.hdr.size;
			goto error;
		}
		/* flush final version of header */

		if((err = pj_i_flush_head(&fc->flif)) < Success)
			goto error;

		pj_fli_close(&flif); /* close source file */
		flipath = cel_fli_name;
	}
	else
	{
		fc->flif = flif; /* source file is cel fli */
		clear_struct(&flif); /* so won't close twice */
	}

	/* make path record */

	if((alloc_flipath(flipath,&fc->flif,&fc->cpath)) < Success)
		goto error;

	/* load image and seek to current frame */

	if((err = seek_fcel_frame(fc,fc->cd.cur_frame)) < Success)
		goto error;

	pj_fli_close(&fc->flif);
	return(Success);
error:
	pj_fli_close(&flif);
	free_fcel(pfc);
	if(tempname)
		pj_delete(tempname);
	if(celfli_name)
		pj_delete(celfli_name);
	return(err);
}

void close_fcelio(Flicel *fc)
{
	pj_fli_close(&fc->flif);
}
Errcode reopen_fcelio(Flicel *fc, int jmode)

/* for an extant fli cel reopen and verify it's file in read only mode */
{
Errcode err;
char *path;
Flifile flif;
Fli_id oid;

	if(fc->flags & FCEL_RAMONLY
		|| get_jmode(fc->flif.fd) == jmode) /* already open */
	{
		return(Success);
	}

	pj_fli_close(&fc->flif);
	if(!fc->cpath)
		return(Err_bad_input);

	path = fc->cpath->path;
	oid = fc->cpath->fid;

	if((err = pj_fli_open(path, &flif, jmode)) < Success)
		goto error;

	if(memcmp(&oid,&flif.hdr.id,sizeof(Fli_id))
		|| flif.hdr.width != fc->rc->width
		|| flif.hdr.height != fc->rc->height )
	{
		err = Err_invalid_id;
		goto error;
	}
	fc->flif = flif;
	return(Success);
error:
	pj_fli_close(&flif);
	err = softerr(err,"!%s", "fcel_reopen", path);
	return(err);
}
Errcode gb_seek_fcel_frame(Flicel *fc, SHORT frame,Fli_frame *cbuf,
						   Boolean force_read)

/* if cbuf is null it will allocate one! */
{
int i;
Errcode err;
LONG frame_oset;
Rcel *rc;
Boolean reopened;
Boolean allocd = FALSE;

	if(fc->flags & FCEL_RAMONLY) /* no seeking on ram cel frames */
		return(Success);

	if(0 != (reopened = (fc->flif.fd == JNONE)))
	{
		/* note that this reports errors */
		if((err = reopen_fcelio(fc,JREADONLY)) < Success)
			goto error;
	}

	rc = fc->rc;

	/* wrap frame */
	frame = fli_wrap_frame(&fc->flif,frame);

	if(fc->frame_loaded != fc->cd.cur_frame)
	{
		/* re-seek from start of fli */
		frame_oset = fc->flif.hdr.frame1_oset;
		i = -1;
	}
	else if(frame > fc->cd.cur_frame)
	{
		frame_oset = fc->cd.next_frame_oset;
		i = fc->cd.cur_frame;
	}
	else if(frame < fc->cd.cur_frame) 
	{
		if(fc->cd.cur_frame == fc->flif.hdr.frame_count - 1)
		{
			frame_oset = fc->cd.next_frame_oset;
			i = fc->cd.cur_frame;
		}
		else /* re seek from start */
		{
			frame_oset = fc->flif.hdr.frame1_oset;
			i = -1;
		}
	}
	else if(force_read) /* frame == cur_frame */
	{
		/* re-seek from start of fli */
		rc = NULL; /* no need to unfli it */
		frame_oset = fc->flif.hdr.frame1_oset;
		i = -1;
	}
	else
		goto done;

	if(cbuf == NULL)
	{
		if((err = pj_fli_cel_alloc_cbuf(&cbuf,fc->rc)) < Success)
			goto error;
		allocd = TRUE;
	}

	if((frame_oset = pj_seek(fc->flif.fd,frame_oset,JSEEK_START)) < Success)
	{
		err = frame_oset;
		goto error;
	}

	while(i++ != frame)
	{
		if((err = pj_fli_read_uncomp(NULL,&fc->flif,rc,cbuf,TRUE)) < Success)
			goto error;

		if(i >= fc->flif.hdr.frame_count)
		{
			if((frame_oset = pj_seek(fc->flif.fd,
						   fc->flif.hdr.frame2_oset,JSEEK_START)) < Success)
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

	if(frame == 0)
		fc->cd.next_frame_oset = fc->flif.hdr.frame2_oset;
	else
		fc->cd.next_frame_oset = frame_oset;

done:
	fc->frame_loaded = fc->cd.cur_frame = frame;
	err = Success;
error:
	if(allocd)
		pj_freez(&cbuf);
	if(reopened)
		close_fcelio(fc);
	return(err);
}
#ifdef SLUFFED
Errcode gb_abseek_fcel_frame(Flicel *fc, SHORT frame,Fli_frame *cbuf)
{
	if(fc->flags & FCEL_RAMONLY) /* no seeking on ram cel frames */
		return(Success);
	fc->frame_loaded =! fc->cd.cur_frame;
	return(gb_seek_fcel_frame(fc,frame,cbuf,TRUE));
}
#endif /* SLUFFED */

LONG fcel_cbuf_size(Flicel *fc)
{
	return(pj_fli_cbuf_size(fc->rc->width, fc->rc->height,
					 		fc->rc->cmap->num_colors));
}
Boolean fcel_needs_seekbuf(Flicel *fc)
{
	if((fc->flags & FCEL_RAMONLY)
		|| (fc->frame_loaded == fc->cd.cur_frame 
				&& fc->flif.hdr.frame_count <= 1))
	{
		return(FALSE);
	}
	return(TRUE);
}
Errcode seek_fcel_frame(Flicel *fc, SHORT frame)
{
	return(gb_seek_fcel_frame(fc,frame,NULL,FALSE));
}
Errcode inc_fcel_frame(Flicel *fc)

/* increment flicel frame */
{
	return(seek_fcel_frame(fc,fc->cd.cur_frame + 1));
}
Errcode load_temp_fcel(char *tempname, Flicel **pfc)

/* loads a cel stored away with a temp file extant as a newly allocated 
 * flicel */
{
Errcode err;
Fat_chunk fchunk;
Flicel *fc;

	if((err = alloc_fcel(pfc)) < Success)
		return(err);
	fc = *pfc;

	if(NULL == (fc->tpath = clone_string(tempname)))
	{
		err = Err_no_memory;
		goto error;
	}

	if((err = pj_fli_open(tempname, &fc->flif, JREADONLY)) < Success)
		goto error;

	if((err = pj_readoset(fc->flif.fd,&fc->cd, CELDATA_OFFSET,
						 sizeof(Celdata)) < sizeof(Celdata)) < Success)
	{
		goto error;
	}
	if(fc->cd.id.type != FP_CELDATA)
	{
		err = Err_corrupted;
		goto error;
	}
	if ((err = alloc_fcel_raster(fc)) < Success)
		goto error;

	if((err = pj_read_ecode(fc->flif.fd,&fchunk,sizeof(fchunk))) < Success)
		goto error;

	if(fchunk.type != FP_FLIPATH)
	{
		err = Err_corrupted;
		goto error;
	}
	if((thecel->cpath = pj_malloc(fchunk.size)) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}
	thecel->cpath->id = fchunk;
	if((err = pj_read_ecode(fc->flif.fd,
						  OPTR(fc->cpath,sizeof(fchunk)),
						  fchunk.size - sizeof(fchunk))) < Success)
	{
		goto error;
	}

	/* close temp file and open fli pointed to and 
	 * verify it's the right one */

	pj_fli_close(&fc->flif);

	if((err = pj_fli_open(fc->cpath->path, 
							&fc->flif, JREADONLY)) < Success)
	{
		goto error;
	}

	if(memcmp(&thecel->cpath->fid,&thecel->flif.hdr.id,sizeof(Fli_id))
		|| thecel->flif.hdr.width != thecel->rc->width
		|| thecel->flif.hdr.height != thecel->rc->height )
	{
		err = softerr(Err_invalid_id,"!%s", "fcel_load", 
					  thecel->cpath->path);
		goto error;
	}
	refresh_flicel_pos(thecel);
	thecel->frame_loaded = -1;
	if((err = seek_fcel_frame(thecel,thecel->cd.cur_frame)) < Success)
		goto error;

	pj_fli_close(&thecel->flif);
	return(Success);
error:
	softerr(err,"!%s", "fcel_temp", tempname);
	free_fcel(pfc);
	return(err);
}
static Errcode load_pic_fcel(char *pdr_name, Anim_info *ainfo,
					 char *picpath, char *tempname,char *celfli_name,
					 Flicel **pfcel)

/* attempts to load a pic file as a flicel with one frame putting pic
 * image in celfli_name and tempfile in tempname (if both are non NULL) 
 * otherwise it makes a cel with the FCEL_RAMONLY flag set and no files */
{
Errcode err;
Rcel *rc = NULL;

	if ((err = valloc_ramcel(&rc,ainfo->width,ainfo->height)) < Success)
		goto error;

	rc->x = ainfo->x;
	rc->y = ainfo->y;

	if((err = pdr_load_picture(pdr_name,picpath,rc)) < Success)
		goto error;

	if((err = make1_flicel(tempname,celfli_name,pfcel,rc)) < Success)
		goto error;

	if(celfli_name == NULL)
	{
		if((err = alloc_flipath(picpath,NULL,&(*pfcel)->cpath)) < Success)
			goto error;
	}

	return(Success);
error:
	pj_rcel_free(rc);
	return(err);
}

/***** specific stuff to "thecel" *****/

Errcode pdr_load_any_flicel(char *path, char *tempname, char *fliname, 
							Flicel **pfcel)

/* Load a flicel, from any file source. Free cel pointed to by pfcel before
 * actually loading a new one. If *pfcel is not a cel it should be NULL.
 * If file is a variable resolution format it will take the vb.pencel as a
 * reference size, used in load_the_cel() and in the join menu to load 
 * the cel to join. Asks to use non-fli multi frame file's first frame 
 * but does not report all errors, searches for any pdr able to load fli
 * or still image type */
{
Errcode err;
Anim_info ainfo;
char pdr_name[PATH_SIZE];

	if((err = find_pdr_loader(path,TRUE,&ainfo,pdr_name,vb.pencel)) < Success)
		goto error;

	if(is_fli_pdr_name(pdr_name))
	{
		free_fcel(pfcel);
		err = load_fli_fcel(path,tempname,fliname,pfcel);
		goto done;
	}
	else if(ainfo.num_frames > 1)
	{
		if(!soft_yes_no_box("!%s%d", "first_only",
					   path, ainfo.num_frames))
		{
			err = Err_abort;
			goto error;
		}
	}

	free_fcel(pfcel);
	err = load_pic_fcel(pdr_name,&ainfo,path,tempname,
					    fliname,pfcel);

done:
error:
	return(err);
}
Errcode load_the_cel(char *path)
{
Errcode err;

	err = pdr_load_any_flicel(path, cel_name, cel_fli_name, &thecel);
	return(cant_load(err,path));
}
#ifdef SLUFFED
Errcode reload_the_cel()
{
	free_the_cel();
	return(load_temp_fcel(cel_name,&thecel));
}
#endif /* SLUFFED */
Errcode go_load_the_cel(void)
{
Errcode err;
char suffi[PDR_SUFFI_SIZE*2 +10];
char *title;
char sbuf[50];

	get_celload_suffi(suffi);

	if ((title = vset_get_filename(stack_string("load_cel",sbuf),
								   suffi,load_str,
								   CEL_PATH,NULL,0))!=NULL)
	{
		unzoom();
		err = load_the_cel(title);
		rezoom();
	}
	else
		err = Err_abort;
	return(err);
}
void qload_the_cel(void)
{
	if(go_load_the_cel() >= 0)
		show_thecel_a_sec();
}
static Errcode save_the_cel(char *path)
{
Errcode err;
char *celpath;
Flifile oflif;
Chunkparse_data pd;
LONG added_size;
LONG rootsize;
LONG chunksize;

	clear_struct(&oflif);
	pj_fli_close(&thecel->flif);
	if(!thecel->cpath) 
		return(Err_bad_input);

	if(!strcmp(path,cel_name) /* no saving here */
	    || !strcmp(path,cel_fli_name))
	{
		err = Err_in_use;
		goto error;
	}
	celpath = thecel->cpath->path;

	if(!strcmp(path,celpath))
	{
		/* we are re-writing (updating only the cel chunk) 
		 * in a file pointed to */

		if((err = pj_fli_open(celpath,&oflif,JREADWRITE)) < Success)
			goto error;

		if(memcmp(&oflif.hdr.id,&thecel->cpath->fid,sizeof(Fli_id)))
		{
			/* if ids dont match we overwrite file */
			pj_fli_close(&oflif);
			goto overwrite_it;
		}

		added_size = 0;
		init_chunkparse(&pd,oflif.fd,FCID_PREFIX,sizeof(Fli_head),0,0);
		while(get_next_chunk(&pd))
		{
			if(pd.type == ROOT_CHUNK_TYPE)
			{
				rootsize = pd.fchunk.size;
			}
			else if(pd.type == FP_CELDATA)
			{
				if(pd.fchunk.size != sizeof(Celdata))
				{
					pd.fchunk.type = FP_FREE; /* declare it empty */
					if((err = pj_writeoset(pd.fd,&pd.fchunk,pd.chunk_offset,
										 sizeof(Chunk_id))) < Success)
					{
						goto error;
					}
					pd.error = Success; /* Oh well, we put in a new one */
					break;
				}
				if((err = update_parsed_chunk(&pd,&thecel->cd)) < Success)
					goto error;
				pd.error = 1;
				break;
			}
		}

		switch(pd.error)
		{
			case 1: /* re-wrote cel chunk successfully above */
				break;
			case Err_no_chunk:	/* no prefix chunk */
			{
				/* we gotta install a new prefix chunk */
				added_size = sizeof(Celdata) + sizeof(Chunk_id);
				pd.fchunk.size = 0;
				goto insert_celchunk;
			}
			case Success: /* have a prefix chunk without cel chunk */
			{
				/* we gotta install a new cel chunk add space, and write it */

				/* actually this will move the old FCID_PREFIX fchunk toward
				 * eof but it will be re written and the old one will be
				 * overwritten by the new celdata */

				pd.fchunk.size = rootsize;
				added_size = sizeof(Celdata);

			insert_celchunk:

				if((err = pj_insert_space(oflif.fd, sizeof(Fli_head), 
										  			added_size)) < Success)
				{
					goto error;
				}

				/* write or rewrite PREFIX fchunk */
				pd.fchunk.size += added_size;
				pd.fchunk.type = FCID_PREFIX;
				if((err = pj_write_ecode(oflif.fd,&pd.fchunk,
									     sizeof(Chunk_id))) < Success)
				{
					goto error;
				}
				/* write cel chunk */
				if((err = pj_write_ecode(oflif.fd,
								      &thecel->cd,sizeof(Celdata))) < Success)
				{
					goto error;
				}
				break;
			}
			default: /* err < Success */
				err = pd.error;
				goto error;
		}

		/* adjust offsets for any added prefix size */

		oflif.hdr.frame1_oset += added_size;
		oflif.hdr.frame2_oset += added_size;
		oflif.hdr.size += added_size;

		if((err = pj_i_flush_head(&oflif)) < Success)
			goto error;
		pj_fli_close(&oflif);
		thecel->cpath->fid = oflif.hdr.id; /* update pointer to it */
		thecel->cd.next_frame_oset += added_size; /* adjust pointer to next 
												   * frame for changes */
		goto done;
	}

overwrite_it:

	/* not the same file */

	if((err = pj_fli_open(celpath,&thecel->flif,JREADONLY)) < Success)
		goto error;

	if((err = pj_fli_create(path,&oflif)) < Success)
		goto error;

	/* fudge to save out cel aspect the same as current screen window */

	thecel->flif.hdr.aspect_dx = vb.pencel->aspect_dx;
	thecel->flif.hdr.aspect_dy = vb.pencel->aspect_dy;
	oflif.hdr = thecel->flif.hdr;

	if((err = jwrite_chunk(oflif.fd,&thecel->cd,
						   sizeof(Celdata),FCID_PREFIX)) < Success)
	{
		goto error;
	}

	if((oflif.hdr.frame1_oset = pj_tell(oflif.fd)) < Success)
	{
		err = oflif.hdr.frame1_oset;
		goto error;
	}

	oflif.hdr.frame2_oset = oflif.hdr.frame1_oset +
		thecel->flif.hdr.frame2_oset - thecel->flif.hdr.frame1_oset;

	chunksize = thecel->flif.hdr.size - thecel->flif.hdr.frame1_oset;

	/* copy all frames to output fli file */

	if((err = pj_copydata_oset(thecel->flif.fd, oflif.fd,
				   			 thecel->flif.hdr.frame1_oset, 
				   			 oflif.hdr.frame1_oset, chunksize )) < Success)
	{
		goto error;
	}
	oflif.hdr.size = oflif.hdr.frame1_oset + chunksize;
	if((err = pj_i_flush_head(&oflif)) < Success)
	{
		goto error;
	}
	goto done;

error:
	return(softerr(err,"!%s", "cant_save", path));
done:
	pj_fli_close(&oflif);
	pj_fli_close(&thecel->flif);
	return(err);
}
void qsave_the_cel(void)
{
char *title;
char sbuf[50];

	if (thecel == NULL)
		return;
	hide_mp();
	if((title = vset_get_filename(stack_string("save_cel",sbuf), 
								  ".CEL",save_str,
								  CEL_PATH,NULL,1))==NULL)
	{
		goto out;
	}

	if(!overwrite_old(title))
		goto out;

	unzoom();
	save_the_cel(title);
	rezoom();

out:
	show_mp();
}
