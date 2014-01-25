/*****************************************************************************
 *
 * poco.h	- Main header file for compiling the Poco compiler.
 *
 * MAINENANCE
 *	08/20/90	(Ian)
 *				General cleanup.  Made sure all globally-visible routines
 *				in poco have a prototype in here, or in one of the header
 *				files included from here.  Changed lotsa #defines to enums.
 *	08/24/90	(Ian)
 *				Added cached_frame field to the exp_frame structure, as an
 *				assist to the new espression frame caching logic.  See
 *				comments in poco.c for more details.
 * 08/25/90 	(Ian)
 *				Added #define for poc_gentle_freemem(), to check for NULL
 *				pointer inline instead of calling a routine to do it.
 * 08/28/90 	(Ian)
 *				Removed the cached_frame field from exp_frame & code_buf
 *				structs; new memory management routines don't require it anymore.
 *				Added CACHED_CODE_SIZE constant to define the size of a
 *				code buffer in cache.  Added Cache_ctl structure definition,
 *				and added instances of this struct into the Poco_cb for
 *				poco_frame, expression_frame, and code_buf caching. Added
 *				prototypes for items in new POCMEMRY.C module.
 * 08/29/90 	(Ian)
 *				Changed prototype for po_compile_file(), it takes less parms now.
 * 08/30/90 	(Ian)
 *				Changed CACHED_CODE_SIZE to SMALLBLK_CACHE_SIZE. Added
 *				prototype for po_code_elsize() routine, now globally visible.
 *				Increased SMALL_CODE_SIZE from 32 to 48, (this reduced the
 *				need for new code_buf areas by, like, a factor of five).
 * 09/04/90 	(Ian)
 *				Added parmcount field to Text_symbol structure for new pp,
 *				and changed is_macro field to flags field, of which is_macro
 *				is one of the possible flag values.
 * 09/08/90 	(Ian)
 *				Added else_done field to Conditional struct, for #elif.
 * 09/19/90 	(Ian)
 *				Added support for union (is_union field in Struct_info).
 *				Added TYPE_xxxx constants for all ANSI type keywords and
 *				modifiers (const, auto, etc).
 * 10/01/90 	(Ian)
 *				Added support for enum (is_union field changed to type in
 *				Struct_info, TYPE_ENUM added to list.)
 * 10/21/90 	(Ian)
 *				Nuked the next_line and next_line_data fields in the pcb,
 *				added the library_protos field.
 * 10/22/90 	(Ian)
 *				Eliminated the ttype, ctoke, val, and dval fields from the
 *				token structure; they are now part of the Tstack structure.
 *				Added curtoken and free_tokens fields, and eliminated ts[1]
 *				from the Poco_cb.
 * 10/24/90 	(Ian)
 *				Removed array_dims field from Exp_frame struct -- nothing
 *				was using it.  Removed proto for po_cat_exp(), it is now
 *				static in bop.c.
 *	10/25/90	(Ian)
 *				Added flags field to File_stack, groundwork for using the
 *				structure for both files and builtin libraries.
 *	10/26/90	(Ian)
 *				Added toktype field to token structure so that token type
 *				can be accessed without dereferencing the curtoken pointer.
 *				Added new datatype PoBoolean, typedef'd as a byte.  This is
 *				used selectively to speed up testing of Booleans.  Right now
 *				it's being used only for the t.reuse flag.
 *	05/01/91	(Ian)
 *				Added enable_debug_trace field to Poco_run_env structure.
 *				This field will contain FALSE when the interpreter is running
 *				to fold constants during the compile phase, and TRUE when
 *				running the compiled program.  If an error occurs when
 *				folding constants, we don't want a function call trace to be
 *				generated (indeed, the code in TRACE.C fails if it is invoked
 *				during the compile phase).	Also, the global builtin_err var
 *				now belongs to us, not to the host.  Added extern for it here.
 *	09/06/91	(Jim)
 *				Started to add in String type.	Changed unused TYPE_RAST
 *				to TYPE_STRING.  Eliminated TYPE_NCPT.
 *				Added po_ido_table.
 *	09/08/91	(Jim)
 *				Made all string stuff conditionally compiled with
 *				STRING_EXPERIMENT, as it doesn't look like this is
 *				going to be a production feature.
 *	06/12/92	(Ian)
 *				Another general cleanup.  Mostly, converted things to enums
 *				and added typedefs for enumerated types, because debuggers
 *				like enumerated things.
 ****************************************************************************/

#ifndef POCO_H
#define POCO_H

/*****************************************************************************
 * Tweakable #define's...
 ****************************************************************************/

#undef STRING_EXPERIMENT

#if 1
  #define DEVELOPMENT	/* Include code to check 'cannot happen' cases. */
#endif

#ifndef VRSN_NUM
  #define VRSN_NUM 184	/* this is usually defined externally via -D	*/
#endif

/*****************************************************************************
 * #include's used by most everything in poco...
 ****************************************************************************/

#include <string.h>

