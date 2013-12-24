#include "torture.h"
#include "ptrmacro.h"
#include "fli.h"
#include <stdio.h>

#define FRAME1_TIME_ITCOUNT 50
#define FRAMES_TIME_ITCOUNT 200

static Boolean uncomp_rect(Raster	 *r,
						 Fli_frame *frame,
						 Rectangle *rect,
						 Boolean   is_hardware,   /* update color registers? */
						 Boolean   log_chunks)
/*****************************************************************************
 *
 ****************************************************************************/
{
#define CHUNK ((Chunk_id *)vp)

	Rastlib *rlib = r->lib;
	int 	j;
	void	*vp;
	Boolean has_pixels = FALSE;

	vp = frame+1;
	for (j=0;j<frame->chunks;j++)
	{
		switch(CHUNK->type)
		{
			case FLI_COLOR:
				if (log_chunks)
					log_progress("C64, ");
				if(is_hardware) {
					pj_wait_rast_vsync(r);
					rlib->uncc64(r, CHUNK+1);
				}
				break;
			case FLI_COLOR256:
				if (log_chunks)
					log_progress("C256, ");
				if(is_hardware) {
					pj_wait_rast_vsync(r);
					rlib->uncc256(r, CHUNK+1);
				}
				break;
			case FLI_LC:
				has_pixels = TRUE;
				if (log_chunks)
					log_progress("LC, ");
				rlib->unlccomp_rect(r,CHUNK+1,1,
								 rect->x,rect->y,rect->width,rect->height);
				break;
			case FLI_SS2:
				has_pixels = TRUE;
				if (log_chunks)
					log_progress("SS2, ");
				rlib->unss2_rect(r,CHUNK+1,1,
							  rect->x,rect->y,rect->width,rect->height);
				break;
			case FLI_COLOR_0:
				has_pixels = TRUE;
				if (log_chunks)
					log_progress("BLACK, ");
				rlib->set_rect(r,0,rect->x,rect->y,rect->width,rect->height);
				break;
			case FLI_BRUN:
				has_pixels = TRUE;
				if (log_chunks)
					log_progress("BRUN, ");
				rlib->unbrun_rect(r,CHUNK+1,1,
							   rect->x,rect->y,rect->width,rect->height);
				break;
			case FLI_COPY:
				has_pixels = TRUE;
				if (log_chunks)
					log_progress("COPY, ");
				rlib->put_rectpix(r,CHUNK+1,
							   rect->x,rect->y,rect->width,rect->height);
				break;
		}
		vp = FOPTR(vp,CHUNK->size);
	}
	return has_pixels;

#undef CHUNK
}

static Errcode load_flic_file(FlicList *theflic)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode 	err = Success;
	Fli_head	flichead;
	void		*flicbuf;
	FILE		*f = NULL;

	theflic->in_memory = NULL;	// it's not in memory yet

	if (NULL == (f = fopen(theflic->name, "rb"))) {
		log_warning("File open failed for '%s'\n");
		err = Err_nogood;
		goto ERROR_EXIT;
	}

	if (1 != fread(&flichead, sizeof(flichead), 1, f)) {
		log_warning("File read failed for '%s'\n");
		err = Err_nogood;
		goto ERROR_EXIT;
	}

	if (flichead.type != FLIH_MAGIC && flichead.type != FLIHR_MAGIC) {
		log_warning("File '%s' is not an animator flic!\n");
		err = Err_bad_magic;
		goto ERROR_EXIT;
	}

	if (NULL == (flicbuf = malloc(flichead.size))) {
		log_warning("File '%s' is too big for the available memory.\n");
		err = Err_no_memory;
		goto ERROR_EXIT;
	}

	fseek(f, 0, SEEK_SET);
	if (1 != fread(flicbuf, flichead.size, 1, f)) {
		log_warning("File load failed for '%s'\n");
		err = Err_truncated;
		goto ERROR_EXIT;
	}

	if (flichead.type == FLIHR_MAGIC) {
		theflic->frame1_pointer = FOPTR(flicbuf, flichead.frame1_oset);
		theflic->frame2_pointer = FOPTR(flicbuf, flichead.frame2_oset);
	} else {
		theflic->frame1_pointer = FOPTR(flicbuf, sizeof(flichead));
		theflic->frame2_pointer = FOPTR(theflic->frame1_pointer,
								  ((Fli_frame *)(theflic->frame1_pointer))->size);
	}

	theflic->in_memory	= flicbuf;
	theflic->num_frames = flichead.frame_count;
	theflic->type		= flichead.type;

ERROR_EXIT:

	if (f != NULL)
		fclose(f);

	return err;
}

static void unload_flic_file(FlicList *theflic)
/*****************************************************************************
 *
 ****************************************************************************/
{
	if (theflic->in_memory != NULL)
		free(theflic->in_memory);
	theflic->in_memory = NULL;
}

