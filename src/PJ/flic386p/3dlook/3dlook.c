/*****************************************************************************
 * 3dLook.c - program to play flics in parallel and go from one to another
 * at the press of a key.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "pjbasics.h"
#include "pjfli.h"
#include "fliundoc.h"
#include "expandat.h"
#include "3dlook.h"

	/* This macro adds a byte offset to any type of pointer. */
#define ADD_BYTE_OFFSET(base,offset) ((void*)((char *)(base)+(offset))) 

#ifdef DEBUG
/*********DEBUGGING STUFF********/
FILE *debug_log;

open_debug_log()
{
	if (debug_log == NULL)
		debug_log = fopen("H:\\debug.log", "w");
}

dlog(char *fmt, ...)
/* Printf to debug file. */
{
va_list argptr;

open_debug_log();
va_start(argptr, fmt);
vfprintf(debug_log,fmt,argptr);
va_end(argptr);
fflush(debug_log);
}
#endif /* DEBUG */


/*----------------------------------------------------------------------------
 * usage text
 *--------------------------------------------------------------------------*/

static char usage[] =
"\n"
"3dlook - A program to demonstrate Carroll Lastinger's 3-D Looking Glasses.\n"
"Currently this is an early prototype.  This program shifts from one flic to\n"
"another when you hit the ',' and '.' keys.\n"
"\n"
"You may run the program in two ways.  Either:\n"
"   3dlook file1.flc file2.flc ... fileN.flc\n"
"or\n"
"   3dlook @file-containing-list-of-flics\n"
"<Esc> will stop the program.\n";

/*----------------------------------------------------------------------------
 * code...
 *--------------------------------------------------------------------------*/

/*
 * Stuff to get us into and out of graphics mode. 
 */
static Boolean in_graphics = FALSE;	/* Keep track if we're in graphics mode. */
FlicRaster *the_raster;				/* Primary display raster. */

static Errcode set_graphics_mode(int width, int height)
/*****************************************************************************
 * Go into graphics mode that is closest to width x height.
 ****************************************************************************/
{
	Errcode err;

	if ((err = pj_video_find_open(width, height, &the_raster)) < Success) 
		{
		return report_error(err
		, "Couldn't find display device for %d x %d screen"
		, width, height);
		}
	in_graphics = TRUE;
	return err;
}

static void set_text_mode()
/*****************************************************************************
 * Go back to text mode.
 ****************************************************************************/
{
	if (in_graphics)
		{
		pj_video_close(&the_raster);
		in_graphics = FALSE;
		}
}

static Boolean in_clock = FALSE;	/* Keep track whether have set up clock. */

static void init_clock()
/*****************************************************************************
 * Set up clock for millisecond timing.
 ****************************************************************************/
{
	if (!in_clock)
		{
		pj_clock_init();
		in_clock = TRUE;
		}
}

static void cleanup_clock()
/*****************************************************************************
 * Return clock back to 18.2 clicks a second.
 ****************************************************************************/
{
	if (in_clock)
		{
		pj_clock_cleanup();
		in_clock = FALSE;
		}
}

void cleanup_all()
/*****************************************************************************
 * Undo all the changes we've made to the hardware.
 ****************************************************************************/
{
	cleanup_clock();
	set_text_mode();
}

Errcode report_error(Errcode err, char *format, ...)
/*****************************************************************************
 * Semi-intelligent error reporting routine.  Prints error message
 * associated with error code, and a (possibly NULL) additional printf
 * formated message.  If the error code indicates there's no real
 * problem (either it's positive or just a user abort) this routine
 * does nothing.  Otherwise it goes to text mode, reports the error,
 * and exits the program.
 ****************************************************************************/
{
	va_list args;

	if (err < Success && err != Err_reported && err != Err_abort) {
		cleanup_all();
		va_start(args,format);
		if (format != NULL) {
			vfprintf(stderr, format, args);
			fprintf(stderr,".\n");
		}
		if (err != Err_no_message)
			fprintf(stderr, "%s.\n", pj_error_get_message(err), err);
		va_end(args);
		exit(err);
	}
	return err;
}

void init_ram_flic(Ram_flic *rf)
/*****************************************************************************
 *	Set ram_flic to harmless non-garbage values.
 ****************************************************************************/
{
	rf->head = NULL;
	rf->head_as_char = NULL;
	rf->name = NULL;
	rf->cur_frame_oset = rf->frame1_oset = rf->frame2_oset = 0;
}

void cleanup_ram_flic(Ram_flic *rf)
/*****************************************************************************
 *  Free up any resources associated with ram_flic.
 ****************************************************************************/
{
	gentle_free(rf->head_as_char);
	gentle_free(rf->name);
	init_ram_flic(rf);
}


