/****************************************************************************
 * main.c - a little testing shell for poco.  Not linked into PJ.
 *
 * Invokes the compiler on the files in the command line.  Also
 * contains the bits of the poco library present in the test shell.
 *
 * MAINTENANCE:
 *	09/06/91	(Jim)	Added -T flag for instruction tracing.
 *
 ***************************************************************************/

#define GENERATE_CTYPE_TABLE

#include "lostdio.h"
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <conio.h>
#include "poco.h"
#include "ptrmacro.h"
#include "pocoface.h"

short	jioerr; /* last io error */

#if defined(IAN)						/* Where Ian keeps poco source */
  Names incdirs[] = {
		{&incdirs[1], ""},
		{NULL,		  "\\paa\\resource\\"},
		};
#elif defined(JIM)						/* Where Jim keeps poco source */
  Names incdirs[] = {
		{&incdirs[1], ""},
		{&incdirs[2], "\\paa\\resource\\"},
		{NULL,		  "c:\\tc\\include\\"},
		};
#else
  Names incdirs[] = {
		{&incdirs[1], ""},
		{NULL,		  "\\paa\\resource\\"}
		};
#endif

/****************************************************************************
 * some memory management routines...
 * (simulation of the facilities available in PJ)
 ***************************************************************************/

extern void *malloc();
extern void free();

#define MMAG 0x1253
#define FMAG 0x2291

void *pj_malloc(unsigned i)
/****************************************************************************
 *
 ***************************************************************************/
{
USHORT *pt;
if ((pt = malloc(i + sizeof(*pt))) != NULL)
	*pt++ = MMAG;
return(pt);
}

void *pj_zalloc(unsigned size)
/****************************************************************************
 *
 ***************************************************************************/
{
void *pt;
if ((pt = pj_malloc(size)) == NULL)
	return(NULL);
poco_zero_bytes(pt,(size_t)size);
return(pt);
}

void pj_free(void *v)
/****************************************************************************
 *
 ***************************************************************************/
{
USHORT *pt = v;

if (pt == NULL)
	{
	fprintf(stdout,"main_freemem: freeing NULL!\n");
	exit(-1);
	}
if (*(--pt) != MMAG)
	{
	if (*pt == FMAG)
		{
		fprintf(stdout,"main_freemem: freeing memory twice\n");
		exit(-1);
		}
	else
		{
		fprintf(stdout,"main_freemem: Bad start magic\n");
		exit(-1);
		}
	}
*pt = FMAG;
free(pt);
}

void pj_gentle_free(void *p)
/****************************************************************************
 *
 ***************************************************************************/
{
if (p != NULL)
	pj_free(p);
}

void pj_freez(void *p)
/****************************************************************************
 *
 ***************************************************************************/
{
pj_gentle_free(*(void**)p);
*(void**)p = NULL;
}

Errcode pj_load_pocorex(Poco_lib **lib, char *name, char *idstring)
{
return(Err_unimpl); 	/* Would drag in too much of PJ to really do this */
}

void pj_free_pocorexes(Poco_lib **libs)
{
}

#if defined(__BORLANDC__) || defined(__TURBOC__)
  static void fpe_handler(int signum)
#else
  static int fpe_handler(int signum)
#endif
/*****************************************************************************
 * this routine catches div-by-zero and overflows in fp math instructions.
 *	 we just set builtin_err so that the poco interpreter will see an error
 *	 upon completion of the current virtual machine instruction, then we
 *	 re-install ourselves since signal handlers are one-shot by definition.
 * IMPORTANT NOTES:
 *	 watcom calls the floating point signal handler from within its
 *	 interupt handler for 80387 exceptions.  upon entry to this routine,
 *	 the hardware stack (ss:esp) is pointing to a 768-byte interupt stack!
 *	 if this routine is ever modified to take more extensive actions (ie,
 *	 calling an error reporting dialog) it will be necessary to switch to
 *	 a bigger stack.
 *	 despite what the watcom docs say, the 'errno' variable is NOT valid
 *	 upon entry to this routine!
 ****************************************************************************/
{
	_fpreset(); 					/* clear status & re-init chip/emulator */
	builtin_err = Err_float;		/* remember error for poco interpreter */
	signal(SIGFPE, fpe_handler);	/* re-install self */
	return(0);						/* don't know who looks at this... */
}

int matherr()
/*****************************************************************************
 *
 ****************************************************************************/
{
	_fpreset();
	builtin_err = Err_float;
	return 1;
}

pj_delete(char *name)
/****************************************************************************
 *
 ***************************************************************************/
{
return(pj_ddelete(name));
}

void boxf(long vargcount, long vargsize, char *fmt,...)
/****************************************************************************
 *
 ***************************************************************************/
/* this puts up a formated textbox for debugging etc */
{
fputs(fmt, stdout);
}

check_abort(void *nobody)
/****************************************************************************
 *
 ***************************************************************************/
{
if (pj_key_is())
	{
	pj_key_in();
	return(TRUE);
	}
return(FALSE);
}

