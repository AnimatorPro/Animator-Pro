/* marqi.c - routines to make creepy dotted lines around selected areas. */
/* Also some "Dotout" family functions that take x/y parameters and act
   on vb.pencel.  Dotout's are used by line drawers, circle drawers etc. */

#include "errcodes.h"
#include "jimk.h"
#include "gfx.h"
#include "marqi.h"
#include "menus.h"


/* strings from muparts.c */

extern char *rub_line_str;
extern char *box_coor_str;
extern char *rub_rect_str;
extern char *rub_circle_str;

/***** new ones ******/

#define DOZOOM 0x0001
#define DOFLI  0x0002
#define DOBOTH 0x0003

static int get_dowins(int bothwins)
{
	if((vl.zoomwndo)
		&& !(vl.zoomwndo->flags & WNDO_HIDDEN))
	{

		if(!bothwins)
			return(DOZOOM);

		return(DOBOTH);

#ifdef ZOOMCURSOR /* didn't erase from pencel */

		if(CODEOBSCURES 
			== clipcode_crects((Cliprect *)&(vl.zoomwndo->CRECTSTART),
							   (Cliprect *)&(vb.pencel->CRECTSTART)))
		{
			return(DOZOOM);
		}
		else
			return(DOBOTH);
#endif

	}
	return(DOFLI);
}

void cinit_marqihdr(Marqihdr *mh,Pixel oncolor,Pixel offcolor,Boolean bothwins)
{
	init_marqihdr(mh,vb.pencel,NULL,oncolor,offcolor);
	switch(get_dowins(bothwins))
	{
		case DOZOOM:
			mh->putdot = zoom_put_dot;
			break;
		case DOBOTH:
			mh->putdot = both_put_dot;
			break;
		default: /* set to put_dot by init_marqihdr */
			break;
	}
}

void vinit_marqihdr(Marqihdr *mh,int marqi_it,Boolean bothwins)
{
Pixel oncolor,offcolor;

	if(marqi_it)
	{
		oncolor = vb.screen->SWHITE;
		offcolor = vb.screen->SBLACK;
	}
	else
		oncolor = offcolor = vs.ccolor;
	cinit_marqihdr(mh,oncolor,offcolor,bothwins);
}


/*************************************************************/
/* additional dot routines for marqis */

void undo_marqidot(SHORT x,SHORT y, Marqihdr *mh)
/* "dotout" routine to erase from undo screen */
{
	if( ((USHORT)x) < vb.pencel->width
		&& ((USHORT)y) < vb.pencel->height)
	{
		(*(mh->putdot))(vb.pencel,pj_get_dot(undof,x,y),x,y);
	}
}
void savedraw_marqidot(SHORT x,SHORT y, Marqihdr *mh)
{
	if( ((USHORT)x) < vb.pencel->width
		&& ((USHORT)y) < vb.pencel->height)
	{
		*mh->dotbuf++ = pj__get_dot(vb.pencel,x,y);
	}
	(*mh->pdot)(x,y,mh); /* marqi or solid */
}
void restore_marqidot(SHORT x, SHORT y, Marqihdr *mh)
{
	if( ((USHORT)x) < vb.pencel->width
		&& ((USHORT)y) < vb.pencel->height)
	{
		(*mh->putdot)(vb.pencel,*mh->dotbuf++,x,y);
	}
}

/**************** vector and polygon marqi routines ***************/

typedef struct vectordat {
	UBYTE *save;
	UBYTE **saves;
	Short_xy *v;
	SHORT count;
	SHORT saved;
	VFUNC dispit;
} Vecdat;

static void marqi_polyvect(Marqihdr *mh, Short_xy *ends,int count)
{
	while(count-- >= 0)
		cvect(ends++,mh->pdot,mh);
}
static void savedraw_polyvect(Marqihdr *mh, Short_xy *ends,int count)
{
Vecdat *vd = mh->adata;
int i;

	mh->dotbuf = vd->save; 
	for(i = 0;i<count;++i)
	{
		vd->saves[i] = mh->dotbuf;
		cvect(ends++, savedraw_marqidot, mh);
	}
	vd->saved = 1;
}
static void restore_polyvect(Marqihdr *mh, Short_xy *ends,int count)

