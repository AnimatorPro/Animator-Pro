
#ifndef POCOFACE_H
#define POCOFACE_H

#ifndef POCOLIB_H
	#include "pocolib.h"
#endif

#define POCO_STACKSIZE_MIN		(6*1024L)
#define POCO_STACKSIZE_MAX		(64*1024L)
#define POCO_STACKSIZE_DEFAULT	(10*1024L) /* default poco runtime stacksize */

extern int	po_version_number;	/* added 10/30/90, poco's version number */

Errcode compile_poco(void **ppev,	/* returns executable pexe on Success */
	char *source_name,	/* name of source file */
	char *errors,		/* error file or NULL for stderr */
	char *dump_name,	/* disassembly file or NULL for none */
	/* for built-in function library */
	Poco_lib *lib,
	/* stuff for location of 1'st error */
	char *err_fname,	/* file where error detected */
	long *err_line, 	/* line where error detected */
	int *err_char,		/* character in line where err detected */
	Names *include_dirs /* include search path */
	);
/* Compile poco function.  Leave error messages in a file named errors.
   Otherwise build up executable structure in *ppev */

Errcode run_poco(void **ppev,  /* value from compile_poco */
	char *trace_name,
	Boolean (*check_abort)(void *),
	void *check_abort_data,
	long *err_line);
/* run_poco:  execute *ppev starting at main() */

void free_poco(void **ppev);
/* free_poco: free up ppev returned by compile_poco and set *pev to NULL */


/**** These next functions are for when you want to run a function
   (not necessarily main) inside a compiled poco program.
   (DON'T USE THEM!  Running something other than main() is untested) */

Errcode pev_alloc_data(void *p);
/* allocate and initialize data areas */

void pev_free_data(void *p);
/* free data areas */


/* Routines useful for calling a specific function in a Poco program
 * in C. */
void *po_fuf_code(void *fuf);
char *po_fuf_name(void *fuf);



#endif /* POCOFACE_H */
