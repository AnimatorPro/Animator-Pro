#ifndef REXLIB_H
#define REXLIB_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

/* header used on library vector table structures declared in host code 
 * and provided to rex code in "next" field of Hostlib headers */

typedef struct libhead {
	LONG size;
	USHORT type;
	USHORT version;
} Libhead;

#ifndef REXLIB_INTERNALS

extern Libhead aa_syslib; /* aasyslib.obj */
extern Libhead aa_gfxlib; /* aagfxlib.obj */
extern Libhead aa_stdiolib; /* stdiolib.obj */
extern Libhead aa_pocolib; /* porexlib.obj */
extern Libhead aa_mathlib; /* mathhost.obj */

#else /* REXLIB_INTERNALS */

/*******************************************************/
/* headers as declared and un-loaded in rex library code 
 * the "next" pointers are replaced with the pointers to the Libheads
 * of the appropriate version and type in load_rexlib() */

typedef struct hostlib {
	void *next;  /* pointer to next as declared Libhead pointer as loaded */
	USHORT type;  /* type and version of library desired by rexlib */ 
	USHORT version; 
} Hostlib;

/* HLIB_OPTIONAL when specified in a rexlib version field will make the load
 * of the library optional and the host will set the "next" field to NULL
 * if the library is unavailable calls to the library will not function
 * if the library should not be used if the library isn't loaded or 
 * severe crash you can test the value of the next field for NULL to see
 * if the library is or isn't loaded */

#define HLIB_OPTIONAL 0x8000

/* HLIB_LIST can be used as an initialization value for the first_hostlib
 * field in the rexlib_header structure.  Initially, this value is NULL,
 * but if the hliblist.h or usehlib.h header files are used, they will
 * redefine HLIB_LIST as the Hostlib linklist is being built.  (See
 * comments in hliblist.h for more details.)
 */

#define HLIB_LIST NULL

/* pj library types 
 *    (11/24/90 - Changed from enum to #define's for use with hliblist.h 
 */

#define	AA_NOT_AVAILABLE    0  	/* library is not available used for host stubs 
								 * loader will ignore this type. */
#define	AA_SYSLIB           1  	/* low level memory and file utilities */
#define	AA_LOADPATH         2	/* special "library" for path used to load
								 * rex code into program */
#define	AA_STDIOLIB 	    3  	/* standard io library calls */
#define	AA_GFXLIB 		    4  	/* low level raster graphics calls */
#define	AA_POCOLIB 		    5  	/* library passed in to poco */
#define AA_MATHLIB          6   /* floating point math library */

#define AA_SYSLIB_VERSION	0
#define AA_LOADPATH_VERSION	0
#define AA_STDIOLIB_VERSION	0
#define AA_GFXLIB_VERSION	0
#define AA_POCOLIB_VERSION	0
#define AA_MATHLIB_VERSION	0


/********** aa rexlib code rondevous with host provided librarys **********
 *
 *    In order to use host provided librarys a "Hostlib" header must be
 * initialized and put in a linked list of hostlibs that has it's head 
 * address put in the field "first_hostlib" of the rexlib_header in the 
 * loaded code.  The first_hostlib should be set to NULL if no librarys are
 * needed.  
 * 
 *    Do not declare libraries that are not needed by your application
 * so the resources will not be allocated by the host. 
 * 
 *    A sample declaration is below here three librarys are linked 
 * _a_a_loadpath is the last one with it's next field set to NULL. 
 * _a_a_syslib is the first library in the list and it's address is placed in 
 * the header's first_hostlib field. The _a_a_gfxlib is optional in that the
 * host will load your code anyway even if it can't provide the library 
 * requested.
 *
 *    It is up to the application to check the _a_a_gfxlib.nex field for
 * non NULL before attmpting to use any calls found in the _a_a_gfxlib.
 * 
 * 
 * Hostlib _a_a_loadpath = { NULL, AA_LOADPATH, AA_LOADPATH_VERSION};
 * 
 * Hostlib _a_a_gfxlib	 = { &_a_a_loadpath, AA_GFXLIB, 
 * 								(AA_GFXLIB_VERSION|HLIB_OPTIONAL)};
 * 
 * Hostlib _a_a_syslib	 = { &_a_a_gfxlib, AA_SYSLIB, AA_SYSLIB_VERSION};
 * 
 * 
 * Rexlib rexlib_header = { rextype, rexversion, init, cleanup, &_a_a_syslib};
 * 
 * 
 *     If you are unsure of what librarys you need,
 * link your rex code without declaring any and you'll find out.
 * 
 *****************************************/


#endif /* REXLIB_INTERNALS */

