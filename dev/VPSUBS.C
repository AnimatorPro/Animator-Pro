
/* vpsubs.c - routines used by all sort of menu panels.  Really a mismatch.
   Not well suited to being in an overlay. */

#include <math.h>
#include "jimk.h"
#include "menus.h"
#include "fli.h"
#include "ptrmacro.h"
#include "errcodes.h"
#include "options.h"
#include "auto.h"
#include "softmenu.h"
#include "broadcas.h"

extern Button dsel1_sel, it0_sel; /* first pen tool slot first ink slot */
extern Menuhdr quick_menu;

#define DEBUG

#ifdef DEBUG
static void poke_bg(Rgb3 *p)
{
	pj_set_colors(&(vb.screen->wndo), 0,1,(UBYTE *)p);
}

static void restore_bg(void)
{
	pj_set_colors(vb.screen, 0,1,(UBYTE *)(vb.screen->wndo.cmap->ctab));
}

static void flash_bg(Rgb3 *p)
{
poke_bg(p);
wait_a_jiffy(4);
restore_bg();
}
#endif /* DEBUG */


#ifdef SLUFFED
#endif /* SLUFFED */
void flash_green(void)
{
extern Rgb3 pure_green;
flash_bg(&pure_green);
}


#ifdef DEBUG
#endif /* DEBUG */
void flash_red(void)
{
extern Rgb3 pure_red;
flash_bg(&pure_red);
}

#ifdef SLUFFED
#endif /* SLUFFED */
void flash_blue(void)
{
extern Rgb3 pure_blue;

flash_bg(&pure_blue);
}

#ifdef SLUFFED
long fli_screen_size()
{
return((long)vb.pencel->width * (long)vb.pencel->height);
}
#endif /* SLUFFED */

int fli_screen_width()
{
return(vb.pencel->width);
}

int fli_screen_height()
{
return(vb.pencel->height);
}


void ccolor_dot(SHORT x,SHORT y)
{
	pj_put_dot(vb.pencel,vs.ccolor,x,y);
}
void undo_dot(SHORT x, SHORT y)
{
	pj_put_dot(vb.pencel,pj_get_dot(undof,x,y),x,y);
}
void undo_rect(Coor x,Coor y,Coor w,Coor h)
{
	pj_blitrect(undof,x,y,vb.pencel,x,y,w,h);
}
void save_undo_rect(Coor x,Coor y,Coor w,Coor h)
{
	pj_blitrect(vb.pencel,x,y,undof,x,y,w,h);
}
void zoom_undo_rect(Coor x,Coor y,Coor w,Coor h)
{
	pj_blitrect(undof,x,y,vb.pencel,x,y,w,h);
	if(vs.zoom_open)
		zoom_blitrect(undof,x,y,x,y,w,h);
}
#ifdef SLUFFED
Boolean check_any_abort()

/* check abort no matter what window you're in */
{
	if(JSTHIT(MBRIGHT) || (JSTHIT(KEYHIT) && is_abortkey()))
	{
		close_group_code((Mugroup *)see_head(&vb.screen->gstack),Err_abort);
		return(1);
	}
	return(0);
}
#endif /* SLUFFED */

static Boolean rclick_on_screen()
{
	return((JSTHIT(MBRIGHT)
			&& (curson_wndo((Wndo *)vb.pencel)
				|| (vs.zoom_open && curson_wndo(vl.zoomwndo))
				|| curson_wndo(&vb.screen->wndo))));
}

Boolean check_esc_abort()
{
	if (JSTHIT(KEYHIT) && (UBYTE)icb.inkey == ESCKEY)
	{
		close_group_code((Mugroup *)see_head(&vb.screen->gstack),Err_abort);
		return(TRUE);
	}
	return(FALSE);
}

