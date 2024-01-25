#ifndef VDEVCALL_H
#define VDEVCALL_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef VDEVICE_H 
	#include "vdevice.h"
#endif

#ifndef RASTER_H
	#include "raster.h"
#endif 

struct screen_mode;
struct wscreen;

extern char pj_mcga_name[]; /* name used by pj_open_ddriver() that opens mcga */

extern Errcode open_screen(struct screen_mode *sm, char *path);
extern void cleanup_screen(void);

Errcode pj_open_loadable_vdriver(Vdevice **pvd, char *name);
Errcode pj_open_mcga_vdriver(Vdevice **pvd);
Errcode pj_open_ddriver(Vdevice **pvd, char *name);
void pj_close_vdriver(Vdevice **pvd);
Errcode pj_vd_verify_hardware(Vdevice *vd);
Errcode pj_vd_open_screen(Vdevice *vd, Raster *r,
					   USHORT width, USHORT height, USHORT mode);
Errcode pj_vd_open_raster(Vdevice *vd, Rasthdr *spec, 
					   Raster *r, UBYTE displayable);

extern int pj_get_vmode(void);
extern void pj_set_vmode(int mode);
extern void restore_ivmode(void);
extern void pj_wait_vsync(void);

extern void set_menu_scale(struct wscreen *s);

#endif
