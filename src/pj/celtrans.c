/* celtrans.c - code for rotate,stretch and moving the cel, Eventually will
   call raster twisting routine in gfx/rastrans.c */

#include "jimk.h"
#include "auto.h"
#include "celmenu.h"
#include "errcodes.h"
#include "flicel.h"
#include "floatgfx.h"
#include "grid.h"
#include "marqi.h"
#include "pentools.h"
#include "rastrans.h"

Boolean isin_fcel(Flicel *fcel,SHORT x,SHORT y)
{
	return(isin_bpoly(&fcel->xf,fcel->rc,x,y));
}

void vstretch_cel(int toolmode)
{
Errcode err;
Rcel *rc;
Short_xy stretch1;
Short_xy cent1;
Short_xy ocent;
SHORT x0, y0, w0, h0;
SHORT swidth, sheight;
SHORT neww, newh;
Short_xy rotcorn;	/* (un) rotated corner of cel */
Short_xy rotm;  	/* rotated mouse point */
int quad;
SHORT multw, multh, propmult;

	if (thecel == NULL)
		return;

	rc = thecel->rc;

	if(!toolmode)
	{
		cent1 = thecel->cd.cent;
		stretch1 = thecel->cd.stretch;
		save_undo();
		if((err = draw_flicel(thecel,DRAW_FIRST,NEW_CFIT)) < 0)
			goto error;
	}

	for (;;)
	{
		if(toolmode)
		{
			if(!tti_input())
				goto cleanup;
			cent1 = thecel->cd.cent;
			stretch1 = thecel->cd.stretch;
		}
		else if((err = quadpoly_in_place(thecel->xf.bpoly)) < 0)
		{
			if(err != Err_abort)
				goto error;
			zoom_unundo();
			draw_flicel(thecel,DRAW_RENDER,NEW_CFIT);
			dirties();
			goto cleanup;
		}

		swidth = rc->width + thecel->cd.stretch.x;
		sheight = rc->height + thecel->cd.stretch.y;

		/* get unrotated cel stretched corner */
		rotcorn.x = thecel->cd.cent.x - (swidth>>1);
		rotcorn.y = thecel->cd.cent.y - (sheight>>1);

		/* rotate mouse to cel's original orientation */
		x0 = rotm.x = icb.mx;
		y0 = rotm.y = icb.my;
		frotate_points2d(itheta_tofloat(-thecel->cd.rotang.z,FCEL_TWOPI),
						 &thecel->cd.cent,&rotm,&rotm,1);
		quad = quad9(rotm.x,rotm.y,rotcorn.x,rotcorn.y,swidth,sheight);

		ocent = thecel->cd.cent;
		w0 = swidth;
		h0 = sheight;

		/* compare signs to determine whether to negate calc or not */

		neww = (rotm.x - thecel->cd.cent.x);
		if((neww < 0 && swidth > 0) || (neww > 0 && swidth < 0))
			multw = -2;
		else
			multw = 2;

		newh = (rotm.y - thecel->cd.cent.y);
		if((newh < 0 && sheight > 0) || (newh > 0 && sheight < 0))
			multh = -2;
		else
			multh = 2;

		if((swidth < 0 && sheight > 0) || (swidth > 0 && sheight < 0))
			propmult = -rc->height;
		else
			propmult = rc->height;

		for(;;)
		{
			neww = multw*(rotm.x - thecel->cd.cent.x);
			newh = multh*(rotm.y - thecel->cd.cent.y);
			switch(quad)
			{
				case 0: /* proportional corners cases */
				case 8:
					sheight = (propmult*(long)neww)/rc->width;
				case 3: /* to right and left */
				case 5:
					swidth = neww;
					break;

				case 2: /* non proportional corners cases */
				case 6:
					swidth = neww; /* fall through and set sheight too */
				case 1: /* above */
				case 7: /* below */
					sheight = newh;
					break;

				case 4: /* inside center of cel */
					thecel->cd.cent.x = ocent.x + icb.mx - x0;
					thecel->cd.cent.y = ocent.y + icb.my - y0;
					break;
			}

			top_textf("%5d%% x %5d%% y", 
					  sscale_by(100,swidth, rc->width), 
					  sscale_by(100, sheight, rc->height) );
			thecel->cd.stretch.x = swidth - rc->width;
			thecel->cd.stretch.y = sheight - rc->height;
			if((err = draw_flicel(thecel,DRAW_DELTA,OLD_CFIT)) < 0)
				goto error;

			wait_input(MBPEN|MBRIGHT|MMOVE|KEYHIT);

			if(JSTHIT(MBPEN|MBRIGHT|KEYHIT))
			{
				if(JSTHIT(MBRIGHT|KEYHIT))
				{
					thecel->cd.stretch.x = w0 - rc->width;
					thecel->cd.stretch.y = h0 - rc->height;
					thecel->cd.cent = ocent;
				}
				else if(!JSTHIT(MMOVE)) /* if no move no need to redraw */
				{
					if(toolmode)
						goto cleanup;
					break;
				}
				if((err = draw_flicel(thecel,DRAW_DELTA,OLD_CFIT)) < 0)
					goto error;
				if(toolmode)
					goto cleanup;
				break;
			}

			/* get new rotated mouse position */
			rotm.x = icb.mx;
			rotm.y = icb.my;
			frotate_points2d(itheta_tofloat(-thecel->cd.rotang.z,FCEL_TWOPI),
							 &thecel->cd.cent,&rotm,&rotm,1);
		}
	}

error:
	if(!toolmode)
		zoom_unundo(); /* restore screen and put cel back where it was */
	thecel->cd.stretch = stretch1;
	thecel->cd.cent = cent1;
	refresh_flicel_pos(thecel);
	softerr(err,"cel_stretch");
cleanup:
	cleanup_toptext();
	return;
}

