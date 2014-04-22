#include <string.h>
#include "errcodes.h"
#include "jfile.h"
#include "fli.h"
#include "animinfo.h"

/* Function: pj_fli_info_open
 *
 *  Opens and verifies a fli file. reads in "Anim_info" data if OK,
 *  and returns.  Will leave file open if successful.  Used by linked
 *  in PDR.
 */
Errcode
pj_fli_info_open(Flifile *flif, char *path, Anim_info *ainfo)
{
	Errcode err;

	err = pj_fli_open(path, flif, XREADONLY);
	if (err < Success)
		return err;

	if (ainfo != NULL) {
		memset(ainfo, 0, sizeof(*ainfo));
		ainfo->width = flif->hdr.width;
		ainfo->height = flif->hdr.height;
		ainfo->depth = flif->hdr.bits_a_pixel;
		ainfo->num_frames = flif->hdr.frame_count;
		ainfo->millisec_per_frame = flif->hdr.speed;
		ainfo->aspect_dx = flif->hdr.aspect_dx;
		ainfo->aspect_dy = flif->hdr.aspect_dy;
	}

	return Success;
}
