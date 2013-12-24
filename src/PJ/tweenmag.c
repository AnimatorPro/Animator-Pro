/* tweenmag.c - implements the magnet tool in the tween system.  The
 * magnet is a way of moving lots of points in a polygon, with the 
 * amount of motion decreasing with the distance of the points from
 * the cursor.
 */

#include "jimk.h"
#include "errcodes.h"
#include "poly.h"
#include "tweenmag.h"

static Errcode init_mag_table(short **ptable,
	Poly *source, int ix, int iy, long pthresh, int mag_mode )
/* Make up a table scaling factors for each point of a polygon.
   Scale falls off as distance squared from ix/iy.  
   Pthresh parameter is the distance-squared such that points
   at this distance (or closer) are SCALE_ONE. */
{
long ldx,ldy;
long ldistance;
int pcount;
LLpoint *sp;
short *table;

pcount = source->pt_count;
if ((table = pj_malloc(pcount*sizeof(*table))) == NULL)
	return(Err_no_memory);
*ptable = table;
switch (mag_mode)
	{
	case MAG_BLOW:
		pthresh = sqr_root(pthresh);
		break;
	}

sp = source->clipped_list;
while (--pcount >= 0)
	{
	ldx = ix-sp->x;
	ldy = iy-sp->y;
	ldistance = ldx*ldx+ldy*ldy;
	switch (mag_mode)
		{
		case MAG_BLOW:
			ldistance = sqr_root(ldistance);
			break;
		}
	if (ldistance <= pthresh)
		{
		*table = SCALE_ONE;
		}
	else
		{
		*table = SCALE_ONE*pthresh/ldistance;
		}
	sp = sp->next;
	table += 1;
	}
return(Success);
}

static void mag_transform(short *mag_table, Poly *source, 
	Poly *dest, int dx, int dy)
/* Apply the offset dx/dy to poly with the strength of offset
   of each point in poly modulated by mag_table */
{
LLpoint *sp, *dp;
int pcount;
short tval;

pcount = dest->pt_count;
sp = source->clipped_list;
dp = dest->clipped_list;
while (--pcount >= 0)
	{
	tval = *mag_table++;
	dp->x = sp->x + itmult(tval, dx);
	dp->y = sp->y + itmult(tval, dy);
	sp = sp->next;
	dp = dp->next;
	}
}

Errcode mag_loop(Poly *poly, Pixel color, long min_dist, int mag_mode)
{
int ix, iy;
Marqihdr mh;
Poly opoly;
Errcode err;
short *mag_table;

ix = icb.mx;
iy = icb.my;
cinit_marqihdr(&mh,color,color,TRUE);
clear_struct(&opoly);
update_poly(poly, &opoly);
if ((err = init_mag_table(&mag_table, &opoly, ix, iy, min_dist, mag_mode)) 
	< Success)
	return(softerr(err,NULL));
for (;;)
	{
	marqi_poly(&mh, poly, vs.closed_curve);
	wait_input(MBPEN|MBRIGHT|MMOVE);
	undo_poly(&mh,poly, vs.closed_curve);
	mag_transform(mag_table,&opoly, poly, icb.mx-ix, icb.my-iy);
	if (JSTHIT(MBRIGHT))
		{
		update_poly(&opoly, poly);
		err = Err_abort;
		break;
		}
	if (JSTHIT(MBPEN))
		{
		err = Success;
		break;
		}
	}
pj_free(mag_table);
free_polypoints(&opoly);
return(err);
}