#ifndef STDTYPES_H
  #include "stdtypes.h"
#endif

#ifndef LOSTDIO_H
  #include "lostdio.h"
#endif

#ifndef TOKEN_H
  #include "token.h"
#endif

#ifndef ERRCODES_H
  #include "errcodes.h"
#endif

#ifndef PTRMACRO_H
  #include "ptrmacro.h"
#endif

#ifndef POCOOP_H
  #include "pocoop.h"
#endif

#ifndef LINKLIST_H
  #include "linklist.h"
#endif

#ifndef POCOFACE_H
  #include "pocoface.h"
#endif

/*****************************************************************************
 * miscellanious macros...
 *	 these could also be considered tweakable, but do it with care.
 ****************************************************************************/

#define VRSN_TO_STR(a) #a
#define VRSN_STR	   VRSN_TO_STR(VRSN_NUM)

#define MAX_STACK		14336	/* 14k stack, used in overflow checking */
#define HASH_SIZE		256 	/* hash table size, must change pocoutil.asm if this changes!!! */
#define SZTOKE			512 	/* max line length, token length */
#define MAX_SYM_LEN 	40		/* max significant chars in sym name */
#define MAX_STRLIT_LEN	4096	/* max size of one string literal */
#define SMALL_CODE_SIZE 48		/* size of small code buffer */
#define SMALLBLK_CACHE_SIZE 512 /* Used for code_buf and line_data caching. */
#define MAX_TYPE_COMPS 21

#define FUNC_MAGIC	0x27680317L /* magic number validates a function frame */

/*****************************************************************************
 * enumerated constants...
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * type names and modifiers...
 *--------------------------------------------------------------------------*/

typedef enum type_comp {
	TYPE_END,
	TYPE_CHAR,
	TYPE_UCHAR,
	TYPE_SHORT,
	TYPE_USHORT,
	TYPE_INT,
	TYPE_UINT,
	TYPE_LONG,
	TYPE_ULONG,
	TYPE_FLOAT,
	TYPE_DOUBLE,
	TYPE_POINTER,
	TYPE_FUNCTION,		/* poco function prototype */
	TYPE_VOID,
	TYPE_ARRAY,
	TYPE_ELLIPSIS,
	TYPE_SCREEN,
#ifdef STRING_EXPERIMENT
	TYPE_STRING,
#endif /* STRING_EXPERIMENT */
	TYPE_FILE,
	TYPE_UNUSED0,
	TYPE_CPT,			/* C pointer - parameter to printf most likely... */
	TYPE_UNUSED1,
	TYPE_STRUCT,		/* End of "Base" types */

	TYPE_UNUSED2,
	TYPE_CFUNCTION, 	/* poco library function prototype */
	TYPE_UNION, 		/* Converted to TYPE_STRUCT shortly */
	TYPE_ENUM,			/* Converted to TYPE_INT shortly */

	TYPE_REGISTER,		/* type modifiers... these are not types, as such, */
	TYPE_AUTO,			/* they are used only during the parsing of a type */
	TYPE_EXTERN,
	TYPE_STATIC,
	TYPE_SIGNED,
	TYPE_UNSIGNED,
	TYPE_CONST,
	TYPE_VOLATILE,

	TYPE_BAD,
} TypeComp;

/*----------------------------------------------------------------------------
 * type flags (used in type_info)...
 *--------------------------------------------------------------------------*/

typedef enum type_flags {
	TFL_STATIC	 = 1,
	TFL_SHORT	 = 2,
	TFL_LONG	 = 4,
	TFL_SIGNED	 = 8,
	TFL_UNSIGNED = 16,
} TypeFlags;

/*----------------------------------------------------------------------------
 * expression types...
 *--------------------------------------------------------------------------*/

typedef enum ido_type {
	IDO_BAD = -1,
	IDO_INT,				/* int or promoted short or char */
	IDO_LONG,				/* long */
	IDO_DOUBLE, 			/* double (or promoted float) */
	IDO_POINTER,			/* a Poco 12 byte bounds checked pointer */
	IDO_CPT,				/* a C type 4 byte pointer */
	IDO_VOID,				/* void - only used to generate C function calls */
	IDO_VPT,				/* a function - no code generated for this type */
#ifdef STRING_EXPERIMENT
	IDO_STRING, 			/* a String type. */
#endif /* STRING_EXPERIMENT */
	IDO_LAST
} IdoType;

#define NUM_IDOS	(IDO_LAST)

/*----------------------------------------------------------------------------
 * token types for keyword tokens...
 *--------------------------------------------------------------------------*/

