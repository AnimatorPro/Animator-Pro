#include "errcodes.h"
#include "memory.h"
#include "gfx.ih"

Errcode polygon(void *r,Pixel color,Short_xy *points,int count,Boolean filled)
{
Errcode err;
Poly *pg;
LLpoint *pt;
Sdat sd;

	if(count <= 0)
		return(0);

	if((pg = pj_malloc(sizeof(Poly)+(sizeof(LLpoint)*count))) == NULL)
		return(Err_no_memory);

	clear_struct(pg);
	pt = (LLpoint *)(pg + 1);
	pg->clipped_list = pt;
	pg->pt_count = count;
	pg->polymagic = POLYMAGIC;

	for(;;)
	{
		*((Short_xy *)&(pt->x)) = *points++;
		pt->z = 0;
		if(--count <= 0)
			break;
		pt = (pt->next = pt+1);
	}
	pt->next = pg->clipped_list;

	sd.rast = r;
	sd.color = color;

	if(filled)
		err = fill_poly_inside(pg,shline,&sd);
	else
		err = Success;
	hollow_polygon(pg,scline,&sd,TRUE); 
	pj_free(pg);
	return(err);
}
