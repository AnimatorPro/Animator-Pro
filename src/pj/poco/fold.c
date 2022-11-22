/*****************************************************************************
 *
 * fold.c - Evaluate constant expressions into simple constant values.
 *
 * Expressions containing only constants and operators are evaluated by
 * executing the expression on the po_run_ops() interpreter.
 *
 * (Note to self:  Hey, what happens if we get a RUNOPS error during the
 *	compile phase???  Does this have implications for the setjmp/longjmp
 *	logic?	Can it happen?)
 *
 * MAINTENANCE
 *	10/02/90	(Ian)
 *				Added po_eval_const_expression() routine as part of the
 *				support for handling enums.  This routine is very much like
 *				the po_fold_const() routine, except that it returns the value
 *				of the constant to the caller instead of generating code to
 *				put the constant on the stack at runtime.
 *				Right now, the routine is coded to handle only integer
 *				expresssions; it could be made to do any numeric type with
 *				a little work, if the need should arise.
 *	10/06/90	(Ian)
 *				Increased fold_stack from 2 to 16 times sizeof(double).
 *	05/12/92	(Ian)
 *				Added new routine po_is_static_init_const().
 ****************************************************************************/

#include "poco.h"

static char 		fold_stack[16*sizeof(double)];
static Poco_run_env foldenv = {fold_stack, sizeof(fold_stack), FALSE};

void po_fold_const(Poco_cb *pcb, Exp_frame *exp)
/*****************************************************************************
 * a little shell to run the interpreter on an expression's code_buf.
 ****************************************************************************/
{
SHORT	 csize;
Code_buf *cb;
Errcode  err;

//	printf("\nWant to fold_const this code (%d):\n", exp->pure_const);
//	po_dump_codebuf(pcb, &exp->ecd);

	if (!exp->pure_const)
		return;

	cb = &exp->ecd;
	po_code_op(pcb, cb, OP_END);
	if (Success > (err = po_run_ops(&foldenv, cb->code_buf, NULL)))
		{
		pcb->global_err = err;
		po_say_fatal(pcb, "cannot evaluate constant expression");
		}
	csize = po_get_type_size(&exp->ctc);
	clear_code_buf(pcb, cb);
	po_add_op(pcb, cb, po_con_ops[exp->ctc.ido_type],
		fold_stack+sizeof(fold_stack)-csize-sizeof(void *), csize);

//	printf("Did it, now it looks like:\n");
//	po_dump_codebuf(pcb, &exp->ecd);

}

int po_eval_const_expression(Poco_cb *pcb, Exp_frame *exp)
/*****************************************************************************
 * return the value of an integer constant expression.
 ****************************************************************************/
{
Code_buf *cb;
Errcode  err;

if (!exp->pure_const)
	po_say_fatal(pcb, "integer constant expression required");

po_coerce_numeric_exp(pcb, exp, IDO_INT);

cb = &exp->ecd;
po_code_op(pcb, cb, OP_END);
if (Success > (err = po_run_ops(&foldenv, cb->code_buf, NULL)))
	{
	pcb->global_err = err;
	po_say_fatal(pcb, "cannot evaluate constant expression");
	}

return *(fold_stack + sizeof(fold_stack) - sizeof(int) - sizeof(void *));

}

Boolean po_is_static_init_const(Poco_cb *pcb, Code_buf *cb)
/*****************************************************************************
 * return TRUE if code in buffer qualifies as purely constant by the rules
 * of static initializer expressions.
 ****************************************************************************/
{
	int 			op;
	Poco_op_table	*pta  = po_ins_table;
	void			*code;
	Boolean 		rv = TRUE;

//	printf("\nTesting is_static_init_const on this code:\n");
//	po_dump_codebuf(pcb, cb);

	for (code = cb->code_buf; code < cb->code_pt; /* nothing */) {
		op = *(int *)code;
		if (pta[op].op_flags & OFL_NOTCON) {
			rv = FALSE;
			break;
		}
		code = OPTR(code, pta[op].op_size+sizeof(op));
	}

//	printf("Result: %d\n\n", rv);

	return rv;
}
