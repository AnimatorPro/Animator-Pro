/*****************************************************************************
 *
 * funccall.c - Code generation routines for making a function call.
 *
 *	10/04/90	(Ian)
 *				Converted sprintf/po_say_fatal pairs to po_say_fatal w/formatting.
 *				Fixed reporting of not enough/too many params in function
 *				mk_function_call().  It used to report po_expecting_got type
 *				messages, now it reports not enough or too many parms.
 *	10/26/90	(Ian)
 *				Fixed a glitch in paramter counting/checking...we were
 *				ensuring that there were pcount-1 parms, now we ensure that
 *				there are pcount parms unless the last parm is an ellipsis,
 *				in which case pcount-1 parms are allowed.  (This bug was
 *				introduced in the 10/04/90 changes.)
 *	10/27/90	(Ian)
 *				Added handling of ANSI-type indirect function calls, such
 *				that "ptr()" is equivelent to "(*ptr)()".
 *	02/11/91	(Ian)
 *				Added logic to push param size and count for variable args
 *				passed to C (lib and poe) functions.  This allows the C code
 *				to do some parm checking on the variable part of the args,
 *				at least to the degree of ensuring the right number were
 *				passed for the given call.	The count is the count of variable
 *				args only, it does not include the number of fixed args, or
 *				itself; likewise the size.	Thus, the call:
 *					printf(fmtstr, int1, int2, double1, ptr1);
 *				would give a count of 4 and a size of 20.
 *	09/06/91	(Jim)
 *				Changed TYPE_NCPT to TYPE_CPT for parameters to library
 *				functions following the ellipsis.
 *				Made string-parameters get converted to TYPE_CPT's in
 *				that situation too.
 *				Changed OP_IPUSH+idot to a call to po_find_push_op().
 *	06/12/92	(Ian)
 *				Changed po_get_function() to clear the left code buffer
 *				after the call is generated.  Also, if the return value type
 *				of the function is TYPE_POINTER, we then code an OP_NOP into
 *				the left buffer.  The presence or abscence of code in the
 *				left buffer is the key used by other parts of the parser
 *				to determine whether we have an lvalue at any given point.
 *				The presence of the OP_NOP is enough to say that an lvalue
 *				is present, allowing a get_dereference() to work.  During
 *				the processing of get_dereference(), the call code we've
 *				built into the right buffer gets copied to the left buffer,
 *				replacing the OP_NOP with the code that actually generates
 *				the lvalue (the call).
 ****************************************************************************/

#include "poco.h"

int po_get_param_size(Poco_cb *pcb, SHORT ido_type)
/*****************************************************************************
 * return the size in bytes of a parameter to a function.
 ****************************************************************************/
{
int psize;

switch (ido_type)
	{
	case IDO_INT:
		psize = sizeof(int);
		break;
	case IDO_LONG:
		psize = sizeof(long);
		break;
	case IDO_DOUBLE:
		psize = sizeof(double);
		break;
	case IDO_POINTER:
		psize = sizeof(Popot);
		break;
	case IDO_CPT:
		psize = sizeof(void *);
		break;
#ifdef STRING_EXPERIMENT
	case IDO_STRING:
		psize = sizeof(PoString);
		break;
#endif /* STRING_EXPERIMENT */
	case IDO_VPT:
		po_say_fatal(pcb, "missing '*' in parameter to function??");
		break;
	default:
		po_say_fatal(pcb, "cannot pass structure by value (perhaps missing '*'?)");
		break;
	}
return(psize);
}


static void mk_function_call(Poco_cb *pcb, Exp_frame *e,
	Func_frame *fff, SHORT ftype)
