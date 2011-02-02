/*****************************************************************************
 *  bop.c - parse binary math/logical operations (not assign or comma).
 *
 *  This section is an operator precedence shift/reduce parser 
 *  (as opposed to the recursive descent in the rest of poco).
 *
 * MAINTENANCE
 *	08/31/90	(Ian)
 *				Made pointer addition work.
 *	09/21/90	(Ian)
 *				Changed runops interpreter to always leave an INT on the
 *				stack following a comparison operation; made corresponding
 *				changes here to reset expression type to INT regardless of
 *				of the types of the operators in the comparison.  This fixes
 *				a bug in which mixed comparisons and logical boolean ops
 *				were getting promoted to the strongest type when they
 *				shouldn't (eg, (1.0 < 2.0 && 1L < 2L) wouldn't work, because
 *				it would promote the results of 1L < 2L to double, then die
 *				due to the double on each side of the && op.)
 *  10/22/90	(Ian)
 *				Changed t.ttype to t.toktype, part of supporting multi-
 *				token lookaheads (t.ttype is no more).
 *  10/24/90	(Ian)
 *				Added code generation for pointer difference.
 ****************************************************************************/

#include "poco.h"

#define PREC_COUNT 11	/* This needs to be big enough to hold one for each */
						/* level of precedence, plus one */

#define PREC_ADD		9	/* Precedence of ADD/SUB ops. */
#define PREC_COMPARE 	7	/* Precedence of LT/LE/GT/GE ops. */
#define PREC_EQ			6	/* Precedence of EQ/NE ops. */

#define NOCOPY_TYPE		0
#define	COPY_TYPE		1

static SHORT enforce_simple(Poco_cb *pcb, Type_info *ti)
/* Make sure can be part of a == or != or <= or + expression */
{
SHORT nt = ti->ido_type;

switch (nt)
	{
	case IDO_INT:
	case IDO_LONG:
	case IDO_DOUBLE:
	case IDO_POINTER:
	case IDO_CPT:
	case IDO_VPT:
#ifdef STRING_EXPERIMENT
	case IDO_STRING:
#endif /* STRING_EXPERIMENT */
		return(nt);
	default:
		po_say_fatal(pcb, "Simple data type expected.");
		return(-1);
	}
}

typedef struct bop_info
	{
	SHORT precedence;
	Op_type *ido_ops;
	SHORT (*enforcer)(Poco_cb *pcb, Type_info *ti);		/*  type enforcer */
	} Bop_info;

static Bop_info bi_table[] = {
	{0,	 NULL,				NULL},
	{10, po_mul_ops, 		po_force_num_exp},
	{10, po_div_ops, 		po_force_num_exp},
	{10, po_mod_ops, 		po_force_int_exp},
	{9,  po_add_ops, 		enforce_simple},
	{9,  po_sub_ops, 		po_force_num_exp},
	{8,  po_lshift_ops,		po_force_int_exp},
	{8,  po_rshift_ops, 	po_force_int_exp},
	{7,  po_le_ops, 		enforce_simple},
	{7,  po_lt_ops, 		enforce_simple},
	{7,  po_ge_ops, 		enforce_simple},
	{7,  po_gt_ops, 		enforce_simple},
	{6,  po_eq_ops, 		enforce_simple},
	{6,  po_ne_ops, 		enforce_simple},
	{5,  po_band_ops, 		po_force_int_exp},
	{4,  po_xor_ops, 		po_force_int_exp},
	{3,  po_bor_ops, 		po_force_int_exp},
	{2,  po_land_ops, 		po_force_int_exp},
	{1,  po_lor_ops, 		po_force_int_exp},
	};

void po_init_qbop_table(Poco_cb *pcb)
/*****************************************************************************
 * fill in values in bop table
 ****************************************************************************/
{
register UBYTE *ptab = pcb->qbop_table;

	ptab['*'] 		 = 1;
	ptab['/'] 		 = 2;
	ptab['%'] 		 = 3;
	ptab['+'] 		 = 4;
	ptab['-'] 		 = 5;
	ptab[TOK_LSHIFT] = 6;
	ptab[TOK_RSHIFT] = 7;
	ptab[TOK_LE] 	 = 8;
	ptab['<'] 		 = 9;
	ptab[TOK_GE] 	 = 10;
	ptab['>'] 		 = 11;
	ptab[TOK_EQ] 	 = 12;
	ptab[TOK_NE] 	 = 13;
	ptab['&'] 		 = 14;
	ptab['^'] 	 	 = 15;
	ptab['|'] 	 	 = 16;
	ptab[TOK_LAND] 	 = 17;
	ptab[TOK_LOR] 	 = 18;

}

static void cat_exp(Poco_cb *pcb, Exp_frame *dest,
					Exp_frame *tail, short copy_type_flag)
/*****************************************************************************
 * concatenate the code buffers and (opt) type_infos of two expression frames.
 ****************************************************************************/
{
po_cat_code(pcb, &(dest->ecd), &(tail->ecd));

dest->pure_const 			&= tail->pure_const;
dest->includes_function 	+= tail->includes_function;
dest->includes_assignment 	+= tail->includes_assignment;
dest->left_complex 			+= tail->left_complex;

dest->var  = tail->var;
dest->doff = tail->doff;

if (copy_type_flag != NOCOPY_TYPE)
	po_copy_type(pcb, &tail->ctc, &dest->ctc);
}