Errcode open_ram_flic(Ram_flic *rf, char *name)
/*****************************************************************************
 *  Load a Flic file into memory and set up ram_flic to keep track of it.
 ****************************************************************************/
{
	Errcode err;
	int size;
	Fli_head *flihdr;
	Fli_frame *flif;
	Fhead_1_0 *ohdr;

	init_ram_flic(rf);
	if ((rf->name = strdup(name)) == NULL)
		{
		err = report_error(Err_no_memory, "%s", name);
		goto OUT;
		}
	if ((err = read_alloc_buf(name, &rf->head_as_char, &size)) < Success)
		goto OUT;
	rf->head = flihdr = (Fli_head *)(rf->head_as_char);
	if (flihdr->type != FLIH_MAGIC && flihdr->type != FLIHR_MAGIC)
		{
		err = report_error(Err_no_message,  "%s isn't a .FLI or .FLC file\n"
		,	name);
		goto OUT;
		}
	if (flihdr->type == FLIH_MAGIC)
		{
		/* Find out offset to first and second frames of old style flic. */
		ohdr = (Fhead_1_0 *)flihdr;
		rf->frame1_oset = sizeof(Fhead_1_0);
		flif = (Fli_frame *)(flihdr+1);
		rf->frame2_oset = rf->frame1_oset + flif->size;
		flihdr->speed = (ohdr->jiffy_speed * 1000L + 35)/ 70;
		}
	else
		{
		rf->frame1_oset = flihdr->frame1_oset;
		rf->frame2_oset = flihdr->frame2_oset;
		}
	rf->cur_frame_ix = 0;
	rf->cur_frame_oset = rf->frame1_oset;
OUT:
	if (err < Success)
		cleanup_ram_flic(rf);
	return err;
}

Fli_frame *ram_flic_get_cur_frame(Ram_flic *rf)
/*****************************************************************************
 * Get pointer to the current frame of a ram_flic.
 ****************************************************************************/
{
	return ADD_BYTE_OFFSET(rf->head_as_char, rf->cur_frame_oset);
}

void ram_flic_advance_cur_frame(Ram_flic *rf)
/*****************************************************************************
 * Move on to the next frame of a ram_flic.  If going past the end,
 * loop back to the first frame.
 ****************************************************************************/
{
	Fli_head *flihdr;
	Fli_frame *flif;
	int frame_count;
	int cur_ix;

	if ((flihdr = rf->head) == NULL)
		return;
	frame_count = flihdr->frame_count;
	cur_ix = rf->cur_frame_ix + 1;
	if (cur_ix == frame_count)		/* We want the ring frame to take us back */
		{
		cur_ix = 0;
		}
	if (frame_count == 1 				/* Special case 1 frame flic. */
	|| cur_ix == 1)	
		{
		rf->cur_frame_oset = rf->frame2_oset;
		}
	else
		{
		flif = ram_flic_get_cur_frame(rf);
		rf->cur_frame_oset += flif->size;
		}
	rf->cur_frame_ix = cur_ix;
}


void init_list_of_ram_flics(List_of_ram_flics *rfl)
/*****************************************************************************
 * Set up list_of_ram_flics to harmless non-garbage values.
 ****************************************************************************/
{
	rfl->flics = NULL;
	rfl->count = 0;
}
void cleanup_list_of_ram_flics(List_of_ram_flics *rfl)
/*****************************************************************************
 * Free any resources associated with a list_of_ram_flics.
 ****************************************************************************/
{
	Ram_flic *rf;
	int count;

	if ((rf = rfl->flics) != NULL)
		{
		count = rfl->count;
		while (--count >= 0)
			{
			cleanup_ram_flic(rf);
			++rf;
			}
		free(rfl->flics);
		}
	init_list_of_ram_flics(rfl);
}
Errcode set_list_of_ram_flics_size(List_of_ram_flics *rfl, int count)
/*****************************************************************************
 * Set up list_of_ram_flics to handle a specific number of flics.
 ****************************************************************************/
{
	if ((rfl->flics = calloc(sizeof(rfl->flics[0]), count)) == NULL)
		{
		return report_error(Err_no_memory, NULL);
		}
	rfl->count = count;
	return Success;
}
Errcode add_to_list_of_ram_flics(List_of_ram_flics *rfl
, char *name, unsigned int ix)
/*****************************************************************************
 * Read a flic into memory and attatch it to 'ix' element in the 
 * list_of_ram_flics.
 ****************************************************************************/
{
	if (ix >= rfl->count)
		return report_error(Err_no_message
		, "Error adding %s.  Index %d of %d??"
		,	name, ix, rfl->count);
	return open_ram_flic(&rfl->flics[ix], name);
}
Errcode open_list_of_ram_flics(List_of_ram_flics *rfl, char *names[], int count
, Boolean report_progress)
/*****************************************************************************
 * Read a bunch of flics into a list_of_ram_flics.  If report_progress, print
 * message to screen for each one.
 ****************************************************************************/
{
	Errcode err;
	int i;

	if ((err = set_list_of_ram_flics_size(rfl, count)) < Success)
		return err;
	for (i=0; i<count; ++i)
		{
		if (report_progress)
			printf("Loading %s\n", names[i]);
		if ((err = add_to_list_of_ram_flics(rfl, names[i], i)) < Success)
			return err;
		}
	return err;
}