/* undraws in reverse order for proper restore */
{
Vecdat *vd = mh->adata;

	if(!vd->saved)
		return;

	ends += count;
	while(--count >= 0)
	{
		mh->dotbuf = vd->saves[count]; 
		cvect(--ends, restore_marqidot, mh);
	}
}


static int anim_rubpoly(Marqihdr *mh)

/* increment mod delta move, clip, and Draw array of polypoints */
{
Vecdat *vd = mh->adata;

	if(JSTHIT(MMOVE) || !(vd->saved))
	{
		restore_polyvect(mh,vd->v,vd->count);
		vd->v[1].x = icb.mx;
		vd->v[1].y = icb.my;
		if(vd->dispit)
			(*(vd->dispit))(vd->v);
		savedraw_polyvect(mh,vd->v,vd->count);
	}
	else
		marqi_polyvect(mh,vd->v,vd->count);

	mh->dmod = mh->smod++;
	return(0);
}


Errcode rubba_vertex(Short_xy *p0, 
					    	Short_xy *p1,  /* the moved vertex */
					    	Short_xy *p2,
							VFUNC dispfunc, /* function to display info */
							Pixel color)

/* display a marqi line until next click.  Display many
 * meaningful number on top lines. load result p1
 * does not alter p1 if aborted */
{
Marqihdr mh;
Vecdat vd;
Short_xy pts[3]; 
UBYTE *saves[3];

	cinit_marqihdr(&mh,color,color,TRUE);
	if(p0->x == p2->x && p0->y == p2->y)  /* if same only one */
		vd.count = 1;
	else
		vd.count = 2;

	if((vd.save = pj_malloc((vb.pencel->width + vb.pencel->height)*vd.count)) 
				== NULL)
	{
		return(Err_no_memory);
	}
	mh.adata = &vd;
	pts[0] = *p0;
	pts[2] = *p2;
	vd.v = &pts[0];
	vd.saved = 0;
	vd.saves = saves;
	vd.dispit = dispfunc;

	anim_wait_input(KEYHIT|MBRIGHT|MBPEN,MMOVE,mh.waitcount,
					anim_rubpoly,&mh);

	restore_polyvect(&mh,vd.v,vd.count);
	pj_free(vd.save);

	if(JSTHIT(MBPEN))
	{
		*p1 = pts[1];
		return(0);
	}
	else
		return(Err_abort);
}
Errcode get_rub_vertex(Short_xy *p0, 
					    Short_xy *p1,  /* the moved vertex */
					    Short_xy *p2, Pixel color)
{
	return(rubba_vertex(p0,p1,p2,NULL,color));
}

void disp_line_alot(Short_xy *v)
{
	top_textf("!%3d%3d%3d%3d%3d%3d%3ld%3d", rub_line_str,
			v[0].x, v[0].y, 
			intabs(v[0].x-v[1].x)+1, intabs(v[0].y-v[1].y)+1, 
			v[1].x, v[1].y, 
			(360L*arcnorm(arctan(v[1].x-v[0].x, v[1].y-v[0].y))+TWOPI/2)/TWOPI, 
			calc_distance(v[0].x, v[0].y, v[1].x, v[1].y));
}
Errcode get_rub_line(Short_xy *xys)
{
Short_xy p0;
Errcode ret;

	p0.x = icb.mx;
	p0.y = icb.my;
	if((ret = rubba_vertex(&p0,&xys[1],&p0,disp_line_alot,vs.ccolor)) >= 0)
		xys[0] = p0;
	cleanup_toptext();
	return(ret);
}


typedef struct rlinedat {
	Marqihdr mh;
	UBYTE *save;
	Short_xy v[2];
	SHORT saved;
} Rlinedat;

static void marqi_maxline(Marqihdr *mh, Short_xy *ends)
{
	max_line(mh->w, ends, mh->pdot, mh); 
}
static void savedraw_maxline(Rlinedat *rd)
{
	rd->saved = 1;
	rd->mh.dotbuf = rd->save;
	max_line(rd->mh.w, rd->v, savedraw_marqidot, &rd->mh); 
}
static void restore_maxline(Rlinedat *rd)
/* undraws in reverse order for proper restore */
{
	if(!rd->saved)
		return;

	rd->mh.dotbuf = rd->save;
	max_line(rd->mh.w, rd->v, restore_marqidot, &rd->mh); 
}