typedef enum ptoken_t {
	PTOK_TYPE	= TOK_TOK_MAX,
	PTOK_UNUSED1,
	PTOK_QUO,
	PTOK_ELLIPSIS,
	PTOK_LIT,
	PTOK_FOR,
	PTOK_IF,
	PTOK_WHILE,
	PTOK_RETURN,
	PTOK_SWITCH,
	PTOK_GOTO,
	PTOK_DO,
	PTOK_ELSE,
	PTOK_BREAK,
	PTOK_CONTINUE,
	PTOK_VAR,
	PTOK_LABEL,
	PTOK_ENUMCONST,
	PTOK_SIZEOF,
	PTOK_UNDEF,
	PTOK_NULL,
	PTOK_TYPEDEF,
	PTOK_USER_TYPE,
	PTOK_CASE,
	PTOK_DEFAULT,
	PTOK_MAX,		/* this must be last! */
} PToken_t;

/*----------------------------------------------------------------------------
 * symbol flags...
 *--------------------------------------------------------------------------*/

typedef enum symbol_flags {
	SFL_USED		= 1,
	SFL_READ		= 2,
	SFL_WRITTEN 	= 4,
	SFL_DEFINED 	= 8,
	SFL_ELLIP		= 16,		/* it's a '...' parameter */
	SFL_STATIC		= 32,		/* it was declared with 'static' */
} SymbolFlags;

typedef enum storage_scope {	/* this indiates the storage scope of a variable */
	SCOPE_GLOBAL	= 0,
	SCOPE_LOCAL,
} StorageScope;

typedef enum frame_type  {		/* this indicates the type of a func/struct frame */
	FTY_GLOBAL	= 0,
	FTY_FUNC,
	FTY_STRUCT,
} FrameType;

/*----------------------------------------------------------------------------
 * function-related flags...
 *--------------------------------------------------------------------------*/

typedef enum cff_type { 	/* this indicates whether function is poco or C */
	CFF_POCO = 0,
	CFF_C,
} Cff_type;

typedef enum code_magic {	/* this validates a code buffer */
	CCRYPTIC = 0x8765,
	CTRASHED = 0x8674,
} CodeMagic;

/*----------------------------------------------------------------------------
 * preprocessor flags...
 *--------------------------------------------------------------------------*/

typedef enum {
	FSF_ISFILE = 1,
	FSF_ISLIB  = 2,
	FSF_MACSUB = 4,
} Fsflags;

typedef enum {
	TSFL_HASPARMS  = 1, 	/* Complex (has parms) macro				*/
	TSFL_ISSPECIAL = 2, 	/* Special-handling macro (__TIME__, etc)	*/
	TSFL_ISBUILTIN = 4, 	/* Builtin (cannot #undef) macro			*/
} Ts_flags;

typedef enum {
	PPTOK_PARMN = 1,		/* For macros with parms, normal parm follows */
	PPTOK_PARMQ = 2,		/* For macros with parms, quoted parm follows */
	PPTOK_SFILE = 1,		/* For ISSPECIAL macros, __FILE__	*/
	PPTOK_SLINE = 2,		/* For ISSPECIAL macros, __LINE__	*/
	PPTOK_SDATE = 3,		/* For ISSPECIAL macros, __DATE__	*/
	PPTOK_STIME = 4,		/* For ISSPECIAL macros, __TIME__	*/
	PPTOK_SNULL = 5,		/* For ISSPECIAL macros, NULL		*/
} PPToken;

typedef enum
	{
	PP_NO_ELSE_SEEN = 0,
	PP_DONE_ELSE,
	PP_DONE_ELIF,
} PP_else_state;

/*****************************************************************************
 * typedef's, structures, etc
 ****************************************************************************/

typedef char PoBoolean; 		/* small/fast boolean datatype for poco */

typedef struct cache_ctl {		/* Control structure for struct caching.	*/
	UBYTE		*pbase; 		/* -> base of struct cache memory			*/
	SHORT		slot_size;		/* size of one item in the cache			*/
	SHORT		num_slots;		/* number of items allocated in the cache	*/
	SHORT		nxt_slot;		/* next slot to check when allocating		*/
	UBYTE		*inuse; 		/* -> table of cache slots in use.			*/
} Cache_ctl;

typedef struct code_buf {		/* code buffer management... */
	Code		*code_pt;
	Code		*code_buf;
	Code		*alloced_end;
	CodeMagic	cryptic;
	Code		cbuf[SMALL_CODE_SIZE];
} Code_buf;

/*----------------------------------------------------------------------------
 * type management structures...
 *--------------------------------------------------------------------------*/

typedef union pt_long { 		/* overlap a pointer and a longword */
	long		l;
	void		*pt;
} Pt_long;

typedef struct type_info {		/* type information management... */
	TypeComp	*comp;
	Pt_long 	*sdims; 		/* just for arrays/structures/functions */
	UBYTE		comp_alloc;
	UBYTE		comp_count;
	TypeFlags	flags;
	IdoType 	ido_type;
} Type_info;

typedef struct itypi			/* structure used during type parsing... */
	{
	Type_info	iti;
	Pt_long 	dimc[MAX_TYPE_COMPS];
	TypeComp	typec[MAX_TYPE_COMPS];
} Itypi;

