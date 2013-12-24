/*****************************************************************************
 * PATCH10A.H - Header for patch10a module.
 ****************************************************************************/

#ifndef PATCH10A_H
#define PATCH10A_H

extern int		init_patches_10a(void); 		/* returns host version # */
extern int		po_get_boxbevel(void);
extern void 	po_set_boxbevel(int newbevel);
extern void 	po_release_cel(void);

#undef poeGetBoxBevel
#undef poeSetBoxBevel
#undef poeCelRelease

#define poeGetBoxBevel po_get_boxbevel
#define poeSetBoxBevel po_set_boxbevel
#define poeCelRelease  po_release_cel

#endif
