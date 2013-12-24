/*****************************************************************************
 *
 * pocoface.c - interface between poco and programs calling poco.
 *
 *	Routine to compile a source file and pass back an executable
 *	memory structure (compile_poco),  execute that structure (run_poco)
 *	and free it up (free_poco).   Most of the work of this module
 *	is converting the function library passed into compile_poco into
 *	the rather complex type-structures used internally.
 *
 * MAINTENANCE:
 *	08/18/90	(Ian)
 *				Added detection and handling of a NULL library list pointer
 *				in compile_poco().	(OOOPS, except it doesn't work right.
 *				actually, poco runs fine this way standalone, but it locks
 *				up the machine if run under TD or TPROF.  I hope this isn't
 *				a symptom of some deeper problem.)
 *	08/27/90	(Ian)
 *				Added calls to init and free the new memory management system.
 *	08/29/90	(Ian)
 *				Added setjmp/longjmp error handling to poco.  This largely
 *				affected the compile_poco() routine; most error handling and
 *				cleanup actions now occur therein.	Also, new logic was added
 *				to close all files in the file_stack linked list during error
 *				handling, (old version closed the most recently opened file).
 *	08/30/90	(Jim)
 *				Added file close call for pcb->t.err_file in compile_poco().
 *	09/04/90	(Ian)
 *				Ooops.	Turns out po_free_pp() contains the logic to close all
 *				open files.  So, the loop I coded to do that was removed,
 *				and a po_free_pp() call was inserted.
 *	10/21/90	(Ian)
 *				New interface to library prototype data.  Deleted routines
 *				pod_init(), pod_next(), prol() and fixup_lib_symbols(),
 *				replaced them with new routine poc_get_libproto_line().
 *	10/25/90	(Ian)
 *				Reworked the library prototype interface some more, added new
 *				routine po_open_library(), but we still need a function on
 *				the PJ side to actually load libraries for us.	Right now,
 *				the open library function handles only the builtin libraries
 *				passed to us by pj at compile time.
 *	04/14/91	(Peter)
 *				Modified po_open_library to take id_string argument
 *	05/01/91	(Ian)
 *				Added builtin_err declaration.	This global variable is now
 *				owned by poco instead of by the host, because we need to use
 *				it during the compile phase to detect math errors during the
 *				folding of constants.  Also, run_poco() was changed to
 *				remove the pointer to builtin_err that used to get passed in.
 *	01/10/92	(Ian)
 *				Major bugfix in po_get_libproto_line(), to fix the bug that
 *				prevented using multiple #pragma library statements.  See
 *				the comments in that function's header block for details.
 *	05/17/92	(Ian)
 *				Added a sanity check to po_get_libproto_line() to catch
 *				NULL proto string pointers.
 *	09/17/92	(Ian)
 *				Tweaked logic used to report the error file name and line
 *				number back to the host.  We used to return the line number
 *				from curtoken, but if the error was in a preprocessor
 *				statement this could be wildly innacurate.	Now, we have a
 *				new field in the pcb that carries the error line number;
 *				it is set by the error reporting routines in either the
 *				parser or the preprocessor.  Also, if an error happens
 *				in parsing prototypes in a library, we now report the file
 *				and line number of the #pragma poco library statement.
 *				And finally, restored handling of the character position
 *				on the line where the error was detected.  Provisions for
 *				this existed in the tokenizer, but it wasn't being used.
 ****************************************************************************/

#include <string.h>
#include <setjmp.h>
#include "poco.h"
#include "errcodes.h"
#include "pocoface.h"
#include "pocorex.h"

/*****************************************************************************
 * some global vars...
 *	 po_compile_errhandler has (by convention) a scope that is internal to
 *	 the poco compiler.  the other globals are shared with the host.
 ****************************************************************************/

jmp_buf po_compile_errhandler;	/* Global jump buffer for compile errors.	*/
int 	po_version_number = VRSN_NUM; /* Global version number for PJ's use.    */
extern Errcode builtin_err; 	/* External library/floating point error.	*/

static Poco_run_env *porunenv;	/* -> run env; valid only when poco pgm running */

