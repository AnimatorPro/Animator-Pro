#include "errcodes.h"
#include "menus.h"

/****** group closing feelmes and subs ********/

void mb_gclose_code(Button *b,LONG code)
{
	mh_gclose_code(get_button_hdr(b),code);
}
void mb_gclose_ok(Button *b)
{
	mb_gclose_code(b,0);
}
void mb_gclose_cancel(Button *b)
{
	mb_gclose_code(b,Err_abort);
}
void mb_gclose_identity(Button *b)
{
	mb_gclose_code(b,b->identity);
}

/******* menu closing feelmes *********/

void mb_close_code(Button *b,LONG code)

/* this is called within a button or menu window function. 
 * Any buttons attached to this
 * menu may not be used until a new menu is opened for them or they are 
 * attached to an open menu this is used to exit requestor menus etc */
{
	close_menu_code(get_button_hdr(b),code);
}
void mb_close_ok(Button *b)
{
	mb_close_code(b,0);
}
void mb_close_cancel(Button *b)
{
	mb_close_code(b,Err_abort);
}

/**** draw hide and show feelmes ****/

void mb_draw_menu(Button *b) 
/* redraws whole menu button is attached to can be a feelme or optme 
 * does not draw a hidden menu */
{
	draw_menu(get_button_hdr(b));
}
void mb_hide_menu(Button *b)
/* hide menu from button we dont have a feelme show menu cause you can't
 * hit it !! */
{
	hide_menu(get_button_hdr(b));
}
void mb_hide_group(Button *b)
/* hide menus group from button */
{
Menuhdr *mh;

	if(NULL != (mh = get_button_hdr(b)))
		hide_group(mh->group);
}
void mb_show_group(Button *b)
/* show the rest of menus group from button */
{
Menuhdr *mh;

	if(NULL != (mh = get_button_hdr(b)))
		show_group(mh->group);
}

