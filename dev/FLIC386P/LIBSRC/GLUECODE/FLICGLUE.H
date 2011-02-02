/*****************************************************************************
 *
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * for Watcom C only, the following pragma names an alias for calling
 * and return value standards to implement the -3s/HighC standard.	We
 * declare (in other header files) all our documented entry points with the
 * FLICLIB3S name, which allows our functions to be called from a program
 * compiled using the (normal) -3r Watcom standard of parms-in-regs.
 * new feature:  if we're compiling the Zortech version of the library,
 * we put an underbar on the front of all our documented entry points.
 *--------------------------------------------------------------------------*/

#ifdef __WATCOMC__
  #ifdef FLILIB_FOR_ZORTECH
	#pragma aux FLICLIB3S	"_*"                     \
							parm caller []			 \
							value no8087			 \
							modify [eax ecx edx gs];
  #else
	#pragma aux FLICLIB3S	"*"                      \
							parm caller []			 \
							value no8087			 \
							modify [eax ecx edx gs];
  #endif
#endif

/*----------------------------------------------------------------------------
 * include the header files we need, including all client headers.
 *--------------------------------------------------------------------------*/

#define LibRast FlicRaster

#include "stdtypes.h"   /* this one has to be first! */
#include "pjltypes.h"   /* this one has to be before fli.h */
#include "fli.h"        /* this one has to be before pjgfx.h */
#include "pjgfx.h"
#include "pjcustom.h"
#include "pjprotos.h"
#include "errcodes.h"
#include "animinfo.h"

/*----------------------------------------------------------------------------
 * a little structure we use to track things during flic playback or creation
 *--------------------------------------------------------------------------*/

typedef struct flilib_control {
	int 		cur_frame;
	UBYTE		iotype;
	} FliLibCtl;

/*----------------------------------------------------------------------------
 * A couple protos for functions in the gluecode that we no longer publish
 * for the clients' use, but we still use internally.
 *--------------------------------------------------------------------------*/

FlicRaster *pj_raster_center_virtual(FlicRaster *root, FlicRaster *virt,
							int width, int height);

FlicRaster *pj_raster_clip_virtual(FlicRaster *root, FlicRaster *virt,
							int x, int y, int width, int height);

Boolean pj_cmaps_same(PjCmap *s1, PjCmap *s2);