void vrotate_cel(int toolmode)

/* Stuff to implement cel/turn. */
{
Errcode err;
Short_xy cent;
Short_xy cent1;
Short_xyz theta1;
Boolean do_move;
SHORT ltheta;
SHORT otheta;
SHORT ttheta;
Short_xyz dtheta;
SHORT theta_accum;
SHORT x0,y0;
int dx,dy;

#define arctan(dx,dy) (float_toitheta(atan2(dy,dx),FCEL_TWOPI))

	if(thecel == NULL)
		return;

	if(!toolmode)
	{
		save_undo();
		if((err = draw_flicel(thecel,DRAW_FIRST,NEW_CFIT)) < 0)
			goto error;
		theta1 = thecel->cd.rotang;
		cent1 = thecel->cd.cent;
	}

	dtheta.x = dtheta.y = 0;

	for(;;)
	{
		if(toolmode)
		{
			if(!tti_input())
				goto cleanup;
			theta1 = thecel->cd.rotang;
			cent1 = thecel->cd.cent;
			do_move = 0;
		}
		else 
		{
			if((err = quadpoly_in_place(thecel->xf.bpoly)) < 0)
			{
				if(err != Err_abort)
					goto error;
				zoom_unundo();
				draw_flicel(thecel,DRAW_RENDER,NEW_CFIT);
				dirties();
				goto cleanup;
			}
			do_move = isin_fcel(thecel,icb.mx,icb.my);
		}
		otheta = thecel->cd.rotang.z;
		cent = thecel->cd.cent;
		theta_accum = 0;

		dx = icb.mx - cent.x;
		dy = icb.my - cent.y;
		if(dx || dy)
			ltheta = arctan(dx,dy);
		else
			ltheta = 0;

		x0 = icb.mx;
		y0 = icb.my;
		for(;;)
		{
			if (do_move)
			{
				thecel->cd.cent.x = cent.x + icb.mx - x0; 
				thecel->cd.cent.y = cent.y + icb.my - y0; 
			}
			else
			{
				dx = icb.mx - cent.x;
				dy = icb.my - cent.y;
				if(dx || dy)
				{
					ttheta = arctan(dx,dy);
					theta_accum += ttheta - ltheta;
					ltheta = ttheta;
				}
				else
					ttheta = ltheta;

				if((dtheta.z = constrain_angle(theta_accum)) != 0)
				{
					theta_accum -= dtheta.z;
					rotate_flicel(thecel,&dtheta);
				}
			}
			if((err = draw_flicel(thecel,DRAW_DELTA,OLD_CFIT)) < 0)
				goto error;
#ifdef NOT_ACCURATE
			top_textf("z %4d", sscale_by(thecel->cd.rotang.z,360,TWOPI));
#endif /* NOT_ACCURATE */
			wait_any_input();

			if(JSTHIT(MBRIGHT|KEYHIT|MBPEN))
			{
				if(JSTHIT(MBRIGHT|KEYHIT))
				{
					thecel->cd.rotang.z = otheta;
					set_fcel_center(thecel,cent.x,cent.y);
				}
				else if(!JSTHIT(MMOVE))
				{
					if(toolmode)
						goto cleanup;
					break;
				}
				if((err = draw_flicel(thecel,DRAW_DELTA,OLD_CFIT)) < 0)
					goto error;
				if(toolmode)
					goto cleanup;
				break;
			}
		}
	}

error:
	if(!toolmode)
		zoom_unundo();
	/* put back cel where you started */
	thecel->cd.rotang = theta1;
	thecel->cd.cent = cent1;
	refresh_flicel_pos(thecel);
	softerr(err,"cel_rotate");
cleanup:
	cleanup_toptext();
	return;
}

