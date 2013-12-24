
#ifndef POCOOP_H
#define POCOOP_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif /* STDTYPES_H */

/****
 **** The op-codes for our stack-based virtual machine
 ****/
enum {
OP_BAD, 			/* Illegal opcode */
OP_END, 			/* End of instruction stream, stop interpreter. */
OP_RET, 			/* Return from function */
OP_ADD_STACK,		/* Clean parameters off stack */
OP_BEQ, 			/* Branch if top of stack is zero */
OP_BNE, 			/* Branch if top of stack is not zero */
OP_BRA, 			/* Branch always */
OP_ENTER,			/* Set up local variables */
OP_LEAVE,			/* Clean off local variables */
OP_PCALL,			/* Call a (Poco) function */
OP_CALLI,			/* Call a (C or Poco) function indirectly */
OP_ICCALL,			/* call a compiled in int valued C function */
OP_LCCALL,			/* call a compiled in long valued C function */
OP_DCCALL,			/* call a compiled in double valued C function */
OP_PCCALL,			/* C function returns poco pointer */
OP_CPCCALL, 		/* C function returns C pointer */
OP_CVCCALL, 		/* C function returns function pointer */
OP_ICON,			/* Integer constant */
OP_LCON,			/* Long constant */
OP_DCON,			/* Double constant */
OP_PCON,			/* Pointer constant */
OP_GLO_CVAR,		/* use global char variable */
OP_GLO_SVAR,		/* use global short variable */
OP_GLO_IVAR,		/* use global int variable */
OP_GLO_PVAR,		/* use global pointer (data) variable */
OP_GLO_VVAR,		/* use global pointer (code) variable */
OP_GLO_LVAR,		/* use global long variable */
OP_GLO_FVAR,		/* use global float variable */
OP_GLO_DVAR,		/* use global double variable */
OP_LOC_CVAR,		/* use local char variable */
OP_LOC_SVAR,		/* use local short variable */
OP_LOC_IVAR,		/* use local int variable */
OP_LOC_PVAR,		/* use local pointer (data) variable */
OP_LOC_VVAR,		/* use local pointer (code) variable */
OP_LOC_LVAR,		/* use local long variable */
OP_LOC_FVAR,		/* use local float variable */
OP_LOC_DVAR,		/* use local double variable */
OP_GLO_CASS,		/* assign top of stack to char variable */
OP_GLO_SASS,		/* assign top of stack to short variable */
OP_GLO_IASS,		/* assign top of stack to int variable */
OP_GLO_PASS,		/* assign top of stack to pointer (data) variable */
OP_GLO_VASS,		/* assign top of stack to pointer (code) variable */
OP_GLO_LASS,		/* assign top of stack to long variable */
OP_GLO_FASS,		/* assign top of stack to float variable */
OP_GLO_DASS,		/* assign top of stack to double variable */
OP_LOC_CASS,		/* assign top of stack to char variable */
OP_LOC_SASS,		/* assign top of stack to short variable */
OP_LOC_IASS,		/* assign top of stack to int variable */
OP_LOC_PASS,		/* assign top of stack to pointer (data) variable */
OP_LOC_VASS,		/* assign top of stack to pointer (code) variable */
OP_LOC_LASS,		/* assign top of stack to long variable */
OP_LOC_FASS,		/* assign top of stack to float variable */
OP_LOC_DASS,		/* assign top of stack to double variable */
OP_IADD,			/* add two integers */
OP_LADD,			/* add two longs */
OP_DADD,			/* add two doubles */
OP_PADD,			/* add integer to a pointer */
OP_ISUB,			/* subtract two integers */
OP_LSUB,			/* subtract two longs */
OP_DSUB,			/* subtract two doubles */
OP_PSUB,			/* subtract two pointers (integer result) */
OP_IMUL,			/* integer multiply */
OP_LMUL,			/* long multiply */
OP_DMUL,			/* double multiply */
OP_IDIV,			/* integer divide */
OP_LDIV,			/* long divide */
OP_DDIV,			/* double divide */
OP_INT_TO_LONG, 	/* convert int to a long */
OP_INT_TO_DOUBLE,	/* convert int to a double */
OP_LONG_TO_INT, 	/* convert long to an int */
OP_LONG_TO_DOUBLE,	/* convert long to a double */
OP_DOUBLE_TO_LONG,	/* convert double to a long */
OP_DOUBLE_TO_INT,	/* convert double to an int */
OP_PPT_TO_CPT,		/* convert Popot to (void *) */
OP_CPT_TO_PPT,		/* convert (void *) to Popot */
OP_INEG,			/* negate an integer */
OP_LNEG,			/* negate a long */
OP_DNEG,			/* negate a double */
OP_IEQ, 			/* test two integers for == */
OP_LEQ, 			/* test two longs for == */
OP_DEQ, 			/* test two doubles for == */
OP_PEQ, 			/* test two pointers for == */
OP_INE, 			/* test two integers for != */
OP_LNE,
OP_DNE,
OP_PNE,
OP_IGE, 			/* test two integers for >= */
OP_LGE,
OP_DGE,
OP_PGE,
OP_ILE, 			/* test two integers for <= */
OP_LLE,
OP_DLE,
OP_PLE,
OP_ILT, 			/* test two integers for < */
OP_LLT,
OP_DLT,
OP_PLT,
OP_IGT, 			/* test two integers for > */
OP_LGT,
OP_DGT,
OP_PGT,
OP_IMOD,			/* integer modulus */
OP_LMOD,
OP_ILSHIFT, 		/* integer << */
OP_LLSHIFT,
OP_IRSHIFT, 		/* integer >> */
OP_LRSHIFT,
OP_IBAND,			/* integer & (bitwise and) */
OP_LBAND,
OP_IBOR,			/* integer | (bitwise or) */
OP_LBOR,
OP_IXOR,			/* integer ^ (bitwise xor) */
OP_LXOR,
OP_ILAND,			/* integer && (logical and) */
OP_LLAND,
OP_ILOR,			/* integer || (logical or) */
OP_LLOR,
OP_ICOMP,			/* integer bitwise complement (swap 0 & 1's) */
OP_LCOMP,
OP_INOT,			/* integer logical not (0->1, everything else -> 0) */
OP_LNOT,
OP_IPUSH,			/* integer push */
OP_LPUSH,			/* long push */
OP_DPUSH,			/* double push */
OP_PPUSH,			/* Popot push */
OP_CPPUSH,			/* (void *) push */
OP_IPOP,			/* integer pop */
OP_LPOP,			/* long pop */
OP_DPOP,			/* double pop */
OP_PPOP,			/* Popot pop */
OP_CPPOP,			/* (void *) pop */
OP_CI_VAR,			/* use indirect char */
OP_SI_VAR,			/* use indirect short */
OP_II_VAR,			/* use indirect int */
OP_PI_VAR,			/* use indirect pointer  (data) */
OP_VI_VAR,			/* use indirect pointer  (code) */
OP_LI_VAR,			/* use indirect long */
OP_FI_VAR,			/* use indirect float */
OP_DI_VAR,			/* use indirect double */
OP_GLO_ADDRESS, 	/* use global variable address */
OP_LOC_ADDRESS, 	/* use local variable address */
OP_CODE_ADDRESS,	/* use function address */
OP_CI_ASS,			/* assign top of stack indirectly to char */
OP_SI_ASS,			/* assign top of stack indirectly to short */
OP_II_ASS,			/* assign top of stack indirectly to int */
OP_PI_ASS,			/* assign top of stack indirectly to pointer  (data) */
OP_VI_ASS,			/* assign top of stack indirectly to pointer  (code) */
OP_LI_ASS,			/* assign top of stack indirectly to long */
OP_FI_ASS,			/* assign top of stack indirectly to float */
OP_DI_ASS,			/* assign top of stack indirectly to double */
OP_NOP, 			/* Do nothing */
OP_IDUPE,			/* duplicate int value on stack */
OP_LDUPE,			/* duplicate long value on stack */
OP_DDUPE,			/* duplicate double value on stack */
OP_PDUPE,			/* duplicate pointer value on stack */
OP_ADD_IOFFSET, 	/* add int offset to a pointer on top */
OP_ADD_LOFFSET, 	/* add long offset to a pointer on top */
OP_COPY,			/* copy between 2 pointers */
OP_MOVE,			/* like copy but stack d,s not s,d */
OP_PTRDIFF, 		/* pointer difference */
#ifdef STRING_EXPERIMENT
OP_STRING_CCALL,	/* C function returns String */
OP_GLO_STRING_VAR,	/* use global String variable */
OP_LOC_STRING_VAR,	/* use local String variable */
OP_GLO_STRING_ASS,	/* assign top of stack to String variable */
OP_LOC_STRING_ASS,	/* assign top of stack to String variable */
OP_PPT_TO_STRING,	/* convert Popot to PoString */
OP_CPT_TO_STRING,	/* convert (void *) to PoString */
OP_STRING_TO_PPT,	/* convert PoString to Popot */
OP_STRING_TO_CPT,	/* convert PoString to (void *) */
OP_STRING_EQ,		/* test two Strings for == */
OP_STRING_NE,
OP_STRING_GE,
OP_STRING_LE,
OP_STRING_LT,
OP_STRING_GT,
OP_STRING_PUSH, 	/* PoString push */
OP_STRING_POP,		/* PoString pop */
OP_STRING_I_VAR,	/* use indirect String */
OP_STRING_I_ASS,	/* assign top of stack indirectly to String */
OP_FREE_STRING, 	/* Free a string */
OP_CLEAN_STRING,	/* Discard a string value... */
OP_STRING_CAT,		/* Concatenate two strings... */
#endif /* STRING_EXPERIMENT */
OP_PAST_LAST,		/* One past last legal op (for error checking) */
};