typedef struct {
	PoBoolean	is_num; 		/* is it numeric */
	PoBoolean	can_add;		/* can you do a+b?	(pointer or numeric) */
	IdoType 	ido_type;		/* cross-check */
}  Ido_table;

/*----------------------------------------------------------------------------
 * symbol management structures...
 *--------------------------------------------------------------------------*/

typedef struct symbol {
	struct symbol	*next;
	struct symbol	*link;
	Pt_num			symval;
	char			*name;
	PToken_t		tok_type;
	SHORT			scope;
	StorageScope	storage_scope;
	SymbolFlags 	flags;
	Type_info		*ti;
} Symbol;

typedef struct struct_info {
	struct struct_info	*next;
	char				*name;
	Symbol				*elements;
	long				size;
	SHORT				el_count;
	TypeComp			type;		/* TYPE_STRUCT, TYPE_UNION, TYPE_ENUM */
} Struct_info;

#ifdef STRING_EXPERIMENT

 typedef struct local_string {
	struct local_string *next;
	Symbol				*string_symbol;
 } Local_string;

#endif /* STRING_EXPERIMENT */

/*----------------------------------------------------------------------------
 * looping and label (goto) management structures...
 *--------------------------------------------------------------------------*/

typedef struct use_label {
	struct use_label	*next;
	long				code_pos;
} Use_label;

typedef struct code_label {
	struct code_label	*next;
	long				code_pos;
	Use_label			*uses;
	Symbol				*lvar;
} Code_label;

typedef struct loop_frame {
	struct loop_frame	*next;
	Code_label			*start;
	Code_label			*end;
	PoBoolean			is_switch;
	PoBoolean			got_default;	/* these fields only valid when */
	Type_info			*con_type;		/* is_switch is TRUE... 		*/
	long				last_case_beq;
	int 				svar_offset;
} Loop_frame;

/*----------------------------------------------------------------------------
 * expression parsing structures...
 *--------------------------------------------------------------------------*/

typedef struct exp_frame {
	void		*next;
	Code_buf	ecd;
	Code_buf	left;
	Type_info	ctc;
	Symbol		*var;
	Pt_long 	ctc_dims[MAX_TYPE_COMPS];
	TypeComp	ctc_comp[MAX_TYPE_COMPS];
	int 		doff;
	PoBoolean	includes_function;
	PoBoolean	includes_assignment;
	PoBoolean	pure_const;
	PoBoolean	left_complex;
} Exp_frame;

typedef struct line_data {
	int 	count;
	int 	alloc;
	long	*offsets;
	long	*lines;
} Line_data;

#define CFF_FIELDS \
	char		*name; \
	short		pcount; \
	short		type; \
	Symbol		*parameters; \
	Type_info	*return_type;

typedef struct poco_frame { 		/* holds the local frame of reference */
	struct poco_frame	*next;		/* during parsing (scoped vars, etc)  */
	CFF_FIELDS
	Symbol				*symbols;
	int 				doff;
	SHORT				scope;
	Symbol				*root_sym;
	Symbol				**hash_table;
	Code_label			*labels;
	Code_buf			fcd;
	Struct_info 		*fsif;
	FrameType			frame_type;
	PoBoolean			is_proto_frame;
#ifdef STRING_EXPERIMENT
	Local_string		*local_string_list; /* tracks string variables */
#endif /* STRING_EXPERIMENT */
	Line_data			*ld;
} Poco_frame;

typedef struct func_frame { 		/* what's left of a poco_frame after    */
	struct							/* a function is all compiled...		*/
	 func_frame *next;
	CFF_FIELDS
	long		magic;
	Code		*code_pt;
	long		code_size;
	Line_data	*ld;
	struct		func_frame *mlink;		/* list of fuf's to free */
	PoBoolean	got_code;
	} Func_frame;

typedef Func_frame C_frame;

/*----------------------------------------------------------------------------
 * preprocessor structures...
 *--------------------------------------------------------------------------*/

typedef struct file_stack { 	/* file management structure for #includes */
	struct
	  file_stack	*pred;
	union {
		FILE		*file;
		Poco_lib	*lib;
	}				source;
	char			*name;
	long			line_count;
	Fsflags 		flags;
} File_stack;

typedef struct text_symbol {
	struct text_symbol	*next;
	char				*name;
	char				*value;
	SHORT				parmcount;
	Ts_flags			flags;
} Text_symbol;

typedef struct conditional {
	struct conditional	*next;
	PoBoolean			state;			/* current arm: active or not?	*/
	PP_else_state		else_state; 	/* track handling of #elif arms */
} Conditional;

/*----------------------------------------------------------------------------
 * token-related structures...
 *--------------------------------------------------------------------------*/

typedef union tunion {
	char	*string;
	long	num;
	Symbol	*symbol;
	double	dnum;
} Tunion;

typedef struct tstack {
	struct tstack	*next;
	Tunion			val;
	PToken_t		type;
	long			line_num;
	short			char_num;
	short			ctoke_size;
	char			ctoke[MAX_SYM_LEN];
	PoBoolean		is_symbol;
} Tstack;