static int anim_axis(Rlinedat *rd)

/* increment mod delta move, clip, and Draw array of polypoints */
{
	if(JSTHIT(MMOVE) || !(rd->saved))
	{
		restore_maxline(rd);
		rd->v[1].x = icb.mx;
		rd->v[1].y = icb.my;

		/* if same point make a vertical line */
		if(rd->v[0].x == rd->v[1].x && rd->v[0].y == rd->v[1].y)
			++rd->v[1].y;
		savedraw_maxline(rd);
		disp_line_alot(rd->v);
	}
	else
		marqi_maxline(&rd->mh,rd->v);

	rd->mh.dmod = rd->mh.smod++;
	return(0);
}
Errcode get_rub_axis(Short_xy *ends,Pixel oncol,Pixel offcol)
{
Rlinedat rd;

	clear_struct(&rd);
	cinit_marqihdr(&rd.mh,oncol,offcol,TRUE);
	if((rd.save = pj_malloc(vb.pencel->width + vb.pencel->height)) == NULL)
		return(Err_no_memory);

	rd.v[0].x = icb.mx;
	rd.v[0].y = icb.my;

	anim_wait_input(KEYHIT|MBRIGHT|MBPEN,MMOVE,rd.mh.waitcount,
					anim_axis,&rd);

	restore_maxline(&rd);
	pj_free(rd.save);
	cleanup_toptext();

	if(JSTHIT(MBPEN))
	{
		ends[0] = rd.v[0];
		ends[1] = rd.v[1];
		return(Success);
	}
	else
		return(Err_abort);
}

/**************** rectangle marqi routines ***************/


typedef struct rectdata {
	UBYTE *save;    /* save buffer */
	SHORT saved;
	SHORT extra;
	Fullrect fr;    /* the converted rectangle */
	Fullrect clip; 	/* clip boundary for rectangle */
	SHORT x0,y0;
	SHORT startx,starty; /* for box coors in move rect */
	USHORT dowins;
} Rectdata;


static void mhmove_frect(Marqihdr *mh, Fullrect *fr, 
				    void (*hmove)(void *r,void *pb,Coor x,Coor y, Ucoor len),
				    void (*vmove)(void *r,void *pb,Coor x,Coor y, Ucoor len))

/* moves a frame defined by fr into or out of save buffer */
{
UBYTE *sbuf;
Rectdata *rd = mh->adata;

	sbuf = rd->save;
	(*hmove)(mh->w,sbuf,fr->x,fr->y,fr->width);
	sbuf += fr->width;
	(*hmove)(mh->w,sbuf,fr->x,fr->MaxY-1,fr->width);
	sbuf += fr->width;
	(*vmove)(mh->w,sbuf,fr->x,fr->y,fr->height);
	sbuf += fr->height;
	(*vmove)(mh->w,sbuf,fr->MaxX-1,fr->y,fr->height);
}
static void save_frect(Marqihdr *mh, Fullrect *fr)
{
	mhmove_frect(mh,fr,pj_get_hseg,pj_get_vseg);
	((Rectdata *)(mh->adata))->saved = 1;
}
static void rest_zoom_frect(Marqihdr *mh)
{
Rectdata *rd = mh->adata;
	if(rd->saved)
		mhmove_frect(mh,&(rd->fr),zoom_put_hseg,zoom_put_vseg);
}
static void rest_fli_frect(Marqihdr *mh)
{
Rectdata *rd = mh->adata;
	if(rd->saved)
		mhmove_frect(mh,&(rd->fr),pj_put_hseg,pj_put_vseg);
}
static void rest_eitheror_frect(Marqihdr *mh)
{
Rectdata *rd = mh->adata;

	if(rd->dowins & DOZOOM)
		rest_zoom_frect(mh);
	if(rd->dowins & DOFLI)
		rest_fli_frect(mh);
}
#ifdef SLUFFED
void marqi_frame(Marqihdr *mh,SHORT x0,SHORT y0,SHORT x1,SHORT y1)
{
	cline_frame(x0,y0,x1,y1,mh->pdot,mh);
}
#endif /* SLUFFED */

