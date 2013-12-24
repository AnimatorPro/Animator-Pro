#include "gfx.h"

Boolean pj_rcel_make_virtual(Rcel *subcel, Rcel *root, Rectangle *toclip)
/************************************************************************* 
 * This makes a cel that is a rectangular piece of a potentially larger
 * cel.  It also forces all drawing and fli-playing routines to clip
 * to the bounds of the cel. 
 *
 * The raster and colormap in the new, clipped cel (subcel) are really the 
 * items in the root and if altered will alter the root.  DO NOT call 
 * free_rcel() or close_rcel() on this or you will be in BIG trouble.
 *
 * Parameters:
 *		Rcel 		*subcel;	The clipped subcel will be created here
 *		Rcel 		*root;		This is the parent cel.
 *		Rectangle	*toclip;	Contains the bounds of the parent you wish
 *								to include in subcel.  If this would
 *								include parts not inside the root,
 *								subcel will be smaller so that it is
 *								inside the root.
 * Returns:
 *		TRUE if some of the subcel is inside of the root cel.
 *		FALSE if the subcel is completely outside the root cel (and
 *			hence amounts to nothing at all).
 *
 *************************************************************************/
{
Boolean ret;

	ret = pj_clipbox_make((Clipbox *)subcel,(Raster *)root, 
				  toclip->x,toclip->y,toclip->width,toclip->height);
	subcel->cmap = root->cmap;
	return(ret);
}
