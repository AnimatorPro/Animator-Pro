#include "jimk.h"
#include "aaconfig.h"
#include "brush.h"
#include "commonst.h"
#include "errcodes.h"
#include "flx.h"
#include "memory.h"
#include "rastext.h"
#include "softmenu.h"
#include "unchunk.h"
#include "util.h"
#include "vsetfile.h"

static Errcode default_tsettings(Vset_flidef *fdef);

static Errcode
load_file_settings(char *path, Vset_flidef *fdef, Boolean default_reset);

void rethink_settings(void)
{
	reres_settings();
	attatch_tools();
	attatch_inks();
	fset_spacing(uvfont, vs.font_spacing, vs.font_leading);

	if(!mask_is_present())
		vs.make_mask = vs.use_mask = FALSE;

	set_brush_type(vs.pen_brush_type);
	set_penwndo_position();
}
Errcode load_default_settings(Vset_flidef *fdef)
/*** called by open_default_flx() ***************/
{
Errcode err;
char path[PATH_SIZE];

	close_zwinmenu();
	make_file_path(vb.init_drawer,default_name,path);
	err = load_file_settings(path,fdef,TRUE);
	if(err < Success)
	{
		if(err != Err_no_file)
			softerr(err,"!%s","set_load", path );
		vs = default_vs;
		pj_get_default_cmap(vb.screen->wndo.cmap);
		if((err = default_tsettings(fdef)) < Success)
			return(err);
	}
	rethink_settings();
	see_cmap();
	return(Success);
}
/***** io routines for vsettings chunk ******/


static void load_vschunk(Vsettings *pvs)
{
	*pvs = vs;
}
/**** io routines for tsettings file with a linear array for path storage ****/

void close_vsetfile(Vsetfile *vsf)
{
	pj_closez(&(vsf->fd));
}
Errcode open_vsetfile(char *path,int jmode, Vsetfile *vsf)
{
Errcode err;

	if((vsf->fd = pj_open(path,jmode)) == JNONE)
		return(pj_ioerr());
	if((err = pj_read_ecode(vsf->fd,&vsf->id,
							POSTOSET(Vsetfile,paths_id))) < Success)
	{
		goto error;
	}
	if(vsf->id.type != VSETFILE_MAGIC)
	{
		err = Err_bad_magic;
		goto error;
	}
	if(vsf->id.version != VSETCHUNK_VERSION)
	{
		err = Err_version;
		goto error;
	}
	return(Success);
error:
	close_vsetfile(vsf);
	return(err);
}

/************************ tsettings file stuff ************************/

#define FAST_FLUSHFIELDS \
	Vset_flidef fdef;\
	Vsettings vs


typedef struct tsetflush {	/* fields we flush for flush call */
	FAST_FLUSHFIELDS;
} Tfastflush;

#define FAST_FLUSHOFFSET OFFSET(Tsettings_file,fdef)

#define SLOW_FLUSHFIELDS \
	Slow_vsettings vslow

typedef struct tslowflush {  /* additional fields for slow flush call */
	SLOW_FLUSHFIELDS;
} Tslowflush;

#define SLOW_FLUSHOFFSET OFFSET(Tsettings_file,vslow)

typedef struct tsettings_file {
	Fat_chunk id;
	Vset_paths vsp;  /* must be the first chunk */
	FAST_FLUSHFIELDS;
	SLOW_FLUSHFIELDS;
} Tsettings_file;

#undef FAST_FLUSHFIELDS
#undef SLOW_FLUSHFIELDS

static void load_default_paths(Vset_paths *vp_chunk)
/* initialize and set internal default paths in a patharray chunk */
{
Vset_path *vsp;

	vp_chunk->id.size = sizeof(Vset_paths);
	vp_chunk->id.type = VSET_PATHARRAY_ID;
	vp_chunk->id.version = VSET_PATHARRAY_VERS;

	vsp = &vp_chunk->path_recs[FONT_PATH];
	make_resource_path("FONT","SYSTEM.FNT",vsp->path);
	strcpy(vsp->wildcard, "*.*");
}
static void load_flidef(Vset_flidef *fdef,Fli_head *fh)