#define mhdraw_frect(mh,fr) (marqi_crect(mh,(Cliprect *)&(fr->CRECTSTART)))
void marqi_crect(Marqihdr *mh, Cliprect *cr)
{
	cline_frame(cr->x,cr->y,cr->MaxX-1,cr->MaxY-1,mh->pdot,mh);
}
void marqi_rect(Marqihdr *mh,Rectangle *r)
{
Cliprect cr;
	rect_tocrect(r,&cr);
	marqi_crect(mh,&cr);
}

static draw_zoom_frect(Marqihdr *mh, Fullrect *fr)
{
	mh->putdot = zoom_put_dot;
	mhdraw_frect(mh,fr);
}
static draw_fli_frect(Marqihdr *mh, Fullrect *fr)
{
	mh->putdot = pj_put_dot;
	mhdraw_frect(mh,fr);
}
static void draw_eitheror_frect(Marqihdr *mh,Fullrect *fr)
{
Rectdata *rd = mh->adata;
	switch(rd->dowins)
	{
		case DOZOOM:
			mh->putdot = zoom_put_dot;
			break;
		case DOBOTH:
			mh->putdot = both_put_dot;
			break;
		default: 
			mh->putdot = pj_put_dot;
	}
	mhdraw_frect(mh,fr);
}


static int anim_rinp(Marqihdr *mh)
/* Draw a hollow rectangle through mh->pdot routine */
{
Rectdata *rd = mh->adata;
	mh->dmod = ++mh->smod;
	draw_eitheror_frect(mh,&(rd->fr));
	return(0);
}

Errcode rect_in_place(Rectangle *rect)
/* marqui box until pendown */
{
Marqihdr mh;
Rectdata rd;

	vinit_marqihdr(&mh,1,1);
	rect_tofrect(rect,&rd.fr);
	if(NULL == (rd.save = pj_malloc((rect->width + rect->height)*2 )))
		return(Err_no_memory);

	mh.adata = &rd;
	rd.dowins = get_dowins(1);

	save_frect(&mh,&rd.fr); /* save frame */

	anim_wndo_input(KEYHIT|MBRIGHT|MBPEN,0,4,anim_rinp,&mh);

	rest_eitheror_frect(&mh); /* restore frame */
	pj_free(rd.save); /* free backup buffer */

	if(JSTHIT(MBPEN))
		return(0);
	else
		return(Err_abort);
}

void box_coors(SHORT x,SHORT y,SHORT ox,SHORT oy)
/* display coordinates and how much a box moved*/
{
	top_textf("!%3d%3d%4d%4d", box_coor_str, x,y,x-ox,y-oy);
}

Errcode rub_rect_in_place(Rectangle *rect)
/* marqui rectangle until pendown displaying rectangle coordinates */
{
Errcode err;
	box_coors(rect->x,rect->y,rect->x,rect->y);
	err = rect_in_place(rect);
	cleanup_toptext();
	return(err);
}

#ifdef SLUFFED
Errcode r_in_place(SHORT x0,SHORT y0,SHORT x1,SHORT y1)
/* marqui box until pendown */
{
Rectangle r;

	frame_torect(x0, y0, x1, y1, &r);
	return(rect_in_place(&r));
}
#endif /* SLUFFED */



static int anim_qpoly(Marqihdr *mh)
/* Draw a hollow rectangle through mh->pdot routine */
{
	mh->dmod = ++mh->smod;
	marqi_vector(mh, mh->adata, 4, sizeof(Short_xy));
	return(0);
}
Errcode quadpoly_in_place(Short_xy *qpoly)
/* marqui 4 sided poly until click this does not save or restore the 
 * the background */
{
Marqihdr mh;

	vinit_marqihdr(&mh,1,1);
	mh.adata = qpoly;

	anim_wndo_input(KEYHIT|MBRIGHT|MBPEN,0,4,anim_qpoly,&mh);

	if(JSTHIT(MBPEN))
		return(0);
	else
		return(Err_abort);
}

static void do_deltarect(Marqihdr *mh, Fullrect *newrect)

