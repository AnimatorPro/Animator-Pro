#ifndef A3D_H
#define A3D_H

#include <stdio.h>

#define ADO_SPIN 0
#define ADO_SIZE 1
#define ADO_MOVE 2
#define ADO_PATH 3
#define SPIN_CENTER 0
#define SPIN_AXIS 1
#define SPIN_TURNS 2
#define PATH_SPLINE 0
#define PATH_POLY 1
#define PATH_SAMPLED 2
#define PATH_CLOCKED 3

#define A3D_MAGIC (0x1A3D+2)

#define OPS_SCREEN 0
#define OPS_CEL 1
#define OPS_POLY 2
#define OPS_SPLINE 3

/* Function: load_ado_setting */
extern int load_ado_setting(FILE *f, struct ado_setting *as);

/* Function: save_ado_setting */
extern int save_ado_setting(FILE *f, const struct ado_setting *as);

#endif
