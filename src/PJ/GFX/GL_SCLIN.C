#include "gfx.ih"

void gl_scline(SHORT x1, SHORT y1, SHORT x2, SHORT y2,Sdat *sd)
{
	pj_cline(x1, y1, x2, y2, sdot, sd);
}