/* loads a flidef chunk from the tempflx header */
{
	fdef->id.size = sizeof(Vset_flidef);
	fdef->id.type = VSET_FLIDEF_ID;
	fdef->id.version = VSET_FLIDEF_VERS;
	if(fh != NULL)
	{
		fdef->rect.width = fh->width;
		fdef->rect.height = fh->height;
		fdef->speed = fh->speed;
		fdef->frame_count = fh->frame_count;
	}
	else /* we default to size of screen */
	{
		fdef->rect.width = vb.screen->wndo.width;
		fdef->rect.height = vb.screen->wndo.height;
		fdef->speed = FLX_DEFAULT_SPEED;
		fdef->frame_count = 1;
	}
}
static void load_vslow(Slow_vsettings *svs, Boolean defaults)
{
	svs->id.size = sizeof(*svs);
	svs->id.type = VSET_SLOWVS_ID;
	svs->id.version = VSET_SLOWVS_VERS;
	memcpy(svs->mc_ideals,vconfg.mc_ideals,sizeof(svs->mc_ideals));
	if(defaults)
		get_default_ink_strengths(svs->inkstrengths);
	else
		save_ink_strengths(svs->inkstrengths);
}

static void load_init_tsettings(Tsettings_file *buf)
{
	buf->id.size = sizeof(*buf);
	buf->id.type = VSETFILE_MAGIC;
	buf->id.version = VSETCHUNK_VERSION;

	load_default_paths(&buf->vsp);
	load_vslow(&buf->vslow, TRUE);
	load_flidef(&buf->fdef,NULL);
	load_vschunk(&buf->vs);
}

static Errcode default_tsettings(Vset_flidef *fdef)
/*** used to create a default empty settings file ***/
{
Errcode err;
Tsettings_file *buf;

	if((buf = pj_zalloc(sizeof(*buf))) == NULL)
		return(Err_no_memory);
	load_init_tsettings(buf);
	*fdef = buf->fdef;
	err = write_gulp(tsettings_name,buf,sizeof(*buf));
	pj_freez(&buf);
	return(err);
}

static Errcode reopen_tsettings(Vsetfile *vsf)
{
Errcode err;

	if((err = open_vsetfile(tsettings_name,JREADWRITE,vsf)) < Success)
		return(err);
	if(vsf->paths_id.type != VSET_PATHARRAY_ID)
	{
		err = Err_version;
		goto error;
	}
	return(Success);
error:
	close_vsetfile(vsf);
	return(err);
}

Errcode reload_tsettings(Vsettings *pvs,Vset_flidef *fdef)
/* called whenever a tempflx is opened to reload ram settings state */
{
Errcode err;
Tsettings_file *buf;

	if((buf = pj_malloc(sizeof(*buf))) == NULL)
		return(Err_no_memory);
	if((err = read_gulp(tsettings_name, buf, (long)sizeof(*buf))) < Success)
		goto error;

	if(pvs)
		*pvs = buf->vs;
	if(fdef)
		*fdef = buf->fdef;

	load_ink_strengths(buf->vslow.inkstrengths);

error:
	pj_freez(&buf);
	return(err);
}

static Errcode tset_flush(Vsetfile *vsf, Boolean full_flush)
/* full flush will flush all the slow stuff too, inkstrengths and menu
 * colors */
{
Errcode err;
union {
Tfastflush tf;
Tslowflush tsf;
} buf;

	load_flidef(&buf.tf.fdef,(Fli_head *)&flix.hdr);
	load_vschunk(&buf.tf.vs);
	err = pj_writeoset(vsf->fd,&buf.tf,FAST_FLUSHOFFSET,sizeof(buf.tf));

	if(full_flush && err >= Success)
	{
		load_vslow(&buf.tsf.vslow, FALSE);
		err = pj_writeoset(vsf->fd,&buf.tsf,SLOW_FLUSHOFFSET,sizeof(buf.tsf));
	}
	return(err);
}

Errcode flush_tsettings(Boolean full_flush)
/* full flush will flush all the "slow" stuff too */
{
Errcode err;
Vsetfile vsf;

	if((err = reopen_tsettings(&vsf)) >= Success)
	{
		err = tset_flush(&vsf,full_flush);
		close_vsetfile(&vsf);
	}
	return(err);
}

static long path_type_offset(Vsetfile *vsf, int ptype)
/* given path type (index) this returns offset of path record in file */
{
	(void)vsf;
	return(sizeof(Fat_chunk)+sizeof(Fat_chunk)
					+(ptype*sizeof(Vset_path)));
}

