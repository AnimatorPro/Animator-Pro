#define VDEV_INTERNALS
#include "vdevice.h"
#include "vdevinfo.h"

char *pj_vd_get_more_info(Vdevice *vd, USHORT mode)
{
char *txt;
if (vd->lib->mode_text == NULL)
	return("");
if ((txt = (*vd->lib->mode_text)(vd,mode)) == NULL)
	return("");
return(txt);
}

