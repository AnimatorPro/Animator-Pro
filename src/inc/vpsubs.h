//
// Created by Charles Wardlaw on 2024-04-06.
//

#ifndef ANIMATOR_PRO_VPSUBS_H
#define ANIMATOR_PRO_VPSUBS_H

#include "jimk.h"
#include "raster.h"
#include "stdtypes.h"

#ifdef WITH_POCO
int fli_screen_width(void);
int fli_screen_height(void);
#endif /* WITH_POCO */

void ccolor_dot(SHORT x, SHORT y, void *data);
void undo_dot(SHORT x, SHORT y, void *data);
void undo_rect(Coor x,Coor y,Coor w,Coor h);
void save_undo_rect(Coor x,Coor y,Coor w,Coor h);
void zoom_undo_rect(Coor x,Coor y,Coor w,Coor h);
bool check_esc_abort(void);
bool check_pen_abort(void);
bool check_toggle_menu(void);
bool check_toggle_abort(void);
int cluster_count(void);
UBYTE *cluster_bundle(void);
void see_cmap(void);
SHORT uscale_vscoor(Vscoor vcoor, SHORT relto);
Vscoor scale_vscoor(SHORT coor, SHORT relto);
void reres_settings(void);
void mb_quickmenu_to_bottom(Button *b);
void mb_move_quickmenu(Button *b);
void menu_to_quickcent(Menuhdr *mh);
void save_undo(void);
void zoom_unundo(void);
void swap_undo(void);
void menu_doundo(void);
bool check_undo_key(void);
void menu_doredo(void);
void restore(void);
void init_seq(void);
void kill_seq(void);
void flush_tempflx(void);
void hide_mp(void);
void show_mp(void);
int interp_range(int c1,int c2,int i,int divi);


#endif // ANIMATOR_PRO_VPSUBS_H