Boolean check_pen_abort()
/* checks for click abort on pen windows only and key abort */
{
	if( rclick_on_screen()
		|| (JSTHIT(KEYHIT) && is_abortkey()))
	{
		close_group_code((Mugroup *)see_head(&vb.screen->gstack),Err_abort);
		return(TRUE);
	}
	return(FALSE);
}
Boolean check_toggle_menu(void)
{
	if( JSTHIT(KEYHIT) && (UBYTE)icb.inkey == ' '
		|| (rclick_on_screen()))
	{
		toggle_menu();
		return(TRUE);
	}
	else
		return(FALSE);
}
Boolean check_toggle_abort(void)
/* returns 0 if input unused 1 if used */
{

	if(JSTHIT(KEYHIT)
		&& (UBYTE)icb.inkey == ESCKEY)
	{
		close_group_code((Mugroup *)see_head(&vb.screen->gstack),Err_abort);
		return(1);
	}
	else
		return(check_toggle_menu());
}

int cluster_count(void)
{
	return(vs.buns[vs.use_bun].bun_count);
}

UBYTE *cluster_bundle(void)
{
	return(vs.buns[vs.use_bun].bundle);
}

void see_cmap(void)
{
	wait_sync();
	pj_cmap_load(vb.screen,vb.screen->wndo.cmap);
}


/* functions to scale and unscale resolution independent variables 
 * in the Vsettings */

SHORT uscale_vscoor(Vscoor vcoor, SHORT relto)
/* scales a Vscoor to get the reltoscaled coordinates for a value */
{
double tcoor;

	tcoor = ((double)vcoor);
	tcoor /= ((double)VS_MAXCOOR);
	tcoor *= ((double)(relto));
	return((SHORT)ceil(tcoor - .5));
}
Vscoor scale_vscoor(SHORT coor, SHORT relto)
/* scales a coordinate to give a vs.coor varying between -VS_MAXCOOR
 * to VS_MAXCOOR coor may vary from -relto*2 to relto*2 */
{
double tcoor;

	tcoor = ((double)VS_MAXCOOR);
	tcoor /= ((double)(relto));
	tcoor *= ((double)coor);
	tcoor = ceil(tcoor);
	if(tcoor > (VS_MAXCOOR*2)) /* overflow */
		return(VS_MAXCOOR*2);
	else if(tcoor < -(VS_MAXCOOR*2)) /* underflow */
		return(-VS_MAXCOOR*2);
	return((SHORT)tcoor);
}

/* scaled field offset tables for fields to scale 
 * organized as:
 *	(offset of vs Vcoor field) (offset of vl field) sizeof(vl field) 
 * terminated by a -1 */

#define INITOSENT(vsfld,vlfld) \
OFFSET(Vsettings,vsfld),OFFSET(Vlcb,vlfld),sizeof(MEMBER(Vlcb,vlfld))

/* penwndo width relative fields */

static SHORT fliwin_width_rel[] = 
{
	INITOSENT(flicentx,flicent.x), 
	INITOSENT(gridx,grid.x),
	INITOSENT(gridw,grid.width),
	INITOSENT(rgcx,rgc.x),
	INITOSENT(expand_x,expand_pos.x),
	-1
};

static SHORT fliwin_height_rel[] = 
{
	INITOSENT(flicenty,flicent.y),
	INITOSENT(gridy,grid.y),
	INITOSENT(gridh,grid.height),
	INITOSENT(rgcy,rgc.y),
	INITOSENT(expand_y,expand_pos.y),
	-1
};

static SHORT screen_width_rel[] = 
{
	INITOSENT(quickcentx,quickcent.x),
	INITOSENT(zwincentx,zwincent.x),
	INITOSENT(zwinw,zwinw),
	-1
};

static SHORT screen_height_rel[] = 
{
	INITOSENT(quickcenty,quickcent.y),
	INITOSENT(zwincenty,zwincent.y),
	INITOSENT(zwinh,zwinh),
	-1
};
static SHORT flidiag_rel[] =
{
	INITOSENT(rgr,rgr),
	-1
};

