#include "errcodes.h"
#include "libdummy.h"
#include "memory.h"
#include "rastcall.h"
#include "rastlib.h"

static Pixel not_implemented()
{
	errline(Err_unimpl,"Invalid call in custom raster");
	return(0);
}
Errcode pj_rast_custom_open(Rasthdr *spec, Raster *cr, Grctype grc_lib_type)

/* this is called to build a custom raster from a library provided by the 
 * user in the spec structure. Calls implemented by the caller are put in the
 * library. The other calls in the library must be set to NULL. They are 
 * replaced by the generic functions or a function that reports an error 
 * if called. The vertical blank, hardware color loading, and close_raster
 * functions do not report error if not included but do nothing.
 *
 * 	   The extent of the hierachical "generic" library calls installed is 
 * determined by the Grctype field.  This is actually a pointer to a global
 * function that loads the appropriate calls into a library.  Multiple rasters
 * may be called with the same library and all will reference it. 
 *
 *		The library is not copied, but is used in the raster and must not be 
 * altered or freed when any raster using it is being used. If you want the 
 * close_raster call in the library can be used to free your custom library 
 * if allocated and any other cleanup operations desired.
 *
 *      All custom rasters should
 * set the type to a value >= RT_FIRST_CUSTOM.  All custom rasters using
 * the same library should have the same type.  Those that do not should 
 * have unique types The hardware portion of the raster is left unaltered and
 * is free for use by the caller. pixel depth if 0 will be set to 8,
 * if either aspect_dx or aspect_dy is 0 both will be set to 1 and 1 */
{
Rastlib buildlib;
union {
	Grctype type;
	void (*load_lib)(void *lib);
} gtype;

	gtype.type = grc_lib_type;

	if(gtype.type == NULL || spec->lib == NULL 
		|| ((USHORT)(spec->pdepth)) > 8
		|| !spec->width || !spec->height
		|| spec->type < RT_FIRST_CUSTOM )
	{
		return(Err_bad_input);
	}

	pj_stuff_pointers(not_implemented,&buildlib,NUM_LIB_CALLS);
	buildlib.close_raster = (rl_type_close_raster)pj_errdo_success;
	buildlib.wait_vsync = (rl_type_wait_vsync)pj_vdo_nutin;

	if(cr->lib->set_colors == NULL
		|| cr->lib->set_colors == pj_vdo_nutin)
	{
		buildlib.set_colors = (rl_type_set_colors)pj_vdo_nutin;
	    buildlib.uncc64 = (rl_type_uncc64)pj_vdo_nutin;
		buildlib.uncc256 = (rl_type_uncc256)pj_vdo_nutin;
	}

	(*gtype.load_lib)(&buildlib); /* if user puts bad data in here BOOM */
	pj_load_array_nulls((void **)&buildlib,(void **)(cr->lib),NUM_LIB_CALLS);
	copy_rasthdr(spec,cr); /* copy in header spec */

	if(cr->pdepth == 0)
		cr->pdepth = 8;
	if(cr->aspect_dy == 0 || cr->aspect_dx == 0)
		cr->aspect_dy = cr->aspect_dx = 1;

	return(Success);
}
