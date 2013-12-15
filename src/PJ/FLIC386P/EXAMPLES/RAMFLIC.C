/*****************************************************************************
 * ramflic.c - a program that loads a flic into RAM and plays it.
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include "pjbasics.h"
#include "pjfli.h"

/*----------------------------------------------------------------------------
 * usage text
 *--------------------------------------------------------------------------*/

static char usage[] =
"\n"
"Ramflic - a program that load a flic and then plays it back out of RAM\n"
"until a key is hit.\n"
"Usage:\n"
"    ramflic  somefile.flc [speed]\n"
"Examples:\n"
"    ramflic clrglobe.fli\n"
"This will play the animation clrglobe.fli at normal speed.\n"
"    ramflic mrnumo.flc 10\n"
"This will play mrnumo.flc with 10 milliseconds between each frame.\n"
"    ramflic quarkorb.flc 0\n"
"This will play quarkorb as fast as it can.\n"
;

Errcode report_error(Errcode err, char *format, ...);

void gentle_free(void *pt)
/*************************************************************************
 * Free non-NULL pointer. 
 *************************************************************************/
{
if (pt != NULL)
	free(pt);
}

Errcode read_alloc_buf(char *filename, char **pbuf, int *psize)
/*************************************************************************
 * Find out size of file, allocate a buffer that big + 1.  Read in
 * file into buffer.  Put a zero tag at end of buffer.  Return buffer
 * and size of buffer.
 *************************************************************************/
{
Errcode err = Success;
long size = 0;
char *buf = NULL;
int handle = -1;

if ((handle = open(filename, O_RDONLY|O_BINARY)) == -1)
	{
	err = report_error(Err_no_message,
		"Couldn't open %s. %s. Sorry", filename, strerror(errno));
	goto OUT;
	}
size = lseek(handle, 0L, SEEK_END);
lseek(handle, 0L, SEEK_SET);
if ((buf = malloc((size_t)size+1)) == NULL)
	{
	err = report_error(Err_no_memory,
		"Couldn't allocate %ld bytes for %s buffer", size, filename);
	goto OUT;
	}
buf[size] = 0;	/* add zero tag at end */
if (read(handle, buf, size) != size)
	{
	err = report_error(Err_no_message,
		"Couldn't read %s. %s.", filename, strerror(errno));
	goto OUT;
	}
OUT:
	if (err < Success)
		{
		gentle_free(buf);
		buf = NULL;
		size = 0;
		}
	if (handle != -1)
		close(handle);
	*pbuf = buf;
	*psize = size;
	return(err);
}

/*
 * Stuff to get us into and out of graphics mode. 
 */
static Boolean in_graphics = FALSE;	/* Keep track if we're in graphics mode. */
FlicRaster *the_raster;				/* Primary display raster. */

static Errcode set_graphics_mode(int width, int height)
/*****************************************************************************
 * Go into graphics mode that is closest to width x height.
 * If can't find a bigger screen, squawk and die.
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
	if (width != the_raster->width || height != the_raster->height)
		{
		return report_error(Err_no_message
		, "Program couldn't open a %dx%d screen\n"
		  "The closest it could come is %dx%d"
		, width, height, the_raster->width, the_raster->height);
		}
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


typedef struct ram_flic
/*****************************************************************************
 * Data to keep track of one flic in RAM.
 ****************************************************************************/
	{
	Fli_head *head;			/* Actually points to the whole flic. */
	char *name;				/* Name of flic file. */
	char *head_as_char;		/* Points same place as head. */
	long frame1_oset;		/* Offset to first frame. */
	long frame2_oset;		/* Offset to second frame. */
	long cur_frame_oset;	/* Byte offset to current frame. */
	int cur_frame_ix;		/* Index of current frame. */
	} Ram_flic;

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
	return (Fli_frame *)(rf->head_as_char + rf->cur_frame_oset);
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
	if (cur_ix == frame_count)	  /* We want the ring frame to take us back */
		{
		cur_ix = 0;
		}
	if (frame_count == 1 		  /* Special case 1 frame flic. */
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

void play_ram_flic(Ram_flic *rf, long speed, FlicRaster *screen)
/*****************************************************************************
 * Play a Ram_flic.   If speed is negative use header speed.  Otherwise try
 * to play it at speed.
 ****************************************************************************/
{
	long time;
	Fli_frame *flif;

	if (speed < 0)
		speed = rf->head->speed;
	for (;;)
		{
		time = speed + pj_clock_1000();			/* Find time at start */
		flif = ram_flic_get_cur_frame(rf);		/* Find current frame delta. */
		pj_fli_uncomp_frame(screen, flif, TRUE);/* Uncompress onto screen. */
		ram_flic_advance_cur_frame(rf);			/* Go to next frame of flic. */
		do 										/* Poll input until time up. */
			{
			if (pj_key_is())			/* Is key ready? */
				{
				pj_key_in();			/* Eat it. */
				return;					/* Go home. */
				} 
			}
		while (time >= pj_clock_1000());
		}
}

int main(int argc, char **argv)
/*****************************************************************************
 * Check the command line,  load up the flic,  set up graphics, set up clock,
 * and play.  Then clean up.
 ****************************************************************************/
{
	Errcode err = Success;
	Ram_flic flic;
	long speed;

	/* Set flic to known empty state. */
	init_ram_flic(&flic);

	/* Check command line and print usage summary if it's not correct. */
	if (argc < 2 || argc > 3) 
		{
		puts(usage);
		goto OUT;
		}

	/* Figure out speed of flic. */
	if (argc == 3)
		speed = atol(argv[2]);
	else
		speed = -1;				/* Default, use speed in file header. */

	/* Load the flic into memory. */
	if ((err = open_ram_flic(&flic, argv[1])) < Success)
		goto OUT;

	/* Load all video drivers. */
	pj_video_add_all(); 

	/* Select video driver and mode which best fits flic resolution. */
	if ((err = set_graphics_mode(flic.head->width,flic.head->height)) 
	< Success)
		goto OUT;

	/* Convert clock from 18.2 beats per second to 1000. */
	init_clock();

	/* Play flic until user hits a key. */
	play_ram_flic(&flic, speed, the_raster);

OUT:
	/* Cleanup everything. */
	cleanup_ram_flic(&flic);
	cleanup_all();
	return err;
}
