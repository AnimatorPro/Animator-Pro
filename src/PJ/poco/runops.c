/*****************************************************************************
 *
 * runops.c - A stack-based virtual machine for running poco programs.
 *
 * MAINTENANCE
 *	08/30/90	(Ian)
 *				Fixed OP_DEQ logic; it was comparing a double to an int,
 *				now it compares two doubles.
 *	09/21/90	(Ian)
 *				Changed all comparison ops (LE, GT, etc) to leave an INT on
 *				the stack.
 *	10/01/90	(Ian)
 *				Changed the OP_LEAVE instruction to clean local vars off the
 *				stack by restoring the base register to the stack pointer,
 *				instead of adding an offset value that was formerly coded
 *				as part of the instruction.  The base register was already
 *				extant in the interpreter.	This fixes a bug in which
 *				incorrect offset values would get coded into the OP_LEAVE
 *				instruction if it got generated several times in a function
 *				(ie, function had multiple return statements), and other
 *				things such as switch() statements grabbed extra temp space
 *				from the stack later in the function.
 *	10/22/90	(Ian)
 *				Fixed a bug in OP_ICCALL...the instruction pointer was being
 *				advanced before the check of 'cerr'. this caused problems
 *				with the po_print_trace() reporting when 'cerr' was non-zero,
 *				because the instruction pointer no longer pointed to the
 *				call instruction that resulted in the error.
 *	10/24/90	(Ian)
 *				Added new opcode OP_PTRDIFF to subtract two pointers.
 *	10/27/90	(Ian)
 *				Added integrity check to OP_CALLI processing.  The func_frame
 *				structure now contains a magic number field, and we verify
 *				that magic number before attempting to make an indirect
 *				function call.	This helps catch situations where something
 *				was cast to a function pointer that never should have been.
 *	11/28/90	(Ian)
 *				Added macros and protos for the new C function call glue
 *				routines found in runccall.asm.  This allows direct calls
 *				to C functions with proper parameter passing (ie, it removes
 *				the old limitation of 8 longwords of parms on the stack, with
 *				the rest accessible via poco_get_stack()).	This is required
 *				to support POE modules, which don't have access to the
 *				poco_get_stack() routine.  When compiled under TC, the old
 *				calling mechanism is used, except that it now stacks 32
 *				longwords of data instead of 8.  This is a big performance
 *				hit, but we don't care about performance under TC anyway.
 *	05/01/91	(Ian)
 *				Revamped trapping of floating point errors on the host side
 *				and detection and reporting in here.  The old 'ferr' global
 *				var was not being used at all, it's gone now.  On the host
 *				side, any fp-related err will now result in Err_float being
 *				stored in builtin_err.	If the
 *				error occurs in a library routine (eg, sqrt()), the existing
 *				error detection following a function call will catch it.
 *				Additional checks of builtin_err were added following floating
 *				point math calculations (OP_DMUL, OP_DDIV, etc) to catch
 *				overflow/underflow and div-by-zero conditions that happen
 *				in the inline code.
 *	09/06/91	(Jim)
 *				Added in String related opcodes.  (Just duplicate cases by
 *				pointer opcodes at the moment.
 ****************************************************************************/

#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include "poco.h"

#define MIN_PCALL_STACK 	 512	/* we check real often, small is fine. */
#define MIN_CCALL_STACK 	4096	/* we guarantee min 2k to poe users */
									/* which means we really ensure 4k: safe */

/* prototypes for functions in runccall.asm... */

#ifdef __TURBOC__
  #define VC_call(s, f) (*((void   (*)() )(f)))(*(Parmdata *)s)
  #define IC_call(s, f) (*((int    (*)() )(f)))(*(Parmdata *)s)
  #define LC_call(s, f) (*((long   (*)() )(f)))(*(Parmdata *)s)
  #define DC_call(s, f) (*((double (*)() )(f)))(*(Parmdata *)s)
  #define PC_call(s, f) (*((Popot  (*)() )(f)))(*(Parmdata *)s)
  #define STRING_C_call(s, f) (*((PoString	(*)() )(f)))(*(Parmdata *)s)
#else
  extern void	po_vccall(void *stack, void *func);
  extern int	po_iccall(void *stack, void *func);
  extern long	po_lccall(void *stack, void *func);
  extern double po_dccall(void *stack, void *func);
  extern Popot	po_pccall(void *stack, void *func);
  extern PoString po_string_ccall(void *stack, void *func);
  #define VC_call(s, f) po_vccall(s, f)
  #define IC_call(s, f) po_iccall(s, f)
  #define LC_call(s, f) po_lccall(s, f)
  #define DC_call(s, f) po_dccall(s, f)
  #define PC_call(s, f) po_pccall(s, f)
  #define STRING_C_call(s, f) po_string_ccall(s, f)
#endif

typedef union eax
	{
	Func_frame	*f;
	Pt_num		ret;
	int 		*iptr;
	} Eax;

typedef struct
	{
	long data[32];
	} Parmdata;

static Poco_run_env *pe;

static nofunc(void *d)
/*****************************************************************************
 * used as a dummy check_abort function when none is provided.
 ****************************************************************************/
{
	return(FALSE);
}
#ifdef DEVELOPMENT
/* variables for runops tracing */
C_frame *po_run_protos;
FILE *po_trace_file;
Boolean po_trace_flag = FALSE;
#endif /* DEVELOPMENT */