void errline(int err, char *fmt, ...)
/****************************************************************************
 *
 ***************************************************************************/
{
va_list argptr;

va_start(argptr, fmt);
vfprintf(stdout,fmt,argptr);
va_end(argptr);
fprintf(stdout,"\nerr code %d", err);
}



int get_errtext(Errcode err, char *buf)
/****************************************************************************
 *
 ***************************************************************************/
{
buf[0] = 0;
if (err < Success)
	{
	switch (err)
		{
		case Err_stack:
			strcpy(buf, "Poco out of stack space");
			break;
		case Err_bad_instruction:
			strcpy(buf, "Illegal instruction in poco interpreter");
			break;
		case Err_null_ref:
			strcpy(buf, "Trying to use a NULL pointer");
			break;
		case Err_no_main:
			strcpy(buf, "No main function.");
			break;
		case Err_zero_divide:
			strcpy(buf, "Attempt to divide by zero");
			break;
		case Err_float:
			strcpy(buf, "Floating point math error (overflow/zero divide)");
			break;
		case Err_invalid_FILE:
			strcpy(buf, "Invalid FILE *");
			break;
		case Err_index_small:
			strcpy(buf, "Pointer/array index too small");
			break;
		case Err_index_big:
			strcpy(buf, "Pointer/array index too big");
			break;
		case Err_poco_free:
			strcpy(buf, "Trying to free an invalid block of memory");
			break;
		case Err_free_null:
			strcpy(buf, "Trying to free(NULL)");
			break;
		case Err_free_resources:
			strcpy(buf, "File/memory management damaged by Poco program");
			break;
		case Err_zero_malloc:
			strcpy(buf, "negative or zero size to malloc/calloc");
			break;
		case Err_string:
			strcpy(buf, "String too small in string operation");
			break;
		case Err_fread_buf:
			strcpy(buf, "Trying to fread past end of buffer");
			break;
		case Err_fwrite_buf:
			strcpy(buf, "Trying to fwrite past end of buffer");
			break;
		default:
			sprintf(buf, "Error code %d\n", err);
			break;
		}
	}
return(strlen(buf));
}

#ifdef I_DIED

int tryme(Popot v)
/****************************************************************************
 *
 ***************************************************************************/
{
Func_frame *f;
Pt_num ret;
long i;
Errcode err;

fprintf(stdout,"going to tryme %p %p %p\n", v.pt,  v.min,  v.max);
f = v.pt;
fprintf(stdout,"should I try %s?\n", f->name);
if (poco_param_left() < 6*sizeof(i))
	return(builtin_err = Err_stack);
i = 3;
while (--i >= 0)
	po_add_param(&i, sizeof(i));
err = builtin_err = poco_cont_ops(f->code_pt, &ret);
po_cleanup_param(3*sizeof(i));
fprintf(stdout,"err = %d ret = %d\n", err, ret.i);
ERR:
return(err);
}
#endif /* I_DIED */


po_puts(Popot s)
/****************************************************************************
 *
 ***************************************************************************/
{
int ret;

if (s.pt == NULL)
	return(builtin_err = Err_null_ref);
fputs(s.pt,stdout);
ret = fputs("\n",stdout);
return(ret);
}

int po_printf(long vargcount, long vargsize, Popot format, ...)
/****************************************************************************
 *
 ***************************************************************************/
{
va_list args;
int 	rv;

if (format.pt == NULL)
	return(builtin_err = Err_null_ref);

va_start(args, format);
rv = vfprintf(stdout,format.pt, args);
va_end(args);
return rv;
}

void po_qtext(long vargcount, long vargsize, Popot format, ...)
/****************************************************************************
 *
 ***************************************************************************/
{
va_list args;
int 	rv;

if (format.pt == NULL) {
	builtin_err = Err_null_ref;
	return;
}

fputs("------ Qtext --------------------------\n\n", stdout);
va_start(args, format);
rv = vfprintf(stdout,format.pt, args);
va_end(args);

fputs("\n\n------ Hit any key to Continue --------\n", stdout);
getch();

return;
}


static Lib_proto proto_lines[] =
{
/*	{tryme, 	"int ptryme(int (*v)(long a, long b, long c));"}, */
	{po_puts,	"int puts(char *s);"},
	{po_printf, "int printf(char *format, ...);"},
	{po_qtext,	"int Qtext(char *format, ...);"},
};

Poco_lib po_main_lib =
	{
	NULL,
	"Poco Library",
	proto_lines,
	Array_els(proto_lines),
	};

extern Poco_lib po_mem_lib;
extern Poco_lib po_FILE_lib;
extern Poco_lib po_math_lib;
extern Poco_lib po_str_lib;
extern Poco_lib po_dummy_lib;

static Poco_lib *poco_libs[] =
	{
	&po_main_lib,
	&po_str_lib,
	&po_mem_lib,
	&po_FILE_lib,
	&po_math_lib,
	&po_dummy_lib,
	};

