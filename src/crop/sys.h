#ifndef SYS_H
#define SYS_H

/* Function: init_system */
extern int init_system(void);

/* Function: cleanup */
extern void cleanup(void);

/* Function: c_input
 *
 *  Record the mouse/keyboard input state in the variables uzx, uzy,
 *  key_hit, key_in, and mouse_button.  (uzx is for un-zoomed-x ... not
 *  really a good name.  Used to be mouse_x, mouse_y, but then things
 *  got complicated in vpaint with input macros, zoom, grid-lock etc.)
 */
extern void c_input(void);

#endif
