/* freem.c -
	This module contains routines to pop you into and out of a state which
	maximizes free memory by swapping everything possible out to disk.
	These nest.  Only when the nesting count transitions between 0 and 1 is
	anything actually performed */

#include "flicel.h"
#include "inks.h"
#include "jfile.h"
#include "jimk.h"
#include "mask.h"
#include "memory.h"
#include "pentools.h"
#include "tfile.h"

static char pushed_cel;
static char pushed_alt;
static char pushed_screen;
static char pushed_mask;

Errcode fake_push(void)
{
	pushed_alt = pushed_cel = pushed_screen = pushed_mask = 1;
	return 0; /* allways successful */
}

void free_buffers(void)
/* frees all non-transient buffers */
{
	release_uvfont();
	pj_rcel_free(vl.alt_cel);
	vl.alt_cel = NULL;
	free_the_cel();
	free_the_mask();
}

static Errcode push_alt_id(LONG id)
{
	Errcode err;

	if (pushed_alt == 0) {
		if (vl.alt_cel) {
			if ((err = save_pic(alt_name, vl.alt_cel, id, TRUE)) < 0)
				return (err);
			pj_rcel_free(vl.alt_cel);
			vl.alt_cel = NULL;
		}
	}
	pushed_alt++;
	return 0;
}

/***** screen push pop *****/

static Errcode push_screen_id(LONG timeid)
{
	Errcode err;

	if (pushed_screen == 0) {
		if ((err = save_pic(screen_name, vb.pencel, timeid, TRUE)) < 0)
			return (err);
	}
	pushed_screen++;
	return 0;
}

Errcode pop_screen_id(LONG check_id)
{
	Errcode err;

	if (--pushed_screen == 0) {
		if (pj_exists(screen_name)) {
			err = load_pic(screen_name, vb.pencel, check_id, TRUE);
			if (err < 0)
				return (err);
			pj_delete(screen_name);
			return Success;
		}
	}
	return 1; /* not popped */
}

/******* cel push pop *******/

void fake_push_cel(void)
{
	++pushed_cel;
}

void fake_pop_cel(void)
{
	--pushed_cel;
}

Errcode push_cel(void)
{
	Errcode err;

	if (pushed_cel == 0) {
		if (thecel != NULL && (err = save_fcel_temp(thecel)) < 0)
			return (err);
		free_fcel(&thecel);
	}
	++pushed_cel;
	return (0);
}

Errcode pop_cel(void)
{
	Errcode err;

	if (--pushed_cel == 0) {
		if (!thecel && pj_exists(cel_name)) {
			err = load_temp_fcel(cel_name, &thecel);
			if (err < 0) {
				pj_delete(cel_name); /* cel fli image file may still be there */
				return (err);
			}
		}
	}
	return (0);
}

static int push_mask(void)
{
	Errcode err;

	if (pushed_mask == 0) {
		if (mask_rast) {
			err = save_the_mask(mask_name);
			if (err < Success)
				return (err);
			free_the_mask();
		}
	}
	pushed_mask++;
	return 0;
}

static Errcode pop_alt_id(LONG check_id)
{
	Errcode err;

	pushed_alt -= 1;
	if (pushed_alt == 0) {
		if (!vl.alt_cel && pj_exists(alt_name)) {
			if (alloc_pencel(&vl.alt_cel) >= 0) {
				err = load_pic(alt_name, vl.alt_cel, check_id, TRUE);
				if (err < 0) {
					pj_rcel_free(vl.alt_cel);
					vl.alt_cel = NULL;
					return (err);
				}
				pj_delete(alt_name);
			}
		}
	}
	return 0;
}

Boolean mask_is_present(void)
{
	return (mask_rast != NULL || (pushed_mask < 0 && pj_exists(mask_name)));
}

static SHORT pop_mask(void)
{
	SHORT err;

	pushed_mask -= 1;
	if (pushed_mask == 0) {
		if ((mask_rast == NULL) && pj_exists(mask_name)) {
			err = alloc_the_mask();
			if (err < Success) {
				return err;
			}
			err = load_the_mask(mask_name);
			if (err < Success) {
				free_the_mask();
				return err;
			}
			pj_delete(mask_name);
		}
	}

	return Success;
}

static Errcode push_most_id(LONG id)
{
	Errcode err;
	release_uvfont();

	err = push_alt_id(id);
	if (err < 0) {
		return err;
	}

	err = push_cel();
	if (err < 0) {
		return err;
	}

	return push_mask();
}

Errcode push_most(void)
{
	Errcode ret;
	ret = push_most_id(0);
	return ret;
}

Errcode push_pics_id(LONG time_id)
{
	Errcode err;

	err = push_most_id(time_id);
	if (err < 0) {
		return err;
	}

	return push_screen_id(time_id);
}

void set_trd_maxmem(void) {

}

/*** checker "task" called from within input loop installed by doauto() ***/
void pop_most(void)
{
	grab_uvfont();
	pop_alt_id(0);
	pop_cel();
	pop_mask();
	if (pushed_mask == 0 || pushed_alt == 0 || pushed_cel == 0)
		set_trd_maxmem();
}

static char pshd, dps;

void maybe_push_most(void)
{
	char* pt;
	long cbufsz;

	if (pshd == 0) {
		cbufsz = pj_fli_cbuf_size(vb.pencel->width, vb.pencel->height, vb.pencel->cmap->num_colors);
		pt = pj_malloc(cbufsz);
		if (pt == NULL) {
			dps = 1;
			push_most();
		} else {
			dps = 0;
			pj_free(pt);
		}
	}
	pshd++;
}

void maybe_pop_most(void)
{
	pshd -= 1;
	if (pshd == 0) {
		if (dps) {
			pop_most();
		}
		dps = 0;
	}
}

void push_inks(void)
{
	if (!(vl.ink && (vl.ink->needs & INK_NEEDS_ALT))) {
		push_alt_id(0);
	}
}

void ink_push_cel(void)
{
	if (vs.ink_id != celt_INKID) {
		push_cel();
	}
}

void ink_pop_cel(void)
{
	if (vs.ink_id != celt_INKID) {
		pop_cel();
	}
}

void pop_inks(void)
{
	if (!(vl.ink && (vl.ink->needs & INK_NEEDS_ALT))) {
		pop_alt_id(0);
	}
}
