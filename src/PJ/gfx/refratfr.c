#include "rectang.h"

void frame_tofrect(SHORT x0, SHORT y0, SHORT x1, SHORT y1, Fullrect *fr)
{
	frame_tocrect(x0,y0,x1,y1,(Cliprect *)&(fr->CRECTSTART));
	crect_tofrect((Cliprect *)&(fr->CRECTSTART),fr);
}