/* HACK ALERT!
 * logic in pp_expand() in module pp.c requires that line_b1 and line_b2
 * be physically adjacent in memory.  see comments in pp.c for details.
 */

typedef struct token {
	PoBoolean			reuse;
	UBYTE				out_of_it;
	PToken_t			toktype;
	FILE				*err_file;
	File_stack			*file_stack;
	char				*line_buf;
	char				*line_pos;
	Conditional 		*ifdef_stack;
	Names				*include_dirs;	/* directory to find include files */
	Names				*pre_defines;	/* symbols DEFINED before we start */
	struct text_symbol	*define_list[HASH_SIZE];
	char				line_b1[SZTOKE];
	char				line_b2[SZTOKE];
} Token;

/*----------------------------------------------------------------------------
 * the run environment structure...
 *--------------------------------------------------------------------------*/

typedef struct poco_run_env
	{
	char		*stack;
	long		stack_size;
	char		*data;
	long		data_size;
	Boolean 	(*check_abort)(void *d);
	void		*check_abort_data;
	Func_frame	*fff;
	Names		*literals;		/* string constants */
	char		*trace_file;
	Poco_lib	*lib;			/* list of arrays of library function info */
	int 		unused1;
	long		*err_line;
	Func_frame	*protos;
	Poco_lib	*loaded_libs;	/* loaded (from disk via pragma) libraries */
	PoBoolean	enable_debug_trace;
	char		pad[24];
} Poco_run_env;

/*----------------------------------------------------------------------------
 * the poco control block, used during compile...
 *--------------------------------------------------------------------------*/

typedef struct poco_cb {
	Poco_run_env		run;
	FILE				*po_dump_file;
	void				*libfunc;
	Poco_lib			*builtin_lib;
	char				*stack_bottom;
	Token				t;
	Tstack				*curtoken;
	Tstack				*free_tokens;
	Symbol				*fsym;
	Errcode 			global_err;
	long				error_line_number;
	long				error_char_number;
	struct poco_frame	*rframe;
	Loop_frame			*loops;
	C_frame 			*cframes;
	UBYTE				qbop_table[PTOK_MAX]; /* table for binary operations */
	char				strlit_work[MAX_STRLIT_LEN+SZTOKE];
	Cache_ctl			smallblk_cache; /* Small block cache control */
	Cache_ctl			expf_cache; 	/* Expression frame cache control */
	Cache_ctl			pocf_cache; 	/* Poco frame cache control */
} Poco_cb;

/*****************************************************************************
 * macros for inline-ing common code fragments...
 ****************************************************************************/

#define Alloc_a(pcb,type)			po_memalloc(pcb,sizeof(type))
#define Alloc_z(pcb,type)			po_memzalloc(pcb,sizeof(type))

#define poc_gentle_freemem(a)		if ((a) != NULL) po_freemem((a))

#define any_code(pcb, c)			((c)->code_pt != (c)->code_buf)
#define clear_code_buf(pcb, c)		((c)->code_pt = (c)->code_buf)

#define pushback_token(t)			(t)->reuse = TRUE
#define lookup_token(pcb)			if ((pcb)->t.reuse) \
										(pcb)->t.reuse = FALSE; \
									else \
										po_lookup_freshtoken((pcb));

#define po_find_pop_op(pcb, ti) 	(po_pop_ops[(ti)->ido_type])
#define po_find_push_op(pcb, ti)	(po_push_ops[(ti)->ido_type])
#define po_find_clean_op(pcb, ti)	(po_clean_ops[(ti)->ido_type])

/*****************************************************************************
 * Prototypes for all global vars and routines in Poco...
 ****************************************************************************/

extern Ido_table po_ido_table[];

/* in pocoface.c */

extern Errcode builtin_err;

/* in pocoutil.asm */

char	*po_skip_space(char *line);
int 	po_hashfunc(UBYTE *s);
char	*po_chop_csym(char *line, char *word, int maxlen, char **wordnext);
char	*po_cmatch_scan(char *line);
void	poco_copy_bytes(void *s, void *d, int count);
void	poco_zero_bytes(void *d, int count);
void	poco_stuff_bytes(void *d, int value, int count);
Boolean po_eqstrcmp(char *s1, char *s2);

#ifdef __WATCOMC__
  #pragma aux poco_zero_bytes "__*__" parm [edi] [ecx];
  #pragma aux poco_copy_bytes "__*__" parm [esi] [edi] [ecx];
#else
  #define po_cmatch_scan(line)		strpbrk((line), "\"'/")
  #define poco_copy_bytes(s,d,c)	memcpy((d),(s),(c))
  #define poco_zero_bytes(d,c)		memset((d), 0, (c))
  #define poco_stuff_bytes(d, v, c) memset((d), (v), (c))
  #define po_eqstrcmp				strcmp
#endif

/* in bop.c */

void	po_init_qbop_table(Poco_cb *pcb);
void	po_get_binop_expression(Poco_cb *pcb, Exp_frame *e);

