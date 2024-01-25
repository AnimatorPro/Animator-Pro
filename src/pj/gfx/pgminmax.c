#include "gfx.h"
#include "poly.h"

void find_pminmax(Poly *poly, Rectangle *r)
{
register LLpoint *pointpt;
int i;
SHORT xmax,ymax;

pointpt = poly->clipped_list;
r->x = xmax = pointpt->x;
r->y = ymax = pointpt->y;
pointpt = pointpt->next;

i = poly->pt_count;
while (--i > 0)
   {
   if (r->x > pointpt->x) r->x = pointpt->x;
   if (xmax < pointpt->x) xmax = pointpt->x;
   if (r->y > pointpt->y) r->y = pointpt->y;
   if (ymax < pointpt->y) ymax = pointpt->y;
   pointpt = pointpt->next;
   }
r->width = xmax - r->x + 1;
r->height = ymax - r->y + 1;
}
