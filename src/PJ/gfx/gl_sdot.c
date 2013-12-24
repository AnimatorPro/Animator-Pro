#include "gfx.ih"

void gl_sdot(SHORT x, SHORT y, Sdat *sd)
{
	pj_put_dot(sd->rast, sd->color, x,y);
}
