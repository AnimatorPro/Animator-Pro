/*****************************************************************************
 * VARINIT.C - Process init expressions in variable definitions.
 *			   (Used to be part of DECLARE.C)
 *
 *	10/30/90	(Ian)
 *				Changed po_var_init() to take the size of a string intializer
 *				from curtoken->ctoke_size instead of strlen(), since an
 *				embedded \0 in a string init would make the string look
 *				shorter than it is.
 *	05/22/92	(Ian)
 *				Split this into its own module.
 *				Big rewrite of the whole concept to fix the problems with
 *				initializing an array of agregate types with dimension
 *				implied by the init expressions.  The old function used to
 *				build init expressions and cat_code them together on the
 *				fly, then go back and patch the ending code buffer; this
 *				worked for arrays of integral types but not agregate types.
 *				Now we build a linked list of init expressions, and after
 *				all the inits are gathered we apply patches and cat_code the
 *				expressions together.
 ****************************************************************************/

#include "poco.h"

/*----------------------------------------------------------------------------
 * local datatype to keep parm-passing count smaller during recursion...
 *--------------------------------------------------------------------------*/

typedef struct init_control {
	Symbol		*var;
	Exp_frame	*exp_head;
	int 		frame_type;
} InitControl;

extern int po_scoped_address_op[2];

void anytype_init(Poco_cb *pcb, InitControl *ctl, Type_info *ti, int doff);

/*----------------------------------------------------------------------------
 * code...
 *--------------------------------------------------------------------------*/

static Exp_frame *new_init_expframe(Poco_cb *pcb, InitControl *ctl,
									Type_info *ti, int doff)
/*****************************************************************************
 * make a new expression frame, init it, add it to the linked list.
 ****************************************************************************/
{
	Exp_frame	*new;

	new = po_new_expframe(pcb);
	new->doff = doff;
	po_copy_type(pcb, ti, &new->ctc);

	new->next = ctl->exp_head;
	ctl->exp_head = new;

	return new;
}

static void integral_init(Poco_cb *pcb, InitControl *ctl,
						  Type_info *ti, int doff)
/*****************************************************************************
 * generate code to initialize an integral type variable/element.
 ****************************************************************************/
{
	Exp_frame	*exp;

	/*------------------------------------------------------------------------
	 * add a new expression frame to the inits list
	 *----------------------------------------------------------------------*/

	exp = new_init_expframe(pcb, ctl, ti, doff);

	/*------------------------------------------------------------------------
	 * code the lval
	 *----------------------------------------------------------------------*/

	po_code_int(pcb, &exp->left,		  /* code our left value */
		po_find_assign_op(pcb, ctl->var, &exp->ctc), doff);

	/*------------------------------------------------------------------------
	 * parse the init expression and code the assign for it.
	 *	if the storage class of the var is static, the expression will be
	 *	checked to make sure it's a proper constant init expression before
	 *	the assignment is generated; po_assign_after_equals() does the check
	 *	if (var->flags & TFL_STATIC) is true.
	 *----------------------------------------------------------------------*/

	po_assign_after_equals(pcb, exp, ctl->var,
		(ctl->var->ti->flags & TFL_STATIC));

	/*------------------------------------------------------------------------
	 * record location of data-offset in the OP_xxx_xASS for later fixup...
	 *	now that the code is generated, the expression frame's doff value
	 *	isn't needed, so we use it to store the offset in the code buffer
	 *	that needs to be patched if we're generating inits for a variable
	 *	that hasn't had space allocated yet (ie, array[] = {}).
	 *----------------------------------------------------------------------*/

	exp->doff = po_cbuf_code_size(&exp->ecd) - sizeof(int);

	/*------------------------------------------------------------------------
	 * generate the pop that follows the assign
	 *----------------------------------------------------------------------*/

	po_pop_off_result(pcb, exp);

	/*------------------------------------------------------------------------
	 * check for bogus init (call to func int a = b = 0; type things)
	 *----------------------------------------------------------------------*/

	if (ctl->frame_type != FTY_FUNC &&
		(exp->includes_assignment > 1 || exp->includes_function)) {
		po_say_fatal(pcb, "illegal initialization (code outside of function)");
	}
}