#ifdef DEBUG
void po_show_basic_sizes()
/*****************************************************************************
 *
 ****************************************************************************/
{
boxf(
	"Symbol.......%5d\n"
	"Type_info....%5d\n"
	"Code_buf.....%5d\n"
	"Exp_frame....%5d\n"
	"Func_frame...%5d\n"
	"Poco_frame...%5d\n"
	"Tstack.......%5d\n"
	"Token........%5d\n"
	"Poco_cb......%5d\n",
		sizeof(Symbol),
		sizeof(Type_info),
		sizeof(Code_buf),
		sizeof(Exp_frame)+HASH_SIZE*sizeof(Symbol *),
		sizeof(Func_frame),
		sizeof(Poco_frame),
		sizeof(Tstack),
		sizeof(Token),
		sizeof(Poco_cb)
	);
}
#endif /* DEBUG */

static FILE *must_fmake(Poco_cb *pcb, char *fn)
/*****************************************************************************
 * create a file, complain if it fails.
 ****************************************************************************/
{
FILE *new;

if ((new = fopen(fn, "w")) == NULL)
	{
	fprintf(pcb->t.err_file, "Can't make %s\n", fn);
	}
return(new);
}

static Errcode wanna_make(Poco_cb *pcb, FILE **f, char *fn)
/*****************************************************************************
 * create a file unless the name is NULL, die if it fails.
 ****************************************************************************/
{

if (fn != NULL)
	if ( (*f = must_fmake(pcb, fn)) == NULL)
		return(Err_create);
return(Success);
}

static void gentle_fclose(FILE *f)
/*****************************************************************************
 * close a file if it's open, and if it's not a device file.
 ****************************************************************************/
{
if (f != NULL && f != stdout && f != stderr)
	fclose(f);
}

static void po_pev_free_data(Poco_run_env *pev)
/*****************************************************************************
 * free a poco_run_env, and the associate stack and data areas.
 ****************************************************************************/
{
if (pev != NULL)
	{
	pj_freez(&pev->data);		/* this gets allocated from PJ, freez() */
	}
}

static Errcode po_pev_alloc_data(Poco_run_env *pev)
/*****************************************************************************
 * alloc poco_run_env, and stack and data areas. run data init code.
 * this routine should only be called after a successfull compile.
 ****************************************************************************/
{
	Errcode err;

	if (pev->data_size > 0)
		{
		if ((pev->data = pj_zalloc(pev->data_size)) == NULL)
			{
			err = Err_no_memory;
			goto ERR;
			}
		}
	if (pev->stack_size == 0)
		pev->stack_size = POCO_STACKSIZE_DEFAULT;

	if ((err = po_run_ops(pev, pev->fff->code_pt, NULL)) < Success)
		goto ERR;
	return(Success);

ERR:

	po_pev_free_data(pev);
	return err;
}

static Func_frame *find_fuf(Poco_run_env *pev, char  *name)
/*****************************************************************************
 * find a fuf for a function with a given name.
 * (linear search...slow, slow.  added first-char quick check to help a bit).
 ****************************************************************************/
{
Func_frame *f;

f = pev->fff;
while (f != NULL)
	{
	if (f->name[0] == *name)
		if (po_eqstrcmp(f->name, name) == 0)
			break;
	f = f->next;
	}
return(f);
}

static Errcode run_file(Poco_run_env *pev, char *entry)
/*****************************************************************************
 * run a given function from a compiled poco program.
 * (at this point in development, the 'given function' had better be 'main'!)
 ****************************************************************************/
{
Errcode err;
Func_frame *f;

if ((f = find_fuf(pev, entry)) == NULL)
	return(Err_no_main);
if ((err = po_pev_alloc_data(pev)) >= Success)
	err = po_run_ops(pev, f->code_pt, NULL);
po_pev_free_data(pev);
return(err);
}

static Errcode lib_run_file(Poco_run_env *pev, char *entry)
/*****************************************************************************
 * run a given function from a poco program after init'ing the libs.
 ****************************************************************************/
{
Errcode err = Success;

if ((err = po_init_libs(pev->lib)) >= Success)
	{
	if ((err = po_init_libs(pev->loaded_libs)) >= Success)
		err = run_file(pev, entry);
	po_cleanup_libs(pev->loaded_libs);
	}
po_cleanup_libs(pev->lib);
return(err);
}