typedef struct looker
/*----------------------------------------------------------------------------
 * This is the main control structure for the 3dLook program. 
 *--------------------------------------------------------------------------*/
	{
	int flic_count;						/* The number of flics. */
	List_of_ram_flics list;				/* RAM copies of all flics. */
	int width, height;					/* Dimensions of flics. */
	int frames;							/* # of frames in each flic. */
	long speed;							/* Speed to play back. */
	FlicRaster **ram_rasters;			/* RAM screens, one for each flic. */
	FlicRaster *screen_raster;			/* Full screen raster. */
	FlicRaster *play_raster;			/* Clipped to fit width/height rast. */
	FlicRaster virt_raster;				/* Data for display_raster. */
	int visible_flic_ix;				/* Flick on play_raster. */
	int frame_ix;						/* Current frame of all flics. */
	} Looker;

void init_looker(Looker *looker)
/*****************************************************************************
 * Set up a Looker to harmless non-garbage values.
 ****************************************************************************/
{
	init_list_of_ram_flics(&looker->list);
	looker->width = looker->height = looker->frames = 0;
	looker->ram_rasters = NULL;
}

void cleanup_looker(Looker *looker)
/*****************************************************************************
 * Free all resources associated with Looker.
 ****************************************************************************/
{
	FlicRaster **ram_rasters;
	int i;

	cleanup_list_of_ram_flics(&looker->list);
	if ((ram_rasters = looker->ram_rasters) != NULL)
		{
		for (i=0; i<looker->flic_count; ++i)
			pj_raster_free_ram(&ram_rasters[i]);
		free(ram_rasters);
		}
	init_looker(looker);
}

Errcode looker_check_same_size(Looker *looker)
/*****************************************************************************
 * Make sure all flics in Looker's list are the same resolution and
 * same # of frames.
 ****************************************************************************/
{
	int width, height, frames;
	int w,h,f;
	Ram_flic *flics;
	int count;
	int i;

	count = looker->list.count;
	flics = looker->list.flics;
	width = flics[0].head->width;
	height = flics[0].head->height;
	frames = flics[0].head->frame_count;
	for (i=1; i<count; ++i)
		{
		w = flics[i].head->width;
		h = flics[i].head->height;
		f = flics[i].head->frame_count;
		if (w != width || h != height)
			{
			set_text_mode();
			puts("Flics aren't same resolution:");
			for (i=0; i<count; ++i)
				printf("%s is (%d x %d).\n", flics[i].name
				, flics[i].head->width, flics[i].head->height);
			exit(Err_reported);
			}
		if (frames != f)
			{
			set_text_mode();
			puts("Flics don't have the same number of frames:");
			for (i=0; i<count; ++i)
				printf("%s is %d frames.\n", flics[i].name
				, flics[i].head->frame_count);
			exit(Err_reported);
			}
		}
	looker->width = width;
	looker->height = height;
	looker->frames = frames;
	return Success;
}

Errcode open_looker_flics(Looker *looker, char *flic_names[], int flic_count)
/*****************************************************************************
 * Read a list of flics into looker,  verify they're all the same size,
 * and set up to play them all and display the middle one.
 ****************************************************************************/
{
	Errcode err;

	if ((err 
	= open_list_of_ram_flics(&looker->list, flic_names, flic_count, TRUE))
	< Success)
		return err;
	looker->flic_count = flic_count;
	looker->visible_flic_ix = flic_count/2;
	looker->frame_ix = 0;
	looker->speed = looker->list.flics[0].head->speed;
	if ((err = looker_check_same_size(looker)) < Success)
		return err;
	return Success;
}

Errcode open_looker_rasters(Looker *looker, FlicRaster *screen)
/*****************************************************************************
 * Allocate a RAM raster for each flic in looker.   Set up a virtual
 * raster exactly the size of the flics on the screen.
 ****************************************************************************/
{
	FlicRaster **ram_rasters;
	int count = looker->flic_count;
	int width = looker->width;
	int height = looker->height;
	int i;
	Errcode err;

	looker->screen_raster = screen;
	looker->play_raster = pj_raster_center_virtual(screen, &looker->virt_raster
	, width, height);
	if ((looker->ram_rasters = ram_rasters 
	= calloc(sizeof(ram_rasters[0]), count)) == NULL)
		return report_error(Err_no_memory, NULL);
	for (i=0; i<count; ++i)
		{
		if ((err = pj_raster_make_ram(&ram_rasters[i], width, height)) 
		< Success)
			{
			return report_error(err, 
			"Couldn't make %d of %d RAM rasters at (%dx%d) pixels\n"
			"(%ld bytes) each.\n"
			,  i+1, count, width, height, width*height);
			}
		}
	return Success;
}

