#ifndef VDEVINFO_H
#define VDEVINFO_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef VDEVICE_H 
	#include "vdevice.h"
#endif

/* This structure specifies a range of allowed values.  Min and max are
   the extremes, actual the current value.  Grain is a constraint on
   actual.  Actual must be a multiple of grain.  (It is common on a
   bit-plane oriented device for the width of the display to be constrained
   to multiples of 8.) */

typedef struct srange {
	SHORT min, max;
	SHORT actual;
	SHORT grain;
} Srange;

/* This is a structure to store information about a 'major' mode supported
   by a graphics device. */
typedef struct vmode_info {
	USHORT struct_size;	/* For future compatibility sizeof(Vmode_info) */
	USHORT mode_ix;	/* Which mode this refers to. */
	char mode_name[20];	/* only 1st 18 displayed in screen size menu */
	USHORT bits;		/* Number of bits per pixel in a single plane */
	USHORT planes;		/* Number of planes of pixels */
	Srange width;
	Srange height;
	UBYTE readable;	/* Is device readable, or output only? */
	UBYTE writeable; /* It's a video digitizer and can read but not write? */
	UBYTE displayable;	/* Can open a primary (displayable) screen */
	UBYTE fields_per_frame;	/* normally 1, interlaced 2 or more */
	SHORT display_pages;	/* # of pages displayable */
	SHORT store_pages;		/* # of full screen pages storable */
	LONG display_bytes;		/* # of bytes used by display */
	LONG store_bytes;		/* # of bytes available including display */
	UBYTE palette_vblank_only;	/* Only set pallete during vsync? */
	UBYTE screen_swap_vblank_only;	/* Only swap screens during vsync? */

	ULONG field_rate;			/* monitor field scan period, ie: time it takes
								 * to get back to the same vertical position 
								 * on the monitor (on interlace cards this is 
								 * one half the frame rate) the time it takes
								 * to scan the field plus the vertical blank
								 * interval period 
								 * in 1/1,000,000 sec */

	ULONG vblank_period;		/* elapsed time for vertical blank from bottom
								 * of field to top of field 
								 * in 1/1,000,000 sec */
} Vmode_info;
 


#endif /* VDEVINFO_H */
