#ifndef ANIMINFO_H
#define ANIMINFO_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#define DEFAULT_AINFO_SPEED 71

typedef struct anim_info {
	USHORT width, height;  /* width and height of image area */

	SHORT x, y;			   /* x and y position of picture's top left corner
							* (if loaded as a cel)
						    * if not available set to 0,0 */

	LONG num_frames;       /* number of image frames in this image file
							* not including any loop back or ring deltas */

	SHORT depth;		   /* pixel depth of source For now pdr loaders use
							* pixel depths of 8 bits and this is ignored 
							* true color pdrs use Rgb byte triplets and this is
							* also ignored.
							* it is the loaders duty to convert it's input
							* to fit the screen provided */

	USHORT millisec_per_frame; /* playback speed of animation 
								* milli seconds (1/1000 sec) per frame
								* if not available, set to 
								* DEFAULT_AINFO_SPEED */
	USHORT aspect_dx;    /* aspect ratio ie: dx X dy is a square */
	USHORT aspect_dy;
	UBYTE reserved[44];  /* total of 64 bytes */
} Anim_info;

#endif /* ANIMINFO_H */
