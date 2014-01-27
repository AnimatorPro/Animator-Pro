#include <string.h>
#include "errcodes.h"
#include "jfile.h"
#include "fli.h"
#include "animinfo.h"

Errcode pj_fli_info_open(Flifile *flif, char *path, Anim_info *ainfo)

/* opens and verifies a fli file reads in "Anim_info" data if OK,
 * and returns, will leave file open if successful.  Used by linked in PDR */ 

{
Errcode err;

	if((err = pj_fli_open(path, flif, JREADONLY)) < Success)
		return(err);

	if(ainfo)
	{
		memset(ainfo, 0, sizeof(*ainfo));
		ainfo->width = flif->hdr.width;
		ainfo->height = flif->hdr.height;
		ainfo->depth = flif->hdr.bits_a_pixel;
		ainfo->num_frames = flif->hdr.frame_count;
		ainfo->millisec_per_frame = flif->hdr.speed;
		ainfo->aspect_dx = flif->hdr.aspect_dx;
		ainfo->aspect_dy = flif->hdr.aspect_dy;
	}
	return(Success);
}
