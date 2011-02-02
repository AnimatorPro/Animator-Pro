#include "rcel.h"

Rcel *center_virtual_rcel(Rcel *root, Rcel *virt, int width, int height)

/* given root cel and pointer to an unused Rcel will center a virtual rcel 
 * on root for the width and height provided. if they are the same as
 * the root screen size the pointer to the root is returned otherwise 
 * pointer to centered virtual rcel */
{
Rectangle rect;

	if(width == root->width && height == root->height)
		return(root);

	rect.x = (root->width - width) / 2;  
	rect.y = (root->height - height) / 2;  
	rect.width = width;
	rect.height = height;
	pj_rcel_make_virtual(virt, root, &rect);
	return(virt);
}
