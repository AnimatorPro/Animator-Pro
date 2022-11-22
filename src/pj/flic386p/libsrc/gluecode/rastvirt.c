/*****************************************************************************
 * RASTVIRT.C - Create a clipped virtual raster at an arbitrary x/y offset
 *				over/within the root raster.  Also contains routine to create
 *				a centered virtual raster.
 *
 *	These functions are primarily for internal use by the other fliclib glue
 *	routines, but there's no real reason that can't be used directly from the
 *	client code.  One major caveat, though:  you had *better* make sure the
 *	width/height values passed to these corresponds to the width/height of
 *	the flic that will be played back on the virtual raster you're creating.
 *
 *	Note that these routines do NOT check for NULL pointers!
 *
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

FlicRaster *pj_raster_clip_virtual(FlicRaster *root,
								   FlicRaster *virt,
								   int x, int y,
								   int width, int height)
/*************************************************************************
 * This makes a raster that is a rectangular piece of a potentially larger
 * raster.	It also forces all drawing and fli-playing routines to clip
 * to the bounds of the raster.
 *
 * The raster and colormap in the new, clipped raster (virt) are really the
 * items in the root and if altered will alter the root.  DO NOT call
 * free_rraster() or close_rraster() on this or you will be in BIG trouble.
 *
 * Parameters:
 *		Rraster  *root			   This is the parent raster.
 *		Rraster  *virt			   The clipped virt will be created here
 *		int   x,y,width,height	Contains the bounds of the parent you wish
 *								to include in virt.  If this would
 *								include parts not inside the root,
 *								virt will be smaller so that it is
 *								inside the root.
 * Returns:
 *		pointer if some of the virt is inside of the root raster.
 *		NULL	if the virt is completely outside the root raster (and
 *				hence amounts to nothing at all).
 *
 * Majorly Important Note:
 *	The flic playback routines, at the very deepest and most intrinsic
 *	levels, are designed for playback on a raster exactly the size of the
 *	flic.  So, it IS NOT POSSIBLE to create a virtual raster with a width
 *	and height that are different (smaller or larger) than the flic size,
 *	to get a plays-a-portion-in-a-little-window effect.  So, this can be
 *	called with any x/y values you want, but the width/height values MUST
 *	be identical to the sizes of the flic you want to play back.  It's
 *	okay if part of the flic will be offscreen in terms of the root raster,
 *	that kind of clipping is allowed.
 *************************************************************************/
{
	Boolean ret;

	ret = pj_clipbox_make((Clipbox *)virt, (Raster *)root, x, y, width, height);
	virt->cmap = root->cmap;
	if (ret)
		return virt;
	else
		return NULL;
}

FlicRaster *pj_raster_center_virtual(FlicRaster *root, FlicRaster *virt,
									 int width, int height)
/*****************************************************************************
 * make a clipped centered virtual raster, if sizes differ from root raster.
 ****************************************************************************/
{
	int x,y;

	if(width == root->width && height == root->height)
		return(root);

	x = (root->width - width) / 2;
	y = (root->height - height) / 2;
	pj_raster_clip_virtual(root, virt, x, y, width, height);
	return(virt);
}
