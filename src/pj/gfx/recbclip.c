#include "rectang.h"

void bclip_rect(register Rectangle *r,  /* rectangle to clip */
		        register Rectangle *b)  /* boundary rectangle */

/* "boundary" clip a rectangle.  Clips it if its larger than bounds.
 * and brings edge to edge of boundary rectangle if rect goes outside
 * of bounds */
{
SHORT overedge;

	if(r->width > b->width)
		r->width = b->width;

	if(r->height > b->height)
		r->height = b->height;

	if (r->x < b->x)
		r->x = b->x;
	else
	{
		overedge = (r->x + r->width) - (b->x + b->width);
		if(overedge > 0)
			r->x -= overedge;
	}

	if (r->y < b->y)
		r->y = b->y;
	else
	{
		overedge = (r->y + r->height) - (b->y + b->height);
		if(overedge > 0)
			r->y -= overedge;
	}
}
