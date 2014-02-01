#include "gfx.ih"

void gl_sdot(SHORT x, SHORT y, void *data)
{
	Sdat *sd = data;
	pj_put_dot(sd->rast, sd->color, x, y);
}
