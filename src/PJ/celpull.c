#include <math.h>
#include "jimk.h"
#include "browse.h"
#include "celmenu.h"
#include "celpul.h"
#include "commonst.h"
#include "errcodes.h"
#include "flicel.h"
#include "flx.h"
#include "floatgfx.h"
#include "marqi.h"
#include "menus.h"
#include "pentools.h"
#include "rastcurs.h"
#include "softmenu.h"

static void cm_save_cursor(void);
static void cm_clip_cel(void);
static void cm_cut_out_cel(void);
static void cm_lasso_cel(void);
static void cm_load_cel(void);
static void cm_browse_cel(void);
static void repos_cel(Fcelpos *pos);
static void z_turn_cel(int angle);
static void unturn_cel(void);
static void center_cel(void);
static void unstretch_cel(void);
static void filfli_with_cel(void);
static void filbox_with_cel(void);
static void flip_cel(Boolean vertical);
static void qmirror_cel(void);

Boolean do_celpull(Menuhdr *mh)
/* set disable flag in items that need cel to exist. */
{
SHORT cel_pulltab[] = {
	POS_REST_PUL,
	POS_MIR_PUL,
	POS_TUR_PUL,
	POS_STR_PUL,
	POS_TO__PUL,
	CEL_SVC_PUL,
	CEL_SAV_PUL,
};
	set_pultab_disable(mh, 
		cel_pulltab,Array_els(cel_pulltab),(thecel == NULL));
	pul_xflag(mh, OPT_CLE_PUL, vs.zero_clear);
	pul_xflag(mh, OPT_AUT_PUL, vs.fit_colors);
	pul_xflag(mh, OPT_REN_PUL, vs.render_under);
	pul_xflag(mh, OPT_ONE_PUL, vs.render_one_color);
	pul_xflag(mh, OPT_ADV_PUL, vs.paste_inc_cel);
	return(menu_dopull(mh));
}

static void cm_stretch(void)
{
switch(soft_qchoice(NULL,"cel_stretch"))
	{
	default:
		break;
	case 0:
		filfli_with_cel();
		break;
	case 1:
		filbox_with_cel();
		break;
	case 2:
		unstretch_cel();
		break;
	}
}

static void cm_turn(void)
{
switch (soft_qchoice(NULL,"cel_turn"))
	{
	default:
		break;
	case 0:
		z_turn_cel(FCEL_TWOPI/4);
		break;
	case 1:
		z_turn_cel(-FCEL_TWOPI/4);
		break;
	case 2:
		unturn_cel();
		break;
	}
}

static void cm_mirror(void)
{
switch (soft_qchoice(NULL,"cel_mirror"))
	{
	default:
		break;
	case 0:
		flip_cel(1);
		break;
	case 1:
		flip_cel(0);
		break;
	case 2:
		qmirror_cel();
		break;
#ifdef SLUFFED
	case 3:
		un_mirror_cel();
		break;
#endif /* SLUFFED */
	}
}
static void cm_restore(void)
{
Fcelpos pos;

	get_fcelpos(thecel,&pos);
	pos.stretch.x = pos.stretch.y = 0;
	pos.rotang.x = pos.rotang.y = pos.rotang.z = 0;
	repos_cel(&pos);
}

void cm_selit(Menuhdr *mh, SHORT hitid)
{
	switch(hitid)
	{
		case CEL_CLI_PUL:
			cm_clip_cel();
			break;
		case CEL_GET_PUL:
			cm_cut_out_cel();
			break;
		case CEL_SNI_PUL:
			cm_lasso_cel();
			break;
		case CEL_BRW_PUL:
			cm_browse_cel();
			break;
		case CEL_LOA_PUL:
			cm_load_cel();
			break;
		case CEL_SAV_PUL:
			qsave_the_cel();
			break;
		case CEL_SVC_PUL:
			cm_save_cursor();
			break;
		case CEL_QUI_PUL:
			mh_gclose_code(mh, Err_abort);
			break;
		case POS_TO__PUL:
			center_cel();
			break;
		case POS_STR_PUL:
			cm_stretch();
			break;
		case POS_TUR_PUL:
			cm_turn();
			break;
		case POS_MIR_PUL:
			cm_mirror();
			break;
		case POS_REST_PUL:
			cm_restore();
			break;
		case OPT_CLE_PUL:
		case OPT_AUT_PUL:
		case OPT_REN_PUL:
		case OPT_ONE_PUL:
			toggle_cel_opt(hitid-OPT_CLE_PUL);
			break;
		case OPT_ADV_PUL:
			vs.paste_inc_cel = !vs.paste_inc_cel;
			break;
	}
}
static void cm_save_cursor(void)
{
SHORT odcoor;
char *title;
Short_xy hot;
char buf[50];

	odcoor = vs.dcoor;

	hide_mp();
	if(thecel->cd.stretch.x
		|| thecel->cd.stretch.y
		|| thecel->cd.rotang.x
		|| thecel->cd.rotang.y
		|| thecel->cd.rotang.z)
	{
		soft_continu_box("rot_cursor");
		goto done;
	}

	vs.dcoor = TRUE;
	soft_top_textf("top_curs");
	vs.dcoor = FALSE;

	if(marqi_cut_xy() < 0)
		goto done;

	hot.x = icb.mx - thecel->rc->x;
	hot.y = icb.my - thecel->rc->y;
	cleanup_toptext();

	if((title = pj_get_filename(stack_string("save_cursor",buf),
								".CUR",
								save_str,"",NULL,1,NULL,NULL))==NULL)
	{
		goto done;
	}

	if(!overwrite_old(title))
		goto done;

	save_cursor(title,thecel->rc,&hot);

done:
	cleanup_toptext();
	vs.dcoor = odcoor;
	show_mp();
}
/********* functions that get new cels ***********/

