#include "menus.h"

/* a simple titlebar with only move and close */

Titbar_group tbg_moveclose = {
 	mb_clipmove_menu,	/* moveit */
	mb_gclose_cancel, 	/* closeit */
	NULL,
};

static void draw_closer(Button *b, int txtwid)
{
int x,y,width,height;
Pixel black;

	black = mc_black(b);
	x = b->x + 2;
	y = b->y + 1;
	height = b->height - 2;
	width = b->width - txtwid;
	if(width < height<<1)
		width = height;
	else
		width = height + height/2;

	for(;;)
	{
		if(width < 1)
			break;

		pj_set_hline(b->root,black,x,y,width);
		pj_set_hline(b->root,black,x,y+height-1,width);

		if(height > 2)
		{
			pj_set_vline(b->root,black,x,y,height);
			pj_set_vline(b->root,black,x+width-1,y,height);
			height -= 4;
			y+=2;
		}
		else 
			break;
		width -= 4;
		x+=2;
	}
}
/* titlebar seer */

void see_titlebar(Button *b)
{
SHORT obx;
int txtwid;
Titbar_group *tbg = (void *)(b->group);
Boolean closeit;

	if(b->y)
	{
		diag_inside(b->root,b->x-1, b->y-1, 
					b->width+2, b->height+2, mc_grey(b),
					b->root->w.W_screen->bbevel);
	}
	else
		mc_block(b,MC_GREY);

	obx = b->x;
	if((closeit = (tbg && tbg->closeit)) != FALSE)
		b->x += (b->height>>1);

	txtwid = mb_centext(b,mc_white(b),b->datme);
	b->x = obx;

	if(closeit)
		draw_closer(b,txtwid);
}
void hang_see_title(Button *b)
{
	hang_children(b);
	see_titlebar(b);
}

/* titlebar feeler */

void feel_titlebar(Button *b)
{
Titbar_group *tbg = (void *)(b->group);
SHORT tstx;

	if((b->width - fstring_width(b->root->font,b->datme)) < (b->height<<1))
		tstx = b->height;
	else
		tstx = b->height + (b->height>>1);

	tstx += (b->x + b->root->w.x); /* cause zoom window has funny mx */

	/* if it's got a close gadget and click was on left closeit.  Also
	   if invoked by a key equiv. close it.  */
	if (tbg->closeit && (icb.cx <= tstx || JSTHIT(KEYHIT))) 
		(*(tbg->closeit))(b,tbg->data);
	else if(tbg->moveit)
		(*(tbg->moveit))(b,tbg->data);
	else
		mb_clipmove_menu(b, NULL);

	return;
}

void feel_req_titlebar(Button *b)
/* Update requestor center position to reflect movement of menu from
 * feel_titlebar */
{
Wscreen *ws;

	ws = get_button_wndo(b)->w.W_screen;
	feel_titlebar(b);
	copy_rectfields(get_button_hdr(b),&ws->last_req_pos);
}
