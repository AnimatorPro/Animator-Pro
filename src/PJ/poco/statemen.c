/*****************************************************************************
 *
 * statemen.c - parsing of statement level syntax and code generation.
 *
 * Mostly deals with calculating jump destination addresses and
 * making sure there are enough semi-colons to go around.
 *
 *	08/25/90	(Ian)
 *				Changed all occurances of freemem() and gentle_freemem() to
 *				po_freemem() and poc_gentle_freemem().
 *	08/28/90	(Ian)
 *				Changed logic in statement() so that the po_add_line_data()
 *				routine won't be called if the line number is zero.
 *	09/18/90	(Ian)
 *				Added case of PTOK_UNION to the statement() routine.
 *	09/19/90	(Ian)
 *				Changes to support all ANSI type keywords (register, const,
 *				etc).  These are all now classified as PTOK_TYPE by the parser
 *				which is reflected herein in the statement() routine, by
 *				removing a bunch of individual cases which are now all covered
 *				by the single PTOK_TYPE token.
 *	10/01/90	(Ian)
 *				Changed coding of OP_LEAVE; it no longer has immediate data
 *				as part of the instruction.
 *	10/07/90	(Ian)
 *				Removed NULL checks from calls to memory allocation.
 *	10/29/90	(Ian)
 *				Changed get_if, get_do, get_while, and get_for to allocate
 *				their conditional expression frames from the cache instead
 *				of allocating them local on the stack.
 *	09/06/91	(Jim)
 *		o		Made coding of OP_POP's to clean off expression stack
 *				go through po_find_clean_op() instead of the
 *				somewhat wierd (OP_IPOP+ido_type) technique.
 *		o		Fixed bug where for (void-exp; xxx ; xxx) or
 *								for (xxx; xxx; void-exp)
 *				would cause problems with bogus-pop-code being generated
 *				and subsequent Poco bombs.	Eliminated pop_result() in
 *				favor of safer and more general po_pop_off_result()
 ****************************************************************************/

#include "poco.h"

#define COMMA_OR_RBRACE "} or ,"

static void statement(Poco_cb *pcb, Poco_frame *pf);

#ifdef DEADWOOD

/* Type structure for simple integer */
static TypeComp ity_comp[1] = {TYPE_INT,};
static LONG ity_dims[1] = {0,};
static Type_info ity = { ity_comp, (Pt_long *)ity_dims, 1, 1, IDO_INT,};

/* Type structure for ... in function parameters */
static TypeComp ely_comp[1] = {TYPE_ELLIPSIS,};
static LONG ely_dims[1] = {0,};
static Type_info ely = { ely_comp, (Pt_long *)ely_dims, 1, 1, IDO_BAD,};
#endif /* DEADWOOD */

Boolean po_eat_semi(Poco_cb *pcb)
/*****************************************************************************
 * if the next token is a semicolon eat it, else complain and 'insert' one.
 ****************************************************************************/
{
lookup_token(pcb);
if (pcb->t.toktype == ';')
	return(TRUE);
else
	{
	po_say_warning(pcb, "Missing semicolon (inserting...)");
	pushback_token(&pcb->t);
	}
return(FALSE);
}

Boolean po_eat_rbrace(Poco_cb *pcb)
/*****************************************************************************
 * if next token is closing brace, eat it, else complain and die.
 ****************************************************************************/
{
return(po_eat_token(pcb, TOK_RBRACE));
}

Boolean po_eat_lbrace(Poco_cb *pcb)
/*****************************************************************************
 * if next token is opening brace, eat it, else complain and die.
 ****************************************************************************/
{
return(po_eat_token(pcb, TOK_LBRACE));
}


void po_get_statements(Poco_cb *pcb, Poco_frame *d)
/*****************************************************************************
 * get list of declarations until EOF or closing brace.
 ****************************************************************************/
{
for (;;)
	{
	if (po_is_next_token(pcb, TOK_RBRACE))
		return;
	statement(pcb,d);
	}
}

