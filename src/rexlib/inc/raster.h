#ifndef RASTER_H
#define RASTER_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif




#ifdef BIGCOMMENT /**************************************************/

    The raster header structure and other sets of structured fields
    used to make rasters have been made into macros so other code may make up 
    rasters without.using.lots.of.dots and still maintain a common structure 
    notice that there is NO semicolon ';' after the last field so that 
    the macro will have a semicolon after it when used so things look and
    act right

	**** Raster header fields (struct rasthdr) ***

	SHORT type;	    /* the type of Raster ie BYTEMAP BITMAP etc */	
	SHORT pdepth;	/* pixel depth in bits if it was converted to a bitplanes.
				     * It is a common sizer for all types */
	struct rastlib *lib; /* An object oriented library for multiple types of
					 	  * hardware rasters and frame buffers windows etc */
	SHORT aspect_dx, aspect_dy; /* aspect dx,dy such that dx by dy is a 
					             * a square loaded by pj_vd_open_screen() */
	SHORT reserved[2];

	/* this is and must be a Rectangle struct */
	USHORT width, height; SHORT x, y 

#endif /* BIGCOMMENT ***************************************************/	

#define RASTHDR_FIELDS \
	SHORT type;\
	SHORT pdepth;\
	struct rastlib *lib;\
	SHORT aspect_dx, aspect_dy;\
	SHORT reserved[2];\
	USHORT width, height; SHORT x, y     /* note no ';' */ 

typedef struct rasthdr {
	RASTHDR_FIELDS;
} Rasthdr;

/* will copy headers between two things that have RASTHDR_FIELDS */

#define copy_rasthdr(s,d)\
 (*((Rasthdr *)((&((d)->pdepth))-1)) = *((Rasthdr *)((&((s)->pdepth))-1))) 

/****** Raster types for type field *******/

#define RT_UNDEF    0  /* invalid type */
#define RT_BITMAP	1  /* raster is a bitmap */
#define RT_BYTEMAP  2  /* raster is a bytemap */
#define RT_MCGA     3  /* built in bytemap style vga screen */
#define RT_WINDOW   4  /* this is a window layer */
#define RT_ROOTWNDO 5  /* this is a screen backdrop window */
#define RT_NULL		6  /* this a null raster just dumps output reads 0's */
#define RT_CLIPBOX  7  /* this is a "clip box" */ 


#define RT_FIRST_VDRIVER  100  /* loaded driver types start here */

#define RT_FIRST_CUSTOM	10000  /* first type used for custom rasters */

/**************** items for Bytemaps and Bitmaps ****************/

/* for now pj is only a 256 color program but that may change! */

#define MAX_BITPLANES 1 /* there will allways be at least one plane */
#define MAX_BYTEPLANES 1 

#define Bitmap_bpr(w) (((w)+7)>>3) /* bitplane bytes per row for width */
#define Bitplane_size(w,h) (Bitmap_bpr(w)*(h)) /* size of bitplane */

#define Bytemap_bpr(w) ((USHORT)(w)) /* byteplane bytes per row */
/* size of byteplane */
#define Byteplane_size(w,h) (((ULONG)(h))*Bytemap_bpr(w)) 

/* byteplane and bitplane pointer typedef */
typedef UBYTE *PLANEPTR;

typedef struct bmap {
	SHORT segment;		   /* this shouldn't be here but makes
							  primitives easier to work on MCGA & RAM... */
	SHORT num_planes;      /* number of bplanes at least 1 */
	LONG bpr;	   	   	   /* bytes per row */
	ULONG psize;		   /* size of a plane in bytes (saves code) */
	PLANEPTR bp[1];        /* at least one plane, the pixelated data */
} Bmap;

/* segments for vga and for the rest of our world... */
#define VGA_SCREEN ((void *)0xa0000)
#define VGA_SEG 0x34
#define PHAR_SEG 0x14

typedef struct bitmap {
	RASTHDR_FIELDS;
	Bmap  bm;
} Bitmap;

typedef struct bytemap {
	RASTHDR_FIELDS;
	Bmap bm;
} Bytemap;

#define Bitmap_IS_Bytemap 1  /* some code wants to know they are the same */

/* this is the big one that can have any of them in it */

typedef union rastbody {
	Bmap bm;
} Rastbody;

/* field definition for larger things like windows that want 
 * to be a raster used the same way RASTHDR_FIELDS is */

#define RAST_FIELDS\
	RASTHDR_FIELDS;\
	Rastbody hw

typedef struct raster {
	RAST_FIELDS;
} Raster;

/* typedef just for inline documentation */
typedef Raster McgaRast;

/* these types are used by raster oriented graphics calls */
typedef unsigned long Ucoor;
typedef long Coor;

/* A little item that when made defines a clip box on a raster which itself
 * can be referenced as a "raster" its x and y will be a relative offset
 * into the raster from its upper left corner Its values must be set with
 * make_clipbox() a sort of mini window that doesn't save whats behind it 
 * get_hseg() as well as get_dot() will return zeros if the area of the 
 * clipbox is outside the root raster */

typedef struct clipbox {
	RASTHDR_FIELDS;
	Raster *root;  	/* first field of rastbody */
} Clipbox;

#endif /* RASTER_H */