/*****************************************************************************
 * generate code to push parms to stack and make call to function.
 ****************************************************************************/
{
Symbol		*param;
SHORT		expected_count;
SHORT		idot;
SHORT		param_count = 0;
int 		param_size = 0;
SHORT		varg_param_count = 0;
int 		varg_param_size = 0;
int 		this_param_size;
Boolean 	is_cvarg_call = FALSE;
Exp_frame	*param_exps = NULL;
Exp_frame	*exp;
Code_buf	callcode;

po_init_code_buf(pcb, &callcode);
param = fff->parameters;
expected_count = fff->pcount;
for (;;)
	{
	if (param == NULL)
		break;
	if (po_is_next_token(pcb, TOK_RPAREN))
		break;

	exp = po_new_expframe(pcb);
	exp->next = param_exps;
	param_exps = exp;
	po_get_expression(pcb, exp);

	if (param->flags & SFL_ELLIP)
		{
		if (fff->type == CFF_C)
			{ /* ptr parms passed to c functions get un-poco-pointerized */
			if (po_is_pointer(&exp->ctc) || po_is_array(&exp->ctc))
				{
				po_code_op(pcb, &exp->ecd, OP_PPT_TO_CPT);
				exp->ctc.comp[exp->ctc.comp_count-1] = TYPE_CPT;
				exp->ctc.ido_type = IDO_CPT;
				}
#ifdef STRING_EXPERIMENT
			else if (po_is_string(&exp->ctc))
				{
				po_code_op(pcb, &exp->ecd, OP_STRING_TO_CPT);
				exp->ctc.comp[0] = TYPE_CHAR;
				exp->ctc.comp[1] = TYPE_CPT;
				exp->ctc.ido_type = IDO_CPT;
				}
#endif /* STRING_EXPERIMENT */
			this_param_size = po_get_param_size(pcb, exp->ctc.ido_type);
			varg_param_size += this_param_size;
			++varg_param_count;
			}
		}
	else
		{
		po_coerce_expression(pcb, exp, param->ti, FALSE);
		this_param_size = po_get_param_size(pcb, exp->ctc.ido_type);
		}

	++param_count;
	param_size += this_param_size;

	if (!(param->flags & SFL_ELLIP))
		param = param->link;

	po_need_token(pcb);

	if (param != NULL)
		{
		if (param->flags&SFL_ELLIP)
			{
			if (pcb->t.toktype != ',')
				{
				if ( pcb->t.toktype != ')')
					po_expecting_got(pcb, ", or )");
				else
					pushback_token(&pcb->t);
				}
			}
		else
			{
			if (pcb->t.toktype != ',')
				{
				if (pcb->t.toktype == ')')
					goto NOT_ENOUGH_PARMS;
				else
					po_expecting_got(pcb, ",");
				}
			}
		}
	else
		{
		pushback_token(&pcb->t);
		}
	}

if (param != NULL && (param->flags & SFL_ELLIP))
	{
	--expected_count;
	if (fff->type == CFF_C)
		is_cvarg_call = TRUE;
	}

if (param_count < expected_count)
	{
NOT_ENOUGH_PARMS:
	po_say_fatal(pcb, "not enough parameters in call to function '%s'",fff->name);
	}

if (pcb->t.toktype == ',')
	{
	po_say_fatal(pcb, "too many parameters in call to function '%s'", fff->name);
	}

po_eat_rparen(pcb);

/* Ok, currently the parameters are (backwards) in the param_exps list and
	the function calling code is in e.	Must reorder things so parameters
	go first in e and then the calling code. */

if (e->left_complex)
	po_copy_code(pcb, &e->ecd, &callcode);

clear_code_buf(pcb, &e->ecd);

exp = param_exps;
while (exp != NULL)
	{
	po_cat_code(pcb, &e->ecd, &exp->ecd);
	exp = exp->next;
	}

if (is_cvarg_call)
	{
	po_code_long(pcb, &e->ecd, OP_LCON, varg_param_size);
	po_code_long(pcb, &e->ecd, OP_LCON, varg_param_count);
	param_size += 2*sizeof(long); /* for stack cleanup instruction later */
	}

po_cat_code(pcb, &e->ecd, &callcode);

idot = e->ctc.ido_type;
if (e->left_complex)
	{
	po_code_op(pcb, &e->ecd, OP_CALLI);
	}
else
	{
	if (fff->type == CFF_C)
		{
		po_code_void_pt(pcb, &e->ecd, po_ccall_ops[idot],
			((C_frame *)fff)->code_pt);
		}
	else
		{
		po_code_void_pt(pcb, &e->ecd, OP_PCALL, fff);
		}
	}

if (param_size > 0)
	{
	po_code_int(pcb, &e->ecd, OP_ADD_STACK, param_size);
	}

if (idot != IDO_VOID)	/* non-void type */
	{
	po_code_op(pcb, &e->ecd, po_find_push_op(pcb,&e->ctc));
	}

e->includes_function = TRUE;
while (param_exps != NULL)
	{
	exp = param_exps->next;
	po_dispose_expframe(pcb, param_exps);
	param_exps = exp;
	}
po_trash_code_buf(pcb, &callcode);
}

void po_get_function(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * entry point from statement parsing.	find function, then code the call.
 *	upon entry, we've seen the opening paren of the function call, and
 *	the expression frame (e) already holds the code to access the symbol
 *	which is the name of the function.
 *
 * if we have a pointer to a function, that wasn't explicitly dereferenced,
 * we code the dereference part first, and adjust the expression type.
 * (allows function calls as either "(*ptr)()" or "ptr()".  in the former
 * case, the deref has already been coded, we handle the latter case here.)
 *
 ****************************************************************************/
{
Type_info	*ti = &e->ctc;
int 		end_type = ti->comp[ti->comp_count-1];
Func_frame	*fuf;


	if (end_type == TYPE_POINTER &&
		ti->comp_count > 2 &&
		ti->comp[ti->comp_count-2] == TYPE_FUNCTION)
		{
		e->ctc.comp_count -= 1;
		end_type = ti->comp[ti->comp_count-1];
		po_set_ido_type(&e->ctc);
		po_make_deref(pcb,e);
		}

	if (end_type != TYPE_FUNCTION)
		{
		po_say_fatal(pcb, "trying to call something that's not a function");
		return;
		}

	fuf = ti->sdims[ti->comp_count-1].pt;

	e->ctc.comp_count -= 1;
	po_set_ido_type(&e->ctc);
	end_type = ti->comp[ti->comp_count-1];

	mk_function_call(pcb, e, fuf, end_type);

	clear_code_buf(pcb, &e->left);	/* if we had code in the left side, nuke  */
	e->left_complex = FALSE;		/* it, we've now processed it into a call.*/

	if (end_type == TYPE_POINTER)			/* if function returns a pointer  */
		{									/* put a NOP into the left buffer,*/
		po_code_op(pcb, &e->left, OP_NOP);	/* to signal we do have an lval.  */
		}									/* if it gets deref'd the NOP gets*/
}											/* replaced by the call code later*/
