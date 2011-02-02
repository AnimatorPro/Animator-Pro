/*****************************************************************************
 * VDRVALL.C - Add all builtin video drivers to the list of eligible drivers.
 *
 *	This function MUST remain in a module by itself and not rolled into the
 *	rest of the video glue.  Keeping this function separate is the only way
 *	to ensure that all the drivers we support don't get linked into the
 *	client application unless the client specifically wants that.
 ****************************************************************************/

#define VDEV_INTERNALS
#include "flicglue.h"
#include "vdevice.h"

void pj_video_add_all(void)
/*****************************************************************************
 * add all our builtin drivers to the current driver list.
 *	note that the order here is important:	the drivers are detected in
 *	reverse order from the way they're added, so mcga should always be
 *	the first driver added, since we want it to be the last-resort driver.
 ****************************************************************************/
{
	pj_video_add(pj_vdev_mcga);
	pj_video_add(pj_vdev_8514);
	pj_video_add(pj_vdev_supervga);
	pj_video_add(pj_vdev_vesa);
}
