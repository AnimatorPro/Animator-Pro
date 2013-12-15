#include "rectang.h"

void sclip_rect(register Rectangle *r,  /* rectangle to clip */
		        register Rectangle *b)  /* boundary rectangle */

/* size clip a rectangle */
{
	if(r->width > b->width)
		r->width = b->width;

	if(r->height > b->height)
		r->height = b->height;
}
