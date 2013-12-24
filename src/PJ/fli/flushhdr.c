#define FLI_1_0
#include "jfile.h"
#include "fli.h"

/*----------------------------------------------------------------------------
 * the following #ifdef changes the name of the standard library time()
 * function when this module is compiled as part of the fliclib.  this helps
 * our fliclib support both -3s and -3r calling convention by routing the
 * time() call through a glue routine called pj_time().  the FLILIB_CODE
 * macro is defined via -DFLILIB_CODE in the make.inc file for the fliclib.
 *--------------------------------------------------------------------------*/

#ifdef FLILIB_CODE
	#define time(a) pj_time(a)
#endif

void pj_i_update_id(Flifile *flif)
/* updates modify time and user id in Flifile header's id field */
{
	flif->hdr.id.update_time = time(NULL);
	flif->hdr.id.update_user = pj__fii_get_user_id();
}
Errcode pj_i_flush_head(Flifile *flif)

/* Updates id and flushes header of a Flifile leaves file offset
 * at end of header */
{
Errcode err;
LONG ospeed;

	pj_i_update_id(flif);
	ospeed = flif->hdr.speed;
	if(flif->hdr.type == FLIH_MAGIC)
	{
		((Fhead_1_0 *)(&flif->hdr))->jiffy_speed
					= ((((long)flif->hdr.speed)*70L)+500L)/1000L;
	}
	err = pj_writeoset(flif->fd,&flif->hdr,0,sizeof(flif->hdr));
	flif->hdr.speed = ospeed;
	return(err);
}
