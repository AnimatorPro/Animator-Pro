#include "menus.h"

static void mb_pathtext(Button *b,Pixel color, char *string) 
/* insures that the right end of a path is always shown in a label */
{
Vfont *f = b->root->font;
Clipbox cb;
int x, wid;

	if (string == NULL)
		return;

	x = (fchar_spacing(f," ")>>1);
	wid = fstring_width(f,string);

	mb_make_iclip(b,&cb);

	if((wid + x) >= cb.width)
		x = cb.width - wid;

	gftext(&cb,f,string, x,
		   font_ycent_oset(f,cb.height),
		   color,TM_MASK1 );
}
void black_pathlabel(Button *b)
{
	white_block(b);
	mb_pathtext(b,mc_black(b),b->datme);
}