/* in chopper.c */

char	*po_get_csource_line(Poco_cb *pcb);
char	*po_chop_to(char *line, char *word, char letter);

/* in code.c */

void	po_init_code_buf(Poco_cb *pcb, Code_buf *c);
void	po_trash_code_buf(Poco_cb *pcb, Code_buf *c);
Boolean po_add_op(Poco_cb *pcb, Code_buf *cbuf,
				int op, void *data, SHORT data_size);
void	po_no_code(Poco_cb *pcb, Code_buf *cb);
void	po_backup_code(Poco_cb *pcb, Code_buf *cb, int op_size);
long	po_cbuf_code_size(Code_buf *c);
Boolean po_cat_code(Poco_cb *pcb, Code_buf *dest, Code_buf *end);
Boolean po_copy_code(Poco_cb *pcb, Code_buf *source, Code_buf *dest);
void	po_code_op(Poco_cb *pcb, Code_buf *cbuf, int op);
void	po_add_code_fixup(Poco_cb *pcb, Code_buf *cbuf, int fixup);
void	po_code_pop(Poco_cb *pcb, Code_buf *cbuf, int op, int pushop);
void	po_code_void_pt(Poco_cb *pcb, Code_buf *cbuf, int op, void *val);
void	po_code_double(Poco_cb *pcb, Code_buf *cbuf, int op, double val);
void	po_code_long(Poco_cb *pcb, Code_buf *cbuf, int op, long val);
void	po_code_address(Poco_cb *pcb, Code_buf *cbuf, int op, int doff,
				long dsize);
long	po_code_int(Poco_cb *pcb, Code_buf *cbuf, int op, int val);
void	po_code_popot(Poco_cb *pcb, Code_buf *cbuf,
					int op, void *min, void *max, void *pt);
void	po_int_fixup(Code_buf *cbuf, long fixup_pos, int val);
Boolean po_compress_func(Poco_cb *pcb, Poco_frame *pf, Func_frame *new);

/* in declare.c */

Func_frame	*po_sym_to_fuf(Symbol *s);
Func_frame	*po_get_proto(Type_info *ti);
void		po_pop_off_result(Poco_cb *pcb, Exp_frame *e);
void		po_get_typedef(Poco_cb *pcb, Poco_frame *pf);
void		po_get_typename(Poco_cb *pcb, Type_info *ti);
void		po_get_declaration(Poco_cb *pcb, Poco_frame *pf);

/* in escape.c */

int 	translate_escapes(unsigned char *inbuf,
				  unsigned char *outbuf);

/* in fold.c */

void	po_fold_const(Poco_cb *pcb, Exp_frame *exp);
int 	po_eval_const_expression(Poco_cb *pcb, Exp_frame *exp);
Boolean po_is_static_init_const(Poco_cb *pcb, Code_buf *cb);

/* in funccall.c */

int 	po_get_param_size(Poco_cb *pcb, SHORT ido_type);
void	po_get_function(Poco_cb *pcb, Exp_frame *e);

/* in linklist.c -- protos are in linklist.h, already #included above */

/* in main.c */

void	*pj_malloc(unsigned i);
void	*pj_zalloc(unsigned size);
void	pj_free(void *v);
void	pj_gentle_free(void *p);
void	pj_freez(void *p);
Boolean check_abort(void *nobody);
int 	pj_delete(char *name);
int 	pj_ioerr(void);
void	upc(char *s);
void	errline(int err, char *fmt, ...);
int 	get_errtext(Errcode err, char *buf);
int 	tryme(Popot v);

/* in pocmemry.c */

Errcode po_init_memory_management(Poco_cb **pcb);
void	po_free_compile_memory(void);
void	po_free_all_memory(void);
void	po_freemem(void *ptr);
void	*po_memalloc(Poco_cb *pcb, long size);
void	*po_memzalloc(Poco_cb *pcb, size_t size);
void	*po_cache_malloc(Poco_cb *pcb, Cache_ctl *pctl);
char	*po_clone_string(Poco_cb *pcb, char *s);

/* in poco.c */

