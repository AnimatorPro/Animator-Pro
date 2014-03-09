#include "animinfo.h"
#include "memory.h"
#include "picfile.h"

Errcode pic_anim_info(char *name, Anim_info *pinfo)
{
Errcode err;
Jfile f;
Pic_header pic;

	if ((f = pj_open(name, JREADONLY)) == JNONE)
		return(pj_ioerr());
	if((err = pj_read_pichead(f,&pic)) < Success)
		goto error;

	clear_struct(pinfo);
	pinfo->x = pic.x;
	pinfo->y = pic.y;
	pinfo->width = pic.width;
	pinfo->height = pic.height;
	pinfo->depth = pic.depth;
	pinfo->num_frames = 1;

error:
	pj_close(f);
	return(err);
}
