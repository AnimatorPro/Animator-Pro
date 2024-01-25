#include <string.h>
#include "procblit.h"
#include "rastcurs.h"
#include "wndo.h"

static Pixel curs_xlat[COLORS];
static Tcolxldat curs_tcxl = { 0, {0}, curs_xlat };
static SHORT xlat_csetid = -1;
static Pixel xlat_ccolor = CURS_CCOLOR;
static Pixel default_ccolor = CURS_CCOLOR;
static Pixel *cccolor = &default_ccolor; /* color for cursor tcxl to track as 
									      * color 250 */
/***** color translation handling ****/

void set_cursor_ccolor(Pixel *pccolor)

/* sets cursor translator to track cursor color 250 to *pccolor.  pccolor must
 * remain a valid address whenever a cursor is up */
{
	if((cccolor = pccolor) == NULL)
		cccolor = &default_ccolor;
}

static Pixel closest_to_tcol(void)
{
Cmap *cmap;

	cmap = icb.input_screen->wndo.cmap;
	/* can't use the tcolor 0, so start with [1] */
	return(1 + closestc(&cmap->ctab[0],&cmap->ctab[1],cmap->num_colors - 1));
}
Tcolxldat *get_cursor_xlat(void)
/* checks current values and re makes transltion table for cursor if 
 * environment has changed */
{
int i;
Pixel color, ccolor;

	if(xlat_csetid == icb.input_screen->mc_csetid 
	    && xlat_ccolor == (ccolor = *cccolor))
	{
		goto done;
	}
	memset(curs_xlat,0,sizeof(curs_xlat));

	for(i = 0;i < NUM_MUCOLORS;++i)
	{
		if(0 == (color = icb.input_screen->mc_colors[i]))
			color = closest_to_tcol();
		curs_xlat[i + CURS_MC0] = color;
	}

	if(ccolor == 0)
		curs_xlat[CURS_CCOLOR] = closest_to_tcol();
	else
		curs_xlat[CURS_CCOLOR] = ccolor;

	xlat_csetid = icb.input_screen->mc_csetid;
	xlat_ccolor = ccolor;
done:
	return(&curs_tcxl);
}
