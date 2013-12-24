/*****************************************************************************
 * CUSTOM.C - This shows you how to implement a "custom" raster to
 *			  support your own type of display or memory organization.
 *			  What's implemented here is a particularly slow driver for
 *			  the MCGA/VGA in 320x200 256 color mode, but as an example
 *			  it should suffice.
 *
 * Major Features Demonstrated:
 *	- Implementing a simple custom raster I/O library.
 *	- Using the X/Y offset values in FlicPlayOptions.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pjbasics.h"
#include "pjcustom.h"

#ifdef __WATCOMC__
	#pragma off(unreferenced);
#endif

/* Debugging macro - if DEBUG define will do copious printfs... */
#undef DEBUG

#ifdef DEBUG
	#define DDT(a) printf a
	#define GOT_HERE printf("%s %d\n", __FILE__, __LINE__)
#else
	#define DDT(a)
	#define GOT_HERE
#endif

typedef unsigned char UBYTE;

/*----------------------------------------------------------------------------
 * usage text
 *--------------------------------------------------------------------------*/

char usage[] =
	"custom - display a flic using a custom built (and slow) driver\n"
	"for 320x200 256 color mode.\n"
	"Usage:\n"
	"   CUSTOM flicname [xoffset yoffset]\n"
	"where xoffset and yoffset if present are numbers telling how far\n"
	"off center to display the flic";


/*----------------------------------------------------------------------------
 * Assembler function to move bytes between different segments.
 * If the USING_PHARLAP macro is defined by the makefile command line,
 * we prototype the external function needed to do a farcopy.  If the
 * macro isn't defined, we're under a DMPI-ish extender that maps video
 * memory into the normal DS segment, and we use a standard memcpy().
 *--------------------------------------------------------------------------*/

#ifdef USING_PHARLAP
  extern far_copy_bytes(void *soff, int sseg, void *doff, int dseg, int size);
#else
  #define far_copy_bytes(soff, sseg, doff, dseg, size) memcpy(doff, soff, size)
#endif

void out_byte(int port, int value); /* Send byte out port */

#define NEAR_SEG 0x14	/* This is the normal Phar Lap protected mode
						 * flat model segment where our data segment
						 * normally resides. */
#define REAL_SEG 0x34	/* A segment you can use to access the lower
						 * 1 Meg of real-mode memory (including the
						 * MCGA/VGA space) */
#define MCGA_BASE_ADDRESS 0xA0000	/* Start of real mode color video memory */

#define MCGA_XMAX 320
#define MCGA_YMAX 200
#define MCGA_PIXEL_ADDRESS(x,y) ((Pixel *)(0xA0000+MCGA_XMAX*(y)+(x)))

/* These two functions are just BIOS calls.  They're part of
 * the fli-lib though not generally documented. */

int 		pj_get_vmode(void); 	/* Get video mode from BIOS */
void		pj_set_vmode(int mode); /* Set video mode with the BIOS */

#ifdef __WATCOMC__
  #pragma aux (FLICLIB3S) out_byte;    /* in case we're compiling -3r style */
  #pragma aux (FLICLIB3S) far_copy_bytes;
  #pragma aux (FLICLIB3S) pj_get_vmode;
  #pragma aux (FLICLIB3S) pj_set_vmode;

  #pragma aux (FLICLIB3S) put_dot;		/* these are called from fliclib,	*/
  #pragma aux (FLICLIB3S) get_dot;		/* and fliclib assumes that they'll */
  #pragma aux (FLICLIB3S) set_hline;	/* be -3s style functions, so we've */
  #pragma aux (FLICLIB3S) put_hseg; 	/* got to force them to be compiled */
  #pragma aux (FLICLIB3S) get_hseg; 	/* that way.						*/
  #pragma aux (FLICLIB3S) set_colors;
  #pragma aux (FLICLIB3S) wait_vsync;
#endif