Errcode vset_get_pathinfo(int ptype, Vset_path *pathinfo)
{
Errcode err = Success;
Vsetfile vsf;

	if(((unsigned int)ptype) < VSET_NUM_PATHS
		&& (err = reopen_tsettings(&vsf)) >= Success)
	{
		err = pj_readoset(vsf.fd,pathinfo,path_type_offset(&vsf,ptype),
						  sizeof(Vset_path));
		close_vsetfile(&vsf);
	}
	else
		clear_struct(pathinfo);

	/* certain types we will initialize from others if they are not set */

	if(pathinfo->scroller_top == 0
		&& pathinfo->path[0] == 0
		&& pathinfo->wildcard[0] == 0)
	{
		switch(ptype)
		{
			case JOIN_PATH:
			case JOIN_MASK_PATH:
				ptype = FLI_PATH;
				break;
			default:
				goto done;
		}
		err = vset_get_pathinfo(ptype, pathinfo);
		*pj_get_path_name(pathinfo->path) = 0;
		pathinfo->wildcard[0] = 0;
	}
done:
	return(err);
}

Errcode vset_get_path(int ptype, char *path)
{
Errcode err;
Vset_path vsp;

	err = vset_get_pathinfo(ptype,&vsp);
	strcpy(path,vsp.path);
	return(err);
}

Errcode vset_set_pathinfo(int ptype, Vset_path *pathinfo)
/* this may be slow but it does the job */
{
Errcode err;
Vsetfile vsf;
char *name;
char save_name[PATH_SIZE];

	if(((unsigned int)ptype) >= VSET_NUM_PATHS) /* unrecognized type */
		return(Success);

	/* remove "unnamed" names */

	name = pj_get_path_name(pathinfo->path);
	strcpy(save_name,name);
	*pj_get_path_suffix(name) = 0;
	if(txtcmp(unnamed_str,name) == 0)
		*name = 0;
	else
		strcpy(name,save_name);

	if((err = reopen_tsettings(&vsf)) >= Success)
	{

		err = pj_writeoset(vsf.fd,pathinfo,path_type_offset(&vsf,ptype),
						  sizeof(Vset_path));
		close_vsetfile(&vsf);
	}
	strcpy(name,save_name);
	return(err);
}

Errcode vset_set_path(int ptype, char *path)
{
Vset_path vsp;
	vsp.scroller_top = 0;
	vsp.wildcard[0] = 0;
	strcpy(vsp.path,path);
	return(vset_set_pathinfo(ptype,&vsp));
}

static Errcode write_settings_chunk(Jfile newfd, SHORT id_type, LONG offset,
									Cmap *cmap, Boolean for_fli_prefix )
/* writes out a settings chunk. leaves file at start of chunk */
{
Errcode err;
Vsetfile vsf;
Chunkparse_data pd;

	if((err = reopen_tsettings(&vsf)) < Success)
		return(err);

	if((err = tset_flush(&vsf,TRUE)) < Success)
		goto error;

	init_chunkparse(&pd,vsf.fd,VSETFILE_MAGIC,0,sizeof(Fat_chunk),0);
	while(get_next_chunk(&pd))
	{
		switch(pd.type)
		{
			case VSET_FLIDEF_ID:
				if(for_fli_prefix)
					break;
			case VSET_PATHARRAY_ID:
			case ROOT_CHUNK_TYPE:
			case VSET_VS_ID:
			case VSET_SLOWVS_ID:
				copy_parsed_chunk(&pd,newfd); /* sets pd.error internally */
				break;
		}
	}
	if((err = pd.error) < Success)
		goto error;

	if((!for_fli_prefix) && cmap)
		err = pj_write_palchunk(newfd,cmap,VSET_CMAP_ID);

	/* flush header chunk with final file size */

	pd.fchunk.type = id_type;
	pd.fchunk.size = pj_tell(newfd) - offset;
	if((err = pj_writeoset(newfd,&pd.fchunk.size,
						   offset,sizeof(Chunk_id))) < Success)
	{
		goto error;
	}
	err = pd.fchunk.size;
error:
	close_vsetfile(&vsf);
	return(err);
}

static Errcode save_settings_file(char *path, Boolean full_defaults)
{
Errcode err;
Jfile newfd;

	if((newfd = pj_create(path,JREADWRITE)) == JNONE)
		return(pj_ioerr());
	err = write_settings_chunk(newfd, VSETFILE_MAGIC, 0,
							   full_defaults?vb.pencel->cmap:NULL, FALSE);
	pj_close(newfd);
	if(err < Success)
		pj_delete(path);
	return(err);
}

