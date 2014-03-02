#include "errcodes.h"
#include "memory.h"
#include "pjbasics.h"
#include "rastcurs.h"
#include "resource.h"
#include "picfile.h"


/*************************************************/

void show_rastcursor(Rastcursor *rc)
{
Cursorcel *r = rc->cel;
SHORT cx, cy;

	cx = icb.cx - r->x;
	cy = icb.cy - r->y;
	pj_blitrect(vb.screen->viscel,
			 cx,cy,rc->save,0,0,r->width,r->height); 
	rc->save->r.x = cx;
	rc->save->r.y = cy;
	procblit(r,0,0,vb.screen->viscel,cx,cy,r->width,r->height,tbli_xlatline,
			 get_cursor_xlat());
}
void hide_rastcursor(Cursorhdr *rastcursor)
{
	Rastcursor *rc = (Rastcursor *)rastcursor;

	pj_blitrect(rc->save,0,0,
	         vb.screen->viscel,rc->save->r.x,rc->save->r.y,
			 rc->cel->width,rc->cel->height); 
}
/************************************************************************/
static erase_rcurs_leftover(Coor x, Coor y, Coor w, Coor h,Rastcursor *rc)
{
Cursorsave *save = rc->save;
	pj_blitrect(save,x - save->r.x,y - save->r.y,vb.screen->viscel,x,y,w,h);
}
static save_newcurs(Coor x, Coor y, Coor w, Coor h,Rastcursor *rc)
{
Cursorsave *save = rc->save;
	pj_blitrect(vb.screen->viscel,x,y,save,x - save->r.x,y - save->r.y,w,h);
}
void move_rastcursor(Cursorhdr *rastcursor)
{
Rastcursor *rc = (Rastcursor *)rastcursor;
Cursorcel *r = rc->cel;
Cursorsave *save = rc->save;
Coor cx, cy, ox, oy;
Ucoor w, h;

	cx = icb.cx - r->x;
	cy = icb.cy - r->y;
	w = r->width;
	h = r->height;
	ox = save->r.x;
	oy = save->r.y;

	/* erase "leftover" area */
	do_leftbehind(ox,oy,cx,cy,w,h,(do_leftbehind_func)erase_rcurs_leftover, rc);

	/* "scroll" part of save that is common to new location */
	blitmove_rect(save,0,0,save,ox - cx, oy - cy, w, h);

	save->r.x = cx;
	save->r.y = cy;

	/* save new area */
	do_leftbehind(cx,cy,ox,oy,w,h, (do_leftbehind_func)save_newcurs, rc);

	/* redraw cursor */
	abprocblit(r,0,0,vb.screen->viscel,cx,cy,w,h,save,0,0,tbli_xlatline,
			   get_cursor_xlat());
}