Errcode pj_error_internal(Errcode err, char *file, int line)
/*****************************************************************************
 * This routine will cause the fliclib to print out a bit more
 * diagnostic information than usual in case of program errors.
 * Also being here it prevents "put_dot" from being located
 * at address 0 in some dos-extender environments,  which would
 * make the FlicLib complain about getting passed NULL pointers
 * when it wants function pointers....
 ****************************************************************************/
{
fprintf(stderr, "Internal error %d on line %d of file %s\n", err, line, file);
return Err_reported;
}


static void put_dot(FlicRaster *f, long x, long y, Pixel color)
/*****************************************************************************
 *
 ****************************************************************************/
{
	DDT(("put_dot(%p %d %d %d)\n", f, x, y, color));
	far_copy_bytes(&color,NEAR_SEG,MCGA_PIXEL_ADDRESS(x,y),REAL_SEG,1);
}

static Pixel get_dot(FlicRaster *f, long x, long y)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Pixel color;

	DDT(("get_dot(%p %d %d)\n", f, x, y));
	far_copy_bytes(MCGA_PIXEL_ADDRESS(x,y),REAL_SEG,&color,NEAR_SEG,1);
	return(color);
}

static void set_hline(FlicRaster *f, Pixel color, long x, long y, long w)
/*****************************************************************************
 *
 ****************************************************************************/
{
	DDT(("set_hline(%p %d %d %d %d)\n", f, color, x, y, w));
	while (--w >= 0)
		put_dot(f,x++,y,color);
}

static void put_hseg(FlicRaster *f, Pixel *pixbuf, long x, long y, long w)
/*****************************************************************************
 *
 ****************************************************************************/
{
	DDT(("set_hseg(%p %p %d %d %d)\n", f, pixbuf,x,y,w));
	far_copy_bytes(pixbuf,NEAR_SEG,MCGA_PIXEL_ADDRESS(x,y),REAL_SEG,w);
}

static void get_hseg(FlicRaster *f, Pixel *pixbuf, long x, long y, long w)
/*****************************************************************************
 *
 ****************************************************************************/
{
	DDT(("get_hseg(%p %p %d %d %d)\n", f, pixbuf,x,y,w));
	far_copy_bytes(MCGA_PIXEL_ADDRESS(x,y),REAL_SEG,pixbuf,NEAR_SEG,w);
}

static void set_colors(FlicRaster *f, long start, long count, UBYTE *cbuf)
/*****************************************************************************
 *
 ****************************************************************************/
{
	#define CMAP_INDEX_REG 0x3C8
	#define CMAP_DATA_REG  0x3C9
	DDT(("set_colors(%p %d %d %p)\n", f, start, count, cbuf));
	out_byte(CMAP_INDEX_REG,start);
	count *= 3; 	/* Do out for each of R G B */
	while (--count >= 0)
		{
		/* Send out one RGB to VGA palette.  It wants values from 0-63
		 * instead of 0-255, hence the >>2 */
		out_byte(CMAP_DATA_REG,(*cbuf++)>>2);
		}
}

static void wait_vsync(FlicRaster *f)
/*****************************************************************************
 *
 ****************************************************************************/
{
	DDT(("wait_vsync(%p)\n", f));
}

/*----------------------------------------------------------------------------
 * A RastlibCustom structure, init'd to point to our custom I/O routines.
 *--------------------------------------------------------------------------*/

static RastlibCustom custom_lib =
	{
	put_dot,		/* Required. Set a single dot. */
	get_dot,		/* Required. Get a single dot. */
	set_hline,		/* Optional.  Set horizontal line a solid color */
	put_hseg,		/* Optional.  Copy pixel buffer to horizontal line. */
	get_hseg,		/* Optional.  Copy horizontal line to pixel buffer. */
	set_colors, 	/* Required for visible rasters. (Not for ones in memory)
					 * Update hardware color map with RGB 0-255 values.  */
	wait_vsync, 	/* Optional. Wait until monitor is in vertical retrace
					 * (to help minimize tear and flicker during drawing */
	};

