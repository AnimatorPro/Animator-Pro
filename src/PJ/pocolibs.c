/*****************************************************************************
 * pocolib.c - Support for poco's builtin libraries, which also constitutes
 *			   the AA_POCOLIB host library for POE modules.
 ****************************************************************************/
#define REXLIB_INTERNALS
#include "errcodes.h"
#include "memory.h"
#include "ptrmacro.h"
#include "jimk.h"       /* import vb and vs declarations */
#include "rexlib.h"
#include "pocorex.h"
#include "pocolib.h"
#include "pocoface.h"
#include "poly.h"

extern Boolean po_check_abort(void *data);

static Poco_lib *poco_libs[] =
	{
	&po_user_lib,
	&po_draw_lib,
	&po_text_lib,
	&po_mode_lib,
	&po_turtle_lib,
	&po_time_lib,
	&po_cel_lib,
	&po_alt_lib,
	&po_optics_lib,
	&po_blit_lib,
	&po_misc_lib,
	&po_load_save_lib,
	&po_FILE_lib,
	&po_str_lib,
	&po_mem_lib,
	&po_math_lib,
	&po_dos_lib,
	&po_globalv_lib,
	&po_title_lib,
	&po_tween_lib,
	&po_flicplay_lib,
	};


Poco_lib *get_poco_libs()
/*****************************************************************************
 *
 ****************************************************************************/
{
	static Poco_lib *list = NULL;
	int i;

	if (list == NULL)
		{
		for (i=Array_els(poco_libs); --i >= 0; )
			{
			poco_libs[i]->next = list;
			list = poco_libs[i];
			}
		}
	return(list);
}

/*****************************************************************************
 * some routines used by more than one poco library...
 ****************************************************************************/

extern Popot poco_lmalloc(long size);
extern void po_free(Popot ppt);

static void poly_to_arrays(Poly *p, int *x, int *y)
/*****************************************************************************
 * Convert a polygon to two int arrays
 ****************************************************************************/
{
	int i;
	LLpoint *pt;

	i = p->pt_count;
	pt = p->clipped_list;
	while (--i >= 0)
		{
		*x++ = pt->x;
		*y++ = pt->y;
		pt = pt->next;
		}
}

Errcode po_poly_to_arrays(Poly *p, Popot *px, Popot *py)
/*****************************************************************************
 * Convert a polygon to two poco int arrays
 ****************************************************************************/
{
	Popot x,y;
	long acount;

	acount = p->pt_count*sizeof(int);
	x = poco_lmalloc(acount);
	if (x.pt == NULL)
		return(Err_no_memory);
	y = poco_lmalloc(acount);
	if (y.pt == NULL)
		{
		po_free(x);
		return(Err_no_memory);
		}
	poly_to_arrays(p, x.pt, y.pt);
	*px = x;
	*py = y;
	return(Success);
}

Errcode po_2_arrays_check(int ptcount, Popot *px, Popot *py)
/* Make sure that 2 poco arrays are big enough to hold ptcount ints. */
{
	int bsize;

	if ((px->pt) == NULL || (py->pt) == NULL)
		return(builtin_err = Err_null_ref);
	bsize = ptcount*sizeof(int);
	if (Popot_bufsize(px) < bsize || Popot_bufsize(py) < bsize)
		{
		return(builtin_err = Err_index_big);
		}
	return Success;
}

Errcode po_arrays_to_poly(Poly *p, int ptcount, Popot *px, Popot *py)
/*****************************************************************************
 * This creates a Poly quickly from Poco array representation.
 * However this poly needs to be disposed with free(p->clipped_list)
 * rather than freeing each point individually.
 ****************************************************************************/
{
	LLpoint *list;
	int i;
	int *x;
	int *y;
	Errcode err;

	if ((err = po_2_arrays_check(ptcount, px, py)) < Success)
		return err;
	x = px->pt;
	y = py->pt;
	if((list = p->clipped_list = begmem(ptcount * sizeof(LLpoint))) == NULL)
		return(Err_no_memory);
	i = p->pt_count = ptcount;
	while (--i >= 0)
		{
		list->x = *x++;
		list->y = *y++;
		list++;
		}
	p->polymagic = POLYMAGIC;
	linkup_poly(p);
	return(Success);
}

Errcode po_arrays_to_ll_poly(Poly *poly, int ptcount, Popot *px, Popot *py)
/*****************************************************************************
 * This creates a Poly quickly from Poco array representation.
 * It creates each point individually.
 ****************************************************************************/
{
	Poly lpoly;
	Errcode err;

	if ((err = po_arrays_to_poly(&lpoly, ptcount, px, py)) < Success)
		return err;
	err = clone_ppoints(&lpoly, poly);
	poly->polymagic = POLYMAGIC;
	pj_free(lpoly.clipped_list);
}


static void *get_pic_screen(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	return(vb.pencel);
}

static int get_menu_colors(Pixel **indicies, Rgb3 **lastrgbs, Rgb3 **idealrgbs)
/*****************************************************************************
 *
 ****************************************************************************/
{

	if (indicies != NULL)
		*indicies = &vb.screen->mc_colors;
	if (lastrgbs != NULL)
		*lastrgbs = &vb.screen->mc_lastrgbs;
	if (idealrgbs != NULL)
		*idealrgbs = vb.screen->mc_ideals;
	return NUM_MUCOLORS;

}

/*****************************************************************************
 *
 ****************************************************************************/

Porexlib aa_pocolib = {
	/* header */
	{sizeof(Porexlib), AA_POCOLIB, AA_POCOLIB_VERSION},
	&builtin_err,
	get_pic_screen,
	po_ppt2ptr,
	po_ptr2ppt,
	get_menu_colors,
	po_findpoe,
	po_poe_overtime,
	po_check_abort,
	po_poe_oversegment,
	po_poe_overall,
	&vb,
	&vs,
	{NULL,NULL,NULL,NULL},	/* reserved1[4] */
	&po_libuser,
	&po_liboptics,
	&po_libswap,
	&po_libscreen,
	&po_libcel,
	&po_libdos,
	&po_libdraw,
	&po_libaafile,
	&po_libmisc,
	&po_libmode,
	&po_libtext,
	&po_libtime,
	&po_libturtle,
	&po_libglobalv,
	&po_libtitle,
	&po_libtween,
	&po_libflicplay,
	{NULL}, 				/* reserved2[1] */
	};