void	po_say_warning(Poco_cb *pcb, char *fmt, ...);
void	po_say_fatal(Poco_cb *pcb, char *fmt, ...);
void	po_say_internal(Poco_cb *pcb, char *fmt, ...);
void	po_expecting_got(Poco_cb *pcb, char *expecting);
void	po_expecting_got_str(Poco_cb *pcb, char *expecting, char *got);
Boolean po_need_token(Poco_cb *pcb);
void	po_redefined(Poco_cb *pcb, char *s);
void	po_undefined(Poco_cb *pcb, char *s);
void	po_unmatched_paren(Poco_cb *pcb);
void	po_expecting_lbrace(Poco_cb *pcb);
void	po_expecting_rbrace(Poco_cb *pcb);
void	po_freelist(void *l);
Symbol	*po_unlink_el(Symbol *list, Symbol *el);
void	po_unhash_symbol(Poco_cb *pcb, Symbol *s);
int 	po_rehash(Poco_cb *pcb, Symbol *s);
void	po_free_symbol(Symbol *s);
void	po_free_symbol_list(Symbol **ps);
Symbol	*po_new_symbol(Poco_cb *pcb, char *name);
int 	po_link_len(Symbol *l);
void	*po_reverse_links(Symbol *el);
void	po_lookup_freshtoken(Poco_cb *pcb);
Boolean po_is_next_token(Poco_cb *pcb, SHORT ttype);
Boolean po_eat_token(Poco_cb *pcb, SHORT ttype);
Boolean po_eat_rbracket(Poco_cb *pcb);
Boolean po_eat_lparen(Poco_cb *pcb);
Boolean po_eat_rparen(Poco_cb *pcb);
Boolean po_check_rparen(Poco_cb *pcb);
SHORT	po_find_local_assign(Poco_cb *pcb, Type_info *ti);
SHORT	po_find_assign_op(Poco_cb *pcb, Symbol *var, Type_info *ti);
void	po_var_too_complex(Poco_cb *pcb);
SHORT	po_find_local_use(Poco_cb *pcb, Type_info *ti);
void	po_code_elsize(Poco_cb *pcb, Exp_frame *e, int el_size);
void	po_make_deref(Poco_cb *pcb, Exp_frame *e);
void	po_new_var_space(Poco_cb *pcb, Symbol *var);
int 	po_get_temp_space(Poco_cb *pcb, int space);
void	po_init_expframe(Poco_cb *pcb, Exp_frame *e);
Exp_frame *po_new_expframe(Poco_cb *pcb);
void	po_trash_expframe(Poco_cb *pcb, Exp_frame *e);
void	po_dispose_expframe(Poco_cb *pcb, Exp_frame *e);
SHORT	po_force_num_exp(Poco_cb *pcb, Type_info *ti);
SHORT	po_force_int_exp(Poco_cb *pcb, Type_info *ti);
SHORT	po_force_ptr_or_num_exp(Poco_cb *pcb, Type_info *ti);
void	po_coerce_to_boolean(Poco_cb *pcb, Exp_frame *e);
void	po_coerce_to_string(Poco_cb *pcb, Exp_frame *e);
void	po_coerce_numeric_exp(Poco_cb *pcb, Exp_frame *e, SHORT ido_type);
void	po_coerce_expression(Poco_cb *pcb, Exp_frame *e, Type_info *ti, Boolean recast);
void	po_get_prim(Poco_cb *pcb, Exp_frame *e);
void	po_get_unop_expression(Poco_cb *pcb, Exp_frame *e);
Boolean po_assign_after_equals(Poco_cb *pcb, Exp_frame *e, Symbol *var, Boolean must_be_static_init);
void	po_get_expression(Poco_cb *pcb, Exp_frame *e);
Boolean po_new_frame(Poco_cb *pcb, int scope, char *name, int type);
void	po_old_frame(Poco_cb *pcb);
Boolean po_check_undefined_funcs(Poco_cb *pcb, Symbol *sl);
Boolean po_compile_file(Poco_cb *pcb, char *name);
void	po_free_run_env(Poco_run_env *pev);

/* in pocodis.c */

Boolean po_check_instr_table(Poco_cb *pcb);
char	*find_c_name(C_frame *list, void *fpt);
void	*po_disasm(FILE *f, void *code, C_frame *cframes);
void	dump_code(Poco_cb *pcb, FILE *file, void *code, long csize);
void	po_dump_file(Poco_cb *pcb);
void	po_dump_codebuf(Poco_cb *pcb, Code_buf *cbuf);

/* in pocoface.c */

Errcode  print_pocolib(char *filename, Poco_lib *lib);
Poco_lib *po_open_library(Poco_cb *pcb, char *libname, char *id_str);
char	 *po_get_libproto_line(Poco_cb *pcb);
Errcode  compile_poco(void **ppexe, char *source_name, char *errors,
					char *dump_name, Poco_lib *lib, char *err_file,
					long *err_line, int *err_char, Names *include_dirs);
Errcode  run_poco(void **ppexe, char *trace_file,
					Boolean (*check_abort)(void *),
					void *check_abort_data, long *err_line);
void	 free_poco(void **ppexe);
char	 *po_fuf_name(void *fuf);
void	 *po_fuf_code(void *fuf);

/* in pocolib.c */

Errcode init_poco_libs(Poco_lib *lib);
void	po_cleanup_libs(Poco_lib *lib);
void	poco_freez(Popot *pt);

/* in pocotype.c */

Boolean po_check_type_names(Poco_cb *pcb);
Type_info *po_new_type_info(Poco_cb *pcb, Type_info *old, int extras);
Boolean po_is_num_ido(SHORT ido);
Boolean po_is_int_ido(SHORT ido);
Boolean po_is_pointer(Type_info *ti);
Boolean po_is_array(Type_info *ti);
Boolean po_is_struct(Type_info *ti);
Boolean po_is_func(Type_info *ti);
void	po_set_ido_type(Type_info *ti);
Boolean po_append_type(Poco_cb *pcb, Type_info *ti, TypeComp tc, long dim,
				void *sif);
