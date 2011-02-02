#ifndef MARQI_H
#define MARQI_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef RASTCALL_H
	#include "rastcall.h"
#endif

#ifndef VERTICES_H
	#include "vertices.h"
#endif


#define DMARQI_MILLIS 100 /* milli-seconds between marqi draws for creepy
					       * marqis */

typedef struct marqihdr {
	struct wndo *w; /* loaded by init current marqi window */
	Fullrect port;	/* clip port for drawing and saving within Raster w */
	Pixel oncolor, offcolor; /* loaded by init on and off colors */
		/* raster put dot function set by init to "put_dot" */ 	
	void (*putdot)(void *r,Pixel c,Coor x, Coor y); 

	SHORT smod;     /* set by init to 0 the start "mod" */
	SHORT dmod;		/* set by init to 0 the current dot mod */
	VFUNC pdot;		/* loaded by init: dotout function */
	void *adata;	/* animation subroutine specific data */
	UBYTE *dotbuf;	/* current dot in save buffer for save and restore
					 * dot calls set to NULL by init calls */
	SHORT waitcount; /* animation timeout set by init_marqihdr */
	SHORT unused[7]; /* for future */
} Marqihdr;

void marqi_cut(Marqihdr *mh,Coor x,Coor y);
Errcode screen_cut_rect(struct wscreen *s,Rectangle *rect,Rectangle *sclip);

typedef struct marqi_circdat {
	Marqihdr mh;
	UBYTE *save;
	SHORT saved;
	Short_xy pos;
	SHORT d;
	Short_xy cent;
	SHORT movecent;
} Marqi_circdat;

Errcode init_circdat(Marqi_circdat *cd, Pixel color);
void savedraw_circle(Marqi_circdat *cd, Short_xy *cent,SHORT r);
void restore_circle(Marqi_circdat *cd, Short_xy *cent, SHORT r);

void undo_marqidot(SHORT x,SHORT y, Marqihdr *mh);
#endif /* MARQI_H */
