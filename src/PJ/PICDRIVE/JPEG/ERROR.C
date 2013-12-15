#include "jinclude.h"
#include "stdtypes.h"
#include <setjmp.h>

/***************** Error Handling Stuff ***************************/
jmp_buf setjmp_buffer;	/* for return to caller */

static external_methods_ptr emethods; /* needed for access to message_parm */


/* This routine is used for any and all trace, debug, or error printouts
 * from the JPEG code.  The parameter is a printf format string; up to 8
 * integer data values for the format string have been stored in the
 * message_parm[] field of the external_methods struct.
 */

METHODDEF void
trace_message (const char *msgtext)
{
  printf(msgtext,
	  emethods->message_parm[0], emethods->message_parm[1],
	  emethods->message_parm[2], emethods->message_parm[3],
	  emethods->message_parm[4], emethods->message_parm[5],
	  emethods->message_parm[6], emethods->message_parm[7]);
}

/*
 * The error_exit() routine should not return to its caller.  The default
 * routine calls exit(), but here we assume that we want to return to
 * read_JPEG_file, which has set up a setjmp context for the purpose.
 * You should make sure that the free_all method is called, either within
 * error_exit or after the return to the outer-level routine.
 */

void bail_out(Errcode err)
{
  (*emethods->free_all) ();	/* clean up memory allocation & temp files */
  longjmp(setjmp_buffer, err);	/* return control to outer routine */
}

METHODDEF void
error_exit (const char *msgtext)
{
  trace_message(msgtext);	/* report the error message */
  bail_out(-1);
}


void set_error_methods(external_methods_ptr e_methods)
{
  emethods = e_methods;	/* save struct addr for possible access */
  e_methods->error_exit = error_exit; /* supply error-exit routine */
  e_methods->trace_message = trace_message; /* supply trace-message routine */
  e_methods->trace_level = 0;	/* default = no tracing */
  e_methods->num_warnings = 0;	/* no warnings emitted yet */
  e_methods->first_warning_level = 0; /* display first corrupt-data warning */
  e_methods->more_warning_level = 3; /* but suppress additional ones */
 }
