#include "rectang.h"

int clipcode_crects(register Cliprect *a,register Cliprect *b)

/*	returns a bit code for which edges of a are on what sides of b
 * if an edge is common it is assumed to cross b if a obscures b and
 * has no common edges CODEOBSCURES is returned */
{
int code;

	if(    (a->x >= b->MaxX)
		|| (a->y >= b->MaxY)
		|| (a->MaxX <= b->x)
		|| (a->MaxY <= b->y))
	{
		return(0);
	}

	if(a->x >= b->x)
		code = CODELEFT;
	else
		code = 0;
	if(a->y >= b->y)
		code |= CODETOP;
	if(a->MaxX <= b->MaxX)
		code |= CODERIGHT;
	if(a->MaxY <= b->MaxY)
		code |= CODEBOTTOM;

	if(!code)
		return(CODEOBSCURES);
	return(code);
}