/* rexlib header structure, used as header to code specific structure 
 * this is returned by the Rex_entry. It should have all rex library
 * loaded values present after the entry point is called. If the code
 * rexentry.asm is used it should be staticly loaded and be the global
 * symbol rexlib_header declared in the rex code */

typedef struct rexlib {
	USHORT type;      /* rex library type pre-initialized in rex code The call
	 				   * by the host load_rexlib() checks this against type 
					   * input to load_rexlib() a mis match will cause load to
					   * abort */

	USHORT version;   /* rex library version pre-initialized in rex code 
					   * the host will check for this ignored by the loader 
					   * for use in library specific protocall */

	EFUNC init;	  	  /* pointer to init vector pre-initialized in rex code
					   * called after code is loaded and host librarys
					   * initialized may be altered by rexlib code but
					   * with Caution: it is not called by the loader and 
					   * must match library specific host protocall may be
					   * NULL if not needed */

	VFUNC cleanup; 	  /* pointer to cleanup vector pre-initialized in rex code 
					   * only called immediately before code is unloaded may
					   * may be altered by rexlib code at any time 
					   * may be NULL if not desired */

	void *first_hostlib; /* list of Hostlib areas for host loaded librarys, 
						  * staticly pre-initialized in rex code,
						  * list not valid after rexlib is loaded. Not to 
						  * be altered by rex code. It is used by the host
						  * loader code. May be set NULL if no librarys 
						  * are needed */

	char *id_string; /* initialized by rexlib code to point to the id 
					  * string. 
					  * If the type is REX_USERTYPE the string must be present
					  * and may contain any sequence of '\0' terminated bytes
					  * If you like copyright notices in code this would be 
					  * a good place to use a terse version, since the host
					  * must have a copy to use the loader.  This must match
					  * byte for byte with an equivalent string passed in to
					  * load_rexlib() or the loader will abort error, 
					  * even if type is not REX_USERTYPE */

	void *host_data; /* This field is used by the host 
					  * NOT to be touched by the rexlib code. Since only the
					  * loader will use this field a NULL here may be used 
					  * to flag that this is a "linked" in library and not 
					  * allocated and loaded from a file */
} Rexlib;

/* open-load and free of rex file simply loads code block and returns entry
 * point vector */

/* note if prexlib is NON NULL this will attempt to verify it is a PJ rex
 * library and load the pointer with a pointer to the rexlib_header */

Errcode pj_rex_load(char *fname, void **entry,void **prexlib);
void pj_rex_free(void **entry);

/* open-load, init, and close animator format rex librarys using rexentry.obj
 * and declaring a rexlib_header */

Errcode pj_rexlib_load(char *name, USHORT type, Rexlib **prl,
					   Libhead **hostlibs, char *id_string);

#ifdef REXLIB_INTERNALS
Errcode pj_rexlib_init(Rexlib *rl,void *data);
#else
	Errcode pj_rexlib_init(Rexlib *rl,...);
#endif

void pj_rexlib_free(Rexlib **prl);

/* rex library soft menu key text handling is based on a prefix char.
 * to make a keytext followed by an imbedded text as in:
 *
 * rextext = RL_KEYTEXT("key_name")"Imbedded text to follow key";
 */

#define RL_KEYTEXT(kt) "\033"kt"\0"
#define RL_ISKEYTEXT(k) (k[0] == '\033')

#ifndef REXLIB_CODE
char *rex_key_or_text(char *kort,char **pname); /* in host lib */
#endif

#ifdef __WATCOMC__
#ifdef REXLIB_CODE
#pragma aux rexlib_header "*";
#endif
#endif

/* rex library types recognized by pj */

#define REX_USERTYPE 	0  /* for use outside of pj domain. Forces id 
						    * string to be checked by loader */

#define REX_VDRIVER  	0x0101U
#define REX_INK      	0x0201U
#define REX_IDRIVER  	0x0301U
#define REX_PICDRIVER 	0x0401U
#define REX_POCO		0x0501U
#define REX_TRUEPDR		0x0601U


#ifdef REXLIB_INTERNALS

/* The (*Rexlib.init)() may be called by using the init_rexlib(rl) call.
 * Init_rexlib() will put the host_program, and host_version of the
 * parent program into the two items on the stack of the init entry point
 * If you wish to use other arguments or provide your own version and type
 * you should call the init entry point from the header of the loaded rexlib
 * code directly. */

/* typedef of header (*init)() entry point as called by init_rexlib() */

typedef Errcode (*Rexlib_init)(Rexlib *rl,
							   USHORT host_program,USHORT host_version,
							   void *data);

/* typedef of (*cleanup)() as called by free_rexlib() */

typedef void (*Rexlib_cleanup)(Rexlib *rl);


#endif /* REXLIB_INTERNALS */


#endif /* REXLIB_H */
