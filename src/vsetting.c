#include <string.h>
#include "jimk.h"
#include "aaconfig.h"
#include "brush.h"
#include "commonst.h"
#include "errcodes.h"
#include "flx.h"
#include "inks.h"
#include "mask.h"
#include "memory.h"
#include "palchunk.h"
#include "pentools.h"
#include "rastext.h"
#include "resource.h"
#include "softmenu.h"
#include "unchunk.h"
#include "util.h"
#include "zoom.h"

static Errcode default_tsettings(Vset_flidef *fdef);

static Errcode
load_file_settings(char *path, Vset_flidef *fdef, bool default_reset);

void rethink_settings(void)
{
	reres_settings();
	attatch_tools();
	attatch_inks();
	fset_spacing(uvfont, vs.font_spacing, vs.font_leading);

	if(!mask_is_present())
		vs.make_mask = vs.use_mask = false;

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
	err = load_file_settings(path,fdef, true);
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

static void close_vsetfile(Vsetfile *vsf)
{
	xffclose(&vsf->xf);
}

static Errcode
open_vsetfile(char *path, enum XReadWriteMode mode, Vsetfile *vsf)
{
	Errcode err;

	err = xffopen(path, &vsf->xf, mode);
	if (err < Success)
		return err;

	err = xffread(vsf->xf, &vsf->id, POSTOSET(Vsetfile,paths_id));
	if (err < Success)
		goto error;

	if (vsf->id.type != VSETFILE_MAGIC) {
		err = Err_bad_magic;
		goto error;
	}

	if (vsf->id.version != VSETCHUNK_VERSION) {
		err = Err_version;
		goto error;
	}

	return Success;

error:
	close_vsetfile(vsf);
	return err;
}

/************************ tsettings file stuff ************************/

#define FAST_FLUSHOFFSET OFFSET(Tsettings_file,fdef)
#define SLOW_FLUSHOFFSET OFFSET(Tsettings_file,vslow)

typedef struct tsettings_file {
	Fat_chunk id;
	Vset_paths vsp;  /* must be the first chunk */

	/* fields we flush for flush call */
	Vset_flidef fdef;
	Vsettings vs;

	/* additional fields for slow flush call */
	Slow_vsettings vslow;
} Tsettings_file;

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
static void load_vslow(Slow_vsettings *svs, bool defaults)
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
	load_vslow(&buf->vslow, true);
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

	err = open_vsetfile(tsettings_name, XREADWRITE_OPEN, vsf);
	if (err < Success)
		return err;

	if (vsf->paths_id.type != VSET_PATHARRAY_ID) {
		err = Err_version;
		goto error;
	}

	return Success;

error:
	close_vsetfile(vsf);
	return err;
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

	if (buf->vs.zoomscale == 0) {
		err = Err_corrupted;
		goto error;
	}

	if(pvs)
		*pvs = buf->vs;
	if(fdef)
		*fdef = buf->fdef;

	load_ink_strengths(buf->vslow.inkstrengths);

error:
	pj_freez(&buf);
	return(err);
}

static Errcode tset_flush(Vsetfile *vsf, bool full_flush)
/* full flush will flush all the slow stuff too, inkstrengths and menu
 * colors */
{
	Errcode err;
	Tsettings_file buf;

	load_flidef(&buf.fdef, (Fli_head *)&flix.hdr);
	load_vschunk(&buf.vs);

	err = xffwriteoset(vsf->xf, &buf.fdef, FAST_FLUSHOFFSET,
			SLOW_FLUSHOFFSET - FAST_FLUSHOFFSET);

	if (full_flush && err >= Success) {
		load_vslow(&buf.vslow, false);

		err = xffwriteoset(vsf->xf, &buf.vslow, SLOW_FLUSHOFFSET,
				sizeof(Tsettings_file) - SLOW_FLUSHOFFSET);
	}

	return err;
}

