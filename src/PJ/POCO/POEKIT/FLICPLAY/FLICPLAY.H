/*****************************************************************************
 *
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * include the header files we need...
 *--------------------------------------------------------------------------*/

#include "stdtypes.h"
#include "errcodes.h"

#include "pocorex.h"
#include "syslib.h"
#include "gfx.h"
#include "stdio.h"
#include "syslib.h"

#include "jfile.h"
#include "fli.h"
#include "animinfo.h"

/*----------------------------------------------------------------------------
 * The Flic type...
 *--------------------------------------------------------------------------*/

typedef struct flic {
	struct flic *next;
	ULONG		magic;
	Flifile 	*flifile;
	Rcel		*root_raster;
	Rcel		*playback_raster;
	void		*framebuf;
	int 		frames_played;
	int 		cur_frame;
	int 		num_frames;
	int 		speed;
	long		eventdata;
	Boolean 	keyhit_stops_playback;
	} Flic;

#define IANS_FLIC_MAGIC 	0x19040259	/* typical magic number */
#define BEFORE_FIRST_FRAME	-1			/* indicates haven't started playback */

/*----------------------------------------------------------------------------
 * prototypes...
 *--------------------------------------------------------------------------*/

/* in OPENFUNC.C */

extern void 	do_flic_close_all(void);
extern void 	do_flic_close(Flic *pflic);
extern Errcode	do_flic_open(char *path, Flic **ppflic);
extern Errcode	do_flic_options(Flic *pflic, int speed, int keyhit, Rcel *newrast, int x, int y);

/* in PLAYFUNC.C */

extern Errcode	do_play(Flic *pflic);
extern Errcode	do_play_once(Flic *pflic);
extern Errcode	do_play_timed(Flic *pflic, ULONG millisecs);
extern Errcode	do_play_count(Flic *pflic, int frame_count);
extern void 	do_rewind(Flic *pflic);
extern void 	do_seek_frame(Flic *pflic, int theframe);