void looker_display_and_advance(Looker *looker)
/*****************************************************************************
 * Display current frame of all flics on memory rasters, and 
 * also visibly with the active flic.  Then advance all flics.
 ****************************************************************************/
{
	int i;
	Ram_flic *rf;
	Fli_frame *flif;

	/* Update all the RAM rasters. */
	for (i=0; i<looker->flic_count; ++i)
		{
		rf = &looker->list.flics[i];
		flif = ram_flic_get_cur_frame(rf);
		pj_fli_uncomp_frame(looker->ram_rasters[i], flif, FALSE);
		}
	/* Update the play raster */
	rf = &looker->list.flics[looker->visible_flic_ix];
	flif = ram_flic_get_cur_frame(rf);
	pj_fli_uncomp_frame(looker->play_raster, flif, TRUE);
	/* Advance all the flics */
	for (i=0; i<looker->flic_count; ++i)
		{
		rf = &looker->list.flics[i];
		ram_flic_advance_cur_frame(rf);
		}
}

void looker_new_visible(Looker *looker, int new_ix)
/*****************************************************************************
 * Display a new visible flic.
 ****************************************************************************/
{
	looker->visible_flic_ix = new_ix;
	pj_raster_copy(looker->ram_rasters[new_ix], looker->play_raster);
}

void looker_move_left(Looker *looker)
/*****************************************************************************
 * Switch one flic to the left.
 ****************************************************************************/
{
	if (looker->visible_flic_ix > 0)
		looker_new_visible(looker, looker->visible_flic_ix-1);
}

void looker_move_right(Looker *looker)
/*****************************************************************************
 * Switch one flic to the right.
 ****************************************************************************/
{
	if (looker->visible_flic_ix < looker->flic_count-1)
		looker_new_visible(looker, looker->visible_flic_ix+1);
}

void looker_play(Looker *looker, int speed)
/*****************************************************************************
 * Play all flics in parallel in RAM, and active one on display screen.
 * If '.' or ',' hit then switch active flic.  If <esc> is hit exit.
 ****************************************************************************/
{
	int key;
	long time;

	for (;;)
		{
		time = speed + pj_clock_1000();		/* Find time at start */
		looker_display_and_advance(looker);	/* Display frame. */
		do 									/* Poll input until time is out. */
			{
			if (pj_key_is())
				{
				key = pj_key_in();
				switch (key&0xFF)
					{
					case '.':
						looker_move_right(looker);
						break;
					case ',':
						looker_move_left(looker);
						break;
					case 0x1b:
						return;
					}
				} 
			}
		while (time >= pj_clock_1000());
		}
}

void print_mem_avail()
/*****************************************************************************
 * Check the amount of memory available and print it.
 ****************************************************************************/
{
struct list 
	{
	struct list *next;
	} *list = NULL, *next;
int i;

/* Allocate 1 meg chunks until run out of memory. */
for (i=0; ; ++i)
	{
	if ((next = malloc(1024*1024)) == NULL)
		break;
	next->next = list;
	list = next;
	}
/* Report memory available. */
printf("%d meg of memory available.\n", i);
/* Free chunks. */
next = list;
while ((list = next) != NULL)
	{
	next = list->next;
	free(list);
	}
}

int main(int argc, char **argv)
/*****************************************************************************
 * Check the command line,  load up the flics,  set up graphics, set up clock,
 * and play.  Then clean up.
 ****************************************************************************/
{
	Errcode err = Success;
	Looker looker;

	init_looker(&looker);
 	if (expand_ats(&argc, &argv) < Success)
		goto OUT;
	if (argc == 1) 
		{
		puts(usage);
		print_mem_avail();
		goto OUT;
		}
	if ((err = open_looker_flics(&looker, argv+1, argc-1)) < Success)
		goto OUT;
/*	pj_video_add_all(); */

	pj_video_add(pj_vdev_mcga);
	pj_video_add(pj_vdev_supervga);
	pj_video_add(pj_vdev_vesa);

	if ((err = set_graphics_mode(looker.width,looker.height)) < Success)
		goto OUT;
	init_clock();
	if ((err = open_looker_rasters(&looker, the_raster)) < Success)
		goto OUT;
	looker_play(&looker, looker.speed);
OUT:
	cleanup_looker(&looker);
	cleanup_all();
	return err;
}