Errcode flush_tsettings(bool full_flush)
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
		err = xffreadoset(vsf.xf, pathinfo, path_type_offset(&vsf,ptype),
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

	err = reopen_tsettings(&vsf);
	if (err >= Success) {
		err = xffwriteoset(vsf.xf, pathinfo, path_type_offset(&vsf,ptype),
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

/* Function: write_settings_chunk
 *
 *  Writes out a settings chunk.  Leaves file at start of chunk.
 */
static Errcode
write_settings_chunk(XFILE *newxf, SHORT id_type, LONG offset,
		Cmap *cmap,
									bool for_fli_prefix)
{
Errcode err;
Vsetfile vsf;
Chunkparse_data pd;

	if((err = reopen_tsettings(&vsf)) < Success)
		return(err);

	if((err = tset_flush(&vsf, true)) < Success)
		goto error;

	init_chunkparse(&pd, vsf.xf, VSETFILE_MAGIC, 0, sizeof(Fat_chunk), 0);
	while(get_next_chunk(&pd))
	{
		switch(pd.type)
		{
			case VSET_FLIDEF_ID:
				if(for_fli_prefix)
					break;
			case VSET_PATHARRAY_ID:
			case (USHORT)ROOT_CHUNK_TYPE:
			case VSET_VS_ID:
			case VSET_SLOWVS_ID:
				copy_parsed_chunk(&pd, newxf); /* sets pd.error internally */
				break;
		}
	}
	if((err = pd.error) < Success)
		goto error;

	if((!for_fli_prefix) && cmap)
		err = pj_write_palchunk(newxf, cmap, VSET_CMAP_ID);

	/* flush header chunk with final file size */

	pd.fchunk.type = id_type;
	pd.fchunk.size = xfftell(newxf) - offset;

	err = xffwriteoset(newxf, &pd.fchunk.size, offset, sizeof(Chunk_id));
	if (err < Success)
		goto error;

	err = pd.fchunk.size;
error:
	close_vsetfile(&vsf);
	return(err);
}

static Errcode save_settings_file(char *path, bool full_defaults)
{
	Errcode err;
	XFILE *newxf;

	err = xffopen(path, &newxf, XREADWRITE_CLOBBER);
	if (err < Success)
		return err;

	err = write_settings_chunk(newxf, VSETFILE_MAGIC, 0,
			full_defaults ? vb.pencel->cmap : NULL, false);
	xffclose(&newxf);
	if (err < Success)
		pj_delete(path);

	return err;
}

void save_default_settings(void)
/* Write out copy of settings to default settings file */
{
char path[PATH_SIZE];
	make_file_path(vb.init_drawer,default_name,path);
	softerr(save_settings_file(path, true),"!%s", "cant_save", path );
}

Errcode write_fli_settings(XFILE *xf, SHORT chunk_id)
/* called by fli saver to load prefix settings in new fli file */
{
	Errcode err;
	long offset;

	offset = xfftell(xf);
	if (offset < 0)
		return (Errcode)offset;

	err = write_settings_chunk(xf, chunk_id, offset, NULL, true);
	if (err < Success)
		return err;

	return xffseek(xf, offset+err, XSEEK_SET);
}

Errcode load_default_flidef(Vset_flidef *fdef)
/* used by flisize menu to load the flidef fields in the buttons */
{
	Errcode err;
	Chunkparse_data pd;
	XFILE *xf;
	char path[PATH_SIZE];

	make_file_path(vb.init_drawer,default_name,path);

	err = xffopen(path, &xf, XREADONLY);
	if (err < Success)
		return err;

	init_chunkparse(&pd, xf, VSETFILE_MAGIC, 0, sizeof(Fat_chunk), 0);
	while (get_next_chunk(&pd)) {
		if (pd.type == VSET_FLIDEF_ID) {
			if (pd.fchunk.version == VSET_FLIDEF_VERS) {
				pd.error = read_parsed_chunk(&pd,fdef,sizeof(*fdef));
				fdef->id.size = sizeof(*fdef); /* keep host size the same */
			}
			else
				pd.error = Err_version;
			goto done;
		}
	}

	if (pd.error >= Success)
		pd.error = Err_no_chunk;

done:
	xffclose(&xf);
	return pd.error;
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

/* Function: load_settings_chunk
 *
 *  This will load the global vsettings if there is no error and will
 *  re-load the tsettings file with the input settings. It will load
 *  the colour map even in some error cases, as it will the flidef.
 */
static Errcode
load_settings_chunk(XFILE *xf, Fat_chunk *id, LONG offset,
		Vset_flidef *fdef, Cmap *cmap,
								   bool load_mucolors,
								   bool as_defaults)
{
Errcode err;
Chunkparse_data pd;
Tsettings_file *tset;
Fat_chunk *buf;
LONG recsize;
bool load_inkstrengths;

	if(id->version != VSETCHUNK_VERSION)
		return(Err_version);
	if((tset = pj_malloc(sizeof(*tset))) == NULL)
		return(Err_no_memory);

	load_inkstrengths = !as_defaults;

	load_init_tsettings(tset); /* start out with defaults */

	init_chunkparse(&pd, xf, DONT_READ_ROOT, offset, sizeof(Fat_chunk),
			id->size);

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
				if (cmap) {
					err = pj_read_palchunk(pd.xf, &pd.fchunk, cmap);
					if (err < Success)
						goto error;

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

	err = pj_fli_open(path, &flif, XREADONLY);
	if (err < Success)
		return err;

	init_chunkparse(&pd, flif.xf, FCID_PREFIX, sizeof(Fli_head), 0, 0);
	for (;;) {
		if (get_next_chunk(&pd)) {
			if (pd.type != FP_VSETTINGS)
				continue;

			err = load_settings_chunk(flif.xf, &pd.fchunk,
					pd.chunk_offset, NULL, NULL, true, false);
			break;
		}

		err = pd.error;
		if (err >= Success)
			err = Err_no_chunk;
		break;
	}

	pj_fli_close(&flif);
	return err;
}

static Errcode load_file_settings(char *path,Vset_flidef *fdef, bool default_reset)
/* note this will not corrupt data in *vset unless read and version verify
 * is successful does not re-load settings */
{
	Errcode err;
	XFILE *xf;
	Fat_chunk id;

	err = xffopen(path, &xf, XREADONLY);
	if (err < Success)
		return err;

	err = xffread(xf, &id, sizeof(id));
	if (err < Success)
		goto error;

	if (id.type == VSETFILE_MAGIC) {
		err = load_settings_chunk(xf, &id, 0, fdef,
				default_reset ? vb.pencel->cmap : NULL, true, default_reset);
	}
	else if (default_reset) {
		err = Err_bad_magic;
	}
	else {
		switch (id.type) {
			case FLIHR_MAGIC:
				xffclose(&xf);
				err = load_fli_settings(path, NULL);
				break;
			case FLIH_MAGIC:
				err = Err_version;
				goto error;
			default:
				err = Err_bad_magic;
				goto error;
		}
	}

	/* Save new loaded menu colours in config file. */
	if (err >= Success)
		rewrite_config();

error:
	xffclose(&xf);
	return err;
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

	if((err = load_file_settings(path, NULL, false)) < Success)
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
			softerr(save_settings_file(title, false),"!%s","cant_save",title);
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
