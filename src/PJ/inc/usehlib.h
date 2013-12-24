/*****************************************************************************
 * USEHLIB.H	Set up a hostlib structure and link it to the current list.
 *
 * ABSTRACT:
 *				This header file helps automate the process of setting up a
 *				list of hostlib structures used by a rex or rexlib module.
 *				It is intended primarily for automatic inclusion from the
 *				HLIBLIST.H header file, but it can also be included from an
 *				application if the right things are set up before inclusion.
 *
 * ENTRY:
 *				C macro HLIB_TYPE contains the type of library to be set up.
 *				C macro HLIB_LIST contains the name of the last Hostlib
 *				structure set up, or NULL if this is the first library.
 * EXIT:
 *				C macro HLIB_TYPE is undefined.
 *				C macro HLIB_LIST names the structure we just built.
 *
 * DEPENDANCIES:
 *				File REXLIB.H must have been #include'd before the #include
 *				for this header file.
 *
 * MAINTENANCE:
 *	 11/24/90	Created.
 *	 01/15/91	Added new library type AA_MATHLIB.
 ****************************************************************************/

#if HLIB_TYPE == AA_SYSLIB

  Hostlib _a_a_syslib = {HLIB_LIST, AA_SYSLIB, AA_SYSLIB_VERSION};
  #undef  HLIB_LIST
  #define HLIB_LIST &_a_a_syslib

#elif HLIB_TYPE == AA_LOADPATH

  Hostlib _a_a_loadpath = {HLIB_LIST, AA_LOADPATH, AA_LOADPATH_VERSION};
  #undef  HLIB_LIST
  #define HLIB_LIST &_a_a_loadpath

#elif HLIB_TYPE == AA_STDIOLIB

  Hostlib _a_a_stdiolib = {HLIB_LIST, AA_STDIOLIB, AA_STDIOLIB_VERSION};
  #undef  HLIB_LIST
  #define HLIB_LIST &_a_a_stdiolib

#elif HLIB_TYPE == AA_GFXLIB

  Hostlib _a_a_gfxlib = {HLIB_LIST, AA_GFXLIB, AA_GFXLIB_VERSION};
  #undef  HLIB_LIST
  #define HLIB_LIST &_a_a_gfxlib

#elif HLIB_TYPE == AA_POCOLIB

  Hostlib _a_a_pocolib = {HLIB_LIST, AA_POCOLIB, AA_POCOLIB_VERSION};
  #undef  HLIB_LIST
  #define HLIB_LIST &_a_a_pocolib

#elif HLIB_TYPE == AA_MATHLIB

  Hostlib _a_a_mathlib = {HLIB_LIST, AA_MATHLIB, AA_MATHLIB_VERSION};
  #undef  HLIB_LIST
  #define HLIB_LIST &_a_a_mathlib

#else
  #error USEHLIB.H: Unknown library type assigned to HLIB_TYPE.
#endif

#undef HLIB_TYPE	/* undef it so it can be #define'd again in main source */
