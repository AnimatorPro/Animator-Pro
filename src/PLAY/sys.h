#ifndef SYS_H
#define SYS_H

/* Function: init_system */
extern int init_system(void);

/* Function: shutdown_system
 *
 *  Go back to old video mode and take out our clock interrupt handler.
 */
extern void shutdown_system(void);

/* Function: get_key */
extern void get_key(void);

/* Function: break_key */
extern int break_key(void);

/* Function: c_poll_input
 *
 *  Record the mouse/keyboard input state in the variables uzx, uzy,
 *  key_hit, key_in, and mouse_button.  (uzx is for un-zoomed-x ... not
 *  really a good name.  Used to be mouse_x, mouse_y, but then things
 *  got complicated in vpaint with input macros, zoom, grid-lock etc.)
 */
extern void c_poll_input(void);

/* Function: c_wait_input */
extern void c_wait_input(void);

/* Function: check_button */
extern void check_button(void);

#endif
