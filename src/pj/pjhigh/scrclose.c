#include "errcodes.h"
#include "pjbasics.h"
#include "vdevcall.h"

#ifdef SLUFFED
void old_video(void)
/* only used to restore screen for bad cookie message in util\memalloc.c */
{
	disable_textboxes();
	pj_close_vdriver(&vb.vd); 
	restore_ivmode();
}
#endif /* SLUFFED */
void cleanup_screen(void)
/* inverse of init_screen() */
{
	hide_mouse(); /* make sure it's gone */
	disable_textboxes();
	set_cursor(NULL); /* no more cursor, no screen */
	cleanup_idriver();
	cleanup_muscreen(vb.screen); /* shutdown menus portion of screen */
	free_menu_font();
	close_wscreen(vb.screen); /* bye bye - no more textboxes */
	vb.screen = NULL;
	pj_rcel_free(vb.cel_a);
	vb.cel_a = NULL;
	pj_rcel_free(vb.cel_b);
	vb.cel_b = NULL;
	pj_close_vdriver(&vb.vd);
}