/* subroutine for rectangle marqi's animators */
{
Rectdata *rd = mh->adata;

	if(rd->dowins & DOZOOM) /* restore and draw zoom frame */
	{
		rest_zoom_frect(mh);
		draw_zoom_frect(mh,newrect);
	}
	if(rd->dowins & DOFLI) /* restore save and redraw fli */
	{
		rest_fli_frect(mh);
		save_frect(mh,newrect);
		draw_fli_frect(mh,newrect);
	}
	else /* save fli only */
		save_frect(mh,newrect);

	rd->fr = *newrect;
}

static int anim_moverect(Marqihdr *mh)

/* increment mod delta move, clip, and Draw frame. */
{
Rectdata *rd = mh->adata;
SHORT dx, dy;
Fullrect newrect;

	if(JSTHIT(MMOVE) || !(rd->saved))
	{
		copy_rectfields(&(rd->fr),&newrect);
		newrect.x += icb.mx - rd->x0;
		newrect.y += icb.my - rd->y0;

		if(rd->clip.width)
			bclip_rect((Rectangle *)&(newrect.RECTSTART),
						(Rectangle *)&(rd->clip.RECTSTART));

		dx = newrect.x - rd->fr.x;
		dy = newrect.y - rd->fr.y;

		if( dx || dy || !(rd->saved)) /* will force first time */
		{
			rect_tofrect((Rectangle *)&(newrect.RECTSTART),&newrect);
			box_coors(newrect.x, newrect.y, rd->startx, rd->starty);
			do_deltarect(mh,&newrect);
			rd->x0 += dx;
			rd->y0 += dy;
		}
	}
	else
		draw_eitheror_frect(mh,&(rd->fr));

	mh->dmod = mh->smod++;
	return(0);
}
static void get_penwndoclip(Fullrect *fr)
{
	fr->x = fr->y = 0;
	fr->width = fr->MaxX = vb.pencel->width;
	fr->height = fr->MaxY = vb.pencel->height;
}
static Errcode move_frect(Fullrect *frect, int marqi_it, int clipit)

/* starting at current coordinates.  marqis a moving fix sized  
 * box intil next pendown... 
 * saving and restoring from "save"... does not alter data in frect if 
 * aborted with right click or keyhit input window must be set to 
 * penwndo or zoom window */
{
Marqihdr mh;
Rectdata rd;

	vinit_marqihdr(&mh,marqi_it,1);

	mh.adata = &rd;
	rd.dowins = get_dowins(1);
	rd.saved = 0;
	rd.x0 = icb.mx;
	rd.y0 = icb.my;
	rd.fr = *frect;
	rd.startx = rd.fr.x; /* original position used in box_coors() */
	rd.starty = rd.fr.y;

	if(clipit)
		get_penwndoclip(&rd.clip);
	else
		rd.clip.width = 0;

	if(NULL == (rd.save = pj_malloc((frect->width + frect->height)*2 )))
		return(Err_no_memory);

	anim_wait_input(KEYHIT|MBRIGHT|MBPEN,MMOVE,mh.waitcount,
					anim_moverect,&mh);

	cleanup_toptext();
	rest_eitheror_frect(&mh);
	pj_free(rd.save);

	if(JSTHIT(MBPEN))
	{
		*frect = rd.fr;
		return(0);
	}
	else
		return(Err_abort);
}
Errcode clip_move_rect(Rectangle *rect)
{
Fullrect fr;
Errcode err;

	rect_tofrect(rect,&fr);
	if((err = move_frect(&fr,1,1)) >= 0)
		copy_rectfields(&fr,rect);
	return(err);
}

static int anim_rubrect(Marqihdr *mh)

