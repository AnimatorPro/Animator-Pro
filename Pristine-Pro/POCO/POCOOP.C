/*****************************************************************************
 * pocoop.c - a table of all the instructions implemented in poco's
 *				stack-oriented virtual machine.
 *
 *	The stuff in this table is used to print disassembly listings and
 *	stack traces following a runtime error.
 *
 *	10/01/90	(Ian)
 *				Fixed values in table for OP_LEAVE, it no longer has
 *				an integer immediate data operand.
 *	10/24/90	(Ian)
 *				Added OP_PTRDIFF for subtracting two pointers.
 *	09/06/91	(Jim)
 *		>		Added in String related opcodes.
 *		>		Put in ido_type indexed tables of related opcodes
 *				(so don't do OP_IXXX + ido_type) but po_xxx_ops[ido_type].
 *		>		Added pointer >= <= > < opcodes.
 ****************************************************************************/

#include "poco.h"
#include "ptrmacro.h"

#define FUNCPSZ sizeof(int(*)())			/* sizeof of function ptr	   */
#define ADDRSZ	(sizeof(int)+sizeof(long))	/* offset+size for OP_xxx_ADDR */

/* op table for virtual stack machine poco */

Poco_op_table po_ins_table[] = {

/* "opname",      operand size,        op flags  operand class,   opcode */

{ "OP_BAD",            0,              OFL_NOTCON, OEX_NONE,      OP_BAD,},
{ "OP_END",            0,              OFL_NONE,   OEX_NONE,      OP_END,},
{ "OP_RET",            0,              OFL_NONE,   OEX_NONE,      OP_RET,},
{ "OP_ADD_STACK",      INTYSZ,         OFL_NONE,   OEX_INT,       OP_ADD_STACK,},
{ "OP_BEQ",            INTYSZ,         OFL_NOTCON, OEX_LABEL,     OP_BEQ,},
{ "OP_BNE",            INTYSZ,         OFL_NOTCON, OEX_LABEL,     OP_BNE,},
{ "OP_BRA",            INTYSZ,         OFL_NOTCON, OEX_LABEL,     OP_BRA,},
{ "OP_ENTER",          INTYSZ,         OFL_NONE,   OEX_INT,       OP_ENTER,},
{ "OP_LEAVE",          0,              OFL_NONE,   OEX_NONE,      OP_LEAVE,},
{ "OP_PCALL",          sizeof(void*),  OFL_NOTCON, OEX_FUNCTION,  OP_PCALL,},
{ "OP_CALLI",          0,              OFL_NOTCON, OEX_NONE,      OP_CALLI,},
{ "OP_ICCALL",         FUNCPSZ,        OFL_NOTCON, OEX_CFUNCTION, OP_ICCALL,},
{ "OP_LCCALL",         FUNCPSZ,        OFL_NOTCON, OEX_CFUNCTION, OP_LCCALL,},
{ "OP_DCCALL",         FUNCPSZ,        OFL_NOTCON, OEX_CFUNCTION, OP_DCCALL,},
{ "OP_PCCALL",         FUNCPSZ,        OFL_NOTCON, OEX_CFUNCTION, OP_PCCALL,},
{ "OP_CPCCALL",        FUNCPSZ,        OFL_NOTCON, OEX_CFUNCTION, OP_CPCCALL,},
{ "OP_CVCCALL",        FUNCPSZ,        OFL_NOTCON, OEX_CFUNCTION, OP_CVCCALL,},
{ "OP_ICON",           INTYSZ,         OFL_NONE,   OEX_INT,       OP_ICON,},
{ "OP_LCON",           sizeof(long),   OFL_NONE,   OEX_LONG,      OP_LCON,},
{ "OP_DCON",           sizeof(double), OFL_NONE,   OEX_DOUBLE,    OP_DCON,},
{ "OP_PCON",           sizeof(Popot),  OFL_NONE,   OEX_POINTER,   OP_PCON,},
{ "OP_GLO_CVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_CVAR,},
{ "OP_GLO_SVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_SVAR,},
{ "OP_GLO_IVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_IVAR,},
{ "OP_GLO_PVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_PVAR,},
{ "OP_GLO_VVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_VVAR,},
{ "OP_GLO_LVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_LVAR,},
{ "OP_GLO_FVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_FVAR,},
{ "OP_GLO_DVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_DVAR,},
{ "OP_LOC_CVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_CVAR,},
{ "OP_LOC_SVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_SVAR,},
{ "OP_LOC_IVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_IVAR,},
{ "OP_LOC_PVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_PVAR,},
{ "OP_LOC_VVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_VVAR,},
{ "OP_LOC_LVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_LVAR,},
{ "OP_LOC_FVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_FVAR,},
{ "OP_LOC_DVAR",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_DVAR,},
{ "OP_GLO_CASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_CASS,},
{ "OP_GLO_SASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_SASS,},
{ "OP_GLO_IASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_IASS,},
{ "OP_GLO_PASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_PASS,},
{ "OP_GLO_VASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_VASS,},
{ "OP_GLO_LASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_LASS,},
{ "OP_GLO_FASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_FASS,},
{ "OP_GLO_DASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_DASS,},
{ "OP_LOC_CASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_CASS,},
{ "OP_LOC_SASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_SASS,},
{ "OP_LOC_IASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_IASS,},
{ "OP_LOC_PASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_PASS,},
{ "OP_LOC_VASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_VASS,},
{ "OP_LOC_LASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_LASS,},
{ "OP_LOC_FASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_FASS,},
{ "OP_LOC_DASS",       INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_DASS,},
{ "OP_IADD",           0,              OFL_NONE,   OEX_NONE,      OP_IADD,},
{ "OP_LADD",           0,              OFL_NONE,   OEX_NONE,      OP_LADD,},
{ "OP_DADD",           0,              OFL_NONE,   OEX_NONE,      OP_DADD,},
{ "OP_PADD",           0,              OFL_NONE,   OEX_NONE,      OP_PADD,},
{ "OP_ISUB",           0,              OFL_NONE,   OEX_NONE,      OP_ISUB,},
{ "OP_LSUB",           0,              OFL_NONE,   OEX_NONE,      OP_LSUB,},
{ "OP_DSUB",           0,              OFL_NONE,   OEX_NONE,      OP_DSUB,},
{ "OP_PSUB",           0,              OFL_NONE,   OEX_NONE,      OP_PSUB,},
{ "OP_IMUL",           0,              OFL_NONE,   OEX_NONE,      OP_IMUL,},
{ "OP_LMUL",           0,              OFL_NONE,   OEX_NONE,      OP_LMUL,},
{ "OP_DMUL",           0,              OFL_NONE,   OEX_NONE,      OP_DMUL,},
{ "OP_IDIV",           0,              OFL_NONE,   OEX_NONE,      OP_IDIV,},
{ "OP_LDIV",           0,              OFL_NONE,   OEX_NONE,      OP_LDIV,},
{ "OP_DDIV",           0,              OFL_NONE,   OEX_NONE,      OP_DDIV,},
{ "OP_INT_TO_LONG",    0,              OFL_NONE,   OEX_NONE,      OP_INT_TO_LONG,},
{ "OP_INT_TO_DOUBLE",  0,              OFL_NONE,   OEX_NONE,      OP_INT_TO_DOUBLE,},
{ "OP_LONG_TO_INT",    0,              OFL_NONE,   OEX_NONE,      OP_LONG_TO_INT,},
{ "OP_LONG_TO_DOUBLE", 0,              OFL_NONE,   OEX_NONE,      OP_LONG_TO_DOUBLE,},
{ "OP_DOUBLE_TO_LONG", 0,              OFL_NONE,   OEX_NONE,      OP_DOUBLE_TO_LONG,},
{ "OP_DOUBLE_TO_INT",  0,              OFL_NONE,   OEX_NONE,      OP_DOUBLE_TO_INT,},
{ "OP_PPT_TO_CPT",     0,              OFL_NONE,   OEX_NONE,      OP_PPT_TO_CPT,},
{ "OP_CPT_TO_PPT",     0,              OFL_NONE,   OEX_NONE,      OP_CPT_TO_PPT,},
{ "OP_INEG",           0,              OFL_NONE,   OEX_NONE,      OP_INEG,},
{ "OP_LNEG",           0,              OFL_NONE,   OEX_NONE,      OP_LNEG,},
{ "OP_DNEG",           0,              OFL_NONE,   OEX_NONE,      OP_DNEG,},
{ "OP_IEQ",            0,              OFL_NONE,   OEX_NONE,      OP_IEQ,},
{ "OP_LEQ",            0,              OFL_NONE,   OEX_NONE,      OP_LEQ,},
{ "OP_DEQ",            0,              OFL_NONE,   OEX_NONE,      OP_DEQ,},
{ "OP_PEQ",            0,              OFL_NONE,   OEX_NONE,      OP_PEQ,},
{ "OP_INE",            0,              OFL_NONE,   OEX_NONE,      OP_INE,},
{ "OP_LNE",            0,              OFL_NONE,   OEX_NONE,      OP_LNE,},
{ "OP_DNE",            0,              OFL_NONE,   OEX_NONE,      OP_DNE,},
{ "OP_PNE",            0,              OFL_NONE,   OEX_NONE,      OP_PNE,},
{ "OP_IGE",            0,              OFL_NONE,   OEX_NONE,      OP_IGE,},
{ "OP_LGE",            0,              OFL_NONE,   OEX_NONE,      OP_LGE,},
{ "OP_DGE",            0,              OFL_NONE,   OEX_NONE,      OP_DGE,},
{ "OP_PGE",            0,              OFL_NONE,   OEX_NONE,      OP_PGE,},
{ "OP_ILE",            0,              OFL_NONE,   OEX_NONE,      OP_ILE,},
{ "OP_LLE",            0,              OFL_NONE,   OEX_NONE,      OP_LLE,},
{ "OP_DLE",            0,              OFL_NONE,   OEX_NONE,      OP_DLE,},
{ "OP_PLE",            0,              OFL_NONE,   OEX_NONE,      OP_PLE,},
{ "OP_ILT",            0,              OFL_NONE,   OEX_NONE,      OP_ILT,},
{ "OP_LLT",            0,              OFL_NONE,   OEX_NONE,      OP_LLT,},
{ "OP_DLT",            0,              OFL_NONE,   OEX_NONE,      OP_DLT,},
{ "OP_PLT",            0,              OFL_NONE,   OEX_NONE,      OP_PLT,},
{ "OP_IGT",            0,              OFL_NONE,   OEX_NONE,      OP_IGT,},
{ "OP_LGT",            0,              OFL_NONE,   OEX_NONE,      OP_LGT,},
{ "OP_DGT",            0,              OFL_NONE,   OEX_NONE,      OP_DGT,},
{ "OP_PGT",            0,              OFL_NONE,   OEX_NONE,      OP_PGT,},
{ "OP_IMOD",           0,              OFL_NONE,   OEX_NONE,      OP_IMOD,},
{ "OP_LMOD",           0,              OFL_NONE,   OEX_NONE,      OP_LMOD,},
{ "OP_ILSHIFT",        0,              OFL_NONE,   OEX_NONE,      OP_ILSHIFT,},
{ "OP_LLSHIFT",        0,              OFL_NONE,   OEX_NONE,      OP_LLSHIFT,},
{ "OP_IRSHIFT",        0,              OFL_NONE,   OEX_NONE,      OP_IRSHIFT,},
{ "OP_LRSHIFT",        0,              OFL_NONE,   OEX_NONE,      OP_LRSHIFT,},
{ "OP_IBAND",          0,              OFL_NONE,   OEX_NONE,      OP_IBAND,},
{ "OP_LBAND",          0,              OFL_NONE,   OEX_NONE,      OP_LBAND,},
{ "OP_IBOR",           0,              OFL_NONE,   OEX_NONE,      OP_IBOR,},
{ "OP_LBOR",           0,              OFL_NONE,   OEX_NONE,      OP_LBOR,},
{ "OP_IXOR",           0,              OFL_NONE,   OEX_NONE,      OP_IXOR,},
{ "OP_LXOR",           0,              OFL_NONE,   OEX_NONE,      OP_LXOR,},
{ "OP_ILAND",          0,              OFL_NONE,   OEX_NONE,      OP_ILAND,},
{ "OP_LLAND",          0,              OFL_NONE,   OEX_NONE,      OP_LLAND,},
{ "OP_ILOR",           0,              OFL_NONE,   OEX_NONE,      OP_ILOR,},
{ "OP_LLOR",           0,              OFL_NONE,   OEX_NONE,      OP_LLOR,},
{ "OP_ICOMP",          0,              OFL_NONE,   OEX_NONE,      OP_ICOMP,},
{ "OP_LCOMP",          0,              OFL_NONE,   OEX_NONE,      OP_LCOMP,},
{ "OP_INOT",           0,              OFL_NONE,   OEX_NONE,      OP_INOT,},
{ "OP_LNOT",           0,              OFL_NONE,   OEX_NONE,      OP_LNOT,},
{ "OP_IPUSH",          0,              OFL_NONE,   OEX_NONE,      OP_IPUSH,},
{ "OP_LPUSH",          0,              OFL_NONE,   OEX_NONE,      OP_LPUSH,},
{ "OP_DPUSH",          0,              OFL_NONE,   OEX_NONE,      OP_DPUSH,},
{ "OP_PPUSH",          0,              OFL_NONE,   OEX_NONE,      OP_PPUSH,},
{ "OP_CPPUSH",         0,              OFL_NONE,   OEX_NONE,      OP_CPPUSH,},
{ "OP_IPOP",           0,              OFL_NONE,   OEX_NONE,      OP_IPOP,},
{ "OP_LPOP",           0,              OFL_NONE,   OEX_NONE,      OP_LPOP,},
{ "OP_DPOP",           0,              OFL_NONE,   OEX_NONE,      OP_DPOP,},
{ "OP_PPOP",           0,              OFL_NONE,   OEX_NONE,      OP_PPOP,},
{ "OP_CPPOP",          0,              OFL_NONE,   OEX_NONE,      OP_CPPOP,},
{ "OP_CI_VAR",         0,              OFL_NOTCON, OEX_NONE,      OP_CI_VAR,},
{ "OP_SI_VAR",         0,              OFL_NOTCON, OEX_NONE,      OP_SI_VAR,},
{ "OP_II_VAR",         0,              OFL_NOTCON, OEX_NONE,      OP_II_VAR,},
{ "OP_PI_VAR",         0,              OFL_NOTCON, OEX_NONE,      OP_PI_VAR,},
{ "OP_VI_VAR",         0,              OFL_NOTCON, OEX_NONE,      OP_VI_VAR,},
{ "OP_LI_VAR",         0,              OFL_NOTCON, OEX_NONE,      OP_LI_VAR,},
{ "OP_FI_VAR",         0,              OFL_NOTCON, OEX_NONE,      OP_FI_VAR,},
{ "OP_DI_VAR",         0,              OFL_NOTCON, OEX_NONE,      OP_DI_VAR,},
{ "OP_GLO_ADDRESS",    ADDRSZ,         OFL_NONE,   OEX_ADDRESS,   OP_GLO_ADDRESS,},
{ "OP_LOC_ADDRESS",    ADDRSZ,         OFL_NOTCON, OEX_ADDRESS,   OP_LOC_ADDRESS,},
{ "OP_CODE_ADDRESS",   sizeof(void*),  OFL_NOTCON, OEX_FUNCTION,  OP_CODE_ADDRESS,},
{ "OP_CI_ASS",         0,              OFL_NOTCON, OEX_NONE,      OP_CI_ASS,},
{ "OP_SI_ASS",         0,              OFL_NOTCON, OEX_NONE,      OP_SI_ASS,},
{ "OP_II_ASS",         0,              OFL_NOTCON, OEX_NONE,      OP_II_ASS,},
{ "OP_PI_ASS",         0,              OFL_NOTCON, OEX_NONE,      OP_PI_ASS,},
{ "OP_VI_ASS",         0,              OFL_NOTCON, OEX_NONE,      OP_VI_ASS,},
{ "OP_LI_ASS",         0,              OFL_NOTCON, OEX_NONE,      OP_LI_ASS,},
{ "OP_FI_ASS",         0,              OFL_NOTCON, OEX_NONE,      OP_FI_ASS,},
{ "OP_DI_ASS",         0,              OFL_NOTCON, OEX_NONE,      OP_DI_ASS,},
{ "OP_NOP",            0,              OFL_NONE,   OEX_NONE,      OP_NOP,},
{ "OP_IDUPE",          0,              OFL_NONE,   OEX_NONE,      OP_IDUPE,},
{ "OP_LDUPE",          0,              OFL_NONE,   OEX_NONE,      OP_LDUPE,},
{ "OP_DDUPE",          0,              OFL_NONE,   OEX_NONE,      OP_DDUPE,},
{ "OP_PDUPE",          0,              OFL_NONE,   OEX_NONE,      OP_PDUPE,},
{ "OP_ADD_IOFFSET",    0,              OFL_NONE,   OEX_NONE,      OP_ADD_IOFFSET,},
{ "OP_ADD_LOFFSET",    0,              OFL_NONE,   OEX_NONE,      OP_ADD_LOFFSET,},
{ "OP_COPY",           sizeof(long),   OFL_NOTCON, OEX_LONG,      OP_COPY,},
{ "OP_MOVE",           sizeof(long),   OFL_NOTCON, OEX_LONG,      OP_MOVE,},
{ "OP_PTRDIFF",        INTYSZ,         OFL_NONE,   OEX_INT,       OP_PTRDIFF,},
#ifdef STRING_EXPERIMENT
{ "OP_STRING_CCALL",   FUNCPSZ,        OFL_NOTCON, OEX_CFUNCTION, OP_STRING_CCALL,},
{ "OP_GLO_STRING_VAR", INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_STRING_VAR,},
{ "OP_LOC_STRING_VAR", INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_STRING_VAR,},
{ "OP_GLO_STRING_ASS", INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_GLO_STRING_ASS,},
{ "OP_LOC_STRING_ASS", INTYSZ,         OFL_NOTCON, OEX_VAR,       OP_LOC_STRING_ASS,},
{ "OP_PPT_TO_STRING",  0,              OFL_NONE,   OEX_NONE,      OP_PPT_TO_STRING,},
{ "OP_CPT_TO_STRING",  0,              OFL_NONE,   OEX_NONE,      OP_CPT_TO_STRING,},
{ "OP_STRING_TO_PPT",  0,              OFL_NONE,   OEX_NONE,      OP_STRING_TO_PPT,},
{ "OP_STRING_TO_CPT",  0,              OFL_NONE,   OEX_NONE,      OP_STRING_TO_CPT,},
{ "OP_STRING_EQ",      0,              OFL_NONE,   OEX_NONE,      OP_STRING_EQ,},
{ "OP_STRING_NE",      0,              OFL_NONE,   OEX_NONE,      OP_STRING_NE,},
{ "OP_STRING_GE",      0,              OFL_NONE,   OEX_NONE,      OP_STRING_GE,},
{ "OP_STRING_LE",      0,              OFL_NONE,   OEX_NONE,      OP_STRING_LE,},
{ "OP_STRING_LT",      0,              OFL_NONE,   OEX_NONE,      OP_STRING_LT,},
{ "OP_STRING_GT",      0,              OFL_NONE,   OEX_NONE,      OP_STRING_GT,},
{ "OP_STRING_PUSH",    0,              OFL_NONE,   OEX_NONE,      OP_STRING_PUSH,},
{ "OP_STRING_POP",     0,              OFL_NONE,   OEX_NONE,      OP_STRING_POP,},
{ "OP_STRING_I_VAR",   0,              OFL_NOTCON, OEX_NONE,      OP_STRING_I_VAR,},
{ "OP_STRING_I_ASS",   0,              OFL_NOTCON, OEX_NONE,      OP_STRING_I_ASS,},
{ "OP_FREE_STRING",    INTYSZ,         OFL_NONE,   OEX_VAR,       OP_FREE_STRING,},
{ "OP_CLEAN_STRING",   0,              OFL_NONE,   OEX_NONE,      OP_CLEAN_STRING,},
{ "OP_STRING_CAT",     0,              OFL_NOTCON, OEX_NONE,      OP_STRING_CAT,},
#endif /* STRING_EXPERIMENT */
};
int po_ins_table_els = Array_els(po_ins_table);

#ifdef STRING_EXPERIMENT
	#define SE(a,b,c,d,e,f,g,h) {a,b,c,d,e,f,g,h,}
/* Code up a little local macro so can use the same data
 * STRING_EXPERIMENT or not for op-tables */
#else
	#define SE(a,b,c,d,e,f,g,h) {a,b,c,d,e,f,g,}
#endif /* STRING_EXPERIMENT */

/** Tables of binary operation virtual machine codes indexed by IDO_TYPE **/
Op_type po_mul_ops[NUM_IDOS] =
	SE(OP_IMUL, OP_LMUL, OP_DMUL, OP_BAD, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_div_ops[NUM_IDOS] =
	SE(OP_IDIV, OP_LDIV, OP_DDIV, OP_BAD, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_mod_ops[NUM_IDOS] =
	SE(OP_IMOD, OP_LMOD, OP_BAD, OP_BAD, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_add_ops[NUM_IDOS] =
	SE(OP_IADD, OP_LADD, OP_DADD, OP_PADD, \
	OP_BAD, OP_BAD, OP_BAD, OP_STRING_CAT);
Op_type po_sub_ops[NUM_IDOS] =
	SE(OP_ISUB, OP_LSUB, OP_DSUB, OP_PSUB, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_lshift_ops[NUM_IDOS] =
	SE(OP_ILSHIFT, OP_LLSHIFT, OP_BAD, OP_BAD, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_rshift_ops[NUM_IDOS] =
	SE(OP_IRSHIFT, OP_LRSHIFT, OP_BAD, OP_BAD, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_lt_ops[NUM_IDOS] =
	SE(OP_ILT, OP_LLT, OP_DLT, OP_PLT, \
	OP_BAD, OP_BAD, OP_BAD, OP_STRING_LT);
Op_type po_le_ops[NUM_IDOS] =
	SE(OP_ILE, OP_LLE, OP_DLE, OP_PLE, \
	OP_BAD, OP_BAD, OP_BAD, OP_STRING_LE);
Op_type po_gt_ops[NUM_IDOS] =
	SE(OP_IGT, OP_LGT, OP_DGT, OP_PGT, \
	OP_BAD, OP_BAD, OP_BAD, OP_STRING_GT);
Op_type po_ge_ops[NUM_IDOS] =
	SE(OP_IGE, OP_LGE, OP_DGE, OP_PGE, \
	OP_BAD, OP_BAD, OP_BAD, OP_STRING_GE);
Op_type po_eq_ops[NUM_IDOS] =
	SE(OP_IEQ, OP_LEQ, OP_DEQ, OP_PEQ, \
	OP_BAD, OP_BAD, OP_BAD, OP_STRING_EQ);
Op_type po_ne_ops[NUM_IDOS] =
	SE(OP_INE, OP_LNE, OP_DNE, OP_PNE, \
	OP_BAD, OP_BAD, OP_BAD, OP_STRING_NE);
Op_type po_band_ops[NUM_IDOS] =
	SE(OP_IBAND, OP_LBAND, OP_BAD, OP_BAD, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_xor_ops[NUM_IDOS] =
	SE(OP_IXOR, OP_LXOR, OP_BAD, OP_BAD, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_bor_ops[NUM_IDOS] =
	SE(OP_IBOR, OP_LBOR, OP_BAD, OP_BAD, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_land_ops[NUM_IDOS] =
	SE(OP_ILAND, OP_LLAND, OP_BAD, OP_BAD, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_lor_ops[NUM_IDOS] =
	SE(OP_ILOR, OP_LLOR, OP_BAD, OP_BAD, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_push_ops[NUM_IDOS] =
	SE(OP_IPUSH,OP_LPUSH,OP_DPUSH,OP_PPUSH, \
	OP_CPPUSH,OP_BAD,OP_PPUSH,OP_STRING_PUSH);
Op_type po_pop_ops[NUM_IDOS] =
	SE(OP_IPOP, OP_LPOP, OP_DPOP, OP_PPOP, \
	OP_CPPOP, OP_BAD,OP_PPOP, OP_STRING_POP);
Op_type po_clean_ops[NUM_IDOS] =
	SE(OP_IPOP, OP_LPOP, OP_DPOP, OP_PPOP, \
	OP_CPPOP, OP_BAD,OP_PPOP, OP_CLEAN_STRING);
Op_type po_ccall_ops[NUM_IDOS] =
	SE(OP_ICCALL,OP_LCCALL,OP_DCCALL,OP_PCCALL,\
	OP_CPCCALL,OP_CVCCALL,OP_BAD, OP_STRING_CCALL);
Op_type po_con_ops[NUM_IDOS] =
	SE(OP_ICON, OP_LCON, OP_DCON, OP_PCON, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_add_offset_ops[NUM_IDOS] =
	SE(OP_ADD_IOFFSET, OP_ADD_LOFFSET, OP_BAD,OP_BAD,\
	OP_BAD,OP_BAD,OP_BAD,OP_BAD);
Op_type po_neg_ops[NUM_IDOS] =
	SE(OP_INEG, OP_LNEG, OP_DNEG, OP_BAD, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_not_ops[NUM_IDOS] =
	SE(OP_INOT, OP_LNOT, OP_BAD,  OP_BAD, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_comp_ops[NUM_IDOS] =
	SE(OP_ICOMP, OP_LCOMP, OP_BAD, OP_BAD,\
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
Op_type po_dupe_ops[NUM_IDOS] =
	SE(OP_IDUPE, OP_LDUPE, OP_DDUPE, OP_PDUPE, \
	OP_BAD, OP_BAD, OP_BAD, OP_BAD);
#undef SE
