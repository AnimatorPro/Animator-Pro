#include "rectang.h"

void bclip0xy_rect(Rectangle *r,  /* rectangle to clip */
		           Rectangle *port)  /* boundary rectangle */

/* does a bclip but substitutes x and y with 0 */
{
Rectangle b;	
	b.x = b.y = 0;
	b.width = port->width;
	b.height = port->height;
	bclip_rect(r,&b);
}
