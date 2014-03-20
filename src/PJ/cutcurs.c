#define CUTCURS_C
#include "errcodes.h"
#include "jimk.h"
#include "marqi.h"

typedef struct cutdata {
	Marqihdr mh;
	SHORT lastx, lasty;
	SHORT wndoid;
	SHORT doneonce;
	UBYTE *hline;
	UBYTE *vline;
	void (*puthseg)(void *r,void *buf,Coor x,Coor y,Ucoor width);
	void (*putvseg)(void *r,void *buf,Coor x,Coor y,Ucoor height);
	Pixel oncol, offcol, zon;
} Cutdata;

extern void zoom_put_hseg();
extern void zoom_put_vseg();

static void save_cut(Cutdata *cd)
{
	pj_get_hseg(cd->mh.w,cd->hline,0,cd->lasty,cd->mh.port.width);
	pj_get_vseg(cd->mh.w,cd->vline,cd->lastx,0,cd->mh.port.height);
	cd->doneonce = 1;
}
static void rest_cut(Cutdata *cd,Boolean zoomed)
{
	if(!cd->doneonce)
		return;

	if(zoomed)
	{
		cd->puthseg = zoom_put_hseg;
		cd->putvseg = zoom_put_vseg;
	}
	else
	{
		cd->puthseg = pj_put_hseg;
		cd->putvseg = pj_put_vseg;
	}

	(*(cd->puthseg))(cd->mh.w,cd->hline,0,cd->lasty,cd->mh.port.width);
	(*(cd->putvseg))(cd->mh.w,cd->vline,cd->lastx,0,cd->mh.port.height);

}
static void restore_cut(Cutdata *cd)
{
	if(vl.zoomwndo)
		rest_cut(cd,1);
	rest_cut(cd,0);
}
static void print_cutpos(SHORT x,SHORT y)
{
	top_textf(" %3d %3d", x, y);
}
static void redraw_cut(Cutdata *cd)
/* increment mod and Draw full-penwndo cross-hair through dotout routine */
{
	if(vl.zoomwndo)
	{
		if(JSTHIT(MMOVE))
			rest_cut(cd,1);
		cd->mh.oncolor = cd->zon;
		cd->mh.putdot = zoom_put_dot;
		marqi_cut(&(cd->mh),icb.mx,icb.my);
	}
	if(JSTHIT(MMOVE) || !(cd->doneonce))
	{
		print_cutpos(icb.mx,icb.my);
		rest_cut(cd,0);
		cd->lastx = icb.mx;
		cd->lasty = icb.my;
		save_cut(cd);
	}
	cd->mh.oncolor = cd->oncol;
	cd->mh.putdot = pj_put_dot;
	marqi_cut(&(cd->mh),icb.mx,icb.my);
}
static int anim_cut(Cutdata *cd)
{
	/* check to see what window the mouse is currently over */

	if(mouseon_wndo((Wndo *)vb.pencel))
	{
		if(cd->wndoid != FLI_WNDOID)
		{
			cd->wndoid = FLI_WNDOID;
			load_wndo_iostate(vb.pencel);
			cd->zon = vb.screen->SGREY;
			cd->oncol = vb.screen->SWHITE;
		}
		else 
			goto redraw;
	}
	else if(vl.zoomwndo && mouseon_wndo(vl.zoomwndo))
	{
		if(cd->wndoid != ZOOM_WNDOID)
		{
			cd->wndoid = ZOOM_WNDOID;
			load_wndo_iostate(vl.zoomwndo);
			cd->zon = vb.screen->SWHITE;
			cd->oncol = vb.screen->SGREY;
		}
		else
			goto redraw;
	}
	else
	{
		if(cd->wndoid)
		{
			restore_cut(cd);
			cd->doneonce = 0;
			load_wndo_iostate(NULL);
			show_mouse();
			cd->wndoid = 0;
		}
		/* if mouse moves re-display coordinates incase cursor is on top of 
		 * coordinate window this will cause window to move out of the way 
		 * if it is under cursor */

		if(JSTHIT(MMOVE))
			top_textf(" ?  ?");

		goto check_out;
	}

	restore_cut(cd);
	cd->doneonce = 0;
	hide_mouse();
	init_marqihdr(&(cd->mh),vb.pencel,
				  NULL,cd->oncol,cd->offcol);
redraw:

	redraw_cut(cd);

check_out:

	if(JSTHIT(KEYHIT|MBRIGHT))
		return(Err_abort);
	if(JSTHIT(MBPEN))
	{
		if(cd->wndoid)
			return(cd->wndoid);
		else
			icb.hitstate &= ~MBPEN; /* cancel if not zoom or fli window */
	}
	return(0);
}

static Errcode get_cut(Pixel offc, Pixel onhi, Pixel onlo)

/* returns Err_abort if right click or key hit returns ZOOM_WNDOID if in zoom 
 * window FLI_WNDOID if in fli window leaves values in icb as of last click 
 * and iostate set to window click was in */
{
Wiostate ios;
Cutdata cd;
int ret;

	if(NULL == (cd.hline = pj_malloc(vb.pencel->width + vb.pencel->height)))
		return(Err_no_memory);

	save_wiostate(&ios);
	cd.vline = cd.hline + vb.pencel->width;
	cd.wndoid = -1;
	cd.doneonce = 0;
	cd.oncol = onhi;
	cd.offcol = offc;
	vinit_marqihdr(&cd.mh,1,1);

	ret = anim_wait_input(KEYHIT|MBRIGHT|MBPEN,KEYHIT|MBRIGHT|MBPEN|MMOVE,
						  cd.mh.waitcount,anim_cut,&cd);

	cleanup_toptext();
	restore_cut(&cd);
	pj_free(cd.hline);
	if(ret < 0)
		rest_wiostate(&ios);
	else 
		show_mouse();
	return(ret);
}
Errcode marqi_cut_xy(void)
{
	return(get_cut(vb.screen->SBLACK,
				   vb.screen->SWHITE,
				   vb.screen->SGREY ));
}

