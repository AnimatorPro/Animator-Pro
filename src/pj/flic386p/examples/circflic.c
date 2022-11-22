/*****************************************************************************
 * CIRCFLIC.C - Example of using a compress-only raster to create a flic.
 *
 * Major Features Demonstrated:
 *
 *	-  The sequence of calls needed to create a flic from scratch.
 *	-  Using a simple compress-only custom raster as the input to the
 *	   flic creation routines.
 *
 ****************************************************************************/

#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include "pjbasics.h"
#include "pjcustom.h"


/*----------------------------------------------------------------------------
 * The usage message.
 *--------------------------------------------------------------------------*/

char usage[] =
	"circflic - a program that creates a 10 frame flic file of\n"
	"a growing circle.  Sample program for compress-only raster.\n\n"
	"Usage:\n  CIRCFLIC outputfile [width height]\n";

/*----------------------------------------------------------------------------
 * some handy macros.
 *--------------------------------------------------------------------------*/


#ifndef ACCESS_RD
	/* HIGH C and CodeBuilder seem to have left this out of io.h... */
#define ACCESS_RD	0x0004
#endif

#define fexists(a) (0 == access((a),ACCESS_RD))

#define XMAX 640
#define YMAX 480
#define TMAX 10

#define PixOffs(x,y) (((x)*YMAX)+(y))

/*----------------------------------------------------------------------------
 * the raster dimensions.
 *	these default to our max sizes, but can be overriden from the command
 *	line at runtime to create a flic of any dimensions.
 *--------------------------------------------------------------------------*/

int x_size = XMAX;
int y_size = YMAX;

/*----------------------------------------------------------------------------
 * This datatype - MyScreen - represents some pre-existing screen structure
 * in the code you're trying to tie the Fliclib into.  Probably in real
 * life it won't be this simple...
 *--------------------------------------------------------------------------*/

typedef struct {Pixel *p;} MyScreen;

MyScreen this_pic;		/* Custom screens */
MyScreen last_pic;

/*----------------------------------------------------------------------------
 * code...
 *--------------------------------------------------------------------------*/

#ifdef __WATCOMC__
  #pragma aux (FLICLIB3S) my_get_hseg; /* called from fliclib, must be 3s style */
#endif

Errcode pj_error_internal(Errcode err, char *file, int line)
/*****************************************************************************
 * This routine will cause the fliclib to print out a bit more
 * diagnostic information than usual in case of program errors.
 * Also being here it prevents "my_get_hseg" from being located
 * at address 0 in some dos-extender environments,  which would
 * make the FlicLib complain about getting passed NULL pointers
 * when it wants function pointers....
 ****************************************************************************/
{
fprintf(stderr, "Internal error %d on line %d of file %s\n", err, line, file);
return Err_reported;
}


void my_get_hseg(FlicRaster *r, Pixel *pixbuf,	long x, long y, long w)
/*****************************************************************************
 * Get a horizontal line segment off of our customized FlicRaster into
 * a memory Pixel buffer.  when we created the raster, we stored a pointer
 * to its pixel data buffer in the custom_data[0] field of the FlicRaster
 * structure.  This routine uses that pointer to fetch the data the caller
 * has requested.  (Not quickly or efficiently, mind you. <grin>)
 ****************************************************************************/
{
	int 	  i;
	MyScreen *s = r->hw.custom_data[0]; /* Fetch MyScreen pointer */

	for (i=0; i<w; ++i)
		pixbuf[i] =  s->p[PixOffs(x+i,y)];
}

void circle(MyScreen *screen, int xcen, int ycen, int rad, Pixel color)
/*****************************************************************************
 * Draw a circle on a screen.  (In this case, a MyScreen, not a video screen).
 ****************************************************************************/
{
	int err;
	int derr, yerr, xerr;
	int aderr, ayerr, axerr;
	register int x,y;

	if (rad <= 0) { /* Do one pixel case */
		screen->p[PixOffs(xcen,ycen)] = color;
		return;
	}
	err = 0;
	x = rad;
	y = 0;
	for (;;) {
		/* draw 4 quadrants of a circle... */
		screen->p[PixOffs(xcen+x,ycen+y)] = color;
		screen->p[PixOffs(xcen-x,ycen+y)] = color;
		screen->p[PixOffs(xcen+x,ycen-y)] = color;
		screen->p[PixOffs(xcen-x,ycen-y)] = color;
		axerr = xerr = err -x-x+1;
		ayerr = yerr = err +y+y+1;
		aderr = derr = yerr+xerr-err;
		if (aderr < 0)
			aderr = -aderr;
		if (ayerr < 0)
			ayerr = -ayerr;
		if (axerr < 0)
			axerr = -axerr;
		if (aderr <= ayerr && aderr <= axerr) {
			err = derr;
			x -= 1;
			y += 1;
		} else if (ayerr <= axerr) {
			err = yerr;
			y += 1;
		} else {
			err = xerr;
			x -= 1;
		}
		if (x < 0)
			break;
	}
}

