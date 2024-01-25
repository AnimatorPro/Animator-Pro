#include "animinfo.h"
#include "errcodes.h"
#include "memory.h"
#include "picfile.h"
#include "pjassert.h"

Errcode
pic_anim_info(char *name, Anim_info *pinfo)
{
	Errcode err;
	XFILE *xf;
	Pic_header pic;

	if (!pj_assert(pinfo != NULL)) return Err_bad_input;

	err = xffopen(name, &xf, XREADONLY);
	if (err < Success)
		return err;

	err = pj_read_pichead(xf, &pic);
	if (err < Success)
		goto error;

	clear_struct(pinfo);
	pinfo->x = pic.x;
	pinfo->y = pic.y;
	pinfo->width = pic.width;
	pinfo->height = pic.height;
	pinfo->depth = pic.depth;
	pinfo->num_frames = 1;

error:
	xffclose(&xf);
	return err;
}
