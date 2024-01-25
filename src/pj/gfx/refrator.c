#include "rectang.h"

void frame_torect(SHORT x0, SHORT y0, SHORT x1, SHORT y1, Rectangle *r)
{
Cliprect cr;

	frame_tocrect(x0,y0,x1,y1,&cr);
	crect_torect(&cr,r);
}