Errcode make_circle_frames(Flic 	  *flic,
						   FlicRaster *this_rast,
						   FlicRaster *last_rast)
/*****************************************************************************
 * Write out a circle to each frame of the flic.
 ****************************************************************************/
{
	int 	i;
	int 	radius_increment = x_size/(4*TMAX);
	int 	radius = 0;
	Errcode err;
	int 	color = 200;		/* A pretty section of default cmap */

	for (i=0; i<TMAX; ++i) {
		fprintf(stdout, "Making circle frame %d\r", i+1);
		fflush(stdout);
		circle(&this_pic, x_size/2, y_size/2, radius, color);/* draw circle */
		radius += radius_increment; 					/* make radius bigger */
		color -= 1; 									/* and color dimmer */
		if (i == 0) 									/* is first frame? */
			err = pj_flic_write_first(flic, this_rast); /* then write first */
		else											/* else write next. */
			err = pj_flic_write_next(flic, this_rast, last_rast);
		if (err < Success)								/* Return status if bad */
			return(err);
		memcpy(last_pic.p, this_pic.p, XMAX*YMAX);		/* copy this frame to
														 * last frame */
		}
	fprintf(stdout, "Making circle ring frame    \r");
	return pj_flic_write_finish(flic, this_rast);		/* Write 'ring' frame */
}


Errcode bind_my_raster(FlicRaster **r, MyScreen *s)
/*****************************************************************************
 * Attatch a MyScreen to a custom raster.
 ****************************************************************************/
{
	Errcode err;

	err = pj_raster_make_compress_only(r, x_size, y_size, my_get_hseg);

	if (err >= Success) {			   /* Stash away pointer to screen in */
		(*r)->hw.custom_data[0] = s;   /* custom/hardware data area. */
	}
	return(err);
}

void unbind_my_raster(FlicRaster **r)
/*****************************************************************************
 * Detach a MyScreen from a custom raster, free the raster.
 ****************************************************************************/
{
	pj_raster_free_compress_only(r);
}

Errcode make_circle_flic(char *flic_name)
/*****************************************************************************
 * Open up all the resources needed to create a flic, and then
 * call make_circle_flic to write out the frames.
 ****************************************************************************/
{
	FlicRaster *this_rast;
	FlicRaster *last_rast;
	Errcode 	err;
	AnimInfo	info;
	Flic		flic;
	char		fullname[128];

	if ((err = bind_my_raster(&this_rast, &this_pic))  >= Success) {
		if ((err = bind_my_raster(&last_rast, &last_pic)) >= Success) {
			if ((err = pj_animinfo_init(&info)) >= Success) {
				info.width	= x_size;
				info.height = y_size;
				strcpy(fullname, flic_name);
				pj_flic_complete_filename(fullname, &info, TRUE);
				if (fexists(fullname)) {
					char response;
					fprintf(stderr, "Okay to overwrite existing %s file? ", fullname);
					fflush(stderr);
					response = getche();
					fprintf(stderr, "\n");
					if (response != 'y' && response != 'Y')
						return Err_abort;
				}
				if ((err = pj_flic_create(fullname, &flic, &info)) >= Success) {
					err = make_circle_frames(&flic, this_rast, last_rast);
				}
			}
			unbind_my_raster(&last_rast);
		}
		unbind_my_raster(&this_rast);
	}
	return err;
}

void main(int argc, char *argv[])
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;
	char *flic_name;

	if (argc != 2 && argc != 4) { /* allow 1 or 3 parms on cmdline */
		fputs(usage, stderr);
		exit(1);
	}
	flic_name = argv[1];

	if (argc > 2) {
		x_size = atoi(argv[2]);
		y_size = atoi(argv[3]);
		if (x_size <= 0 || y_size <= 0 || x_size > XMAX || y_size > YMAX) {
			fprintf(stderr, "Error in flic size argument(s).\n");
			fputs(usage, stderr);
			exit(1);
		}
	}

	if (NULL == (this_pic.p = calloc(XMAX,YMAX)) ||
		NULL == (last_pic.p = calloc(XMAX,YMAX))) {
		fprintf(stderr, "Not enough memory for screen buffers\n");
		exit(1);
	}

	err = make_circle_flic(flic_name);

	if (err < Success)
		fprintf(stderr, "circflic %s failed code %d\n", flic_name, err);

	exit(err);
}
