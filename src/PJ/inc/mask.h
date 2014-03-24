#ifndef MASK_H
#define MASK_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

struct bitmap;
struct button;

extern struct bitmap *mask_rast;

/* freem.c */
extern Boolean mask_is_present(void);

/* mask.c. */
extern void free_mask(struct bitmap *mask);
extern Errcode alloc_mask(struct bitmap **mask, USHORT width, USHORT height);
extern void free_the_mask(void);
extern int alloc_the_mask(void);
extern int save_the_mask(char *name);
extern int load_the_mask(char *name);
extern void mb_toggle_mask(struct button *b);
extern void qmask(void);
extern void qmask_keep_undo(void);

/* quickdat.c. */
extern void see_mask_button(struct button *b);

/* vpaint.c */
extern void qload_mask(void);
extern void qsave_mask(void);

#endif