static void quo_init(Poco_cb *pcb, InitControl *ctl, Type_info *ti, int doff)
/*****************************************************************************
 * generate code to initialize a char array from a string constant.
 *	 this is a sort of special case of an integral init.
 ****************************************************************************/
{
	long		dsize;
	long		asize;
	Exp_frame	*exp;

	/*------------------------------------------------------------------------
	 * make sure we've got array-of-char as the type to initialize
	 *----------------------------------------------------------------------*/

	if (ti->comp_count != 2 && ti->comp[0] != TYPE_CHAR) {
		po_say_fatal(pcb, "quoted string initializing non-char array");
	}

	/*------------------------------------------------------------------------
	 * add a new expression frame to the inits list
	 *----------------------------------------------------------------------*/

	exp = new_init_expframe(pcb, ctl, ti, doff);

	/*------------------------------------------------------------------------
	 * get the size allocated to the item, and the size of the init string.
	 *	 the allocated size may be zero if we have char array[] = "...";
	 *	 the init string size comes from the token size, not a strlen(),
	 *	 because we need to handle strings like "ab\0cd" properly.
	 *----------------------------------------------------------------------*/

	asize = ti->sdims[1].l; 				/* array size */
	dsize = pcb->curtoken->ctoke_size+1;	/* data size */

	/*------------------------------------------------------------------------
	 * if the allocated size is zero, store the init string size as the
	 * array dimension.  the variable will be allocated later based on the
	 * size we set for it here.
	 * if the allocated size isn't zero, make sure the init string fits in
	 * the allocate space.	we allow char array[3] = "abc"; per ANSI (ie,
	 * the nullterm char doesn't have to fit).
	 *----------------------------------------------------------------------*/

	if (asize == 0) {
		ctl->var->ti->sdims[1].l = asize = dsize;
	} else {
		if (asize < dsize-1) {
			po_say_fatal(pcb, "string too big for array");
			return;
		}
	}

	/*------------------------------------------------------------------------
	 * process the string (saves it in global constants list and generates
	 * an OP_PCON to refer to it).
	 *----------------------------------------------------------------------*/

	po_get_prim(pcb, exp);

	/*------------------------------------------------------------------------
	 * generate an OP_xxx_ADDRESS/OP_COPY sequence to init the array.
	 *----------------------------------------------------------------------*/

	po_code_address(pcb, &exp->ecd,
		po_scoped_address_op[ctl->var->storage_scope], doff, asize-1);

	po_code_long(pcb, &exp->ecd, OP_COPY, (dsize < asize) ? dsize : asize);

	/*------------------------------------------------------------------------
	 * record location of data-offset in the OP_xxx_ADDRESS for later fixup...
	 *	now that the code is generated, the expression frame's doff value
	 *	isn't needed, so we use it to store the offset in the code buffer
	 *	that needs to be patched if we're generating inits for a variable
	 *	that hasn't had space allocated yet (ie, char array[] = "...").
	 *----------------------------------------------------------------------*/

	exp->doff = po_cbuf_code_size(&exp->ecd) - 2*sizeof(int) - 2*sizeof(long);

}

static void array_init(Poco_cb *pcb, InitControl *ctl, Type_info *ti, int doff)
/*****************************************************************************
 * generate code to initialize an array.
 ****************************************************************************/
{
	int 		dim;
	long		dsize;
	int 		rdim;

	/*------------------------------------------------------------------------
	 * back off to the next lower level of the type info
	 *----------------------------------------------------------------------*/

	ti->comp_count -= 1;
	dim 	= ti->sdims[ti->comp_count].l;
	rdim	= 0;
	dsize	= po_get_type_size(ti);
	po_set_ido_type(ti);

	/*------------------------------------------------------------------------
	 * loop to process comma-separated init expressions in the braces
	 *----------------------------------------------------------------------*/

	for (;;) {
		if (po_is_next_token(pcb, TOK_RBRACE)) {
			po_eat_rbrace(pcb);
			break;
		}

		if (dim != 0 && rdim >= dim) {					/* if array allocated */
			po_say_fatal(pcb, "too many initializers.");/* and more inits than*/
		}												/* array elements, die*/

		anytype_init(pcb, ctl, ti, doff);	  /* recurse to handle expression */
		doff += dsize;						  /* add size to current offset   */
		++rdim; 							  /* count array element		  */

		if (po_need_comma_or_brace(pcb) !=	',') /* must have comma or brace; */
			break;								 /* if brace, all done. 	  */
	}

	/*------------------------------------------------------------------------
	 * jump back up to the current level of type info
	 *----------------------------------------------------------------------*/

	ti->comp_count+=1;
	po_set_ido_type(ti);

	/*------------------------------------------------------------------------
	 * if the allocated size is zero, store the init expression count as the
	 * array dimension.  the variable will be allocated later based on the
	 * size we set for it here.
	 *----------------------------------------------------------------------*/

	if (dim == 0) {
		ctl->var->ti->sdims[ctl->var->ti->comp_count-1].l = rdim;
	}

}

