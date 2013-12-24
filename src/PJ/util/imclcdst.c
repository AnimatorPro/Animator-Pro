#include "imath.h"

int calc_distance(short x1,short y1,short x2,short y2)
{
long delta_x, delta_y;

delta_x = x1 - x2;
delta_y = y1 - y2;

return(sqr_root( (long)(delta_x*delta_x) + (long)(delta_y*delta_y)));
}