Boolean po_set_base_type(Poco_cb *pcb, Type_info *ti, TypeComp tc,
				long dim, Struct_info *sif);
Boolean po_copy_type(Poco_cb *pcb, Type_info *s, Type_info *d);
Boolean po_cat_type(Poco_cb *pcb, Type_info *d, Type_info *s);
Boolean po_is_void_ptr(Type_info *ti);
Boolean po_ptypes_same(Type_info *st, Type_info *dt);
Boolean po_fuf_types_same(Func_frame *sf, Func_frame *df);
Boolean po_types_same(Type_info *s, Type_info *d, int start);
void	po_print_type(Poco_cb *pcb, FILE *f, Type_info *ti);
long	po_get_type_size(Type_info *ti);
long	po_get_subtype_size(Poco_cb *pcb, Type_info *ti);
Boolean po_get_base_type(Poco_cb *pcb, Poco_frame *pf, Type_info *ti);
Symbol *po_need_local_symbol(Poco_cb *pcb);
Type_info *po_typi_type(Itypi *tip);

/* in pp.c */

void	pp_fatal(char *s);
void	po_free_pp(Poco_cb *pcb);
Boolean po_init_pp(Poco_cb *pcb, char *filename);
char	*po_pp_next_line(Poco_cb *pcb);

/* in ppeval.c */

long po_pp_eval(char *line, char *buf);

/* in runops.c */

Errcode poco_cont_ops(void *code_pt, Pt_num *pret, int arglength, ...);
Errcode po_run_ops(Poco_run_env *p, Code *code_pt, Pt_num *pret);

/* in safefile.c */

Errcode po_file_to_stdout(char *name);

/* in statemen.c */

Boolean po_eat_semi(Poco_cb *pcb);
Boolean po_eat_rbrace(Poco_cb *pcb);
Boolean po_eat_lbrace(Poco_cb *pcb);
void	po_get_statements(Poco_cb *pcb, Poco_frame *d);
void	po_get_block(Poco_cb *pcb, Poco_frame *d);
int 	po_need_comma_or_brace(Poco_cb *pcb);
void	po_check_array_dim(Poco_cb *pcb, Symbol *var);
Code_label *po_label_to_symbol(Poco_cb *pcb, Poco_frame *pf, Symbol *lsym);
void	po_exp_statement(Poco_cb *pcb, Poco_frame *pf);

/* in struct.c */

void		po_free_sif_list(Struct_info **psif);
void		po_move_sifs_to_parent(Poco_cb *pcb);
Struct_info *po_get_struct(Poco_cb *pcb, Poco_frame *pf, SHORT ttype);

/* in token.c  -- protos are in token.h, already #included above. */

/* in trace.c */

Line_data	*po_new_line_data(Poco_cb *pcb);
Boolean 	po_compress_line_data(Poco_cb *pcb,Line_data *ld);
void		po_free_line_data(Line_data *ld);
Boolean 	po_add_line_data(Poco_cb *pcb, Line_data *ld, long offset, long line);
void		po_print_trace(Poco_run_env *pe, FILE *tfile, Pt_num *stack, Pt_num *base,
				Pt_num *globals, Pt_num *ip, Errcode cerr);

/* in varinit.c */

void po_var_init(Poco_cb *pcb, Exp_frame *e, Symbol *var, SHORT frame_type);

#ifdef STRING_EXPERIMENT
/* in postring.c */
void po_add_local_string(Poco_cb *pcb, Poco_frame *pf, Symbol *symbol);
void po_free_local_string_list(Poco_cb *pcb, Poco_frame *pf);
void po_code_free_string_ops(Poco_cb *pcb, Poco_frame *pf);

String_ref *po_sr_new(int len);
String_ref *po_sr_new_copy(char *pt, int len);
String_ref *po_sr_new_string(char *pt);
String_ref *po_sr_cat(String_ref *a, String_ref *b);
void po_sr_inc_ref(String_ref *ref);
void po_sr_destroy(String_ref *ref);
Boolean po_sr_clean_ref(String_ref *ref);
void po_sr_dec_ref(String_ref *ref);
Boolean po_sr_eq(String_ref *a, String_ref *b);
Boolean po_sr_ge(String_ref *a, String_ref *b);
Boolean po_sr_le(String_ref *a, String_ref *b);
Boolean po_sr_eq_and_clean(String_ref *a, String_ref *b);
Boolean po_sr_ge_and_clean(String_ref *a, String_ref *b);
Boolean po_sr_le_and_clean(String_ref *a, String_ref *b);
String_ref *po_sr_cat_and_clean(String_ref *a, String_ref *b);
#endif /* STRING_EXPERIMENT */
/* end of protos */

#endif /* POCO_H */