FlicRaster *raster = NULL;		/* Customized Raster */


Errcode play_on_custom_mcga(char *flic_name, int xoff, int yoff)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;
	int old_bios_video_mode;
	FlicPlayOptions options;

	old_bios_video_mode = pj_get_vmode();	/* Assembler call to BIOS */
	pj_set_vmode(0x13); 					/* Ask bios for 320x200 256 color */
	GOT_HERE;
	if (pj_get_vmode() == 0x13) 			/* Made it ? */
		{
		GOT_HERE;
		if ((err = pj_raster_make_custom(&raster,
										 MCGA_XMAX, MCGA_YMAX, &custom_lib))
										 >= Success)
			{
			GOT_HERE;
			pj_playoptions_init(&options);
			options.x = xoff;		/* X-offset from command line */
			options.y = yoff;		/* Y-offset from command line */
			options.playback_raster = raster;
			GOT_HERE;
			err = pj_flic_play(flic_name,&options);
			GOT_HERE;
			pj_raster_free_custom(&raster);
			GOT_HERE;
			}
		pj_set_vmode(old_bios_video_mode);
		}
	else
		err = Err_no_display;
	return(err);
}


void main(int argc, char *argv[])
/*****************************************************************************
 *
 ****************************************************************************/
{
	int xoff = 0, yoff = 0; 	/* default offset of flic on screen */
	char *title;
	Errcode err;

	/* Process command line, and print usage message if it looks bad. */
	if (argc < 2)
		{
		puts(usage);
		exit(0);
		}
	title = argv[1];
	if (argc > 2)
		xoff = atoi(argv[2]);
	if (argc > 3)
		yoff = atoi(argv[3]);
	if ((err = play_on_custom_mcga(title,xoff,yoff)) < Success)
		fprintf(stderr, "custom %s failed code %d\n", title, err);
	exit(err);
}


#ifdef NEVER
/**   DEBUGGING ONLY! **/
#define RT_CLIPBOX  7  /* this is a "clip box" */ 
struct rastlib	*pj_get_safecbox_lib();
struct rastlib	*pj_get_cbox_lib();

Boolean pj_clipbox_make(FlicRaster *cb, FlicRaster *r,
					 Coor x,Coor y,Coor width,Coor height)

/* makes a clip box usable, returns 0 if clipped out 0 if some part of it
 * is on the raster puts a null lib in cbox if clipped out may be called
 * repeatedly for moving the box */
{
extern void *pj_get_null_lib();
Boolean outside = FALSE;

	cb->type = RT_CLIPBOX;
	cb->pdepth = r->pdepth;
	cb->aspect_dx = r->aspect_dx;
	cb->x = x;
	cb->width = width;
	cb->y = y;
	cb->height = height;
	cb->hw.custom_data[0] = r;

	/* check if the rect overlaps the raster at least a little bit */

	if(x < 0)
	{
		if(width <= -x)
			goto clipout;
		outside = TRUE;
	}
	else if( x > r->width)
		goto clipout;

	if(y < 0)
	{
		if(height <= -y)
			goto clipout;
		outside = TRUE;
	}
	else if( y > r->height)
		goto clipout;

	/* if the clip rectangle extends outside the root raster we have to install
	 * the library to force clip for the root raster as well */

	if(outside
		|| r->width < (x + width)
		|| r->height < (y + height))
	{
		cb->lib = pj_get_safecbox_lib();
		printf("Getting safe lib\n");
	}
	else
	{
		cb->lib = pj_get_cbox_lib();
		printf("Getting quick lib\n");
	}

	printf("width %d, height %d, x %d, y %d, r->width %d, r->height %d\n",
		width, height, x, y, r->width, r->height);
	pj_key_in();

	return(1);
clipout:
	cb->lib = pj_get_null_lib();
	return(0);
}
#endif /* NEVER */
