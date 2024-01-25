#ifndef IO_DOS_H
#define IO_DOS_H

/* Function: set_vmode
 *
 *  Tell the VGA what resolution to be in.
 */
extern void set_vmode(int mode);

/* Function: get_vmode
 *
 *  Get current video mode.
 */
extern int get_vmode(void);

#endif
