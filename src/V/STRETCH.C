
/* stretch.c - user interface for cel/stretch function.  Eventually will
   call the same raster twisting routine in assemble rotate.c and
   a3d.c (optics) use. */

#include "jimk.h"
#include "fli.h"

extern Vcel *ccc;



static int nw, nh, cenx, ceny;
int render_first;

static
see_stretch_cel(c)
Vcel *c;
{
struct point dpoly[4];

sq_poly(nw, nh, c->x = cenx-nw/2, c->y = ceny-nh/2, dpoly);
raster_transform(c, dpoly, render_first);
zoom_it();
}

finish_stretch_rot(v)
Vector v;
{
unundo();
render_first = 2;
cel->x = ccc->x;
cel->y = ccc->y;
free_ccc();
if (need_fit_cel(cel))
	{
	if ((ccc = clone_cel(cel)) != NULL)
		{
		cfit_cel(ccc, render_form->cmap);
		(*v)(ccc);
		free_cel(ccc);
		}
	}
else
	(*v)(cel);
dirties();
}

vstretch_cel()
{
int ow,oh;
int dx,dy, odx,ody;
int quad;
int cx,cy;
int x0, y0, ocenx, oceny, w0, h0;
int cccx1, cccy1;
char buf[20];

if (cel == NULL)
	return;
if (!make_ccc())
	return;
cccx1 = cel->x;
cccy1 = cel->y;
save_undo();
render_first = 1;
save_undo();
cenx = (ccc->w>>1) + ccc->x;
ceny = (ccc->h>>1) + ccc->y;
nw = ccc->w;
nh = ccc->h;
for (;;)
	{
	see_stretch_cel(ccc);
	render_first = 0;
	cx = cenx - (nw>>1);
	cy = ceny - (nh>>1);
	if (r_in_place(cx, cy, cx+nw-1, cy+nh-1))
		{
		x0 = grid_x;
		y0 = grid_y;
		ocenx = cenx;
		oceny = ceny;
		w0 = nw;
		h0 = nh;
		quad = quad9(cx,cy,nw,nh);
		for (;;)
			{
			ow = nw;
			oh = nh;
			dx = 2*(grid_x - cenx);
			dy = 2*(grid_y - ceny);
			switch (quad)
				{
				/* proportional corners cases */
				case 0:
				case 8:
					nw = dx;
					nh = ccc->h * (long)dx / ccc->w;
					break;
				/* non proportional corners cases */
				case 2:
				case 6:
					nw = dx;
					nh = dy;
					break;
				/* above and below */
				case 1:
				case 7:
					nw = ow;
					nh = dy;
					break;
				/* to right and left */
				case 3:
				case 5:
					nw = dx;
					nh = oh;
					break;
				/* center */
				case 4:
					nw = ow;
					nh = oh;
					cenx = ocenx + grid_x - x0;
					ceny = oceny + grid_y - y0;
					break;
				}
			sprintf(buf, "%5d%% x %5d%% y", sscale_by(100,nw,ccc->w), 
				sscale_by(100, nh, ccc->h) );
			ltop_text(buf);
			see_stretch_cel(ccc);
			wait_input();
			if (PJSTDN)
				break;
			if (RJSTDN)
				{
				nw = w0;
				nh = h0;
				cenx = ocenx;
				ceny = oceny;
				break;
				}
			}
		}
	else
		{
		finish_stretch_rot(see_stretch_cel);
		break;
		}
	}
cel->x = cccx1;
cel->y = cccy1;
}

