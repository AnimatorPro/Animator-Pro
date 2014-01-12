#ifndef GFX_H
#define GFX_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef CMAP_H
	#include "cmap.h"
#endif

#ifndef RASTCALL_H
	#include "rastcall.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif

#ifndef REXLIB_CODE

#ifndef RECTANG_H
	#include "rectang.h"
#endif

#ifndef PROCBLIT_H
	#include "procblit.h"
#endif

#ifdef FLILIB_CODE			/* IF building the FLI library... */
  typedef void Short_xy;		/* the flilib needs some protos below, but */
  typedef void Short_xyz;		/* none of the ones that use these types.  */
#else						/* ELSE */
  #ifndef VERTICES_H			/* if not flilib code, we need the full    */
	  #include "vertices.h"     /* definitions and types provided by       */
  #endif						/* these header files.					   */
  #ifndef POLY_H
	  #include "poly.h"
  #endif
#endif /* FLILIB_CODE */


/* image structure used for staticly declared icons etc */

typedef struct image {	/* generic bit image structure */
	UBYTE type; 		/* type */
	UBYTE depth;		/* bit depth of image */
	UBYTE *image;		/* the data */
	SHORT width,height; /* size */
} Image;

#define ITYPE_BITPLANES 0
#define IMAGE_INIT1(t,d,i,w,h) {t,d,i,w,h}

/******* raster graphics calls ******/

#ifdef RASTGFX_INTERNALS
	#define RASType Raster
	#define OPTdata void *
#else
	#define RASType void
	#define OPTdata ...
#endif /* RASTGFX_INTERNALS */

/******* image - raster graphics calls ******/

Errcode image_to_rast(Image *i, RASType *rast, Raster *tomatch);
void image_scale_tblit(Image *i,Pixel *colors,
						RASType *dst,Coor dx, Coor dy, Ucoor dw, Ucoor dh);

void blit_image(Image *i,Pixel *colors, RASType *rast, Coor x,Coor y);
void tblit_image(Image *i,Pixel *colors, RASType *rast, Coor x,Coor y);

/******* pure raster graphics calls ******/

void pj_scale_blit(RASType *src,Coor sx, Coor sy, Ucoor sw, Ucoor sh,
				   RASType *dst,Coor dx, Coor dy, Ucoor dw, Ucoor dh,
				   Tcolxldat *tcxl);

				   /* note: if TCXL is NULL blit is solid and untranslated */

void xlat_rast(RASType *r, UBYTE *ttable, LONG pixsize);

Errcode make_cused(RASType *r, UBYTE *c, int max_colors);

void ublitrect(const RASType *s, Coor sx, Coor sy,
		  const RASType *d, Coor dx, Coor dy, Coor width, Coor height,
		  const Pixel tcolor );


void blitmove_rect(RASType *s,Coor sx, Coor sy, RASType *d,
				   Coor dx, Coor dy, Coor width, Coor height);


typedef void (*do_leftbehind_func)(Coor x,Coor y,Coor w,Coor h,OPTdata);
void do_leftbehind(Coor sx,Coor sy,
				   Coor dx,Coor dy,Coor width,Coor height,
				   do_leftbehind_func func,
				   OPTdata);

void set_leftbehind(RASType *s,Pixel color,Coor sx,Coor sy,
					Coor dx,Coor dy,Coor width,Coor height);

#undef RASType
#undef OPTdata

#ifdef GFX_INTERNALS
	#define OPTdata void *dat
#else
	#define OPTdata ...
#endif /* GFX_INTERNALS */

typedef void (*dotout_type)(SHORT x, SHORT y, void *dotdat);

void pj_cline(SHORT x1, SHORT y1, SHORT x2, SHORT y2,
		   dotout_type dotout,
		   OPTdata /* void *datdat */);

void pj_do_linscale(int sx, int sw, int dx, int dw,
					void (*doinc)(int sx, int dx, void *dat), OPTdata );

void pj_make_scale_table(int ssize,int dsize,SHORT *stable);

void cvect(Short_xy *ends, VFUNC dotout, OPTdata /* void *dotdat */ );

void cline_frame(SHORT x0,SHORT y0,SHORT x1,SHORT y1,
		   void (*dotout)(SHORT x,SHORT y,void *dotdat),
		   OPTdata /* void *dotdat */ );

Errcode rcircle(SHORT xcen, SHORT ycen, SHORT rad,
			 void (*dotout)(SHORT x,SHORT y,void *dotdat), void *dotdat,
			 Errcode (*hlineout)(SHORT y, SHORT x1, SHORT x2), void *hlinedat,
			 Boolean filled);

Errcode dcircle(SHORT xcen, SHORT ycen, SHORT diam,
			 void (*dotout)(SHORT x,SHORT y,void *dotdat), void *dotdat,
			 Errcode (*hlineout)(SHORT y, SHORT x1, SHORT x2), void *hlinedat,
			 Boolean filled);
Errcode doval(SHORT xcen, SHORT ycen, SHORT xdiam,
			 SHORT xaspect, SHORT yaspect,
			 void (*dotout)(SHORT x,SHORT y,void *dotdat), void *dotdat,
			 Errcode (*hlineout)(SHORT y, SHORT x1, SHORT x2), void *hlinedat,
			 Boolean filled);

void sq_poly(SHORT w, SHORT h, SHORT x, SHORT y, Short_xy *points);
void rect_to_xyz(Rectangle *r, Short_xyz *dest);

void line(void *r, Pixel color, Coor x1, Coor y1, Coor x2, Coor y2);
void circle(void *r,Pixel color,Coor centx,Coor centy,
			Ucoor diam,Boolean filled);
Errcode polygon(void *r,Pixel color,Short_xy *points,int count,Boolean filled);

#undef OPTdata

/* colormap functions */

int closestc_excl(Rgb3 *rgb, Rgb3 *ctab, int ccount,
				  UBYTE *ignore, int icount);
Boolean inctable(Rgb3 *rgb,Rgb3 *ctab,int count);

Cmap *clone_cmap(Cmap *toclone);
void swap_cmaps(Cmap *a, Cmap *b);
int cmaps_same(Cmap *s1, Cmap *s2);
ULONG cmap_crcsum(Cmap *cmap);
void set_color_rgb(Rgb3 *rgb, USHORT cnum, Cmap *cmap);
void get_color_rgb(USHORT cnum, Cmap *cmap, Rgb3 *rgb);
void stuff_cmap(Cmap *cmap, Rgb3 *color);

/* REXLIB_CODE */ #endif

void pj_cmap_load(void *rast, Cmap *cmap);
void pj_cmap_copy(Cmap *s,Cmap *d);

/* Set a horizontal line on a bitplane.  No clipping, and x1 better
 * be less than or equal to x2! */
void set_bit_hline(unsigned char *buf, 
	unsigned int bpr, unsigned int y, unsigned int x1, unsigned int x2);
extern unsigned char bit_masks[8];

#endif /* GFX_H */