void save_default_settings(void)
/* Write out copy of settings to default settings file */
{
char path[PATH_SIZE];
	make_file_path(vb.init_drawer,default_name,path);
	softerr(save_settings_file(path,TRUE),"!%s", "cant_save", path );
}

Errcode write_fli_settings(Jfile *fd, SHORT chunk_id )
/* called by fli saver to load prefix settings in new fli file */
{
Errcode err;
LONG offset;

	if((offset = pj_tell(fd)) < 0)
		return(offset);
	if((err = write_settings_chunk(fd, chunk_id, offset, NULL, TRUE)) >= 0)
		return(pj_seek(fd,offset+err,JSEEK_START));
	return(err);
}

Errcode load_default_flidef(Vset_flidef *fdef)
/* used by flisize menu to load the flidef fields in the buttons */
{
Chunkparse_data pd;
Jfile fd;
char path[PATH_SIZE];

	make_file_path(vb.init_drawer,default_name,path);
	if((fd = pj_open(path,JREADONLY)) == JNONE)
		return(pj_ioerr());
	init_chunkparse(&pd,fd,VSETFILE_MAGIC,0,sizeof(Fat_chunk),0);
	while(get_next_chunk(&pd))
	{
		if(pd.type == VSET_FLIDEF_ID)
		{
			if(pd.fchunk.version == VSET_FLIDEF_VERS)
			{
				pd.error = read_parsed_chunk(&pd,fdef,sizeof(*fdef));
				fdef->id.size = sizeof(*fdef); /* keep host size the same */
			}
			else
				pd.error = Err_version;
			goto done;
		}
	}
	if(pd.error >= Success)
		pd.error = Err_no_chunk;
done:
	pj_close(fd);
	return(pd.error);
}

static void chop_default_paths(Vset_paths *vsp)
/* truncates names off of settings paths loaded as defaults */
{
int i;
Vset_path *vp;

	vp = &vsp->path_recs[0];
	for(i = 0;i < VSET_NUM_PATHS;++i,++vp)
	{
		if(i == FONT_PATH)
			continue;
		else
			*pj_get_path_name(vp->path) = 0; /* truncate name */
	}
}

static Errcode load_settings_chunk(Jfile fd, Fat_chunk *id, LONG offset,
								   Vset_flidef *fdef,
								   Cmap *cmap, Boolean load_mucolors,
								   Boolean as_defaults)

/* this will load the global vsettings if there is no error and will re-load
 * the tsettings file with the input settings. It will load the color map
 * even in some error cases, as it will the flidef */
{
Errcode err;
Chunkparse_data pd;
Tsettings_file *tset;
Fat_chunk *buf;
LONG recsize;
Boolean load_inkstrengths;

	if(id->version != VSETCHUNK_VERSION)
		return(Err_version);
	if((tset = pj_malloc(sizeof(*tset))) == NULL)
		return(Err_no_memory);

	load_inkstrengths = !as_defaults;

	load_init_tsettings(tset); /* start out with defaults */

	init_chunkparse(&pd,fd,DONT_READ_ROOT,offset,sizeof(Fat_chunk),id->size);

	while(get_next_chunk(&pd))
	{
		switch(pd.type)
		{
			case VSET_FLIDEF_ID:
				if(!fdef)
					break;
				fdef->id = tset->fdef.id;
				buf = &(fdef->id);
				fdef = NULL;
				goto read_chunk;
			case VSET_VS_ID:
				buf = &(tset->vs.id);
				goto read_chunk;
			case VSET_SLOWVS_ID:
				if(!(load_inkstrengths || load_mucolors))
					break;
				buf = &(tset->vslow.id);
				goto read_chunk;
			case VSET_PATHARRAY_ID:
				buf = &(tset->vsp.id);
			read_chunk:
				/* if the version is ok it will overlay or truncate to fit */
				if(pd.fchunk.version != buf->version)
					goto version_error;
				recsize = buf->size;
				if((err = read_parsed_chunk(&pd,buf,recsize)) < Success)
					goto error;
				buf->size = recsize; /* OUR record should still
									  * still have the same size NOT inputs */
				break;
			case VSET_CMAP_ID:
				if(cmap)
				{
					if((err = pj_read_palchunk(pd.fd,
									&pd.fchunk, cmap)) < Success)
					{
						goto error;
					}
					cmap = NULL;
				}
				break;
		}
	}
	if((err = pd.error) < Success)
		goto error;

	if(as_defaults)
		chop_default_paths(&(tset->vsp));

	if((err = write_gulp(tsettings_name,tset,sizeof(*tset))) < Success)
		goto error;

	if(load_mucolors)
	{
		memcpy(vconfg.mc_ideals,tset->vslow.mc_ideals,
								sizeof(vconfg.mc_ideals));
	}

	if(load_inkstrengths)
		load_ink_strengths(tset->vslow.inkstrengths);

	if(fdef)
		*fdef = tset->fdef;

	if(cmap)
	{
		if(soft_yes_no_box("setting_cmap"))
			pj_get_default_cmap(cmap);
	}

	vs = tset->vs;
	goto done;

version_error:
	err = Err_version;
error:
done:
	pj_freez(&tset);
	return(err);
}