void po_get_binop_expression(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
/* Put binary expression into e.
 ****************************************************************************/
{
Exp_frame 	*exp_buf[PREC_COUNT],
			**exp_stack;
Bop_info 	*bop_buf[PREC_COUNT],
			**bop_stack;
Exp_frame 	*exp0,
			*exp1;
Bop_info 	*bi;
int 		stack_size;
SHORT 		dot0,		/* ido_types of binary components */
			dot1;

exp_stack  = exp_buf + Array_els(exp_buf);
bop_stack  = bop_buf + Array_els(bop_buf);
stack_size = 0;

for (;;)
	{
	exp0 = po_new_expframe(pcb);
	po_get_unop_expression(pcb,exp0);
	lookup_token(pcb);
	bi = &bi_table[pcb->qbop_table[pcb->t.toktype]];
	*(--exp_stack) = exp0;
	*(--bop_stack) = bi;
	++stack_size;
	/* do reductions while possible */
	for (;;)
		{
		if (stack_size == 1)
			{
			if (bop_stack[0]->precedence == 0)		/* not a binary op.. */
				{
				pushback_token(&pcb->t);
				goto ALLDONE;
				}
			else		/* can't reduce further, oh well */
				break;
			}
		else
			{
			bi = bop_stack[1];
			if (bop_stack[0]->precedence <= bi->precedence)
				{
REDO_POINTER_ARITHMETIC:
				exp0 = exp_stack[0];
				exp1 = exp_stack[1];
				dot0 = exp0->ctc.ido_type;
				dot1 = exp1->ctc.ido_type;

				/*
				 * pointer addition/subtraction get special handling...
				 */

				if (bi->precedence == PREC_ADD 
#ifdef STRING_EXPERIMENT
				&&  (dot0 != IDO_STRING && dot1 != IDO_STRING)
#endif /* STRING_EXPERIMENT */
				&&	(dot0 == IDO_POINTER || dot1 == IDO_POINTER))
					{
					if (dot0 == dot1)
						{
						int typesize;
						if (bi->ido_ops == po_add_ops)
							po_say_fatal(pcb, "cannot add two pointers");
						/*
						 * code the subtraction of two pointers...
						 */
						if (!po_types_same(&exp1->ctc, &exp0->ctc, 0))
							po_say_fatal(pcb
							, "type mismatch in pointer subtraction");
						if (0 == (typesize 
						= po_get_subtype_size(pcb, &(exp1->ctc))))
							po_say_fatal(pcb
							, "size of type is zero (void pointer)");
						cat_exp(pcb, exp1, exp0, NOCOPY_TYPE);
						clear_code_buf(pcb, &exp1->left);
						po_code_int(pcb, &(exp1->ecd), OP_PTRDIFF, typesize);
						po_set_base_type(pcb, &exp1->ctc, TYPE_INT, 0, NULL);
						po_fold_const(pcb, exp1);
						}
					else if (dot0 == IDO_POINTER)
						{
						/*
						 * tricky dept:
						 *  if the number came before the pointer, swap the
						 *  expressions on the stack & jump back up to where
						 *  the pointers and dot values are set, so that they
						 *  appear in the right order for the OP_PADD/PSUB
						 *  instructions in the virtual machine.
						 */
						exp_stack[0] = exp1;
						exp_stack[1] = exp0;
						goto REDO_POINTER_ARITHMETIC;
						}
					else
						{
						/*
						 * code the add or sub of pointer and number...
						 */
						po_coerce_numeric_exp(pcb, exp0, IDO_LONG);
						po_code_elsize(pcb, exp0,
									  po_get_subtype_size(pcb, &(exp1->ctc)));
						po_coerce_numeric_exp(pcb, exp0, IDO_INT);
						cat_exp(pcb, exp1, exp0, NOCOPY_TYPE);
						po_code_op(pcb, &(exp1->ecd), bi->ido_ops[IDO_POINTER]);
						po_fold_const(pcb, exp1);
						}
					}
				else 			
					{
					/*
					 * 'normal' expression (ie, no pointer arithmetic)...
					 */
					if (dot0 != dot1)
						{
						if (dot0 == IDO_VOID || dot1 == IDO_VOID)
							po_say_fatal(pcb, "can't operate on a void");
#ifdef STRING_EXPERIMENT
						if (dot0 == IDO_STRING || dot1 == IDO_STRING)
							{
							po_coerce_to_string(pcb,exp0);
							po_coerce_to_string(pcb,exp1);
							}
#endif /* STRING_EXPERIMENT */
						else if (dot0>dot1) /* coerce num exp to larger type */
							po_coerce_numeric_exp(pcb, exp1, dot0);
						else
							po_coerce_numeric_exp(pcb, exp0, dot1);
						}
					/*
					 * ensure the op is legal for the target type...
					 * if so, everything has checked out, code it...
					 */
					dot0 = bi->enforcer(pcb, &(exp0->ctc));
					cat_exp(pcb, exp1, exp0, COPY_TYPE);
					clear_code_buf(pcb, &exp1->left);
					po_code_op(pcb, &(exp1->ecd), bi->ido_ops[dot0]);
					if (bi->precedence == PREC_EQ || bi->precedence == PREC_COMPARE)
						po_set_base_type(pcb, &exp1->ctc, TYPE_INT, 0, NULL);
					po_fold_const(pcb, exp1);
				}
				po_dispose_expframe(pcb,exp0);
				exp_stack+=1;
				bop_stack[1] = bop_stack[0];
				bop_stack+=1;
				stack_size-=1;
				}
			else
				break;
			}
		}
	}
ALLDONE:
exp0 = *exp_stack;
cat_exp(pcb, e, exp0, COPY_TYPE);
po_cat_code(pcb, &e->left, &exp0->left);
po_dispose_expframe(pcb,exp0);
}