static Errcode test_one_flic(Raster *r, FlicList *theflic)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode 	err;
	Rectangle	playrect;
	void		*frameptr;
	int 		curframe;
	int 		curtest;
	int 		numtests;
	Raster		*vr = &tcb.verification_raster;
	static struct {
		int x,y;
		}		testoffsets[] ={
				{0,0},				// standard 0,0
				{1,1},				// tricky 1,1
				{0,0},				// screenw-flicw-1,screenh-flich-1 goes here
				{0,0},				// screenw-flicw-1,screenh-flich-1 goes here
				{0,0},				// user-requested coords go here
				};

	/*------------------------------------------------------------------------
	 * first, try to load the flic, and if that works, try to get a big
	 * pixel buffer that verify_raster() will use to verify the flic frames.
	 * if we can't get the big buffer, that's okay, because verify_raster()
	 * will then fall back to its line-at-a-time logic.
	 *----------------------------------------------------------------------*/

	if (Success > (err = load_flic_file(theflic)))
		goto ERROR_EXIT;

	tcb.playback_verify_buffer = malloc(r->width*r->height);

	/*------------------------------------------------------------------------
	 * if the screen is at all bigger than the flic, we can do tests at a
	 * variety of offsets.	if the flic only exactly fits, we can only do
	 * the first test, at x,y=0,0.	(if the flic is bigger than the screen,
	 * it won't be on the list of flics to test.)
	 *----------------------------------------------------------------------*/

	if (theflic->width < r->width && theflic->height < r->height) {
		testoffsets[2].x = r->width  - theflic->width;
		testoffsets[2].y = r->height - theflic->height;
		testoffsets[3].x = r->width  - theflic->width - 1;
		testoffsets[3].y = r->height - theflic->height - 1;
		if (theflic->xoffset == 0 && theflic->yoffset == 0) {
			numtests = Array_els(testoffsets) - 1;
		} else {
			numtests = Array_els(testoffsets);
			testoffsets[4].x = theflic->xoffset;
			testoffsets[4].y = theflic->yoffset;
		}
	} else {
		numtests = 1;
	}

	playrect.width	= theflic->width;
	playrect.height = theflic->height;

	for (curtest = 0; curtest < numtests; ++curtest) {

		playrect.x = testoffsets[curtest].x;
		playrect.y = testoffsets[curtest].y;

		log_progress("Testing flic %s at x=%d, y=%d...\n",
			theflic->name, playrect.x, playrect.y);

		pj_set_rast(r, 0);	// clear screen
		pj_set_rast(vr, 0); // clear verification raster

		frameptr = theflic->frame1_pointer;
		for (curframe = 0; curframe < theflic->num_frames; ++curframe) {
			log_progress("  Frame %3d: ", curframe);
			if (uncomp_rect(r, frameptr, &playrect,  TRUE,	TRUE)) {
				uncomp_rect(vr, frameptr, &playrect, FALSE, FALSE);
				if (!verify_raster(r, vr, FALSE))
					goto ERROR_EXIT;
			}
			frameptr = FOPTR(frameptr, ((Chunk_id *)frameptr)->size);
			log_progress("\n");
			if (!single_step())
				goto ERROR_EXIT;
		}

		if (err < Success)
			log_warning("...failed!\n\n");
		else
			log_progress("...completed successfully.\n\n");
	}

ERROR_EXIT:

	if (tcb.playback_verify_buffer != NULL) {
		free(tcb.playback_verify_buffer);
		tcb.playback_verify_buffer = NULL;
	}

	unload_flic_file(theflic);

	return	err;
}

static Errcode time_one_flic(Raster *r, FlicList *theflic)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode 	err;
	Rectangle	playrect;
	void		*frameptr;
	int 		curframe;
	int 		curtest;
	int 		numtests;
	int 		i;
	static struct {
		int x,y;
		}		testoffsets[] ={
				{0,0},				// standard 0,0
				{1,1},				// tricky 1,1
				};

	/*------------------------------------------------------------------------
	 * first try to load the flic.
	 *----------------------------------------------------------------------*/

	if (Success > (err = load_flic_file(theflic)))
		goto ERROR_EXIT;

	/*------------------------------------------------------------------------
	 * if the screen is at all bigger than the flic, we can do 1,1 test too.
	 *----------------------------------------------------------------------*/

	if (theflic->width < r->width && theflic->height < r->height) {
		numtests = Array_els(testoffsets);
	} else {
		numtests = 1;
	}

	playrect.width	= theflic->width;
	playrect.height = theflic->height;

	for (curtest = 0; curtest < numtests; ++curtest) {

		playrect.x = testoffsets[curtest].x;
		playrect.y = testoffsets[curtest].y;

		log_progress("Timing flic %s (%dx%d) at x=%d, y=%d...\n",
			theflic->name, theflic->width, theflic->height,
			playrect.x, playrect.y);

		pj_set_rast(r, 0);	// clear screen

		frameptr = theflic->frame1_pointer;

		log_start("  Timing first frame (BRUN)...\n  ");
			for (i = 0; i < FRAME1_TIME_ITCOUNT; ++i) {
				uncomp_rect(r, frameptr, &playrect, !i, FALSE);
			}
		log_end("  ...first frame timing complete.\n");

		frameptr = FOPTR(frameptr, ((Chunk_id *)frameptr)->size);
		curframe = 1;

		log_start("  Timing delta frames (%s)...\n  ",
					(theflic->type == FLIHR_MAGIC) ? "SS2":"LC");
			for (i = 0; i < FRAMES_TIME_ITCOUNT; ++i) {
				uncomp_rect(r, frameptr, &playrect, FALSE, FALSE);
				if (++curframe > theflic->num_frames) {
					curframe = 1;
					frameptr = theflic->frame2_pointer;
				} else {
					frameptr = FOPTR(frameptr, ((Chunk_id *)frameptr)->size);
				}
			}
		log_end("  ...delta frame timing complete.\n\n");
	}

ERROR_EXIT:

	unload_flic_file(theflic);

	return	err;
}

void test_playback(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
	FlicList	*cur;

	if (Success > get_flic_names())
		return;

	for (cur = tcb.fliclist; cur != NULL; cur = cur->next) {
		if (tcb.timing_only_run)
			time_one_flic(r, cur);
		else
			test_one_flic(r, cur);
	}
}