/*** translation of the cel ************************/

static void fcel_toptext_info(Flicel *fc, Short_xy *ocent)
{
Rcel *rc = fc->rc;

	soft_top_textf("!%4d%4d%4d%4d%4d%4d", "top_celpos",
			   fc->cd.cent.x,
			   fc->cd.cent.y,
			   fc->cd.cent.x - ocent->x,
			   fc->cd.cent.y - ocent->y,
			   sscale_by(100,rc->width + thecel->cd.stretch.x,rc->width),
			   sscale_by(100,rc->height + thecel->cd.stretch.y,rc->height));
}

/* to keep track of how far user moved the cel this time... */

typedef struct paste1dat {
	Short_xy ncent, ocent;
} Paste1dat;

static Errcode
paste1(void *paste1dat, int ix, int intween, int scale, Autoarg *aa)
/* auto vec for moving a cel in a straight line.  Ie paste multi */
{
	Paste1dat *pdat = paste1dat;
	Errcode err;
	int dx,dy;
	(void)ix;
	(void)intween;

	if(aa->cur_frame == 0) 	/* first frame set end position to current */
		pdat->ncent = thecel->cd.cent;

	dx = pdat->ncent.x - pdat->ocent.x;
	dy = pdat->ncent.y - pdat->ocent.y;
	set_fcel_center(thecel, pdat->ocent.x + itmult(dx,scale),
				    		  pdat->ocent.y + itmult(dy,scale));
	err = draw_flicel(thecel,DRAW_RENDER,NEW_CFIT);

	return(err);
}

Errcode inc_thecel(void)
/* Advance cel to next frame if vs.paste_inc_cel says so. */
{
if(vs.paste_inc_cel && thecel->flif.hdr.frame_count > 1)
	return(inc_fcel_frame(thecel));
else
	return(Success);
}

Errcode mp_thecel(int paste,int toolmode)
/* Move or paste a flicel. */
{
Errcode err;
Paste1dat pdat;
SHORT ox, oy;

	if(thecel == NULL)
		return(Err_abort);

	pdat.ocent = thecel->cd.cent;

	if(toolmode)
	{
		if(toolmode == 2)
		{
			thecel->cd.cent.x = icb.mx;
			thecel->cd.cent.y = icb.my;
			if((err = draw_flicel(thecel,DRAW_DELTA,NEW_CFIT)) < 0)
				goto error;
		}
	}
	else
	{
		save_undo();
		if((err = draw_flicel(thecel,DRAW_FIRST,NEW_CFIT)) < 0)
			goto error;

		if((err = quadpoly_in_place(thecel->xf.bpoly)) < 0)
		{
			if(err == Err_abort)
				goto do_paste;
			goto error;
		}
	}

	for (;;)
	{
		fcel_toptext_info(thecel,&pdat.ocent);

		ox = icb.mx;
		oy = icb.my;
		wait_input(MMOVE|KEYHIT|MBPEN|MBRIGHT);
		if(JSTHIT(MMOVE))
		{
			translate_flicel(thecel,icb.mx - ox,icb.my - oy);
			draw_flicel(thecel,DRAW_DELTA,OLD_CFIT);
		}
		if(JSTHIT(KEYHIT|MBPEN|MBRIGHT))
		{
			if(JSTHIT(KEYHIT|MBRIGHT)) /* user abort */
			{
				err = Err_abort;
				goto error;
			}
			break;
		}
	}

do_paste:

	if(!toolmode)
		zoom_unundo();

	if(paste)
	{
		cleanup_toptext();
		if(toolmode)
			unsee_flicel(thecel);

		if (vs.multi)
		{
			if((err = go_autodraw(paste1,&pdat,
							  (AUTO_USESCEL|AUTO_UNZOOM|AUTO_PUSHINKS))) < 0)
			{
				goto error;
			}
		}
		else
		{
			if((err = draw_flicel(thecel,DRAW_RENDER,NEW_CFIT)) < Success)
				goto error;
			if ((err = inc_thecel()) < Success)
				goto error;
			dirties();
		}
	}

	err = 0;
	goto done;
error:
	thecel->cd.cent = pdat.ocent;
	refresh_flicel_pos(thecel);

	if(toolmode)
		draw_flicel(thecel,DRAW_DELTA,OLD_CFIT);
	else
		zoom_unundo();
done:
	cleanup_toptext();
	return(err);
}

void move_the_cel(void)
/* Interactively move cel, but don't paste it when done */
{
	mp_thecel(0,0);
}

void paste_the_cel(void)
/* Interactively move and then paste cel */
{
	mp_thecel(1,0);
}