static Errcode print_one_lib(FILE *f, Lib_proto *lib, int lib_size)
/*****************************************************************************
 * print a list of function names in a poco lib.
 ****************************************************************************/
{
while (--lib_size >= 0)
	{
	fprintf(f, "%s\n", lib->proto);
	lib+=1;
	}
return(Success);
}


Errcode print_pocolib(char *filename, Poco_lib *lib)
/*****************************************************************************
 * print the names of all functions in all poco libs.
 ****************************************************************************/
{
FILE *f;
Errcode err = Success;

if ((f = fopen(filename, "w")) != NULL)
	{
	while (lib != NULL)
		{
		fprintf(f, "/********* %s library ***********/\n", lib->name);
		if ((err = print_one_lib(f, lib->lib, lib->count)) < Success)
			break;
		lib = lib->next;
		}
	}
else
	{
	err = Err_create;
	}
fclose(f);
return(err);
}

int po_findpoe(char *libname, Lib_proto **plibreturn)
/*****************************************************************************
 * find a library or loaded poe module.
 * returns a pointer to its Lib_protos via *plibreturn and count of protos.
 *
 * this function now wears several hats.  if a specific library name is
 * passed, we search the linked list of loaded poe libraries for that name.
 * if the requested library name is "poco$builtin" or "poco$loaded", the
 * first library in the corresponding linked list is returned.	in this case,
 * subsequent calls with a NULL libname pointer will return the next library
 * in the list.
 *
 * if the requested library is not found, or when the end of a list of libs
 * is reached, the return value is Err_not_found, and the returned proto
 * pointer is set to NULL.
 *
 * NOTE!  This routine is part of the POE interface; a pointer to it is
 *		  provided to poe routines in the interface structure.	This routine
 *		  IS NOT intended for use by PJ internally -- it can only return
 *		  valid results while a poco program is currently executing!
 ****************************************************************************/
{
	static Poco_lib *listnext = NULL;
	Poco_lib		*ll;

	/*------------------------------------------------------------------------
	 * first make sure we don't get crashed by a naughty caller...
	 *----------------------------------------------------------------------*/

	if (NULL == plibreturn)
		return Err_null_ref;	/* defensive programming */

	if (NULL == porunenv)
		goto ERROR_EXIT;		/* "can't happen" */

	/*------------------------------------------------------------------------
	 * if the libname pointer is NULL, the caller wants the next library in
	 * the linked list, go return it...
	 *----------------------------------------------------------------------*/

	if (NULL == libname)
		{
		ll = listnext;
		goto RETURN_LIST_ITEM;
		}

	/*------------------------------------------------------------------------
	 * if the requested name is poco$builtin or poco$loaded, the caller wants
	 * to start a series of calls to walk the corresponding list...
	 *----------------------------------------------------------------------*/

	if (0 == stricmp(libname, "poco$builtin"))
		{
		ll = porunenv->lib;
		goto RETURN_LIST_ITEM;
		}

	if (0 == stricmp(libname, "poco$loaded"))
		{
		ll = porunenv->loaded_libs;
		goto RETURN_LIST_ITEM;
		}

	/*------------------------------------------------------------------------
	 * if we get to here, the caller wants a specific library, look for it...
	 *----------------------------------------------------------------------*/

	for (ll = porunenv->loaded_libs; ll != NULL; ll = ll->next)
		if (0 == stricmp(libname, ll->name))
			goto GOOD_EXIT;

ERROR_EXIT:

	*plibreturn = NULL;
	return Err_not_found;

RETURN_LIST_ITEM:

	if (NULL == ll)
		goto ERROR_EXIT;		/* return not-found/end-of-list status */

	listnext = ll->next;		/* reset list ptr for next call */

GOOD_EXIT:

	*plibreturn = ll->lib;
	return ll->count;

}

Poco_lib *po_open_library(Poco_cb *pcb, char *libname, char *id_string)
/*****************************************************************************
 *
 ****************************************************************************/
{
Errcode err;
Poco_lib *ll;

	if (0 == po_eqstrcmp("poco$builtin", libname))
		return pcb->builtin_lib;
	else
	{
#ifndef __TURBOC__
		if ((err = pj_load_pocorex(&ll, libname, id_string)) < Success)
		{
			errline(err, "can't load poco lib.");
			return(NULL);
		}
		ll->next = pcb->run.loaded_libs;
		pcb->run.loaded_libs = ll;
		return(ll);
#else
		return(NULL);
#endif /* __TURBOC__ */
	}
}