Errcode poco_cont_ops(void *code_pt, Pt_num *pret, int arglength, ...)
/*****************************************************************************
 * interpret code stream - the heart of the runtime interpreter.
 ****************************************************************************/
{
	int 	end_op	 = OP_END;
	FILE	*tfile	 = NULL;
	Pt_num	*ip 	 = code_pt;
	Pt_num	*globals = (Pt_num *)(pe->data + pe->data_size);
	Errcode err;
	va_list args;
	Pt_num	*stack;
	Pt_num	*base;
	UBYTE	*stack_area;
	Eax 	acc;
	int 	op;

#define STKOVFL(limit) ((char *)stack < (stack_area+(limit)))

#if defined(__BORLANDC__) || defined(__TURBOC__)
  #define vargptr args
#else
  #define vargptr args[0]
#endif

	if (pe->stack == NULL)
		{
		if (NULL == (stack_area = pj_malloc(pe->stack_size)))
			return Err_no_memory;
		}
	else
		{
		stack_area = pe->stack;
		}

	stack = (Pt_num *)(stack_area + pe->stack_size);

	if (arglength > 0)
		{
		va_start(args, arglength);
		stack = OPTR(stack, -arglength);
		poco_copy_bytes(vargptr, stack, arglength);
		va_end(args);
		}

	/* final return address is to an end-op */

	stack = OPTR(stack, -sizeof(ip));
	stack->p = &end_op;
	base = stack;

	/* assume a starting condition of success */

	builtin_err = Success;

	/* supply a default abort checker if none provided */

	if (pe->check_abort == NULL)
		pe->check_abort = nofunc;

	for (;;)
		{
#ifdef DEVELOPMENT
		{
		extern C_frame *po_run_protos;
		extern FILE *po_trace_file;
		extern Boolean po_trace_flag;
		if (po_trace_flag)
			po_disasm(po_trace_file, ip, po_run_protos);
		}
#endif /* DEVELOPMENT */
		op = ip->inty;
		ip = OPTR(ip, OPY_SIZE);
		switch (op)
			{

		default:
			err = Err_bad_instruction;
			goto DEBUG;

		case OP_END:	/* finished instruction stream */
			*pret = acc.ret;
			err = Success;
			goto DEALLOC_AND_EXIT;

/*----------------------------------------------------------------------------
 * DATATYPE CONVERSIONS
 *--------------------------------------------------------------------------*/

		case OP_INT_TO_LONG:
			acc.ret.l = stack->inty;
			stack = OPTR(stack, INT_SIZE-sizeof(long) );
			stack->l = acc.ret.l;
			break;
		case OP_LONG_TO_INT:
			acc.ret.inty = stack->l;
			stack = OPTR(stack, sizeof(long)-INT_SIZE );
			stack->inty = acc.ret.inty;
			break;
		case OP_INT_TO_DOUBLE:
			acc.ret.d = stack->inty;
			stack = OPTR(stack, INT_SIZE-sizeof(double) );
			stack->d = acc.ret.d;
			break;
		case OP_LONG_TO_DOUBLE:
			acc.ret.d = stack->l;
			stack = OPTR(stack, sizeof(long)-sizeof(double) );
			stack->d = acc.ret.d;
			break;
		case OP_DOUBLE_TO_LONG:
			acc.ret.l = stack->d;
			stack = OPTR(stack, sizeof(double)-sizeof(long) );
			stack->l = acc.ret.l;
			break;
		case OP_DOUBLE_TO_INT:
			acc.ret.inty = stack->d;
			stack = OPTR(stack, sizeof(double)-sizeof(int) );
			stack->inty = acc.ret.inty;
			break;
		case OP_PPT_TO_CPT:
			acc.ret.p = stack->ppt.pt;
			stack = OPTR(stack, sizeof(stack->ppt)-sizeof(stack->ppt.pt));
			stack->p = acc.ret.p;
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_TO_CPT:
			po_sr_dec_ref(stack->postring); /* dec ref count but
												 * don't deallocate yet */
			stack->p= PoStringBuf(&stack->postring);
			break;
#endif /* STRING_EXPERIMENT */
		case OP_CPT_TO_PPT:
			acc.ret.p = stack->p;
			stack = OPTR(stack, sizeof(stack->p)-sizeof(stack->ppt));
			stack->ppt.min = stack->ppt.max = stack->ppt.pt = acc.ret.p;
			break;
#ifdef STRING_EXPERIMENT
		case OP_CPT_TO_STRING:
			err = Err_bad_instruction;	/* Right now we don't generate
										 * these and so it'd be hard
										 * to test the code required.... */
			goto DEBUG;
			break;
#endif /* STRING_EXPERIMENT */

#ifdef STRING_EXPERIMENT
		case OP_STRING_TO_PPT:
			acc.ret.postring = stack->postring;
			po_sr_dec_ref(acc.ret.postring);	/* dec ref count but
												 * don't deallocate yet */
			stack = OPTR(stack, sizeof(stack->postring)-sizeof(stack->ppt));
			stack->ppt = acc.ret.postring->string;
			break;
		case OP_PPT_TO_STRING:
			acc.ret.postring = po_sr_new_copy(stack->ppt.pt
			,Popot_bufsize(&stack->ppt));
			if (builtin_err < Success)
				goto ERR_IN_LIBROUTINE;
			stack = OPTR(stack, sizeof(stack->ppt)-sizeof(stack->postring));
			stack->postring = acc.ret.postring;
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * FUNCTION CALLS
 *--------------------------------------------------------------------------*/

		case OP_ICCALL:  /* call int valued C function */
			if (STKOVFL(MIN_CCALL_STACK))
				{
				err = Err_stack;
				goto DEBUG;
				}
			acc.ret.i = IC_call(stack, ip->func);
			if (builtin_err < Success)
				goto ERR_IN_LIBROUTINE;
			ip = OPTR(ip,sizeof(ip->func));
			break;
		case OP_LCCALL: 	/* call long valued C function */
			if (STKOVFL(MIN_CCALL_STACK))
				{
				err = Err_stack;
				goto DEBUG;
				}
			acc.ret.l = LC_call(stack, ip->func);
			if (builtin_err < Success)
				goto ERR_IN_LIBROUTINE;
			ip = OPTR(ip,sizeof(ip->func));
			break;
		case OP_DCCALL: 	/* call double valued C function */
			if (STKOVFL(MIN_CCALL_STACK))
				{
				err = Err_stack;
				goto DEBUG;
				}
			acc.ret.d = DC_call(stack, ip->func);
			if (builtin_err < Success)
				goto ERR_IN_LIBROUTINE;
			ip = OPTR(ip,sizeof(ip->func));
			break;
		case OP_PCCALL: 	/* call (popot) pointer valued C function */
			if (STKOVFL(MIN_CCALL_STACK))
				{
				err = Err_stack;
				goto DEBUG;
				}
			acc.ret.ppt = PC_call(stack, ip->func);
			if (builtin_err < Success)
				goto ERR_IN_LIBROUTINE;
			ip = OPTR(ip,sizeof(ip->func));
			break;
		case OP_CVCCALL:	/* call void valued C function */
			if (STKOVFL(MIN_CCALL_STACK))
				{
				err = Err_stack;
				goto DEBUG;
				}
			VC_call(stack, ip->func);
			if (builtin_err < Success)
				goto ERR_IN_LIBROUTINE;
			ip = OPTR(ip,sizeof(ip->func));
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_CCALL:	/* call string valued C function */
			if (STKOVFL(MIN_CCALL_STACK))
				{
				err = Err_stack;
				goto DEBUG;
				}
			STRING_C_call(stack, ip->func);
			if (builtin_err < Success)
				goto ERR_IN_LIBROUTINE;
			ip = OPTR(ip,sizeof(ip->func));
			break;
#endif /* STRING_EXPERIMENT */

		case OP_CALLI:		/* Call Indirect (via pointer) */
			if ((acc.f = stack->ppt.pt) == NULL)
				goto ERR_NULL;
			if (acc.f->magic != FUNC_MAGIC)
				goto ERR_NOTAFUNC;
			switch (acc.f->type)
				{
				case CFF_C:
					if (STKOVFL(MIN_CCALL_STACK))
						{
						err = Err_stack;
						goto DEBUG;
						}
					stack = OPTR(stack, sizeof(stack->ppt));
					switch (acc.f->return_type->ido_type)
						{
						case IDO_INT:
							acc.ret.i = IC_call(stack, acc.f->code_pt);
							break;
						case IDO_LONG:
							acc.ret.l = LC_call(stack, acc.f->code_pt);
							break;
						case IDO_DOUBLE:
							acc.ret.d = DC_call(stack, acc.f->code_pt);
							break;
						case IDO_POINTER:
							acc.ret.ppt = PC_call(stack, acc.f->code_pt);
							break;
						case IDO_VOID:
							VC_call(stack, acc.f->code_pt);
							break;
						default:
							err = Err_unimpl;
							goto DEBUG;
						}
					if (builtin_err < Success)
						goto ERR_IN_LIBROUTINE;
					break;
				case CFF_POCO:
					if (STKOVFL(MIN_PCALL_STACK))
						{
						err = Err_stack;
						goto DEBUG;
						}
					stack = OPTR(stack, sizeof(stack->ppt)-sizeof(stack->p));
					stack->p = ip;
					ip = (Pt_num *)acc.f->code_pt;
					break;
				}
			if ((pe->check_abort)(pe->check_abort_data))
				goto ABORT;
			break;
		case OP_PCALL:		/* Call Poco function */
			if (STKOVFL(MIN_PCALL_STACK))
				{
				err = Err_stack;
				goto DEBUG;
				}
			acc.f = ip->p;
			ip = OPTR(ip,sizeof(acc.f));
			stack = OPTR(stack, -sizeof(ip));
			stack->p = ip;
			ip = (Pt_num *)acc.f->code_pt;
			if ((pe->check_abort)(pe->check_abort_data))
				goto ABORT;
			break;

/*----------------------------------------------------------------------------
 * FUNCTION ENTRY/EXIT
 *--------------------------------------------------------------------------*/

		case OP_RET:
			ip = stack->p;
			stack = OPTR(stack, sizeof(ip) );
			break;
		case OP_ADD_STACK:
			stack = OPTR(stack, ip->doff);
			ip = OPTR(ip, sizeof(ip->doff));
			break;
		case OP_ENTER:
			if (STKOVFL(ip->doff + MIN_PCALL_STACK))
				{
				err = Err_stack;
				goto DEBUG;
				}
			stack = OPTR(stack, -sizeof(base));
			stack->p = base;
			base = stack;
			stack = OPTR(stack, -ip->doff);
			poco_zero_bytes(stack, ip->doff);
			ip = OPTR(ip, sizeof(ip->doff) );
			break;
		case OP_LEAVE:
			stack = base;						/* clear off local vars  */
			base  = stack->p;					/* restore parent base	 */
			stack = OPTR(stack, sizeof(base));	/* clean off parent base */
			break;

/*----------------------------------------------------------------------------
 * BRANCHES
 *--------------------------------------------------------------------------*/

		case OP_BRA:
			if (ip->inty < 0)
				if ((pe->check_abort)(pe->check_abort_data))
					goto ABORT;
			ip	= OPTR(ip, ip->inty);
			break;
		case OP_BEQ:
			if (stack->inty == 0)
				{
				if (ip->inty < 0)
					if ((pe->check_abort)(pe->check_abort_data))
						goto ABORT;
				ip	= OPTR(ip, ip->inty);
				}
			else
				{
				ip = OPTR(ip, sizeof(ip->inty));
				}
			stack = OPTR(stack, sizeof(stack->inty));
			break;
		case OP_BNE:
			if (stack->inty != 0)
				{
				if (ip->inty < 0)
					if ((pe->check_abort)(pe->check_abort_data))
						goto ABORT;
				ip	= OPTR(ip, ip->inty);
				}
			else
				ip = OPTR(ip, sizeof(ip->inty));
			stack = OPTR(stack, sizeof(stack->inty));
			break;

/*----------------------------------------------------------------------------
 * PUSH CONSTANTS
 *--------------------------------------------------------------------------*/

		case OP_ICON:	/* push int constant onto data stack */
			stack = OPTR(stack, -INT_SIZE);
			stack->inty = ip->inty;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LCON:	/* push long constant onto stack */
			stack = OPTR(stack, -sizeof(long));
			stack->l = ip->l;
			ip = OPTR(ip, sizeof(long));
			break;
		case OP_DCON:	/* push floating point constant onto stack */
			stack = OPTR(stack, -sizeof(double));
			stack->d = ip->d;
			ip = OPTR(ip, sizeof(double));
			break;
		case OP_PCON:	/* push pointer constant onto stack */
			stack = OPTR(stack, -sizeof(stack->ppt));
			stack->ppt = ip->ppt;
			ip = OPTR(ip, sizeof(ip->ppt));
			break;

/*----------------------------------------------------------------------------
 * LOAD EFFECTIVE ADDRESS
 *--------------------------------------------------------------------------*/

		case OP_GLO_ADDRESS:
			stack = OPTR(stack, -sizeof(stack->ppt));
			stack->ppt.min = stack->ppt.max = stack->ppt.pt
				= OPTR(globals, ip->inty);
			ip = OPTR(ip, sizeof(ip->inty));
			stack->ppt.max = OPTR(stack->ppt.max, ip->l);
			ip = OPTR(ip, sizeof(ip->l));
			break;
		case OP_LOC_ADDRESS:
			stack = OPTR(stack, -sizeof(stack->ppt));
			stack->ppt.min = stack->ppt.max = stack->ppt.pt
				= OPTR(base, ip->inty);
			ip = OPTR(ip, sizeof(ip->inty));
			stack->ppt.max = OPTR(stack->ppt.max, ip->l);
			ip = OPTR(ip, sizeof(ip->l));
			break;
		case OP_CODE_ADDRESS:	/* put immediate code address onto stack */
			stack = OPTR(stack, -sizeof(stack->ppt));
			stack->ppt.min = stack->ppt.max = NULL;
			stack->ppt.pt = ip->p;
			ip = OPTR(ip, sizeof(void *));
			break;

/*----------------------------------------------------------------------------
 * LOAD A VARIABLE (DIRECT)
 *--------------------------------------------------------------------------*/

		case OP_GLO_CVAR:	/* push a global variable onto data stack */
			stack = OPTR(stack, -INT_SIZE);
			stack->inty = ((char *)(OPTR(globals, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_GLO_SVAR:	/* push a global variable onto data stack */
			stack = OPTR(stack, -INT_SIZE);
			stack->inty = ((short *)(OPTR(globals, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_GLO_IVAR:	/* push a global variable onto data stack */
			stack = OPTR(stack, -INT_SIZE);
			stack->inty = ((int *)(OPTR(globals, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_GLO_LVAR:	/* push a global variable onto data stack */
			stack = OPTR(stack, -sizeof(long));
			stack->l = ((long *)(OPTR(globals, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_GLO_PVAR:	/* push a global pointer onto data stack */
		case OP_GLO_VVAR:	/* push a global function pointer onto data stack */
			stack = OPTR(stack, -sizeof(stack->ppt));
			stack->ppt = ((Popot *)(OPTR(globals, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_GLO_FVAR:	/* push a global variable onto data stack */
			stack = OPTR(stack, -sizeof(double));
			stack->d = ((float *)(OPTR(globals, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_GLO_DVAR:	/* push a global variable onto data stack */
			stack = OPTR(stack, -sizeof(double));
			stack->d = ((double *)(OPTR(globals, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
#ifdef STRING_EXPERIMENT
		case OP_GLO_STRING_VAR: /* push a global string onto data stack */
			stack = OPTR(stack, -sizeof(PoString));
			stack->postring = ((PoString *)(OPTR(globals, ip->doff)))[0];
			po_sr_inc_ref(stack->postring);
			ip = OPTR(ip, INTY_SIZE);
			break;
#endif /* STRING_EXPERIMENT */

		case OP_LOC_CVAR:	/* push a local variable onto data stack */
			stack = OPTR(stack, -INT_SIZE);
			stack->inty = ((char *)(OPTR(base, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LOC_SVAR:	/* push a local variable onto data stack */
			stack = OPTR(stack, -INT_SIZE);
			stack->inty = ((short *)(OPTR(base, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LOC_IVAR:	/* push a local variable onto data stack */
			stack = OPTR(stack, -INT_SIZE);
			stack->inty = ((int *)(OPTR(base, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LOC_LVAR:	/* push a local variable onto data stack */
			stack = OPTR(stack, -sizeof(long));
			stack->l = ((long *)(OPTR(base, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LOC_PVAR:	/* push a local variable onto data stack */
			stack = OPTR(stack, -sizeof(stack->ppt));
			stack->ppt = ((Popot *)(OPTR(base, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LOC_FVAR:	/* push a local variable onto data stack */
			stack = OPTR(stack, -sizeof(double));
			stack->d = ((float *)(OPTR(base, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LOC_DVAR:	/* push a local variable onto data stack */
			stack = OPTR(stack, -sizeof(double));
			stack->d = ((double *)(OPTR(base, ip->doff)))[0];
			ip = OPTR(ip, INTY_SIZE);
			break;
#ifdef STRING_EXPERIMENT
		case OP_LOC_STRING_VAR: /* push a local string onto data stack */
			stack = OPTR(stack, -sizeof(PoString));
			stack->postring = ((PoString *)(OPTR(base, ip->doff)))[0];
			po_sr_inc_ref(stack->postring);
			ip = OPTR(ip, INTY_SIZE);
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * STORE A VARIABLE (DIRECT)
 *--------------------------------------------------------------------------*/

		case OP_GLO_CASS:	/* move top of stack to global variable */
			(((char *)OPTR(globals, ip->doff))[0]) = stack->inty;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_GLO_SASS:	/* move top of stack to global variable */
			((short *)(OPTR(globals, ip->doff)))[0] = stack->inty;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_GLO_IASS:	/* move top of stack to global variable */
			((int *)(OPTR(globals, ip->doff)))[0] = stack->inty;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_GLO_LASS:	/* move top of stack to global variable */
			((long *)(OPTR(globals, ip->doff)))[0] = stack->l;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_GLO_PASS:	/* move top of stack to global pointer variable */
			((Popot *)(OPTR(globals, ip->doff)))[0] = stack->ppt;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_GLO_FASS:	/* move top of stack to global variable */
			((float *)(OPTR(globals, ip->doff)))[0] = stack->d;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_GLO_DASS:	/* move top of stack to global variable */
			((double *)(OPTR(globals, ip->doff)))[0] = stack->d;
			ip = OPTR(ip, INTY_SIZE);
			break;
#ifdef STRING_EXPERIMENT
		case OP_GLO_STRING_ASS: /* top of stack to global string variable */
			po_sr_clean_ref((((PoString *)(OPTR(globals, ip->doff)))[0]));
			((PoString *)(OPTR(globals, ip->doff)))[0] = stack->postring;
			po_sr_inc_ref(stack->postring);
			ip = OPTR(ip, INTY_SIZE);
			break;
#endif /* STRING_EXPERIMENT */

		case OP_LOC_CASS:	/* move top of stack to local variable */
			((char *)(OPTR(base, ip->doff)))[0] = stack->inty;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LOC_SASS:	/* move top of stack to local variable */
			((short *)(OPTR(base, ip->doff)))[0] = stack->inty;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LOC_IASS:	/* move top of stack to local variable */
			((int *)(OPTR(base, ip->doff)))[0] = stack->inty;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LOC_LASS:	/* move top of stack to local variable */
			((long *)(OPTR(base, ip->doff)))[0] = stack->l;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LOC_PASS:	/* move top of stack to local ptr variable */
			((Popot *)(OPTR(base, ip->doff)))[0] = stack->ppt;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LOC_FASS:	/* move top of stack to local variable */
			((float *)(OPTR(base, ip->doff)))[0] = stack->d;
			ip = OPTR(ip, INTY_SIZE);
			break;
		case OP_LOC_DASS:	/* move top of stack to local variable */
			((double *)(OPTR(base, ip->doff)))[0] = stack->d;
			ip = OPTR(ip, INTY_SIZE);
			break;
#ifdef STRING_EXPERIMENT
		case OP_LOC_STRING_ASS: /* top of stack to local string variable */
			po_sr_clean_ref((((PoString *)(OPTR(base, ip->doff)))[0]));
			((PoString *)(OPTR(base, ip->doff)))[0] = stack->postring;
			po_sr_inc_ref(stack->postring);
			ip = OPTR(ip, INTY_SIZE);
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * LOAD A VARIABLE (INDIRECT)
 *--------------------------------------------------------------------------*/

		case OP_CI_VAR:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(acc.ret.ppt) - sizeof(acc.ret.inty));
			stack->inty = *((char *)(acc.ret.ppt.pt));
			break;
		case OP_SI_VAR:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(acc.ret.ppt) - sizeof(acc.ret.inty));
			stack->inty = *((short *)(acc.ret.ppt.pt));
			break;
		case OP_II_VAR:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(acc.ret.ppt) - sizeof(acc.ret.inty));
			stack->inty = *((int *)(acc.ret.ppt.pt));
			break;
		case OP_PI_VAR:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(acc.ret.ppt) - sizeof(acc.ret.ppt));
			stack->ppt = *((Popot *)(acc.ret.ppt.pt));
			break;
		case OP_LI_VAR:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(acc.ret.ppt) - sizeof(acc.ret.l));
			stack->l = *((long *)(acc.ret.ppt.pt));
			break;
		case OP_FI_VAR:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(acc.ret.ppt) - sizeof(acc.ret.f));
			stack->d = *((float *)(acc.ret.ppt.pt));
			break;
		case OP_DI_VAR:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(acc.ret.ppt) - sizeof(acc.ret.d));
			stack->d = *((double *)(acc.ret.ppt.pt));
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_I_VAR:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(acc.ret.ppt) - sizeof(acc.ret.postring));
			stack->postring = *((PoString *)(acc.ret.ppt.pt));
			po_sr_inc_ref(stack->postring);
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * STORE A VARIABLE (INDIRECT)
 *--------------------------------------------------------------------------*/

		case OP_CI_ASS:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(stack->ppt));
			((char *)(acc.ret.ppt.pt))[0] = stack->inty;
			break;
		case OP_SI_ASS:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(stack->ppt));
			((short *)(acc.ret.ppt.pt))[0] = stack->inty;
			break;
		case OP_II_ASS:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(stack->ppt));
			((int *)(acc.ret.ppt.pt))[0] = stack->inty;
			break;
		case OP_PI_ASS:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(stack->ppt));
			((Popot *)(acc.ret.ppt.pt))[0] = stack->ppt;
			break;
		case OP_LI_ASS:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(stack->ppt));
			((long *)(acc.ret.ppt.pt))[0] = stack->l;
			break;
		case OP_FI_ASS:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(stack->ppt));
			((float *)(acc.ret.ppt.pt))[0] = stack->d;
			break;
		case OP_DI_ASS:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(stack->ppt));
			((double *)(acc.ret.ppt.pt))[0] = stack->d;
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_I_ASS:
			acc.ret.ppt = stack->ppt;
			if (acc.ret.ppt.pt == NULL)
				goto ERR_NULL;
			if (acc.ret.ppt.pt < acc.ret.ppt.min)
				goto ERR_SMALL;
			if (acc.ret.ppt.pt > acc.ret.ppt.max)
				goto ERR_BIG;
			stack = OPTR(stack, sizeof(stack->ppt));
			po_sr_clean_ref(((PoString *)(acc.ret.ppt.pt))[0]);
			((PoString *)(acc.ret.ppt.pt))[0] = stack->postring;
			po_sr_inc_ref(stack->postring);
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * ADDITION
 *--------------------------------------------------------------------------*/

		case OP_IADD:	/* replace top two elements of stack one result */
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, INT_SIZE);
			stack->inty += acc.ret.inty;
			break;
		case OP_LADD:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(long));
			stack->l += acc.ret.l;
			break;
		case OP_DADD:
			acc.ret.d = stack->d;
			stack = OPTR(stack, sizeof(double));
			stack->d += acc.ret.d;
			if (builtin_err != Success)
				goto ERR_INLINE_FPMATH;
			break;
		case OP_PADD:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty) );
			stack->ppt.pt = OPTR(stack->ppt.pt, acc.ret.inty);
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_CAT: 	/* Concatenate top two strings */
			acc.ret.postring = po_sr_cat_and_clean(
			((Pt_num *)OPTR(stack,sizeof(stack->postring)))->postring
			,stack->postring);
			stack = OPTR(stack
			, 2*sizeof(stack->postring)-sizeof(stack->postring));
			stack->postring = acc.ret.postring;
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * SUBTRACTION
 *--------------------------------------------------------------------------*/

		case OP_ISUB:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, INT_SIZE);
			stack->inty -= acc.ret.inty;
			break;
		case OP_LSUB:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(long));
			stack->l -= acc.ret.l;
			break;
		case OP_DSUB:
			acc.ret.d = stack->d;
			stack = OPTR(stack, sizeof(double));
			stack->d -= acc.ret.d;
			if (builtin_err != Success)
				goto ERR_INLINE_FPMATH;
			break;
		case OP_PSUB:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty) );
			stack->ppt.pt = OPTR(stack->ppt.pt, -acc.ret.inty);
			break;

/*----------------------------------------------------------------------------
 * MULTIPLICATION
 *--------------------------------------------------------------------------*/

		case OP_IMUL:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, INT_SIZE);
			stack->inty *= acc.ret.inty;
			break;
		case OP_LMUL:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(long));
			stack->l *= acc.ret.l;
			break;
		case OP_DMUL:
			acc.ret.d = stack->d;
			stack = OPTR(stack, sizeof(double));
			stack->d *= acc.ret.d;
			if (builtin_err != Success)
				goto ERR_INLINE_FPMATH;
			break;

/*----------------------------------------------------------------------------
 * DIVISION
 *--------------------------------------------------------------------------*/

		case OP_IDIV:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, INT_SIZE);
			if (acc.ret.inty == 0)
				{
				err = Err_zero_divide;
				goto DEBUG;
				}
			stack->inty /= acc.ret.inty;
			break;
		case OP_LDIV:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(long));
			if (acc.ret.l == 0)
				{
				err = Err_zero_divide;
				goto DEBUG;
				}
			stack->l /= acc.ret.l;
			break;
		case OP_DDIV:
			acc.ret.d = stack->d;
			stack = OPTR(stack, sizeof(double));
			stack->d /= acc.ret.d;
			if (builtin_err != Success)
				goto ERR_INLINE_FPMATH;
			break;

/*----------------------------------------------------------------------------
 * COMPARISONS - EQ
 *--------------------------------------------------------------------------*/

		case OP_IEQ:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty = (acc.ret.inty == stack->inty);
			break;
		case OP_LEQ:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			acc.ret.inty = (acc.ret.l == stack->l);
			stack = OPTR(stack, (sizeof(stack->l)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			break;
		case OP_DEQ:
			acc.ret.d = stack->d;
			stack = OPTR(stack, sizeof(stack->d));
			acc.ret.inty = (acc.ret.d == stack->d);
			stack = OPTR(stack, (sizeof(stack->d)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			if (builtin_err != Success)
				goto ERR_INLINE_FPMATH;
			break;
		case OP_PEQ:
			acc.ret.inty =
				(stack->ppt.pt ==
					((Pt_num *)OPTR(stack,sizeof(stack->ppt)))->ppt.pt);
			stack = OPTR(stack, 2*sizeof(stack->ppt)-sizeof(stack->inty));
			stack->inty = acc.ret.inty;
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_EQ:
			acc.ret.inty = po_sr_eq_and_clean(stack->postring
			,((Pt_num *)OPTR(stack,sizeof(stack->postring)))->postring);
			stack = OPTR(stack, 2*sizeof(stack->postring)-sizeof(stack->inty));
			stack->inty = acc.ret.inty;
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * COMPARISONS - NE
 *--------------------------------------------------------------------------*/

		case OP_INE:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty = (acc.ret.inty != stack->inty);
			break;
		case OP_LNE:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			acc.ret.inty = (acc.ret.l != stack->l);
			stack = OPTR(stack, (sizeof(stack->l)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			break;
		case OP_DNE:
			acc.ret.d = stack->d;
			stack = OPTR(stack, sizeof(stack->d));
			acc.ret.inty = (acc.ret.d != stack->d);
			stack = OPTR(stack, (sizeof(stack->d)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			if (builtin_err != Success)
				goto ERR_INLINE_FPMATH;
			break;
		case OP_PNE:
			acc.ret.inty =
				(stack->ppt.pt !=
					((Pt_num *)OPTR(stack,sizeof(stack->ppt)))->ppt.pt);
			stack = OPTR(stack, 2*sizeof(stack->ppt)-sizeof(stack->inty));
			stack->inty = acc.ret.inty;
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_NE:
			acc.ret.inty = !po_sr_eq_and_clean(stack->postring
			,((Pt_num *)OPTR(stack,sizeof(stack->postring)))->postring);
			stack = OPTR(stack, 2*sizeof(stack->postring)-sizeof(stack->inty));
			stack->inty = acc.ret.inty;
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * COMPARISONS - GE
 *--------------------------------------------------------------------------*/

		case OP_IGE:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty = (stack->inty >= acc.ret.inty);
			break;
		case OP_LGE:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			acc.ret.inty = (stack->l >= acc.ret.l);
			stack = OPTR(stack, (sizeof(stack->l)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			break;
		case OP_DGE:
			acc.ret.d = stack->d;
			stack = OPTR(stack, sizeof(stack->d));
			acc.ret.inty = (stack->d >= acc.ret.d);
			stack = OPTR(stack, (sizeof(stack->d)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			if (builtin_err != Success)
				goto ERR_INLINE_FPMATH;
			break;
		case OP_PGE:
			acc.ret.ppt = stack->ppt;
			stack = OPTR(stack, sizeof(stack->ppt));
			acc.ret.inty = ((char*)(stack->ppt.pt) >= ((char *)acc.ret.ppt.pt));
			stack = OPTR(stack, (sizeof(stack->ppt)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_GE:
			acc.ret.inty = po_sr_ge_and_clean(
				((Pt_num *)OPTR(stack,sizeof(stack->postring)))->postring
				,stack->postring);
			stack = OPTR(stack, 2*sizeof(stack->postring)-sizeof(stack->inty));
			stack->inty = acc.ret.inty;
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * COMPARISONS - GT
 *--------------------------------------------------------------------------*/

		case OP_IGT:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty = (stack->inty > acc.ret.inty);
			break;
		case OP_LGT:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			acc.ret.inty = (stack->l > acc.ret.l);
			stack = OPTR(stack, (sizeof(stack->l)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			break;
		case OP_DGT:
			acc.ret.d = stack->d;
			stack = OPTR(stack, sizeof(stack->d));
			acc.ret.inty = (stack->d > acc.ret.d);
			stack = OPTR(stack, (sizeof(stack->d)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			if (builtin_err != Success)
				goto ERR_INLINE_FPMATH;
			break;
		case OP_PGT:
			acc.ret.ppt = stack->ppt;
			stack = OPTR(stack, sizeof(stack->ppt));
			acc.ret.inty = ((char*)(stack->ppt.pt) > ((char *)acc.ret.ppt.pt));
			stack = OPTR(stack, (sizeof(stack->ppt)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_GT:
			acc.ret.inty  =  !po_sr_le_and_clean(
				((Pt_num *)OPTR(stack ,sizeof(stack->postring)))->postring
				,stack->postring);
			stack = OPTR(stack, 2*sizeof(stack->postring)-sizeof(stack->inty));
			stack->inty = acc.ret.inty;
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * COMPARISONS - LE
 *--------------------------------------------------------------------------*/

		case OP_ILE:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty = (stack->inty <= acc.ret.inty);
			break;
		case OP_LLE:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			acc.ret.inty = (stack->l <= acc.ret.l);
			stack = OPTR(stack, (sizeof(stack->l)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			break;
		case OP_DLE:
			acc.ret.d = stack->d;
			stack = OPTR(stack, sizeof(stack->d));
			acc.ret.inty = (stack->d <= acc.ret.d);
			stack = OPTR(stack, (sizeof(stack->d)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			if (builtin_err != Success)
				goto ERR_INLINE_FPMATH;
			break;
		case OP_PLE:
			acc.ret.ppt = stack->ppt;
			stack = OPTR(stack, sizeof(stack->ppt));
			acc.ret.inty = ((char*)(stack->ppt.pt) <= ((char *)acc.ret.ppt.pt));
			stack = OPTR(stack, (sizeof(stack->ppt)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_LE:
			acc.ret.inty = po_sr_le_and_clean(
				((Pt_num *)OPTR(stack,sizeof(stack->postring)))->postring
				,stack->postring);
			stack = OPTR(stack, 2*sizeof(stack->postring)-sizeof(stack->inty));
			stack->inty = acc.ret.inty;
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * COMPARISONS - LT
 *--------------------------------------------------------------------------*/

		case OP_ILT:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty = (stack->inty < acc.ret.inty);
			break;
		case OP_LLT:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			acc.ret.inty = (stack->l < acc.ret.l);
			stack = OPTR(stack, (sizeof(stack->l)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			break;
		case OP_DLT:
			acc.ret.d = stack->d;
			stack = OPTR(stack, sizeof(stack->d));
			acc.ret.inty = (stack->d < acc.ret.d);
			stack = OPTR(stack, (sizeof(stack->d)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			if (builtin_err != Success)
				goto ERR_INLINE_FPMATH;
			break;
		case OP_PLT:
			acc.ret.ppt = stack->ppt;
			stack = OPTR(stack, sizeof(stack->ppt));
			acc.ret.inty = ((char*)(stack->ppt.pt) < ((char *)acc.ret.ppt.pt));
			stack = OPTR(stack, (sizeof(stack->ppt)-sizeof(stack->inty)));
			stack->inty = acc.ret.inty;
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_LT:
			acc.ret.inty = !po_sr_ge_and_clean(
				((Pt_num *)OPTR(stack,sizeof(stack->postring)))->postring
				,stack->postring);
			stack = OPTR(stack, 2*sizeof(stack->postring)-sizeof(stack->inty));
			stack->inty = acc.ret.inty;
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * NEGATION
 *--------------------------------------------------------------------------*/

		case OP_INEG:
			stack->inty = -stack->inty;
			break;
		case OP_LNEG:
			stack->l = -stack->l;
			break;
		case OP_DNEG:
			stack->d = -stack->d;
			break;

/*----------------------------------------------------------------------------
 * MODULO
 *--------------------------------------------------------------------------*/

		case OP_IMOD:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty %= acc.ret.inty;
			break;
		case OP_LMOD:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			stack->l %= acc.ret.l;
			break;

/*----------------------------------------------------------------------------
 * SHIFT LEFT
 *--------------------------------------------------------------------------*/

		case OP_ILSHIFT:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty <<= acc.ret.inty;
			break;
		case OP_LLSHIFT:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			stack->l <<= acc.ret.l;
			break;

/*----------------------------------------------------------------------------
 * SHIFT RIGHT
 *--------------------------------------------------------------------------*/

		case OP_IRSHIFT:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty >>= acc.ret.inty;
			break;
		case OP_LRSHIFT:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			stack->l >>= acc.ret.l;
			break;

/*----------------------------------------------------------------------------
 * BINARY AND
 *--------------------------------------------------------------------------*/

		case OP_IBAND:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty = stack->inty & acc.ret.inty;
			break;
		case OP_LBAND:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			stack->l = stack->l & acc.ret.l;
			break;

/*----------------------------------------------------------------------------
 * BINARY OR
 *--------------------------------------------------------------------------*/

		case OP_IBOR:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty = stack->inty | acc.ret.inty;
			break;
		case OP_LBOR:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			stack->l = stack->l | acc.ret.l;
			break;

/*----------------------------------------------------------------------------
 * BINARY XOR
 *--------------------------------------------------------------------------*/

		case OP_IXOR:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty = stack->inty ^ acc.ret.inty;
			break;
		case OP_LXOR:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			stack->l = stack->l ^ acc.ret.l;
			break;

/*----------------------------------------------------------------------------
 * LOGICAL AND
 *--------------------------------------------------------------------------*/

		case OP_ILAND:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty = stack->inty && acc.ret.inty;
			break;
		case OP_LLAND:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			stack->l = stack->l && acc.ret.l;
			break;

/*----------------------------------------------------------------------------
 * LOGICAL OR
 *--------------------------------------------------------------------------*/

		case OP_ILOR:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			stack->inty = stack->inty || acc.ret.inty;
			break;
		case OP_LLOR:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			stack->l = stack->l || acc.ret.l;
			break;

/*----------------------------------------------------------------------------
 * BINARY NOT
 *--------------------------------------------------------------------------*/

		case OP_ICOMP:
			stack->inty = ~stack->inty;
			break;
		case OP_LCOMP:
			stack->l = ~stack->l;
			break;

/*----------------------------------------------------------------------------
 * LOGICAL NOT
 *--------------------------------------------------------------------------*/

		case OP_INOT:
			stack->inty = !stack->inty;
			break;
		case OP_LNOT:
			stack->l = !stack->l;
			break;

/*----------------------------------------------------------------------------
 * PUSH ACCUMULATOR TO STACK
 *--------------------------------------------------------------------------*/

		case OP_IPUSH:
			stack = OPTR(stack, -sizeof(stack->inty));
			stack->inty = acc.ret.inty;
			break;
		case OP_LPUSH:
		case OP_CPPUSH:
			stack = OPTR(stack, -sizeof(stack->l));
			stack->l = acc.ret.l;
			break;
		case OP_DPUSH:
			stack = OPTR(stack, -sizeof(stack->d));
			stack->d = acc.ret.d;
			break;
		case OP_PPUSH:
			stack = OPTR(stack, -sizeof(stack->ppt));
			stack->ppt = acc.ret.ppt;
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_PUSH:
			stack = OPTR(stack, -sizeof(stack->postring));
			stack->postring = acc.ret.postring;
			break;
#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * POP STACK TO ACCUMULATOR (RETURN VALUE)
 *--------------------------------------------------------------------------*/

		case OP_IPOP:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty));
			break;
		case OP_LPOP:
		case OP_CPPOP:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l));
			break;
		case OP_DPOP:
			acc.ret.d = stack->d;
			stack = OPTR(stack, sizeof(stack->d));
			break;
		case OP_PPOP:
			acc.ret.ppt = stack->ppt;
			stack = OPTR(stack, sizeof(stack->ppt));
			break;
#ifdef STRING_EXPERIMENT
		case OP_STRING_POP:
			acc.ret.postring = stack->postring;
			stack = OPTR(stack, sizeof(stack->postring));
			break;
		case OP_CLEAN_STRING:	/* Pop string and dec reference count */
			acc.ret.postring = stack->postring;
			po_sr_clean_ref(acc.ret.postring);
			stack = OPTR(stack, sizeof(stack->postring));
			break;
#endif /* STRING_EXPERIMENT */


/*----------------------------------------------------------------------------
 * DUPLICATE TOP-OF-STACK ITEM
 *--------------------------------------------------------------------------*/

		case OP_IDUPE:
			stack = OPTR(stack, -sizeof(stack->inty));
			stack->inty = ((int *)(OPTR(stack, sizeof(stack->inty))))[0];
			break;
		case OP_LDUPE:
			stack = OPTR(stack, -sizeof(stack->l));
			stack->l = ((long *)(OPTR(stack, sizeof(stack->l))))[0];
			break;
		case OP_DDUPE:
			stack = OPTR(stack, -sizeof(stack->d));
			stack->d = ((double *)(OPTR(stack, sizeof(stack->d))))[0];
			break;
		case OP_PDUPE:
			stack = OPTR(stack, -sizeof(stack->ppt));
			stack->ppt = ((Popot *)(OPTR(stack, sizeof(stack->ppt))))[0];
			break;

/*----------------------------------------------------------------------------
 * POINTER MATH
 *--------------------------------------------------------------------------*/

		case OP_ADD_IOFFSET:
			acc.ret.inty = stack->inty;
			stack = OPTR(stack, sizeof(stack->inty) );
			stack->ppt.pt = OPTR(stack->ppt.pt, acc.ret.inty);
			break;
		case OP_ADD_LOFFSET:
			acc.ret.l = stack->l;
			stack = OPTR(stack, sizeof(stack->l) );
			stack->ppt.pt = OPTR(stack->ppt.pt, acc.ret.l);
			break;
		case OP_PTRDIFF:					/* subtract two pointers */
			acc.ret.ppt.pt = stack->ppt.pt;
			stack = OPTR(stack, sizeof(stack->ppt));
			acc.ret.l = (char *)stack->ppt.pt - (char *)acc.ret.ppt.pt;
			stack = OPTR(stack, sizeof(stack->ppt)-sizeof(long));
			stack->l = acc.ret.l / ip->inty; /* scale result */
			ip = OPTR(ip, sizeof(ip->inty));
			break;

/*----------------------------------------------------------------------------
 * MISCELLANIOUS
 *--------------------------------------------------------------------------*/

		case OP_COPY:
			poco_copy_bytes(
				((Popot *)OPTR(stack, sizeof(stack->ppt)))->pt,
				stack->ppt.pt,
				ip->l);
			ip = OPTR(ip, sizeof(ip->l));
			stack = OPTR(stack, 2*sizeof(stack->ppt));
			break;
		case OP_MOVE:
			poco_copy_bytes(
				stack->ppt.pt,
				((Popot *)OPTR(stack, sizeof(stack->ppt)))->pt,
				ip->l);
			ip = OPTR(ip, sizeof(ip->l));
			stack = OPTR(stack, 2*sizeof(stack->ppt));
			break;
#ifdef STRING_EXPERIMENT
		case OP_FREE_STRING:
			po_sr_clean_ref((((PoString *)(OPTR(base, ip->doff)))[0]));
			ip = OPTR(ip, INTY_SIZE);
			break;
#endif /* STRING_EXPERIMENT */
		case OP_NOP:
			break;
		}
	}

ABORT:

	err = Err_abort;
	goto DEALLOC_AND_EXIT;

ERR_NOTAFUNC:

	err = Err_function_not_found;
	goto DEBUG;

ERR_NULL:

	err = Err_null_ref;
	goto DEBUG;

ERR_SMALL:

	err = Err_index_small;
	goto DEBUG;

ERR_BIG:

	err = Err_index_big;
	goto DEBUG;

ERR_INLINE_FPMATH:		// the host has indicated an 80x87 math err happened
						// while interpreting poco instructions.  we remember
	err = builtin_err;		  // the status and clear the global status in builtin_err.
	builtin_err = Success;	  // this causes the DEBUG tracing to report the error
	goto DEBUG; 		// as occurring in the poco code, not a lib routine.

ERR_IN_LIBROUTINE:

	err = builtin_err;
	if (err == Err_poco_exit)			// the ONLY thing that can set this
		{								// is poco's builtin exit() function,
		builtin_err = err = Success;	// invoked with a code >= Success.
		goto DEALLOC_AND_EXIT;			// this gets us out with a good status.
		}

	if (err == Err_early_exit || err == Err_abort)
		goto DEALLOC_AND_EXIT;

	goto DEBUG;

DEBUG:

	if (!pe->enable_debug_trace)
		goto DEALLOC_AND_EXIT;

	if (err != Err_in_err_file)
		{
		char buf[128];

		if (pe->trace_file == NULL)
			tfile = stdout;
		else if ((tfile = fopen(pe->trace_file, "w")) == NULL)
			goto DEALLOC_AND_EXIT;
		if (tfile != NULL)
			{
			get_errtext(err, buf);
			fprintf(tfile, "%s ", buf);
			po_print_trace(pe,tfile,stack,base,globals,ip,builtin_err);
			if (tfile != stdout)
				fclose(tfile);
			err = Err_in_err_file;
			goto DEALLOC_AND_EXIT;
			}
		}

DEALLOC_AND_EXIT:

	if (pe->stack == NULL)
		pj_free(stack_area);

	return err;
}

Errcode po_run_ops(Poco_run_env *p, Code *code_pt, Pt_num *pret)
/*****************************************************************************
 * set up the runtime environment and call the interpreter.
 *
 * this must be called only by the routines in pocoface.c or fold.c.
 * to call from a lib/poe routine back into poco via a pointer passed to
 * you from a poco program you must enter at the poco_cont_ops routine above.
 ****************************************************************************/
{
	Pt_num dummy_ret;

	pe	 = p;
	if (pret == NULL)
		pret = &dummy_ret;

	return(poco_cont_ops(code_pt, pret, 0));
}