#undef INITOSENT

static void vs_uscaleum(SHORT *tab,SHORT relto)
{
void *vlroot = &vl;
void *vsroot = &vs;
SHORT usval;

	while(*tab >= 0)
	{
		usval = uscale_vscoor(*(Vscoor *)OPTR(vsroot,tab[0]),relto); 
		if(tab[2] == sizeof(BYTE))
			*((BYTE *)OPTR(vlroot,tab[1])) = usval;
		else if(tab[2] == sizeof(SHORT))
			*((SHORT *)OPTR(vlroot,tab[1])) = usval;
		else if(tab[2] == sizeof(LONG))
			*((LONG *)OPTR(vlroot,tab[1])) = usval;
		tab += 3;
	}
}
#ifdef SLUFFED
static void vs_scaleum(SHORT *tab,SHORT relto)

/* scale items in vl to the vs vscoor values */
{
void *vlroot = &vl;
void *vsroot = &vs;
SHORT usval;

	while(*tab >= 0)
	{
		if(tab[2] == sizeof(BYTE))
			usval = *((BYTE *)OPTR(vlroot,tab[1]));
		else if(tab[2] == sizeof(SHORT))
			usval = *((SHORT *)OPTR(vlroot,tab[1]));
		else if(tab[2] == sizeof(LONG))
			usval = *((LONG *)OPTR(vlroot,tab[1]));

		*(Vscoor *)OPTR(vsroot,tab[0]) = scale_vscoor(usval,relto); 
		tab += 3;
	}
}
#endif /* SLUFFED */

void reres_settings(void)

/* this must be called any time the resolution of the fli-window or the 
 * screen is changed */
{
	vl.flidiag_scale = (vb.pencel->width + vb.pencel->height) * 2;
	vs_uscaleum(screen_width_rel,vb.screen->wndo.width); 
	vs_uscaleum(screen_height_rel,vb.screen->wndo.height); 
	vs_uscaleum(fliwin_width_rel,vb.pencel->width); 
	vs_uscaleum(fliwin_height_rel,vb.pencel->height); 
	vs_uscaleum(flidiag_rel,vl.flidiag_scale); 
}


typedef struct qkmove
	{
	SHORT ox, oy;
	Menuhdr *mh;
	} Qkmove;

static void init_qkmove(Qkmove *qk, Button *b)
{
qk->mh = get_button_hdr(b);
qk->ox = qk->mh->x;
qk->oy = qk->mh->y;
}

static void finish_qkmove(Qkmove *qk)
{
	vl.quickcent.x = qk->mh->x +(qk->mh->width/2); /* for now same width */
	if(qk->mh == &quick_menu)
	{
		vl.quickcent.y = quick_menu.y+(quick_menu.height/2);
	}
	else
	{
		vl.quickcent.y += qk->mh->y = qk->oy;
		menu_to_quickcent(&quick_menu);
	}
	vs.quickcentx = scale_vscoor(vl.quickcent.x, vb.screen->wndo.width);
	vs.quickcenty = scale_vscoor(vl.quickcent.y, vb.screen->wndo.height);
}

void mb_quickmenu_to_bottom(Button *b)
{
Qkmove sqk;

	init_qkmove(&sqk, b);
	mb_menu_to_bottom(b);
	finish_qkmove(&sqk);
}

