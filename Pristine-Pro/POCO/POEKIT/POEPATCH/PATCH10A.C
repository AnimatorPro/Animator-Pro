/*****************************************************************************
 * patch10a.c - Routines to apply patches & lib extensions to Ani Pro v1.0.
 *
 *	This module requires a POCOLIB.H file dated 01/13/92 or later.
 ****************************************************************************/

#pragma aux int3 = 0xCC parm caller [] modify exact []; /* for debugging */

/*----------------------------------------------------------------------------
 * include the usual header files...
 *--------------------------------------------------------------------------*/

#include "pocorex.h"    /* required header file, also includes pocolib.h */
#include "errcodes.h"   /* most POE programs will need error codes info  */

/*----------------------------------------------------------------------------
 * code and data here...
 *--------------------------------------------------------------------------*/

#define BOX_BEVEL_FIELD (*(SHORT *)&(_plptr->vs[0x0310]))

static void (*noask_delete_the_cel)(void) = NULL;

#define OFFSET_FROM_RENDERFUNC_TO_DELETEFUNC (-(0x17F0))

/*----------------------------------------------------------------------------
 * New po_get_libproto_line() function data...
 *
 *	This array contains a new version of the po_get_libproto_line() function,
 *	encoded as an array of bytes.  This array is the binary code generated
 *	from the patchglp.asm routine.	It completely replaces the function of
 *	the same name in Ani Pro v1.0.
 *
 *	This patch fixes the bug that prevented the use of multiple POE modules
 *	in a Poco program, by replacing an internal routine in the Poco compiler.
 *--------------------------------------------------------------------------*/

static char new_get_libproto[] = {
	0x53, 0x8B, 0x5C, 0x24, 0x08, 0x8B, 0x43, 0x6B, 0x8B, 0x50, 0x04, 0x8B,
	0x48, 0x0C, 0x3B, 0x4A, 0x0C, 0x7C, 0x1B, 0x83, 0x7B, 0x38, 0x00, 0x75,
	0x06, 0x8B, 0x12, 0x85, 0xD2, 0x75, 0x07, 0x33, 0xC0, 0x89, 0x43, 0x58,
	0xEB, 0x18, 0x89, 0x50, 0x04, 0x33, 0xC9, 0x89, 0x48, 0x0C, 0xFF, 0x40,
	0x0C, 0x8B, 0x42, 0x08, 0x8B, 0x14, 0xC8, 0x89, 0x53, 0x58, 0x8B, 0x44,
	0xC8, 0x04, 0x5B, 0xC3, 0x70, 0x61, 0x74, 0x63, 0x68, 0x67, 0x6C, 0x70,
	};

#define OFFSET_FROM_FINDPOE_TO_GETLIBPROTO 0x010D
#define OFFSET_TO_PATCH_MAGIC			   0x0040
#define PATCH_MAGIC_0					   0x63746170
#define PATCH_MAGIC_1					   0x706C6768