/* increment mod delta size, clip, and Draw frame. */
{
Rectdata *rd = mh->adata;
Fullrect newrect;
SHORT clipx, clipy;

	if(JSTHIT(MMOVE) || !(rd->saved))
	{
		frame_tocrect(rd->x0,rd->y0,icb.mx,icb.my,
					  (Cliprect *)&(newrect.CRECTSTART));

		and_cliprects((Cliprect *)&(rd->clip.CRECTSTART),
					   (Cliprect *)&(newrect.CRECTSTART),
					   (Cliprect *)&(newrect.CRECTSTART));

		if( !crects_same((Cliprect *)&(newrect.CRECTSTART),
						 (Cliprect *)&(rd->fr.CRECTSTART))
			|| !(rd->saved)) /* will force first time */
		{
			if(icb.mx == newrect.x)
				clipx = newrect.x;
			else
				clipx = newrect.MaxX - 1; 

			if(icb.my == newrect.y)
				clipy = newrect.y;
			else
				clipy = newrect.MaxY - 1; 

			crect_tofrect((Cliprect *)&(newrect.CRECTSTART),&newrect);
			top_textf("!%3d%3d%3d%3d%3d%3d", rub_rect_str, rd->x0, rd->y0,
						newrect.width+rd->extra, newrect.height+rd->extra,
						clipx,clipy);

			do_deltarect(mh,&newrect);
		}
	}
	else
		draw_eitheror_frect(mh,&(rd->fr));

	mh->dmod = mh->smod++;
	return(0);
}

static Errcode rub_frect(Fullrect *frect, int marqi_it, SHORT extra)

/* rub box.  starting at current coordinates.  marqis a variable 
 * box intil next pendown... 
 * saving and restoring from "save"... does not alter data in clip if 
 * aborted with right click or keyhit input window must be set to 
 * penwndo or zoom window */
{
Marqihdr mh;
Rectdata rd;

	vinit_marqihdr(&mh,marqi_it,1);

	mh.adata = &rd;
	rd.dowins = get_dowins(1);
	rd.saved = 0;
	rd.x0 = icb.mx;
	rd.y0 = icb.my;
	rd.extra = extra - 1;

	get_penwndoclip(&rd.clip);

	if(NULL == (rd.save = pj_malloc((rd.clip.width + rd.clip.height)*2 )))
		return(Err_no_memory);

	anim_wait_input(KEYHIT|MBRIGHT|MBPEN,MMOVE,mh.waitcount,
					anim_rubrect,&mh);

	rest_eitheror_frect(&mh);
	pj_free(rd.save);

	cleanup_toptext();
	if(JSTHIT(MBPEN))
	{
		rd.fr.width += rd.extra;
		rd.fr.height += rd.extra;
		rect_tofrect((Rectangle *)&(rd.fr.RECTSTART),frect);
		return(0);
	}
	else
		return(Err_abort);
}

Errcode get_rub_clip(Cliprect *clip)
{
Fullrect fr;
Errcode err;

	if((err = rub_frect(&fr,1,1)) >= 0)
		copy_crectfields(&fr,clip);
	return(err);
}

Errcode cut_out_clip(Cliprect *clip)
{
Errcode err;
	if((err = marqi_cut_xy()) >= 0)
		err = get_rub_clip(clip);
	return(err);
}

static Errcode rub_rect(Rectangle *rect,int marqi_it,SHORT extra)
{
Fullrect fr;
Errcode err;

	if((err = rub_frect(&fr,marqi_it,extra)) >= 0)
		copy_rectfields(&fr,rect);
	return(err);
}

Errcode get_rub_rect(Rectangle *rect)
/* gets rectangle with marqi'd rubber box on screen */
{
	return(rub_rect(rect,1,1));
}

Errcode get_srub_rect(Rectangle *rect)
/* gets rectangle with solid current color rubber box on screen */
{
	return(rub_rect(rect,0,1));
}
Errcode cut_out_rect(Rectangle *rect)
{
Errcode err;
	if((err = marqi_cut_xy()) >= 0)
		err = rub_rect(rect,1,1);
	return(err);
}
Errcode gcut_out_rect(Rectangle *rect)
{
Errcode err;

	if((err = marqi_cut_xy()) >= 0)
		err = rub_rect(rect,1,0);
	return(err);
}

/************** circle marqis *****************/

void savedraw_circle(Marqi_circdat *cd, Short_xy *cent,SHORT d)
{
	cd->mh.dotbuf = cd->save; 
	doval(cent->x,cent->y,d,
		  vb.pencel->aspect_dx, vb.pencel->aspect_dy,
		  savedraw_marqidot,&cd->mh,NULL,NULL,FALSE);
	cd->saved = 1;
}
void restore_circle(Marqi_circdat *cd, Short_xy *cent, SHORT d)
{
	if(!cd->saved)
		return;

	cd->mh.dotbuf = cd->save; 
	doval(cent->x,cent->y,d,
		  vb.pencel->aspect_dx, vb.pencel->aspect_dy,
	      restore_marqidot,&cd->mh,NULL,NULL,FALSE);
}