char *po_get_libproto_line(Poco_cb *pcb)
/*****************************************************************************
 * get the next line of library proto data, return NULL if at end of lib(s).
 *	this is called from the preprocessor, which is now the single source of
 *	input lines (no more indirect calls to a 'get next line' routine).
 *
 * 01/10/92 - (Ian)
 *			A fix to a major bug:  The builtin libs are linked into a list
 *			by our parent, and when we hit the end of a given library, we
 *			use pl->next to start working the next library in the list.
 *			Loaded libraries aren't pre-linked, they're loaded and
 *			processed one at a time.  But, as we load them (in the routine
 *			above), we link them together so that they make a list we can
 *			walk at the end of the run to free the loaded libs.  The problem
 *			occurred when multiple libraries were loaded in a single poco
 *			program via #pragma.  Upon hitting the second #pragma, we'd
 *			load the lib, and link it to the prior lib we just got done
 *			processing, then we'd start processing protos from it.  When
 *			we hit the end of the protos, the logic below would follow the
 *			link into the prior library (which had already been processed)
 *			and we'd end up with 'function XXXX redefined' errors.  (Yuck!)
 *			To fix this, another test was added:  if pcb->run.loaded_libs
 *			is non-NULL, that means we've already done the builtin libs and
 *			we're now into loaded libs.  In this case, we don't try to
 *			follow the library links, we just return NULL to say we're done
 *			with the current library.  If the pcb->run.loaded_libs pointer
 *			*is* NULL, that means we haven't started on loaded libs yet,
 *			we're still doing the builtins, and so we follow the library
 *			links until we hit the end of the linked list of builtin libs.
 * 05/17/92 (Ian)
 *			We now 'sanity check' the prototypes by ensuring that we don't
 *			get NULL proto string pointers.  If we find one, we die; it
 *			means that POCOLIB.H has gotten out of sync with our builtin
 *			libs code (POCO*.C in the root), or we have a sick POE module.
 ****************************************************************************/
{
File_stack	*fs = pcb->t.file_stack;
Poco_lib	*pl = fs->source.lib;
Lib_proto	*pp;

	if(fs->line_count >= pl->count)
		{
		if (NULL != pcb->run.loaded_libs || NULL == (pl = pl->next))
			{
			pcb->libfunc = NULL;
			return NULL;
			}
		fs->source.lib = pl;
		fs->line_count = 0;
		}
	pp = &pl->lib[fs->line_count++];
	pcb->libfunc = pp->func;

	if (pp->proto == NULL) {
		pcb->global_err = Err_poco_internal;
		po_say_fatal(pcb, "NULL prototype string pointer in library %s",
			pl->name);
	}

	return pp->proto;
}


Errcode compile_poco(
	void	 **ppexe,		/* returns executable pexe on Success */
	char	 *source_name,	/* name of source file */
	char	 *errors_name,	/* error file or NULL for stderr */
	char	 *dump_name,	/* disassembly file or NULL for none */
	Poco_lib *lib,			/* for built-in function library */
	char	 *err_file, 	/* file where error detected */
	long	 *err_line, 	/* line where error detected */
	int 	 *err_char, 	/* character in line where err detected */
	Names	 *include_dirs	/* include search path */
	)
