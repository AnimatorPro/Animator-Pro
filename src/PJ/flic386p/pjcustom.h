/*****************************************************************************
 * PJCUSTOM_H - Datatypes and prototypes for implementing custom rasters.
 *
 *	Note:  The PJSTYPES.H or STDTYPES.H file must be included before this
 *		   this file.  Don't code an include for it below, or the glue code
 *		   will break!
 ****************************************************************************/

#ifndef PJCUSTOM_H
#define PJCUSTOM_H

/*----------------------------------------------------------------------------
 * First a list of all the function pointer types that make up the
 * custom library...
 *--------------------------------------------------------------------------*/

typedef void  (*put_dot_type)(FlicRaster *r,long x,long y,Pixel color);
typedef Pixel (*get_dot_type)(FlicRaster *f,long x,long y);
typedef void  (*set_hline_type)(FlicRaster *f,Pixel color,long x,long y,long w);
typedef void  (*put_hseg_type)(FlicRaster *f,Pixel *pixbuf,long x,long y,long w);
typedef void  (*get_hseg_type)(FlicRaster *r,Pixel *pixbuf,long x,long y,long w);
typedef void  (*set_colors_type)(FlicRaster *f,long start,long count,unsigned char *cbuf);
typedef void  (*wait_vsync_type)(FlicRaster *f);

/*----------------------------------------------------------------------------
 * The RastlibCustom type...
 *
 *	This datatype provides a list of pointers to client-code routines that
 *	implement the basic raster I/O functions.  The client code can load the
 *	pointers into this structure, then pass it to pj_raster_make_custom()
 *	to have a complete raster bound together with generics being used for
 *	the functions not in this list.
 *--------------------------------------------------------------------------*/

/* Then the custom library structure itself. */
typedef struct rastlib_custom {
	put_dot_type	put_dot;
	get_dot_type	get_dot;
	set_hline_type	set_hline;
	put_hseg_type	put_hseg;
	get_hseg_type	get_hseg;
	set_colors_type set_colors;
	wait_vsync_type wait_vsync;
	} RastlibCustom;

/*----------------------------------------------------------------------------
 * Custom raster functions...
 *--------------------------------------------------------------------------*/

Errcode 	pj_raster_make_custom(FlicRaster **pprast,
							int width, int height, RastlibCustom *lib);
Errcode 	pj_raster_free_custom(FlicRaster **pprast);

Errcode 	pj_raster_make_compress_only(FlicRaster **pprast,
							int width, int height,
							get_hseg_type get_hseg_function);
Errcode 	pj_raster_free_compress_only(FlicRaster **pprast);

/*----------------------------------------------------------------------------
 * these pragmas allow -3r clients to use our -3s style functions...
 *	 the FLICLIB3S alias is defined in PJSTYPES.H.
 *--------------------------------------------------------------------------*/

#ifdef __WATCOMC__
	#pragma aux (FLICLIB3S) pj_raster_make_custom;
	#pragma aux (FLICLIB3S) pj_raster_free_custom;
	#pragma aux (FLICLIB3S) pj_raster_make_compress_only;
	#pragma aux (FLICLIB3S) pj_raster_free_compress_only;
#endif

#endif /* PJCUSTOM_H */