static Poco_lib *get_poco_libs(void)
/****************************************************************************
 *
 ***************************************************************************/
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

FILE	 redirection_save;
FILE	 *f;

static Errcode open_redirect_stdout(char *fname)
/*****************************************************************************
 * this routine fools the PJ lfile library into thinking it is writing to
 * stdout but the stuff really goes into a file.
 ****************************************************************************/
{
	if (NULL == (f = fopen(fname, "w")))    /* create the file */
		return Err_create;

	redirection_save = *stdout; 			/* save state of stdout */
	*stdout = *f;							/* redirect stdout to file */

	return Success;
}

static void close_redirect_stdout(void)
/*****************************************************************************
 * this un-directs stdout from a file back to the screen.
 ****************************************************************************/
{
	*f = *stdout;					/* update buffer count, etc, in file */
	fclose(f);						/* close file */
	*stdout = redirection_save; 	/* restore stdout state */
}

Errcode builtin_err;	/* Error status for libraries. */

#ifdef DEVELOPMENT
/* variables for runops tracing */
extern C_frame *po_run_protos;
extern FILE *po_trace_file;
extern Boolean po_trace_flag;
#endif /* DEVELOPMENT */

int main(int argc, char *argv[])
/****************************************************************************
 *
 ***************************************************************************/
{
char	 err_file[100];
long	 err_line;
int 	 err_char;
int 	 err;
void	 *pexe;
char	 *efname = NULL;						/* Errors file name.	*/
char	 *sfname = NULL;						/* Source file name.	*/
char	 *dfname = NULL;						/* Dump file name.		*/
Boolean   runflag = TRUE;
char	 *argp;
int 	 counter;
Poco_lib *builtin_libs;

builtin_libs = get_poco_libs();

init_stdfiles(); /* initialize PJ stdin, stdout, etc */

signal(SIGFPE, fpe_handler);	// install floating point error trapping

for (counter = 1; counter < argc; counter++)
	{
	argp = argv[counter];
	if (*argp == '-')
		{
		switch (toupper(*++argp))
			{
			case 'C':                           /* Compile-only switch...   */
				runflag = FALSE;
				break;
#ifdef DEVELOPMENT
			case 'T':                           /* Trace... */
				po_trace_flag = TRUE;
				po_trace_file = stdout;
				break;
#endif /* DEVELOPMENT */
			case 'D':                           /* Dump file name...        */
				if (*++argp != '\0')
					dfname = argp;
				else
					dfname = "DUMP.";
				break;
			case 'O':                           /* Redirection file name... */
				if (*++argp != 0)
					efname = argp;
				else
					efname = "stdout.txt";
				break;
			case 'L':                           /* punt builtin libs...*/
				builtin_libs = NULL;
				break;
			case 'V':
				fprintf(stdout,"Poco version " VRSN_STR "\n");
				break;
			default:							/* Fat-finger case...		*/
				break;
			}
		}
	else
		{
		sfname = argp;		/* It's not a switch, must be the source file. */
		}
	}

if (NULL == sfname)
	sfname = "test.poc";

if (NULL == strchr(sfname, '.'))        /* If no '.' in name, tack on .POC */
	strcat(sfname, ".poc");

if (NULL != efname)
	if (Success != (err = open_redirect_stdout(efname)))
		{
		fprintf(stdout, "Error attempting to redirect stdout to '%s'\n", efname);
		exit(-1);
		}

if (Success == (err = compile_poco(&pexe, sfname, NULL, dfname,
					builtin_libs,
					err_file, &err_line, &err_char,
					incdirs
					)))
	{
#ifdef DEVELOPMENT
	po_run_protos = (((Poco_run_env *)pexe)->protos);	/* for trace */
#endif /* DEVELOPMENT */
	if (runflag)
		{
		err = run_poco(&pexe, NULL,
							check_abort, NULL, &err_line);
		}

	free_poco(&pexe);
	}

if (err < Success )
	{
	switch (err)
		{
		case Err_no_memory:
			fprintf(stdout,"Out of memory\n");
			break;
		case Err_no_file:
			fprintf(stdout,"Couldn't find %s\n", sfname);
			break;
		case Err_create:
			fprintf(stdout,"Couldn't create error/dump/trace files (disk full?)\n");
			break;
		case Err_syntax:
			fprintf(stdout,"Poco C syntax error.\n");
			break;
		case Err_poco_internal:
			fprintf(stdout,"Poco compiler failed self-check.\n");
			break;
		case Err_no_main:
			fprintf(stdout,"Program does not contain a main() routine.\n");
			break;
		case Err_in_err_file:
		case Err_abort:
		default:
			break;
		}
	fprintf(stdout,"Error code %d\n", err);
	}

	if (efname != NULL)
		close_redirect_stdout();

cleanup_lfiles(); /* cleanup PJ stdin, stdout, etc */

	return err;
}