static Errcode load_fli_settings(char *path,Cmap *cmap)
{
Errcode err;
Flifile flif;
Chunkparse_data pd;
(void)cmap;

	if((err = pj_fli_open(path,&flif,JREADONLY)) < Success)
		return(err);
	init_chunkparse(&pd,flif.fd,FCID_PREFIX,sizeof(Fli_head),0,0);
	for(;;)
	{
		if(get_next_chunk(&pd))
		{
			if(pd.type != FP_VSETTINGS)
				continue;
			err = load_settings_chunk(flif.fd, &pd.fchunk,
									pd.chunk_offset, NULL, NULL, TRUE, FALSE);
			break;
		}
		if((err = pd.error) >= Success)
			err = Err_no_chunk;
		break;
	}
	pj_fli_close(&flif);
	return(err);
}

static Errcode load_file_settings(char *path,Vset_flidef *fdef,
								  Boolean default_reset)
/* note this will not corrupt data in *vset unless read and version verify
 * is successful does not re-load settings */
{
Errcode err;
Jfile fd;
Fat_chunk id;

	if((fd = pj_open(path,JREADONLY)) == JNONE)
		return(pj_ioerr());
	if((err = pj_read_ecode(fd, &id, sizeof(id))) < Success)
		goto error;

	if(id.type == VSETFILE_MAGIC)
	{
		err = load_settings_chunk(fd, &id, 0, fdef,
								  default_reset?vb.pencel->cmap:NULL,
								  TRUE, default_reset );
	}
	else if(default_reset)
	{
		err = Err_bad_magic;
	}
	else switch(id.type)
	{
		case FLIHR_MAGIC:
			pj_closez(&fd);
			err = load_fli_settings(path, NULL);
			break;
		case FLIH_MAGIC:
			err = Err_version;
			goto error;
		default:
			err = Err_bad_magic;
			goto error;
	}
	if(err >= Success)
		rewrite_config();  /* save new loaded menu colors in config file */
error:
	pj_closez(&fd);
	return(err);
}

static Errcode load_vsettings(char *path)
{
Errcode err;
SHORT oframe_ix;
SHORT obframe_ix;
UBYTE ozoom_open;

	hide_mp();
	unzoom();

	/* keep some things the same */

	oframe_ix = vs.frame_ix;	/* keep frame ix */
	obframe_ix = vs.bframe_ix; /* and back frame cashe */
	ozoom_open = vs.zoom_open;

	if((err = load_file_settings(path, NULL, FALSE)) < Success)
		goto error;

	vs.frame_ix = oframe_ix;
	vs.bframe_ix = obframe_ix;
	vs.zoom_open = ozoom_open;

	rewrite_config();  /* save new loaded menu colors in config file */
	rethink_settings();
	menu_to_quickcent(&quick_menu);
error:

	rezoom();
	show_mp();
	return(cant_load(err,path));
}

void qsave_vsettings(void)
{
char *title;
char sbuf[50];

	if ((title = vset_get_filename(stack_string("save_set",sbuf),
						".SET", save_str, SETTINGS_PATH, NULL, 1)) != NULL)
	{
		if(overwrite_old(title))
			softerr(save_settings_file(title,FALSE),"!%s","cant_save",title);
	}
}

void qload_vsettings(void)
{
char *title;
char sbuf[50];

	if ((title = vset_get_filename(stack_string("load_set",sbuf),
					".SET;.FLC", load_str, SETTINGS_PATH, NULL, 1)) !=NULL)
	{
		load_vsettings(title);
	}
}