void po_get_block(Poco_cb *pcb, Poco_frame *d)
/*****************************************************************************
 * get block of statements from just past open brace to closing brace.
 ****************************************************************************/
{
po_get_statements(pcb, d);
po_eat_rbrace(pcb);
}

int po_need_comma_or_brace(Poco_cb *pcb)
/*****************************************************************************
 * return next token if it's a comma or closing brace; if not, complain & die.
 ****************************************************************************/
{
int type;

if (!po_need_token(pcb))
	return(0);
switch (type = pcb->t.toktype)
	{
	case ',':
	case TOK_RBRACE:
		return(type);
	default:
		po_expecting_got(pcb, COMMA_OR_RBRACE);
		return(0);
	}
}


void po_check_array_dim(Poco_cb *pcb, Symbol *var)
/*****************************************************************************
 * ensure we have an array's dimension specified or implied via init data.
 ****************************************************************************/
{
Type_info *ti;
int cct;

ti = var->ti;
cct = ti->comp_count-1;
if (ti->comp[cct] == TYPE_ARRAY)
	{
	if (ti->sdims[cct].l == 0)
		{
		po_say_fatal(pcb, "need array dimension or initialization");
		}
	}
}

static void warn_no_effect(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * warn the dweeb (er, user) that code such as ";a+2;" is pointless.
 ****************************************************************************/
{
if (!e->includes_assignment && !e->includes_function)
	po_say_warning(pcb, "Warning, code has no effect");
}


static Code_label *new_label(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * alloc and init a new code_label struct, attach it to a poco_frame.
 ****************************************************************************/
{
Code_label *new;

new = po_memzalloc(pcb, sizeof(*new));
new->next = pf->labels;
pf->labels = new;
return(new);
}

static Use_label *new_use(Poco_cb *pcb, Code_label *l, long code_pos)
/*****************************************************************************
 * alloc & init a new use_label struct, attach it to its code_label.
 ****************************************************************************/
{
Use_label *new;

new = po_memzalloc(pcb, sizeof(*new));
new->next = l->uses;
l->uses = new;
new->code_pos = code_pos;
return(new);
}


Code_label *po_label_to_symbol(Poco_cb *pcb, Poco_frame *pf, Symbol *lsym)
/*****************************************************************************
 * alloc a code_label and attach it to its parent symbol.
 ****************************************************************************/
{
Code_label *cl;

if ((cl = lsym->symval.p = new_label(pcb, pf)) != NULL)
	{
	cl->lvar = lsym;
	lsym->tok_type = PTOK_LABEL;
	}
return(cl);
}

static Loop_frame *start_loop(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * alloc & init a loop_frame, attach it to the parent poco_frame.
 ****************************************************************************/
{
Loop_frame *lf;

lf = po_memzalloc(pcb, sizeof(*lf));
if (((lf->start = new_label(pcb, pf)) == NULL)
	|| ((lf->end = new_label(pcb, pf)) == NULL))
	{
	po_freemem(lf);
	return(NULL);
	}
lf->next = pcb->loops;
pcb->loops = lf;
return(lf);
}

static void end_loop(Poco_cb *pcb)
/*****************************************************************************
 * detach a loop_frame from its poco_frame and free it.
 ****************************************************************************/
{
Loop_frame *lf;

lf = pcb->loops;
pcb->loops = lf->next;
po_freemem(lf);
}


static void get_while(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * parse and code a while() statement.
 ****************************************************************************/
{
Exp_frame  *ef;
Loop_frame *lf;
long cpos;

	if ((lf = start_loop(pcb,pf)) == NULL)
		return;
	lf->start->code_pos = po_cbuf_code_size(&pf->fcd);
	ef = po_new_expframe(pcb);
	if (!po_eat_lparen(pcb))
		goto OUT;
	po_get_expression(pcb, ef);
	if (!po_eat_rparen(pcb))
		goto OUT;
	po_coerce_to_boolean(pcb, ef);
	if (!po_cat_code(pcb, &pf->fcd, &ef->ecd))
		goto OUT;
	cpos = po_cbuf_code_size(&pf->fcd) + OPY_SIZE;
	if (new_use(pcb, lf->end, cpos) == NULL)
		goto OUT;
	po_code_int(pcb, &pf->fcd, OP_BEQ, 0);
	statement(pcb, pf);
	cpos = po_cbuf_code_size(&pf->fcd) + OPY_SIZE;
	if (new_use(pcb, lf->start, cpos) == NULL)
		goto OUT;
	po_code_int(pcb, &pf->fcd, OP_BRA, 0);
	lf->end->code_pos = po_cbuf_code_size(&pf->fcd);
OUT:
	po_dispose_expframe(pcb, ef);
	end_loop(pcb);
	return;
}

static void get_do(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * parse and code a do {stuff} while(); statement.
 * upon entry to this routine, the 'do' has been parsed.  we cruise until
 * we eat the semicolon after the while().
 ****************************************************************************/
{
Exp_frame  *ef;
Loop_frame *lf;
long cpos;

	if ((lf = start_loop(pcb,pf)) == NULL)
		return;
	lf->start->code_pos = po_cbuf_code_size(&pf->fcd);

	ef = po_new_expframe(pcb);

	/* parse the body of the do ... while (); */
	statement(pcb, pf);

	/* parse the while () */
	if (!po_need_token(pcb))
		goto OUT;
	if (pcb->t.toktype != PTOK_WHILE)
		{
		po_expecting_got(pcb, "while");
		goto OUT;
		}
	if (!po_eat_lparen(pcb))
		goto OUT;
	po_get_expression(pcb, ef);
	if (!po_eat_rparen(pcb))
		goto OUT;

	/* generate code for the conditional expression and the branches */
	po_coerce_to_boolean(pcb, ef);
	if (!po_cat_code(pcb, &pf->fcd, &ef->ecd))
		goto OUT;

	cpos = po_cbuf_code_size(&pf->fcd) + OPY_SIZE;
	if (new_use(pcb, lf->start, cpos) == NULL)
		goto OUT;
	po_code_int(pcb, &pf->fcd, OP_BNE, 0);
	lf->end->code_pos = po_cbuf_code_size(&pf->fcd);
	po_eat_semi(pcb);
OUT:
	po_dispose_expframe(pcb, ef);
	end_loop(pcb);
	return;
}

static void offset_last_case_beq(Poco_cb *pcb, Poco_frame *pf, Loop_frame *lf)
/*****************************************************************************
 * fixup the offset when the last case is a beq.
 ****************************************************************************/
{
int lcb;
Code *cbuf;

if ((lcb = lf->last_case_beq) != 0)
	{
	cbuf = pf->fcd.code_buf + lcb;
	((int *)(cbuf))[0] = pf->fcd.code_pt - cbuf;
	}
}

static void get_switch(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * parse and code switch() {stuff} statement.  continue until we eat RBRACE.
 ****************************************************************************/
{
Exp_frame ef;
Loop_frame *lf;

if ((lf = start_loop(pcb,pf)) == NULL)
	return;
lf->is_switch = TRUE;
lf->start->code_pos = po_cbuf_code_size(&pf->fcd);	/* not really used... */

/* get the expression inside parens of	switch (expr) */
po_init_expframe(pcb, &ef);
if (!po_eat_lparen(pcb))
	goto OUT;
po_get_expression(pcb, &ef);
if (po_force_int_exp(pcb, &ef.ctc) < 0)
	goto OUT;
if (!po_eat_rparen(pcb))
	goto OUT;
lf->con_type = &ef.ctc;

/* push the initial expression */
if (!po_cat_code(pcb, &pf->fcd, &ef.ecd))
	goto OUT;

/* assign it to a temporary */
po_code_int(pcb, &pf->fcd, po_find_local_assign(pcb, &ef.ctc),
	lf->svar_offset = po_get_temp_space(pcb, po_get_type_size(&ef.ctc)));
po_code_op(pcb, &pf->fcd, po_find_clean_op(pcb, &ef.ctc));

/* make sure there's an opening brace and either a case or default
   following the switch (expr) */
if (!po_eat_lbrace(pcb))
	goto OUT;
if (!po_need_token(pcb))
	goto OUT;
switch (pcb->t.toktype)
	{
	case PTOK_CASE:
	case PTOK_DEFAULT:
	case TOK_RBRACE:
		break;
	default:
		po_expecting_got(pcb, "case or default");
		goto OUT;
	}
pushback_token(&pcb->t);

/* Now go slurp up code until the matching close brace */
po_get_block(pcb, pf);
lf->end->code_pos = po_cbuf_code_size(&pf->fcd);
offset_last_case_beq(pcb, pf, lf);
OUT:
po_trash_expframe(pcb, &ef);
end_loop(pcb);
return;
}

static void get_case_after(Poco_cb *pcb, Poco_frame *pf, Loop_frame *lf)
/*****************************************************************************
 * parse case statement, generate code for test. also handles 'fall thru'.
 ****************************************************************************/
{
Exp_frame ef;
Type_info *lft = lf->con_type;
long fixp = 0;

if (lf->last_case_beq != 0)
	{
	/* have fall through from last case jump past test for this case */
	fixp = po_code_int(pcb, &pf->fcd, OP_BRA, 0);
	/* direct negative result of last case beq to here */
	offset_last_case_beq(pcb, pf, lf);
	}
po_init_expframe(pcb, &ef);
po_get_expression(pcb, &ef);
if (!ef.pure_const)
	{
	po_say_fatal(pcb, "value for case must be a constant expression");
	goto OUT;
	}
po_coerce_expression(pcb, &ef, lft, FALSE);
po_eat_token(pcb, ':');

/* now generate code to test switch temp variable against constant */
po_code_int(pcb, &pf->fcd, po_find_local_use(pcb, lft), lf->svar_offset);
po_code_op(pcb, &ef.ecd, po_eq_ops[lft->ido_type]);
po_coerce_to_boolean(pcb, &ef);
if (!po_cat_code(pcb, &pf->fcd, &ef.ecd))
	goto OUT;
/* now issue goto next case... */
lf->last_case_beq = po_cbuf_code_size(&pf->fcd) + OPY_SIZE;
po_code_int(pcb, &pf->fcd, OP_BEQ, 2);
if (fixp != 0)
	po_int_fixup(&pf->fcd, fixp,po_cbuf_code_size(&pf->fcd)-fixp);
OUT:
po_trash_expframe(pcb, &ef);
}

Loop_frame *po_get_top_switch(Poco_cb *pcb)
/*****************************************************************************
 * return pointer to loop_frame associated with innermost switch statement.
 ****************************************************************************/
{
Loop_frame *lf;

lf = pcb->loops;
while (lf != NULL)
	{
	if (lf->is_switch)
		{
		break;
		}
	lf = lf->next;
	}
return(lf);
}

static void get_case(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * parse a 'case' statement, call case code gen or issue error if needed.
 ****************************************************************************/
{
Loop_frame *lf;

if ((lf = po_get_top_switch(pcb)) == NULL)
	{
	po_say_fatal(pcb, "case outside of a switch");
	}
else
	{
	if (lf->got_default)
		po_say_fatal(pcb, "sorry, poco can't handle case after default.  "
					   "please move default to appear after last case in switch.");
	else
		get_case_after(pcb,pf,lf);
	}
}

static void get_default(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * parse default statement, issue error message if needed.
 ****************************************************************************/
{
Loop_frame *lf;

if ((lf = po_get_top_switch(pcb)) == NULL)
	{
	po_say_fatal(pcb, "default outside of a switch");
	}
else
	{
	po_eat_token(pcb, ':');
	lf->got_default = TRUE;
	offset_last_case_beq(pcb, pf, lf);
	lf->last_case_beq = 0;
	}
}

static get_if(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * parse and code an if statement, and associate else, if else exists.
 ****************************************************************************/
{
Code_label *false, *end;
Exp_frame  *ef;
long cpos;

if ((false = new_label(pcb,pf)) == NULL)
	return;
ef = po_new_expframe(pcb);
if (!po_eat_lparen(pcb))
	goto OUT;
po_get_expression(pcb, ef);
if (!po_eat_rparen(pcb))
	goto OUT;
po_coerce_to_boolean(pcb, ef);
if (!po_cat_code(pcb, &pf->fcd, &ef->ecd))
	goto OUT;
cpos = po_cbuf_code_size(&pf->fcd) + OPY_SIZE;
if (new_use(pcb, false, cpos) == NULL)
	goto OUT;
po_code_int(pcb, &pf->fcd, OP_BEQ, 0);
statement(pcb, pf);
lookup_token(pcb);
if (pcb->t.toktype == PTOK_ELSE)
	{
	/* if got an else code up branch from end of true statement to end of
	   if statement */
	if ((end = new_label(pcb,pf)) == NULL)
		goto OUT;
	cpos = po_cbuf_code_size(&pf->fcd) + OPY_SIZE;
	if (new_use(pcb, end, cpos) == NULL)
		goto OUT;
	po_code_int(pcb, &pf->fcd, OP_BRA, 0);
	false->code_pos = po_cbuf_code_size(&pf->fcd);
	statement(pcb, pf);
	end->code_pos = po_cbuf_code_size(&pf->fcd);
	}
else
	{
	pushback_token(&pcb->t);
	false->code_pos = po_cbuf_code_size(&pf->fcd);
	}
OUT:
po_dispose_expframe(pcb, ef);
return;
}

static void get_return(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * parse and code a return statement, including the related expression.
 ****************************************************************************/
{
Exp_frame ef;

if (pf->frame_type != FTY_FUNC)
	{
	po_say_fatal(pcb, "return statement outside of function");
	return;
	}
if (!po_need_token(pcb))
	return;
if (pcb->t.toktype != ';')
	{
	if (pf->return_type->ido_type == IDO_VOID)
		po_say_fatal(pcb, "can't return something from a void function");
	pushback_token(&pcb->t);
	po_init_expframe(pcb, &ef);
	po_get_expression(pcb, &ef);
	po_coerce_expression(pcb, &ef, pf->return_type, FALSE);
	po_code_op(pcb, &ef.ecd, po_find_pop_op(pcb, &ef.ctc));
	po_cat_code(pcb, &pf->fcd, &ef.ecd);
	po_trash_expframe(pcb, &ef);
	po_eat_semi(pcb);
	}
#ifdef STRING_EXPERIMENT
po_code_free_string_ops(pcb, pf);
#endif /* STRING_EXPERIMENT */
po_code_op(pcb, &pf->fcd, OP_LEAVE);
po_code_op(pcb, &pf->fcd, OP_RET);
}

static void get_comma(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * parse and code comma-separated expressions.
 ****************************************************************************/
{
Exp_frame ef;

po_get_expression(pcb, e);
for (;;)
	{
	lookup_token(pcb);
	if (pcb->t.toktype == ',')
		{
		po_init_expframe(pcb, &ef);
		po_get_expression(pcb, &ef);
		po_pop_off_result(pcb, &ef);
		/* po_code_op(pcb, &ef.ecd, po_find_pop_op(pcb, &ef.ctc)); ~~~*/
		po_cat_code(pcb, &e->ecd, &ef.ecd);
		po_trash_expframe(pcb, &ef);
		clear_code_buf(pcb, &e->left);
		}
	else
		{
		pushback_token(&pcb->t);
		break;
		}
	}
}


static void get_for(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * parse and code a for statement.
 ****************************************************************************/
{
Exp_frame	*efstart,
			*efend,
			*efcond;
Loop_frame	*lf;
long		cpos;
long		for_line;
Boolean 	got_cond = FALSE;

	for_line = pcb->curtoken->line_num;
	if ((lf = start_loop(pcb,pf))==NULL)
		return;

	efstart = po_new_expframe(pcb);
	efend	= po_new_expframe(pcb);
	efcond	= po_new_expframe(pcb);

	/* do syntax analysis and build up code parts */
	if (!po_eat_lparen(pcb))
		goto OUT;
	if (!po_is_next_token(pcb, ';'))
		{
		get_comma(pcb, efstart);
		po_pop_off_result(pcb, efstart);
		warn_no_effect(pcb, efstart);
		}
	if (!po_eat_semi(pcb))
		goto OUT;
	if (!po_is_next_token(pcb, ';'))
		{
		po_get_expression(pcb, efcond);
		po_coerce_to_boolean(pcb,  efcond);
		got_cond = TRUE;
		}
	if (!po_eat_semi(pcb))
		goto OUT;
	if (!po_is_next_token(pcb, TOK_RPAREN))
		{
		get_comma(pcb, efend);
		po_pop_off_result(pcb, efend);
		warn_no_effect(pcb, efend);
		}
	if (!po_eat_rparen(pcb))
		goto OUT;
	if (!po_cat_code(pcb, &pf->fcd, &efstart->ecd))
		goto OUT;
	lf->start->code_pos = po_cbuf_code_size(&pf->fcd);
	if (got_cond)
		{
		if (!po_cat_code(pcb, &pf->fcd, &efcond->ecd))
			goto OUT;
		cpos = po_cbuf_code_size(&pf->fcd) + OPY_SIZE;
		if (new_use(pcb, lf->end, cpos) == NULL)
			goto OUT;
		po_code_int(pcb, &pf->fcd, OP_BEQ, 0);
		}
	statement(pcb, pf);
	po_add_line_data(pcb, pf->ld, po_cbuf_code_size(&pf->fcd),
		for_line);
	if (!po_cat_code(pcb, &pf->fcd, &efend->ecd))
		goto OUT;
	cpos = po_cbuf_code_size(&pf->fcd) + OPY_SIZE;
	if (new_use(pcb, lf->start, cpos) == NULL)
		goto OUT;
	po_code_int(pcb, &pf->fcd, OP_BRA, 0);
	lf->end->code_pos = po_cbuf_code_size(&pf->fcd);

OUT:

	po_dispose_expframe(pcb, efcond);
	po_dispose_expframe(pcb, efend);
	po_dispose_expframe(pcb, efstart);

	end_loop(pcb);
}

static void get_break(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * parse and code a break statement, complain and die if loop scope is wrong..
 ****************************************************************************/
{
Loop_frame *lf;
long cpos;

if ((lf = pcb->loops) == NULL)
	{
	po_say_fatal(pcb, "break statement outside of while/for/do/switch");
	return;
	}
cpos = po_cbuf_code_size(&pf->fcd) + OPY_SIZE;
if (new_use(pcb, lf->end, cpos) == NULL)
	return;
po_code_int(pcb, &pf->fcd, OP_BRA, 0);
po_eat_semi(pcb);
}

static void get_continue(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * parse and code a continue statement, complain & die if loop scope is wrong.
 ****************************************************************************/
{
Loop_frame *lf;
long cpos;

/* skip past any switch loop-frames */
lf = pcb->loops;
while (lf != NULL)
	{
	if (!lf->is_switch)
		break;
	lf = lf->next;
	}
if (lf == NULL)
	{
	po_say_fatal(pcb, "continue statement outside of while/for/do");
	return;
	}
cpos = po_cbuf_code_size(&pf->fcd) + OPY_SIZE;
if (new_use(pcb, lf->start, cpos) == NULL)
	return;
po_code_int(pcb, &pf->fcd, OP_BRA, 0);
po_eat_semi(pcb);
}

void po_get_goto(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * parse and code a goto statement.
 * (Note to self: need to investigate replacing po_need_local_symbol with the
 * routine everyone else uses for finding/making local symbols.)
 ****************************************************************************/
{
Symbol *lsym;
Code_label *cl;
long cpos;

if (po_is_next_token(pcb, PTOK_LABEL))
	{
	lookup_token(pcb);
	lsym = pcb->curtoken->val.symbol;
	}
else
	{
	if ((lsym = po_need_local_symbol(pcb)) == NULL)
		return;
	if (!po_label_to_symbol(pcb,pf,lsym))
		return;
	}
cl = lsym->symval.p;
cpos = po_cbuf_code_size(&pf->fcd) + OPY_SIZE;
if (new_use(pcb, cl, cpos) == NULL)
	return;
po_code_int(pcb, &pf->fcd, OP_BRA, 0);
po_eat_semi(pcb);
}

static void set_named_label(Poco_cb *pcb, Poco_frame *pf, Symbol *lsym)
/*****************************************************************************
 * we found a label, record it's position in the code buffer.
 ****************************************************************************/
{
Code_label *cl;

cl = lsym->symval.p;
cl->code_pos = po_cbuf_code_size(&pf->fcd);
}


static void get_label(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * we think we found a label, handle it if ':' follows, or complain & die.
 ****************************************************************************/
{
Symbol *var;

var = pcb->curtoken->val.symbol;
lookup_token(pcb);
if (pcb->t.toktype == ':')
	set_named_label(pcb,pf,var);
else
	po_expecting_got(pcb, ":");
}

static void get_undef_label(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * we maybe found a label, or just an undefined symbol?
 * (sheesh, I'm losing it...)
 ****************************************************************************/
{
Symbol *var;

var = pcb->curtoken->val.symbol;
lookup_token(pcb);
if (pcb->t.toktype == ':')
	{
	po_label_to_symbol(pcb,pf,var);
	set_named_label(pcb,pf,var);
	}
else
	{
	po_undefined(pcb, var->name);
	}
}

void po_exp_statement(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * get an expression -- something that doesn't start with a reserved word.
 ****************************************************************************/
{
Exp_frame eee;

pushback_token(&pcb->t);
po_init_expframe(pcb, &eee);
po_get_expression(pcb, &eee);
po_pop_off_result(pcb,&eee);
po_eat_semi(pcb);
po_cat_code(pcb, &pf->fcd, &eee.ecd);
po_trash_expframe(pcb, &eee);
}

static void statement(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * figure out what to do with current statement, call proper parser routine.
 ****************************************************************************/
{
long	linenum = pcb->curtoken->line_num;

/*
 * added 08/18/90...
 *	 struct/proto frames don't get any line data attached to them.
 * added 08/28/90...
 *	 if the line number is zero, we don't add it.
 */

if (pf->frame_type != FTY_STRUCT && linenum != 0)
	{
	po_add_line_data(pcb, pf->ld, po_cbuf_code_size(&pf->fcd), linenum);
	}

if (!po_need_token(pcb))
	return;

if (pf->frame_type == FTY_FUNC)
	{
	switch (pcb->t.toktype)
		{
		case PTOK_FOR:
			get_for(pcb, pf);
			break;
		case PTOK_IF:
			get_if(pcb, pf);
			break;
		case PTOK_WHILE:
			get_while(pcb, pf);
			break;
		case PTOK_RETURN:
			get_return(pcb, pf);
			break;
		case PTOK_SWITCH:
			get_switch(pcb, pf);
			break;
		case PTOK_GOTO:
			po_get_goto(pcb, pf);
			break;
		case PTOK_DO:
			get_do(pcb, pf);
			break;
		case PTOK_ELSE:
			po_say_fatal(pcb, "else without if");
			break;
		case PTOK_BREAK:
			get_break(pcb, pf);
			break;
		case PTOK_CONTINUE:
			get_continue(pcb, pf);
			break;
		case TOK_LBRACE:
			po_get_block(pcb, pf);
			break;
		case PTOK_CASE:
			get_case(pcb, pf);
			break;
		case PTOK_DEFAULT:
			get_default(pcb, pf);
			break;
		case PTOK_LABEL:
			get_label(pcb, pf);
			break;
		case PTOK_UNDEF:
			get_undef_label(pcb, pf);
			break;
		case PTOK_TYPEDEF:
			po_get_typedef(pcb, pf);
			break;
		case PTOK_USER_TYPE:
		case PTOK_TYPE:
			po_get_declaration(pcb, pf);
			break;
		case ';':
			break;
		default:
			po_exp_statement(pcb, pf);
			break;
		}
	}
else
	{
	switch (pcb->t.toktype)
		{
		case PTOK_TYPEDEF:
			po_get_typedef(pcb, pf);
			break;
		case ';':
			break;
		default:
			po_get_declaration(pcb, pf);
			break;
		}
	}
}