static int anim_rubcirc(Marqi_circdat *cd)
{
SHORT d;
Short_xy cent;

	if(JSTHIT(MMOVE) || !(cd->saved))
	{
		d = calc_distance(cd->pos.x<<1,cd->pos.y<<1,icb.mx<<1,icb.my<<1);

		if(cd->movecent)
		{
			d = d/2;
			cent.x = (cd->pos.x+icb.mx)/2;
			cent.y = (cd->pos.y+icb.my)/2;
		}
		else
			cent = cd->pos;

		if( d != cd->d 
			|| cd->cent.x != cent.x 
			|| cd->cent.y != cent.y 
			|| !(cd->saved))
		{
			if(!(cd->movecent))
				top_textf("!%d%d", rub_circle_str, d/2, d);
			restore_circle(cd,&(cd->cent),cd->d);
			savedraw_circle(cd,&cent,d);
			cd->d = d;
			cd->cent = cent;
			goto out;
		}
	}
	doval(cd->cent.x,cd->cent.y,cd->d,
		  vb.pencel->aspect_dx, vb.pencel->aspect_dy,
	      cd->mh.pdot,&(cd->mh),NULL,NULL,FALSE);

out:
	cd->mh.dmod = cd->mh.smod++;
	return(0);
}

Errcode init_circdat(Marqi_circdat *cd, Pixel color)
{
cinit_marqihdr(&cd->mh,color,color,TRUE);

if(NULL == (cd->save = pj_malloc((vb.pencel->width + vb.pencel->height)*3)))
	return(Err_no_memory);
cd->saved = 0;
return(Success);
}


static Errcode rubcirc(Pixel color, Short_xy *cent,SHORT *diam, int movecent)

/* Display a circle from click at x0,y0 to cursor until next click 
 * does not alter data in cent or rad if aborted if movecent is true it will
 * move the center rather than the radius */
{
Marqi_circdat cd;
Errcode err;

	if ((err = init_circdat(&cd, color)) < Success)
		return(err);
	cd.pos.x = icb.mx;
	cd.pos.y = icb.my;
	cd.movecent = movecent;

	anim_wait_input(KEYHIT|MBRIGHT|MBPEN,MMOVE,cd.mh.waitcount,
					anim_rubcirc,&cd);

	restore_circle(&cd,&(cd.cent),cd.d);
	pj_free(cd.save);
	cleanup_toptext();
	if(JSTHIT(MBPEN))
	{
		*cent = cd.cent;
		*diam = cd.d;
		return(Success);
	}
	return(Err_abort);
}
int get_rub_circle(Short_xy *cent,SHORT *diam, Pixel color)
{
	return(rubcirc(color,cent,diam,0));
}
int rub_circle_diagonal(Short_xy *cent,SHORT *diam, Pixel color)
{
	return(rubcirc(color,cent,diam,1));
}

/***** vector list things used in a3d.c celfli.c and tweenpul.c *********/

void msome_vector(Short_xy *pts,
				  int count, 
				  VFUNC dotout, void *dotdat, 
				  int open, int pt_size)
/* display a point-list through dotout note this increments the start mod */
{
Short_xy *last;

	if (open)
	{
		last = pts;
		pts = OPTR(pts,pt_size);
		count -= 1;
	}
	else
	{
		last = OPTR(pts,pt_size*(count-1) );
	}
	while (--count >= 0)
	{
		pj_cline( last->x, last->y, pts->x, pts->y, dotout, dotdat);
		last = pts;
		pts = OPTR(pts,pt_size);
	}
}

void marqi_vector(Marqihdr *mh, Short_xy *pts, int count, int pt_size)
/* marqi a poly vector point list */
{
	msome_vector(pts, count, mh->pdot,mh, 0, pt_size);
}

#ifdef SLUFFED
void undo_vector(Marqihdr *mh, Short_xy *pts, int count, int pt_size)
/* erase a poly vector point list */
{
	msome_vector(pts, count, undo_marqidot, mh, 0, pt_size);
}
#endif /* SLUFFED */