Errcode init_patches_10a(void)
/*****************************************************************************
 * init patches for known bugs as of 01/13/92.
 *	 this routine must be executed as part of the rex-layer init() function.
 *	 if you have your own init() function already, just code a call to this
 *	 function as the first thing you do in your init routine.  if you don't
 *	 have a specific init function of your own, just name this routine as
 *	 the rex-layer init function in your Setup_Pocorex() macro.
 ****************************************************************************/
{
	char	*workptr;
	long	worklong;
	char	*old_get_libproto;
	long	*patch_magic;
	int 	hostversion;

	hostversion = poePocoVersion();

	/*------------------------------------------------------------------------
	 * the poeCelRelease function needs a patch that lets po_release_cel()
	 * (coded below) work properly with either an early or a recent host.
	 * this block gets the pointer from the AA_POCOLIB interface for recent
	 * host versions, or from inside the host code for earlier hosts.
	 *
	 * for early hosts, where we have to snoop out the pointer from inside
	 * an existing routine, what we're really doing is pulling the offset
	 * out of an 80386 CALL NEAR instruction.  the offset is relative to
	 * the start of the next instruction, so we pull the offset, add four to
	 * it (to get to the next instruction), then add it to the address where
	 * we pulled the offset from, to convert the offset to a regular pointer.
	 * all of that gives us a pointer to render_thecel(), a function inside
	 * the FLICEL.C host module.  at a known offset from that function is the
	 * noask_delete_the_cel() function, so we add that known offset value to
	 * get the final pointer we really need.
	 *----------------------------------------------------------------------*/

	if (hostversion >= 179) {
		noask_delete_the_cel = _plptr->plcel->plCelRelease;
	} else {
		workptr  = (void *)(((char *)_plptr->plcel->plCelPaste) + 6);
		worklong = *(long *)workptr;
		workptr += worklong + 4;
		noask_delete_the_cel = (void *)(workptr + OFFSET_FROM_RENDERFUNC_TO_DELETEFUNC);
	}

	/*------------------------------------------------------------------------
	 * remaining patches are only needed in early versions of AniPro/Poco...
	 *----------------------------------------------------------------------*/

	if (hostversion >= 179)
		goto PUNT;

	/*------------------------------------------------------------------------
	 * Ani Pro v1.0 didn't include the vb and vs 'backdoor' pointers in the
	 * AA_POCOLIB interface structure.	these pointers let us add new
	 * functions to older versions of anipro, and also to init certain
	 * bugfixes.  the following code pulls these pointers out of the anipro
	 * host code by using known offsets from existing functions.  basically,
	 * the GetDot and GetColor functions contain references to fields in the
	 * vb and vs structures, so we go into the code and extract the pointers.
	 * since the pointers we extract point to fields within the structures,
	 * we subtract a constant value to make pointers to the beginning of the
	 * structures.	the contents of the vb and vs structures are NOT mapped
	 * out in any header file or documented anywhere.  these structures are
	 * NOT for general use in arbitrary POE modules, they are just for use
	 * by those of us who have the anipro source code, as a way of issuing
	 * field-patches and library extensions to early versions of anipro.
	 *----------------------------------------------------------------------*/

	workptr    = *(void **)(((char *)_plptr->pldraw->plGetDot) + 11);
	_plptr->vb = workptr - 0x006A;

	workptr    = *(void **)(((char *)_plptr->pldraw->plGetColor) + 3);
	_plptr->vs = workptr - 0x000A;

	/*------------------------------------------------------------------------
	 * fix the multiple-poe-inclusion bug...
	 *
	 * the routine we need to patch, po_get_libproto_line(), lives at a
	 * known fixed offset from the po_findpoe() function inside Ani Pro.
	 * we have a pointer to the findpoe function via the AA_POCOLIB host
	 * interface, so we add the known offset to that pointer to get a pointer
	 * to the function that needs to be replaced.
	 *
	 * at the end of the replacement code is an 8-byte magic value.  we calc
	 * a pointer to it, and see if the magic value is there.  if it is,
	 * that means another POE module (loaded before us) has already applied
	 * the patches, and we can just punt.
	 *
	 * init the patch by copying the new binary code over the old.
	 *----------------------------------------------------------------------*/

	old_get_libproto  = (char *)_plptr->pl_findpoe;
	old_get_libproto += OFFSET_FROM_FINDPOE_TO_GETLIBPROTO;
	patch_magic = (long *)(old_get_libproto + OFFSET_TO_PATCH_MAGIC);

	if (patch_magic[0] == PATCH_MAGIC_0 && patch_magic[1] == PATCH_MAGIC_1)
		goto PUNT;

	memcpy(old_get_libproto, new_get_libproto, sizeof(new_get_libproto));

PUNT:

	return hostversion;
}

int po_get_boxbevel(void)
/*****************************************************************************
 * int GetBoxBevel(void)
 *	 return the current bevel setting for the Box() builtin function/tool.
 ****************************************************************************/
{
	if (_plptr->vs == NULL)
		init_patches_10a();
	return BOX_BEVEL_FIELD;
}

void po_set_boxbevel(int newbevel)
/*****************************************************************************
 * void SetBoxBevel(int newbevel)
 *	 set the new bevel for drawing via the Box() builtin function/tool.
 ****************************************************************************/
{
	if (_plptr->vs == NULL)
		init_patches_10a();
	if (newbevel < 0) {
		builtin_err = Err_parameter_range;
		return;
	}

	BOX_BEVEL_FIELD = newbevel;
}

void po_release_cel(void)
/*****************************************************************************
 * void CelRelease(void)
 *	 release the current cel, and all resources it holds.
 ****************************************************************************/
{
	if (noask_delete_the_cel == NULL) {
		init_patches_10a();
	}
	noask_delete_the_cel();
}