void mb_move_quickmenu(Button *b)
{
Qkmove sqk;

	init_qkmove(&sqk, b);
	mb_clipmove_menu(b);
	finish_qkmove(&sqk);
}
void menu_to_quickcent(Menuhdr *mh)
{
	menu_to_point(vb.screen,mh,vl.quickcent.x,vl.quickcent.y);
}
#ifdef SLUFFED
void pal_save_undo(void)
/* saves palette only  */
{
	pj_cmap_copy(vb.pencel->cmap,undof->cmap);
}
#endif /* SLUFFED */
#undef save_undo  /* this is defined to reset the widget */
void save_undo(void)
{
	pj_blitrect(vb.pencel,0,0,
		 	 undof, 0, 0, undof->width, undof->height);
	pj_cmap_copy(vb.pencel->cmap,undof->cmap);
}
void zoom_unundo(void)
{
	pj_blitrect(undof,0,0,
			 vb.pencel, 0, 0, vb.pencel->width, vb.pencel->height);
	zoom_it();
	if(!cmaps_same(vb.pencel->cmap,undof->cmap))
	{
		pj_cmap_copy(undof->cmap,vb.pencel->cmap);
		see_cmap();
	}
}
void swap_undo(void)
{
	swap_pencels(undof, (Rcel *)(vb.pencel));
	if(!cmaps_same(vb.pencel->cmap,undof->cmap))
	{
		see_cmap();
		do_color_redraw(NEW_CMAP);
	}
	zoom_it();
	dirties();
}
void menu_doundo(void)

/* undo called from menus */
{
	if(vl.undoit != NULL)
		(*vl.undoit)();
}
Boolean check_undo_key()
{
	if(hit_undo_key())
	{
		menu_doundo();
		return(TRUE);
	}
	return(FALSE);
}

void menu_doredo(void)
/* redo called from menus */
{
	if (vl.redoit != NULL)
		(*vl.redoit)();
	return;
}


void restore(void)
{
	save_undo();
	fli_abs_tseek(vb.pencel,vs.frame_ix);
	see_cmap();
	zoom_it();
	dirty_frame = 0;
}

void init_seq(void)
{
	vs.frame_ix = 0;
	pj_clear_rast(vb.pencel);
	see_cmap();
}

void kill_seq(void)
{
	flx_clear_olays();
	save_undo();
	init_seq();
	empty_tempflx(1);
	vs.bframe_ix = 0; /* back frame buffer no good now */
	flx_draw_olays();
}

void flush_tempflx(void)
{
	scrub_cur_frame();
	flush_tflx();
}

void delete_file_list(char **list)
{
char *name;

while ((name = *list++) != NULL)
	pj_delete(name);
}

void cleanup(Boolean save_state)
{
	/* delete back buffer screen */
	pj_delete(bscreen_name);
	vs.bframe_ix = 0;
	if (save_state)
	{
		soft_put_wait_box("wait_quit");
		flush_tempflx(); /* update tempflx header and stuff */
		flush_tsettings(TRUE); /* update temp settings file */
	}
	close_tflx();
	/* push a copy of current screen and alt,cel etc for when program started 
	   again with id of last tflx flush... */

	delete_file_list(work_temp_files);
	if (save_state)
	{
		push_pics_id(flix.hdr.id.update_time);
			/* move files from memory to filing system */
		softerr(trd_ram_to_files(), "!%s", "temp_copy", get_temp_path());
		rcompact();			/* free blocks used for ram-disk */
	}
	else
	{
		delete_file_list(state_temp_files);
	}
	cleanup_all(Success);
}

void outofhere(Boolean save_state)
{
	cleanup(save_state);
	exit(0);
}

void hide_mp(void)
{
	fliborder_off();
	stack_hide_cgroup(vb.screen);
}
void show_mp(void)
{
	if(stack_show_cgroup(vb.screen))
		fliborder_on();
	else
		fliborder_off();
}

void toggle_menu(void)
{
	if(curr_group(vb.screen)->non_hidden)
		hide_mp();
	else
		show_mp();
}

#ifdef SLUFFED
void defrag(void)
{
	hide_mp();
	unzoom();
	flush_tflx();
	push_pics();
	close_tflx();
	open_tempflx(TRUE);
	pop_pics();
	rezoom();
	show_mp();
}
#endif /* SLUFFED */

int interp_range(int c1,int c2,int i,int divi)
{
	if (divi == 1)
		return(c1);
	else
		divi-=1;
	return( (c2*i + c1*(divi-i) + divi/2)/divi);
}
