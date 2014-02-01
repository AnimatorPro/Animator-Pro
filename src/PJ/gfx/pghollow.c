#include "stdtypes.h"
#include "memory.h"
#include "poly.h"
#include "errcodes.h"

void
hollow_polygon(Poly *poly,
		line_func lineout, void *linedat,
		Boolean closed)
{
register LLpoint *this, *next;
int i;

	i = poly->pt_count;
	if (!closed)
		--i;
	this = poly->clipped_list;
	while (--i >= 0)
	{
		next = this->next;
		(*lineout)(this->x, this->y, next->x, next->y, linedat); 
		this = next;
	}
}
