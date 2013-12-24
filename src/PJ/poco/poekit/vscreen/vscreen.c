/*****************************************************************************
 * vscreen.c - provide access to virtual rasters to a Poco program.
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * include the usual header files...
 *--------------------------------------------------------------------------*/

#include "pocorex.h"    /* required header file, also includes pocolib.h */
#include "errcodes.h"
#include "syslib.h"
#include "gfx.h"

/*----------------------------------------------------------------------------
 * set up the host libraries we need...
 *--------------------------------------------------------------------------*/

#ifndef EMBEDDED_IN_FLICPLAY
	#define HLIB_TYPE_1 AA_POCOLIB	/* this one is always required in a POE */
	#define HLIB_TYPE_2 AA_SYSLIB
	#define HLIB_TYPE_3 AA_GFXLIB
	#include <hliblist.h>
#endif

/*----------------------------------------------------------------------------
 * your data and code goes here...
 *--------------------------------------------------------------------------*/

typedef struct rast_list {
	struct rast_list	*next;
	void				*raster;
	} RastList;

static RastList *rastlist = NULL;

static void add_to_list(void *raster)
/*****************************************************************************
 *
 ****************************************************************************/
{
	RastList *cur;

	if (NULL == (cur = malloc(sizeof(*cur)))) {
		builtin_err = Err_no_memory;
		return;
	}
	cur->raster = raster;
	cur->next = rastlist;
	rastlist = cur;
}

static void free_raster(void *raster)
/*****************************************************************************
 *
 ****************************************************************************/
{
	RastList *cur;
	RastList **prev;

	for (prev = &rastlist, cur = rastlist; cur != NULL; cur = cur->next) {
		if (raster == cur->raster) {
			*prev = cur->next;
			free(raster);
			free(cur);
			return;
		}
		prev = &cur->next;
	}
}

Popot vs_make_centered(Popot proot, int width, int height)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Popot		pvirt;
	Rcel		*root;
	Rcel		*virt;
	Rectangle	rect;

	if (NULL == proot.pt) {
		builtin_err = Err_null_ref;
		return proot;
	} else {
		root = proot.pt;
	}

	if (width <= 0 || height <= 0) {
		builtin_err = Err_parameter_range;
		return proot;
	}

	if (NULL == (virt = malloc(sizeof(*virt)))) {
		builtin_err = Err_no_memory;
		return proot;
	} else {
		pvirt = ptr2ppt(virt, sizeof(*virt));
	}

	add_to_list(virt);

	if (width == root->width && height == root->height) {
		*virt = *root;
	} else {
		rect.x		= (root->width - width) / 2;
		rect.y		= (root->height - height) / 2;
		rect.width	= width;
		rect.height = height;
		pj_rcel_make_virtual(virt, root, &rect);
	}

	return pvirt;

}

Popot vs_make_virtual(Popot proot, int x, int y, int width, int height)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Popot		pvirt;
	Rcel		*root;
	Rcel		*virt;
	Rectangle	rect;

	if (NULL == proot.pt) {
		builtin_err = Err_null_ref;
		return proot;
	} else {
		root = proot.pt;
	}

	if (width <= 0 || height <= 0) {
		builtin_err = Err_parameter_range;
		return proot;
	}

	if (NULL == (virt = malloc(sizeof(*virt)))) {
		builtin_err = Err_no_memory;
		return proot;
	} else {
		pvirt = ptr2ppt(virt, sizeof(*virt));
	}

	add_to_list(virt);

	rect.x		= x;
	rect.y		= y;
	rect.width	= width;
	rect.height = height;
	pj_rcel_make_virtual(virt, root, &rect);

	return pvirt;

}

void vs_free(Popot thescreen)
/*****************************************************************************
 *
 ****************************************************************************/
{
	if (NULL == thescreen.pt) {
		builtin_err = Err_null_ref;
		return;
	}

	free_raster(thescreen.pt);
}

void vs_cleanup_func(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	RastList *cur;
	RastList *next;

	for (cur = rastlist; cur != NULL; cur = next) {
		next = cur->next;
		free_raster(cur->raster);
	}
}

/*----------------------------------------------------------------------------
 * Setup rexlib/pocorex interface structures...
 *--------------------------------------------------------------------------*/

#ifndef EMBEDDED_IN_FLICPLAY
	static Lib_proto poe_calls[] = {
		{ vs_make_centered, "Screen *MakeCenteredScreen(Screen *root, int width, int height);"},
		{ vs_make_virtual,	"Screen *MakeVirtualScreen(Screen *root,  int x, int y, int width, int height);"},
		{ vs_free,			"void   FreeCenteredScreen(Screen *thescreen);"},
		{ vs_free,			"void   FreeVirtualScreen(Screen *thescreen);"},
	};

	Setup_Pocorex(NOFUNC, vs_cleanup_func, "Virtual Screen Manager", poe_calls);
#endif