/* This enum tells us the data associated with an op.
 * These items name the values in the op_ext field.
 */
enum {
OEX_NONE,			/* no data */
OEX_VAR,			/* offset of variable */
OEX_FUNCTION,		/* function address */
OEX_INT,			/* some int value */
OEX_LONG,			/* some long value */
OEX_DOUBLE, 		/* some double value */
OEX_POINTER,		/* some Popot value */
OEX_CFUNCTION,		/* some (void *) value */
OEX_LABEL,			/* branch target */
OEX_ADDRESS,		/* Full address of varaible or function (not just offset) */
};

/* this enum tells us whether the opcode is one that disqualifies the
 * overall expression from being a constant expression.
 * these items name the values in the op_flags field; they are
 * treated as bitmapped flags.
 */
enum {
OFL_NONE	 = 0x00,		/* this is an okay opcode */
OFL_NOTCON	 = 0x01,		/* this opcode implies expression is not const */
};

typedef UBYTE Code;
typedef UBYTE Stack;
typedef int   Op_type;

#define INT_SIZE	(sizeof(int))
#define INTY_SIZE	(sizeof(int))
#define OPY_SIZE	(sizeof(int))
#define OPSZ		OPY_SIZE
#define INTYSZ		INTY_SIZE

typedef struct poco_op_tab
	{
	char	*op_name;
	short	op_size;
	UBYTE	op_flags;
	UBYTE	op_ext;
	Op_type op_type;
	} Poco_op_table;

extern Poco_op_table po_ins_table[];
extern int po_ins_table_els;

extern Op_type po_mul_ops[];
extern Op_type po_div_ops[];
extern Op_type po_mod_ops[];
extern Op_type po_add_ops[];
extern Op_type po_sub_ops[];
extern Op_type po_lshift_ops[];
extern Op_type po_rshift_ops[];
extern Op_type po_lt_ops[];
extern Op_type po_le_ops[];
extern Op_type po_gt_ops[];
extern Op_type po_ge_ops[];
extern Op_type po_eq_ops[];
extern Op_type po_ne_ops[];
extern Op_type po_band_ops[];
extern Op_type po_xor_ops[];
extern Op_type po_bor_ops[];
extern Op_type po_land_ops[];
extern Op_type po_lor_ops[];
extern Op_type po_push_ops[];
extern Op_type po_pop_ops[];
extern Op_type po_clean_ops[];
extern Op_type po_ccall_ops[];
extern Op_type po_con_ops[];
extern Op_type po_add_offset_ops[];
extern Op_type po_neg_ops[];
extern Op_type po_not_ops[];
extern Op_type po_comp_ops[];
extern Op_type po_dupe_ops[];


#endif /* POCOOP_H */
