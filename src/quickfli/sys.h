#ifndef SYS_H
#define SYS_H

#include "jimk.h"

/* Function: init_system */
extern int init_system(void);

/* Function: cleanup
 *
 *  Go back to old video mode and take out our clock interrupt handler.
 */
extern void cleanup(void);

/* Function: strobe_keys
 *
 *  Return 0 if no key, key scan code if there is a key.
 */
extern unsigned int strobe_keys(void);

#endif