static void cm_getcel_bracket(Errcode (*getcel)(void))
{
Errcode err;

	hide_mp();
	flx_clear_olays(); /* clear cel if present */
	set_curptool(NULL); /* free buffers, etc */
	cel_cancel_undo();
	(*getcel)(); /* get new cel */

	/* reload tool set do not draw cel */

	if((err = reset_celmenu(vs.cur_cel_tool,FALSE)) < Success)
		mh_gclose_code(&cmcb->tpull, err); /* AAAck reset failed, close it */

	flx_draw_olays(); /* draw new cel using overlay stack pop */
	show_mp();
}

static void cm_clip_cel(void)
{
	cm_getcel_bracket(clip_cel);
}
static void cm_cut_out_cel(void)
{
	cm_getcel_bracket(cut_out_cel);
}
static void cm_lasso_cel(void)
{
	cm_getcel_bracket(lasso_cel);
}
static void cm_load_cel(void)
{
	cm_getcel_bracket(go_load_the_cel);
}
static void cm_browse_cel(void)
{
	cm_getcel_bracket(go_browse_cels);
}
/************************************************/
static void cmu_undo_celpos(void)
{
Fcelpos opos;

	if(thecel)
	{
		opos = cmcb->lastpos;
		get_fcelpos(thecel,&cmcb->lastpos);
		repos_cel(&opos);
	}
	else
		vl.undoit = NULL;
}
void save_celpos_undo(void)
{
	if(thecel)
	{
		get_fcelpos(thecel,&cmcb->lastpos);
		vl.undoit = cmu_undo_celpos;
	}
	else
		vl.undoit = NULL;
}
void cel_cancel_undo(void)
{
	vl.undoit = NULL;
}
static void repos_cel(Fcelpos *pos)
{
	save_celpos_undo();
	cm_erase_toolcel();
	put_fcelpos(thecel,pos);
	cm_restore_toolcel();
}
static void z_turn_cel(int angle)
{
Fcelpos pos;

	get_fcelpos(thecel,&pos);
	pos.rotang.z += angle;
	repos_cel(&pos);
}
static void unturn_cel(void)
{
Fcelpos pos;

	get_fcelpos(thecel,&pos);
	pos.rotang.x = pos.rotang.y = pos.rotang.z = 0;
	repos_cel(&pos);
}
static void center_cel(void)
{
Fcelpos pos;

	get_fcelpos(thecel,&pos);
	pos.cent.x = vb.pencel->width >> 1;
	pos.cent.y = vb.pencel->height >> 1;
	repos_cel(&pos);
}
static void mirror_stretch(Rcel *rc, Boolean along_y_axis, Fcelpos *pos)
{
	if(along_y_axis)
		pos->stretch.y = -(pos->stretch.y + 2*rc->height);
	else
		pos->stretch.x = -(pos->stretch.x + 2*rc->width);
}
static void unstretch_cel(void)
{
Rcel *rc;
Boolean xflip, yflip;
Fcelpos pos;

	rc = thecel->rc;
	xflip = (rc->width + thecel->cd.stretch.x) < 0;
	yflip = (rc->height + thecel->cd.stretch.y) < 0;
	get_fcelpos(thecel,&pos);
	pos.stretch.x = pos.stretch.y = 0;
	if(xflip)
	{
		if(!yflip)
			mirror_stretch(rc,0,&pos);
		else
			pos.rotang.z += FCEL_TWOPI/2;
	}
	else if(yflip) /* && !xflip */
	{
		mirror_stretch(rc,1,&pos);
	}
	repos_cel(&pos);
}
#ifdef SLUFFED
static void un_mirror_cel(void)
{
Rcel *rc;
Boolean xflip, yflip;
Fcelpos pos;

	get_fcelpos(thecel,&pos);
	rc = thecel->rc;
	xflip = (rc->width + pos.stretch.x) < 0;
	yflip = (rc->height + pos.stretch.y) < 0;

	if(xflip)
		mirror_stretch(rc,0,&pos);
	if(yflip)
	{
		mirror_stretch(rc,1,&pos);
		if(xflip)
			pos.rotang.z += FCEL_TWOPI/2;
	}
	repos_cel(&pos);
}
#endif /* SLUFFED */
static void mirrored_cel_to_box(Flicel *fcel, Fcelpos *newpos, Rectangle *rect)

