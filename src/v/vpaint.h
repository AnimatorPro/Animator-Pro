#ifndef VPAINT_H
#define VPAINT_H

#include <stdio.h>
#include "jimk.h"

/* Function: load_settings */
extern int load_settings(FILE *f, Vsettings *s);

/* Function: save_settings */
extern int save_settings(FILE *f, const Vsettings *s);

#endif