static void struct_init(Poco_cb *pcb, InitControl *ctl, Type_info *ti, int doff)
/*****************************************************************************
 * generate code to initialize a structure or union.
 ****************************************************************************/
{
	Struct_info *sif;
	Symbol		*sel;

	/*------------------------------------------------------------------------
	 * eat the opening brace and get the structure info
	 *----------------------------------------------------------------------*/

	po_eat_lbrace(pcb);

	sif = ti->sdims[0].pt;
	sel = sif->elements;

	/*------------------------------------------------------------------------
	 * loop to process structure elements
	 *----------------------------------------------------------------------*/

	while (sel != NULL) {

		anytype_init(pcb, ctl, sel->ti, doff); /* recurse to handle expression*/
		doff += po_get_type_size(sel->ti);	   /* add size to current offset  */

		if (sif->type == TYPE_UNION) {			/* if doing a union, we're    */
			goto GOT_FIELDS;					/* all done.				  */
		}

		sel = sel->next;						/* move to next field.		  */

		switch (po_need_comma_or_brace(pcb)) {	/* must have comma or brace   */
		  case TOK_RBRACE:						/* if brace, we're done, and  */
			return; 							/* we've eaten it, just return*/
		  case ',':                             /* if comma, look ahead to see*/
			if (po_is_next_token(pcb, TOK_RBRACE)) {	/* if it's followed by*/
				goto GOT_FIELDS;				/* a brace; if so we're done. */
			}
			break;
		}
	}

GOT_FIELDS:

	po_eat_rbrace(pcb);
	return;

}

static void anytype_init(Poco_cb *pcb, InitControl *ctl, Type_info *ti, int doff)
/*****************************************************************************
 * call routines to initialize an integral, array, or structure type.
 *	this is the recursion point for handling array elements and struct fields.
 ****************************************************************************/
{

	if (po_is_array(ti)) {	/* array inits come in a couple flavors... */
		lookup_token(pcb);
		switch (pcb->t.toktype) {
		  case PTOK_QUO:
			quo_init(pcb, ctl, ti, doff);
			break;
		  case TOK_LBRACE:
			array_init(pcb, ctl, ti, doff);
			break;
		  default:
			po_expecting_lbrace(pcb);
			break;
		}
	} else if (po_is_struct(ti)) {
		struct_init(pcb, ctl, ti, doff);
	} else	{
		integral_init(pcb, ctl, ti, doff);
	}
}

void po_var_init(Poco_cb *pcb, Exp_frame *e, Symbol *var, SHORT frame_type)
/*****************************************************************************
 * generate code to initialize a variable.
 *	this is the entry point from the declaration parser.
 ****************************************************************************/
{
	InitControl ctl;
	Type_info	*ti = var->ti;
	Boolean 	fixup_needed = FALSE;
	int 		fixup_offset = 0;
	int 		*fixword;
	Exp_frame	*exp, *next;

	/*------------------------------------------------------------------------
	 * init our control structure that we pass around during recursion
	 *----------------------------------------------------------------------*/

	poco_zero_bytes(&ctl, sizeof(ctl));
	ctl.var = var;
	ctl.frame_type = frame_type;

	/*------------------------------------------------------------------------
	 * remember whether variable is an array without a size (array[] = {...})
	 *----------------------------------------------------------------------*/

	if (po_is_array(ti) && ti->sdims[ti->comp_count-1].l == 0) {
		fixup_needed = TRUE;
	}

	/*------------------------------------------------------------------------
	 * build the init expressions list.
	 *	 we start with an offset of zero.  as we build arrays and structs,
	 *	 this will grow upwards.  after all inits are built, we go back
	 *	 and patch all generated offsets in instructions.
	 *----------------------------------------------------------------------*/

	anytype_init(pcb, &ctl, ti, 0);

	/*------------------------------------------------------------------------
	 * if the variable needed to be allocated do it.
	 * set the fixup offset to the offset of the variable.
	 *----------------------------------------------------------------------*/

	if (fixup_needed) {
		po_new_var_space(pcb, var);
	}

	fixup_offset = var->symval.doff;

	/*------------------------------------------------------------------------
	 * walk the list of init expressions.  for each expression, apply the
	 * patch in the assign/copy instruction, then cat the init code onto
	 * the caller's expression code buffer, and free the expression from
	 * the list.
	 *----------------------------------------------------------------------*/

	for (exp = ctl.exp_head; exp != NULL; exp = next) {
		next = exp->next;
		fixword = OPTR(exp->ecd.code_buf, exp->doff);
		*fixword += fixup_offset;
		po_cat_code(pcb, &e->ecd, &exp->ecd);
		po_dispose_expframe(pcb, exp);
	}
}