/*****************************************************************************
 * compile a poco program.	entry point to the compiler from PJ.
 ****************************************************************************/
{
Poco_cb 	 *pcb;
Errcode 	 err;
Poco_run_env *pev;
File_stack	 *fs;

*ppexe = NULL;

if (Success != (err = setjmp(po_compile_errhandler)))
	{
	err = Err_in_err_file; /* Got here via longjmp */
	}
else
	{
	if (Success != (err = po_init_memory_management(&pcb)))
		return Err_no_memory;	/* MUST return immediately if init fails. */

	pcb->stack_bottom = ((char *)&ppexe) - MAX_STACK;

	pcb->t.err_file 	= stdout;
	pcb->t.include_dirs = include_dirs;

	pcb->libfunc	 = NULL;
	pcb->builtin_lib = lib;

	if (Success != (err = wanna_make(pcb, &pcb->t.err_file, errors_name)))
		goto OUT;

	if (dump_name != NULL)
		pcb->po_dump_file = fopen(dump_name, "w");

	if (Success != (err = po_compile_file(pcb, source_name)))
		{

#ifdef DEVELOPMENT /* all errs s/b via longjump, not return value...*/
		fprintf(stdout,"\ncompile_poco: got a non-zero return from po_compile_file!!!\n");
#endif

		err = Err_in_err_file;		/* we reported it in error file */
		goto OUT;
		}

	if (NULL == (pev = pj_zalloc((long)sizeof(*pev))))
		{
		err = Err_no_memory;
		goto OUT;
		}

	pcb->run.lib = lib;
	*pev = pcb->run;
	*ppexe = pev;
	}

OUT:

gentle_fclose(pcb->po_dump_file);
gentle_fclose(pcb->t.err_file);

if (err == Success)
	{
	if (errors_name != NULL)
		pj_delete(errors_name);
	po_free_compile_memory();
	}
else	/* Post-error cleanup goes goes here... */
	{

	/*
	 * let caller know where the err was
	 *	 if no files are open (eg, error was unexpected EOF) we say that.
	 *	 otherwise the error line number comes from the global error line
	 *	 number that is set by the error reporter in the parser or preprocessor.
	 *	 if the error happened in a library, and the library was included from
	 *	 a file, we report the filename and line number of the #pragma
	 *	 poco library statement that included the library.
	 */

	*err_char = 0;
	fs = pcb->t.file_stack;
	if (fs == NULL || fs->line_count == 0)
		{
		*err_line = 0;
		strcpy(err_file, "<no files open>");
		}
	else
		{
		if ((fs->flags & FSF_ISLIB) && fs->pred != NULL)
			{
			*err_line = fs->pred->line_count;
			strcpy(err_file, fs->pred->name);
			}
		else
			{
			*err_line = pcb->error_line_number;
			*err_char = pcb->error_char_number;
			strcpy(err_file, fs->name);
			}
		}

	/*
	 * close all files.
	 *	 changed this to a call to po_free_pp() - it contains the loop
	 *	 to close all the files.
	 */

	po_free_pp(pcb);

	/*
	 * free all memory allocated since the compile started...
	 */

	po_free_all_memory();

	/*
	 * free any libraries from #pragma poco library "xxx"
	 */

	pj_free_pocorexes(&pcb->run.loaded_libs);

	} /* end of post-error cleanup handling */

return(err);
}

Errcode run_poco(void **ppexe,
				 char *trace_file,
				 Boolean (*check_abort)(void *),
				 void *check_abort_data,
				 long *err_line)
/*****************************************************************************
 * run a poco program compiled earlier using compile_poco. exe entry from PJ.
 ****************************************************************************/
{

if ((porunenv = *ppexe) == NULL)
	return(Err_not_found);

porunenv->enable_debug_trace = TRUE;
porunenv->check_abort		 = check_abort;
porunenv->check_abort_data	 = check_abort_data;
porunenv->trace_file		 = trace_file;
porunenv->err_line			 = err_line;

return(lib_run_file(porunenv, "main"));
}

void free_poco(void **ppexe)
/*****************************************************************************
 * free runtime resources used by a poco program.
 ****************************************************************************/
{
Poco_run_env *pp;

if ((pp = *ppexe) != NULL)
	{
	po_pev_free_data(pp);
	po_free_run_env(pp);
	pj_free_pocorexes(&pp->loaded_libs);
	pj_free(pp);	/* this is allocated from PJ, free back to PJ. */
	*ppexe = NULL;
	}
po_free_all_memory();
}

char *po_fuf_name(void *fuf)
/*****************************************************************************
 * return pointer to name of function associated with a given fuf.
 ****************************************************************************/
{
return(((Func_frame *)fuf)->name);
}

void *po_fuf_code(void *fuf)
/*****************************************************************************
 * return pointer to code buffer associated with a given fuf.
 ****************************************************************************/
{
return(((Func_frame *)fuf)->code_pt);
}


Errcode po_file_to_stdout(char *name)
/*****************************************************************************
 * used by PJ when run as 'PJ filename.POC' and an error occurs.
 ****************************************************************************/
{
FILE *f;
int c;

if ((f = fopen(name, "r")) == NULL)
	return(Err_create);
while ((c = fgetc(f)) != EOF)
	fputc(c,stdout);
fputc('\n',stdout);
fclose(f);
return Success;
}

