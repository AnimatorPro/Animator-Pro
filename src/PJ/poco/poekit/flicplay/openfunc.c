/*****************************************************************************
 * FLICOPEN.C - Open a flic file for input.
 ****************************************************************************/

#include "flicplay.h"

static Flic *fliclist = NULL;

static void free_playback_raster(Flic *pflic)
/*****************************************************************************
 *
 ****************************************************************************/
{
	if (NULL == pflic || NULL == pflic->playback_raster)
		return;

	if (pflic->playback_raster != pflic->root_raster) {
		free(pflic->playback_raster);
		pflic->playback_raster = NULL;
	}
}

static Errcode build_playback_raster(Flic *pflic, Rcel *root, int x, int y)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Flifile 	*flifile;
	Rectangle	therect;

	if (NULL == pflic)
		return Err_null_ref;

	if (NULL == (flifile = pflic->flifile))
		return Err_file_not_open;

	free_playback_raster(pflic);

	if (NULL == root) {
		if (NULL == pflic->root_raster)
			pflic->root_raster = GetPicScreen();
		root = pflic->root_raster;
	} else {
		pflic->root_raster = root;
	}

	if (root->width == flifile->hdr.width
	 && root->height == flifile->hdr.height
	 && x == 0
	 && y == 0) {
		pflic->playback_raster = root;
	} else {
		if (x == 0 && y == 0) {
			therect.x = (root->width  - flifile->hdr.width)  / 2;
			therect.y = (root->height - flifile->hdr.height) / 2;
		} else {
			therect.x = x;
			therect.y = y;
		}
		therect.width  = flifile->hdr.width;
		therect.height = flifile->hdr.height;
		if (NULL == (pflic->playback_raster = malloc(sizeof(Rcel)))) {
			return Err_no_memory;
		}
		pj_rcel_make_virtual(pflic->playback_raster, root, &therect);
	}

	return Success;
}

void do_flic_close(Flic *pflic)
/*****************************************************************************
 * close flic file, free all associated resources, remove from resource list.
 ****************************************************************************/
{
	Flic *cur, **prev;

	if (NULL == pflic) {
		return;
	}

	if (NULL != pflic->flifile) {
		pj_fli_close(pflic->flifile);
		free(pflic->flifile);
	}

	if (NULL != pflic->framebuf) {
		free(pflic->framebuf);
	}

	free_playback_raster(pflic);

	for (prev = &fliclist, cur = fliclist; cur != NULL; cur = cur->next) {
		if (cur == pflic) {
			*prev = cur->next;
			break;
		}
		prev = &cur->next;
	}

	pflic->magic = 0xDEADDEAD;	/* prevent re-use */

	free(pflic);
}

void do_flic_close_all(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Flic	*cur, *next;

	for (cur = fliclist; cur != NULL; cur = next) {
		next = cur->next;
		do_flic_close(cur);
	}
}

Errcode do_flic_open(char *path, Flic **ppflic)
/*****************************************************************************
 * alloc flic control structures, open flic file, return status.
 ****************************************************************************/
{
	Errcode err;
	Flic	*pflic;
	Flifile *flifile;

	if ('\0' == *path) {
		return Err_parameter_range;
	}

	err = Err_no_memory;

	if (NULL == (pflic = zalloc(sizeof(Flic)))) {
		goto ERROR_EXIT;
	}

	if (NULL == (flifile = pflic->flifile = zalloc(sizeof(Flifile)))) {
		goto ERROR_EXIT;
	}

	if (Success > (err = pj_fli_open(path, pflic->flifile, JREADONLY))) {
		goto ERROR_EXIT;
	}

	if (Success > (err = pj_fli_alloc_cbuf(&pflic->framebuf,
							flifile->hdr.width, flifile->hdr.height, COLORS))) {
		goto ERROR_EXIT;
	}

	if (Success > (err = build_playback_raster(pflic, NULL, 0, 0)))
		goto ERROR_EXIT;

	pflic->magic	  = IANS_FLIC_MAGIC;
	pflic->cur_frame  = BEFORE_FIRST_FRAME;
	pflic->num_frames = flifile->hdr.frame_count;
	pflic->speed	  = flifile->hdr.speed;

	pflic->next = fliclist;
	fliclist = pflic;

	*ppflic = pflic;
	return Success;

ERROR_EXIT:

	do_flic_close(pflic);
	*ppflic = NULL;
	return err;

}

Errcode do_flic_options(Flic	  *pflic,
						int 	  speed,
						int 	  keyhit_stops_playback,
						Rcel	  *new_raster,
						int 	  x,
						int 	  y)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;

	if (NULL == pflic)
		return Err_null_ref;

	if (speed >= 0)
		pflic->speed = speed;

	if (keyhit_stops_playback >= 0)
		pflic->keyhit_stops_playback = keyhit_stops_playback;

	if (NULL != new_raster || x != 0 || y != 0) {
		if (Success > (err = build_playback_raster(pflic, new_raster, x, y))) {
			return err;
		}
		do_rewind(pflic); /* force rewind if the raster moved/changed */
	}

}