/* will preserve mirroring of cel in newpos */
{
Boolean xflip, yflip;
Rcel *rc;

	rc = fcel->rc;
	xflip = (rc->width + fcel->cd.stretch.x) < 0;
	yflip = (rc->height + fcel->cd.stretch.y) < 0;
	fcelpos_to_box(fcel,newpos,rect);
	if(xflip)
	{
		if(!yflip)
			mirror_stretch(rc,0,newpos);
	}
	else if(yflip) /* && !xflip */
	{
		mirror_stretch(rc,1,newpos);
	}
}
static void filfli_with_cel(void)
{
Fcelpos pos;
Rectangle box;


	copy_rectfields(vb.pencel,&box);
	box.x = box.y = 0;
	mirrored_cel_to_box(thecel,&pos,&box);
	repos_cel(&pos);
}
static void filbox_with_cel(void)
{
Fcelpos pos;
Rectangle box;

	hide_mp();
	cm_erase_toolcel();
	if(cut_out_rect(&box) >= 0)
	{
		mirrored_cel_to_box(thecel,&pos,&box);
		repos_cel(&pos);
	}
	cm_restore_toolcel();
	show_mp();
}
static void make_mirror_celpos(Flicel *cel, Fcelpos *mirpos, Short_xy *axis)
{
Short_xy orig;  /* line origin */
double dx, dy;  /* line dx and dy */
double cx, cy; /* cel cent x cel cent y */
double ltheta; /* line theta */
double ctheta; /* cel center theta */
double theta;  /* difference */
double celzang; /* current rotation angle of cel */
double sint;   /* sin theta */
double cost;   /* cos theta */

	orig = axis[0];
	dx = axis[1].x - orig.x;
	dy = axis[1].y - orig.y;

	if(dx == 0 && dy == 0) /* gotta have an axis */
		return;

	get_fcelpos(cel,mirpos);

	/* translate old center to coors of line origin and make doubles */

	cx = mirpos->cent.x - orig.x;
	cy = mirpos->cent.y - orig.y;

	ltheta = atan2(dy,dx);

	if(cx != 0 || cy != 0) /* if center is on origin leave it where it is */
	{
		/* get angle between center and line relative to line end point */

		ctheta = atan2(cy,cx);

		theta = clipto_pi(-2.0 * (ctheta - ltheta));

		/* rotate center about line's end to get new center on other side 
		 * by -2x this angle */

		if((cost = cos(theta)) == -1.0)
			sint = 0.0;
		else
			sint = sin(theta);

		mirpos->cent.x = roundtoint(cx*cost - cy*sint) + orig.x;
		mirpos->cent.y = roundtoint(cy*cost + cx*sint) + orig.y;
	}

	/* find new rotation for cel. Get double of cel angle */

	celzang = itheta_tofloat(mirpos->rotang.z,FCEL_TWOPI);

	theta = clipto_pi(2.0*(celzang - ltheta));

	mirpos->rotang.z = float_toitheta(celzang - theta,FCEL_TWOPI);

	/* flip in y */
	mirror_stretch(cel->rc,TRUE,mirpos);
	return;
}
static void flip_cel(Boolean vertical)
{
Fcelpos pos;

	get_fcelpos(thecel,&pos);
	mirror_stretch(thecel->rc,vertical,&pos);
	repos_cel(&pos);
}
static void qmirror_cel(void)
{
Short_xy axis[2];
Fcelpos pos;

	hide_mp();
	wait_wndo_input(ANY_CLICK);
	if(!tti_input())
		goto done;
	if((get_rub_axis(axis,swhite,sblack)) < 0)
		goto done;
	make_mirror_celpos(thecel, &pos, axis);
	repos_cel(&pos);

done:
	show_mp();
}
