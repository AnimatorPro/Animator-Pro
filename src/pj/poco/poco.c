/****************************************************************************
 *
 * poco.c - contains a lot of routines shared by other modules.
 *
 * Error handling and symbol table management.	Also the
 * expression parser, reserved word / built in function list initializers,
 * and the 'frame' builder/destructors (one frame is built for each
 * function, and one for the program as a whole.)
 *
 * MAINTENANCE:
 *	08/18/90	(Ian)
 *				Took some of the logic from next_token in token.c, and
 *				moved it to lookup_token in here.  There was a switch
 *				statement essentially duplicated between the two, and
 *				lookup_token was the only routine to call next_token.
 *				A #define was placed in poco.h to inline the lookup_token
 *				routine's check for the reuse flag, so the routine in here
 *				is now called po_lookup_freshtoken -- it will only be called
 *				when a new token is really needed.
 *				A change was made to po_memzalloc such that it directly calls
 *				laskcmem instead of being routed through po_memalloc.
 *				po_expecting_got now calls po_expecting_got_str.
 *				Changed po_hashfunc to use simple addition.
 *				Changed in_symbol_list to do a quick first-char test
 *				before calling the more expensive strcmp.
 *	08/22/90	(Ian)
 *				Moved po_say_err from token.c to here.
 *	08/22/90	(Jim)
 *				Changed name of static routine new_name to str_to_name.
 *	08/23/90	(Ian)
 *				Fixed bug in po_lookup_freshtoken.	Case TOK_SQUO now assigns
 *				the constant value to both pcb->curtoken->val.num and t->tval.
 *				Also fixed bug in handling end-of-file in po_lookup_freshtoken.
 *				The 'return' statement following the setting of TOK_EOF was
 *				changed to a 'break' statement.
 *	08/25/90	(Ian)
 *				Added the expression frame caching logic to po_new_expframe.
 *				See comments in that area of the code for details.
 *	08/25/90	(Ian)
 *				Changed all occurances of freemem and gentle_freemem to
 *				po_freemem and poc_gentle_freemem.
 *	08/26/90	(Ian)
 *				Added 'mblk' memory management routines.  The basic idea here
 *				is that we aquire a few large blocks of memory from the
 *				parent, and hand them out to callers of po_memalloc and po_memzalloc.
 *				When a po_freemem call is made, we don't free the memory,
 *				then when the poco program is done running, we free everything.
 *				Yes, it's a horrible kludge, but it makes poco pretty fast.
 *	08/28/90	(Ian)
 *				All the 'mblk' stuff was moved to POCMEMRY.C.  Also, the
 *				caching of expression frames and poco frames was consolidated
 *				under the new generic cache handler coded in POCMEMRY.C.
 *	08/29/90	(Ian)
 *				Began setting up setjmp/longjmp error handling (yay!).	Any
 *				call to fatal or po_say_fatal will result in a longjmp back
 *				to the compile_poco routine in POCOFACE.  Moved some error
 *				cleanup functions out of po_compile_file and into POCOFACE.C,
 *				since a longjmp will return to there.
 *				Also, physically deleted a lot of DEADWOOD code left over
 *				from the memory management changes a few versions back.
 *	08/30/90	(Ian)
 *				Made po_code_elsize routine globally visible, now used by bop.
 *	09/01/90	(Ian)
 *				Nuked po_check_rparen routine.	Tweaked po_get_unop_expression.
 *	09/18/90	(Ian)
 *				Added reserved word 'union'.
 *				Added additional error handling to not_a_member. If the
 *				structure has been defined, the "not a member" message is
 *				still issue.  If the structure is incompletely defined (a
 *				tag exists, but no members were ever specified), it issues
 *				the "undefined structure" message.
 *	09/19/90	(Ian)
 *				Big changes in type parsing to support all ANSI keywords. In
 *				this module, the various keywords were added to the table of
 *				reserved words as PTOK_TYPE.  In addition, all other type-
 *				related entries (register, struct, etc) were also changed to
 *				PTOK_TYPE instead of having their own token values.  Changes
 *				in STATEMEN, DECLARE, and STRUCT also reflect this.
 *	09/20/90	(Ian)
 *				Added error checking for attempting to index a void
 *				pointer (eg, void *a; a[2] = 3; is illegal).  Also, attempt
 *				to dereference a void pointer using '*' now has its own error
 *				message, instead of reporting 'confused pointer dereference'.
 *	10/01/90	(Ian)
 *				Added support for enum constant symbols to po_lookup_freshtoken.
 *	10/02/90	(Ian)
 *				Modifications to sizeof handling; it now deals with any
 *				expression (ANSI-style).  It does not yet handle type or
 *				typedef names, (or expressions involving types: sizeof(int *)).
 *	10/02/90	(Ian)
 *				Fixed bug in not_an_element that was causing a page fault.
 *	10/04/90	(Ian)
 *				Fixed a glitch in make_assign, it now propogates the
 *				pure_const value to the target expression frame, so that we
 *				can properly detect non-constant init expressions for static,
 *				array, and struct initializer expressions.
 *				Added support for static vars, which constisted primarily
 *				of changing instances of var->scope to var->storage_scope
 *				when generating code to access a variable.	Changes in the
 *				po_new_var_space routine cause storage_scope to be set
 *				based upon the scope the variable was declared at and whether
 *				it was declared static or not.	(A static declaration implies
 *				a local symbol scope, but a global storage scope for that
 *				symbol.)
 *	10/05/90	(Ian)
 *				Tweaked po_say_err and po_say_fatal.  po_say_err now take a
 *				pointer to the poco_cb, instead of to a token, just for
 *				consistancy.  po_say_fatal now takes printf-type args.
 *	10/05/90	(Ian)
 *				Converted sprintf/po_say_fatal pairs to po_say_fatal w/formatting.
 *				Added po_say_internal to issue internal error messages.
 *	10/07/90	(Ian)
 *				Removed NULL checks from calls to memory allocation.
 *	10/20/90	(Ian)
 *				Fixed ref_op so that arrays can be accessed via '*'.
 *				Fixed po_find_local_assign and find_global_assign so that
 *				the value of the name of an array cannot be modified.  EG:
 *					char a[5];
 *					something = *a; 	// this is now legal
 *					++a;				// this is now illegal
 *	10/21/90	(Ian)
 *				Changed pcb->next_line to po_pp_next_line in freshtoken.  The
 *				preprocessor is now the single source of input lines from
 *				our point of view in the parser...no more indirection.
 *	10/22/90	(Ian)
 *				Added new_token, free_token, free_token_lists, and
 *				build_token_list routines, all part of supporting multi-
 *				token lookahead. Rewrote po_lookup_freshtoken correspondingly.
 *				Changed references to ts[0].whatever to curtoken->whatever.
 *				Changed references to t.ctoke to curtoken->ctoke.
 *	10/24/90	(Ian)
 *				Moved po_cat_exp to bop.c and made it static.
 *				Added TYPE_ARRAY case to outer switch of po_coerce_expression,
 *				to make array names and pointers more interchangable. Also,
 *				removed prior changes to ref_op and ind_op.
 *	10/26/90	(Ian)
 *				Added check for stack overflow in lookup_freshtoken.
 *				Changed all references to curtoken->type to t.toktype; added
 *				line to freshtoken to set t.toktype.  This removes one
 *				level of indirection in accessing the most-frequently used
 *				field in the token data.
 *				Added an extra check to get_pmember, now the type_info of
 *				the pointer must have exactly 2 type_comps, eg, it must be
 *				a single level of indirection and point directly to a struct.
 *	10/28/90	(Ian)
 *				Implemented concatenation of adjacent string literals.	Also,
 *				removed calls to translate_escapes, that functionality is
 *				now built in to the tokenize_word routine.
 *				Fixed a glitch in freshtoken; now a token is only classified
 *				as is_symbol == TRUE if the symbol token type is PTOK_VAR,
 *				PTOK_LABEL, or PTOK_UNDEF.	This prevents keywords from
 *				being used as variable names within non-global scopes, which
 *				was occuring because of the way force_local_symbol works.
 *	10/29/90	(Ian)
 *				Tightened type conversion rules in upgrade_to_ido, we now
 *				prevent attempts to convert IDO_CPT or IDO_VPT to numbers.
 *	10/29/90	(Ian)
 *				Once again attempted to fix problem with making pointers and
 *				array names interchangable.  The 'fix' done on 10/20/90 was
 *				to change ref_op and ind_op to handle TYPE_ARRAY the
 *				same as TYPE_POINTER.  This failed dismally, and was backed
 *				out on 10/24/90.  This time, taking our clue from the way
 *				get_array works, the make_deref routine looks at the
 *				return value from ref_op or ind_op, and if the value
 *				is -1 and the current end type is TYPE_ARRAY, we exit without
 *				further action (a -1 return for any other end type is still
 *				an error.)	The weirdness here is in ref_op and ind_op.
 *				If they see a comp_count of 1, they will return the right
 *				opcode.  But, in the case of code such as...
 *					char a[5][5];
 *					**a = 0x00;
 *				...the comp_count will be two during the making of the first
 *				of the two derefs.	Thus, ind_op returns -1, to say that
 *				there is no opcode which does this type of derefence.  And,
 *				that's pretty much true.  Since 'a' is an array, an
 *				OP_xxx_ADDRESS has already been generated to stack the
 *				pointer/address value.	To make the first deref, we would
 *				add an offset of zero to the address on the stack, which is
 *				pointless, so we just exit without complaining.  When the
 *				parser un-recurses back to the point of making the deref
 *				on the first star, ind_op will now see a comp_count of just
 *				one, so it will return OP_CI_ASS to make the deref.
 *	10/29/90	(Ian)
 *				Added logic to po_rehash to detect a symbol with a null
 *				string (\0) name.  This indicates that within a prototype
 *				which has a code block following it, somebody forgot to
 *				code the parm name(s).	Since po_rehash doesn't get called
 *				unless there is a code block, this allows true prototypes
 *				to be specified without names.
 *	02/07/91	(Ian)
 *				Added 'Errcode' as a variant spelling of 'ErrCode' in the
 *				builtin datatypes list.
 *	05/01/91	(Ian)
 *				Added formatting of error text to po_say_error when the
 *				error status is not Err_syntax.
 *	09/06/91	(Jim)
 *		o		Started to add in String type.	Changed unused TYPE_RAST
 *				to TYPE_STRING.
 *		o		Noticed that the use of TYPE_CPT and
 *				TYPE_NCPT were inconsistant.  The only place either
 *				was generated is TYPE_NCPT for pointer parameters to
 *				library routines that take variable arguments past
 *				the ellipsis.  But elsewhere in code TYPE_CPT was
 *				looked for.  The difference between the two
 *				was simply whether a NULL-check was inserted.
 *				This ended up not being such a hot idea, so
 *				I took the NULL-check out of the type-coercion
 *				of TYPE_CPT, and eliminated TYPE_NCPT
 *		o		Changed name of upgrade_to_ido to
 *				upgrade_numerical_expression.
 *		o		Made po_coerce_numeric_exp not accept void as a numeric type.
 *		o		Got rid of OP_IXXX+ido_type stuff.	Now is
 *				po_xxx_ops[ido_type].
 *	05/31/92	(Ian)
 *				Changed get_member so that the code from the left buffer
 *				gets moved to the right buffer to access an array inside
 *				a structure.
 *	06/03/92	(Ian)
 *				Changed make_deref so that the code from the right buffer
 *				(the pointer expression) gets moved to the left buffer when
 *				the end type is TYPE_ARRAY.  This keeps the lval properly
 *				updated during expression parsing, but defers the generation
 *				of an actual ref-op until the subscript expression or
 *				dereferencing '*' is processed.  Oddly enough, this was
 *				only a problem with arrays in structures accessed via a
 *				pointer to the structure, because that's the only time when
 *				the pointer expression in the right buffer differed from
 *				the lval expression in the left buffer.
 *	08/17/92	(Ian)
 *			  > Changed error reporting a bit.	The pcb has a new field,
 *				error_line_number.	If this is zero, we take the line number
 *				from curtoken or fs->line_count as we used to, and we save
 *				the number in error_line_number.  If it is non-zero, we
 *				just go with it.  The latter happens when the preprocessor
 *				detects an error -- it sets error_line_number before calling
 *				po_say_fatal, since curtoken won't have the right line
 *				number on a preprocessor error.  error_line_number gets passed
 *				back to the host as the line to position the builtin editor to.
 *				Also, added a new function, po_say_warning.  This is like
 *				po_say_error except that error_line_number is NOT saved.
 *			  > An unexpected '}' is now reported as error, by compile_file.
 *			  > Changed po_assign_after_equals to handle checking for
 *				constant init expressions before making the assignment.
 *			  > get_address now propogates pure_const to the destination
 *				expression frame.
 *	09/14/92	(Ian)
 *				Fixed po_say_fatal to check for t.file_stack being NULL
 *				before it tries to access t.file_stack->line_number.
 ****************************************************************************/

#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <limits.h>  /* so we can properly determine max int value */
#include "poco.h"

extern jmp_buf po_compile_errhandler;

/* internal predeclarations */

static void get_prec1(Poco_cb *pcb, Exp_frame *e);
static void assign_after_value(Poco_cb *pcb, Exp_frame *e, Symbol *var);

/****** MODULE ERROR error handling messages ********/

static void po_say_err(Poco_cb *pcb, char *s)
/*****************************************************************************
 * po_say_err - Format error message text, including input filename and line #.
 ****************************************************************************/
{
Token	   *t  = &pcb->t;
File_stack *fs = t->file_stack;
char	   errtxtbuf[128];
long	   line_num = 0;
long	   char_num = 0;

if (0 == (line_num = pcb->error_line_number))
	{
	if (pcb->curtoken != NULL)
		{
		line_num = pcb->curtoken->line_num;
		char_num = pcb->curtoken->char_num;
		}
	else if (fs != NULL)
		{
		line_num = fs->line_count;
		char_num = 0;
		}
	pcb->error_line_number = line_num;
	pcb->error_char_number = char_num;
	}

if (pcb->global_err == Err_syntax)
	strcpy(errtxtbuf,"Error");
else
	get_errtext(pcb->global_err, errtxtbuf);

if (fs == NULL)
	fprintf(t->err_file, "%s\n%s\n", errtxtbuf, s);
else
	fprintf(t->err_file, "%s in %s near line %ld:\n%s\n",
		errtxtbuf, fs->name, line_num, s);
}

void po_say_warning(Poco_cb *pcb, char *fmt, ...)
/*****************************************************************************
 * format & output an error message but don't stop & don't save errline #.
 ****************************************************************************/
{
char	sbuf[512];
va_list args;

va_start(args, fmt);
vsprintf(sbuf, fmt, args);
va_end(args);

if (pcb->global_err >= 0)
	pcb->global_err = Err_syntax;

po_say_err(pcb, sbuf);
pcb->error_line_number = 0; // forget error line #, just a warning
}

void po_say_fatal(Poco_cb *pcb, char *fmt, ...)
/*****************************************************************************
 * format & output an error message, then longjump to error handler.
 ****************************************************************************/
{
char	sbuf[512];
va_list args;

va_start(args, fmt);
vsprintf(sbuf, fmt, args);
va_end(args);

if (pcb->global_err >= 0)
	pcb->global_err = Err_syntax;

po_say_err(pcb, sbuf);

longjmp(po_compile_errhandler, pcb->global_err);
}

void po_say_internal(Poco_cb *pcb, char *fmt, ...)
/*****************************************************************************
 * format & output an error message, then longjump to error handler.
 ****************************************************************************/
{
char	sbuf[512];
va_list args;

va_start(args, fmt);
vsprintf(sbuf, fmt, args);
va_end(args);

pcb->global_err = Err_poco_internal;
po_say_fatal(pcb, "poco internal error: %s", sbuf);
}

void po_expecting_got(Poco_cb *pcb, char *expecting)
/*****************************************************************************
 * output a message saying we were expecting one token and got another.
 ****************************************************************************/
{
po_expecting_got_str(pcb, expecting, pcb->curtoken->ctoke);
}

void po_expecting_got_str(Poco_cb *pcb, char *expecting, char *got)
/*****************************************************************************
 * output a message saying we expecting something and got something else.
 ****************************************************************************/
{
po_say_fatal(pcb, "expecting %s got '%s'", expecting, got);
}

Boolean po_need_token(Poco_cb *pcb)
/*****************************************************************************
 * get a token, and if it is TOK_EOF, complain and die.
 ****************************************************************************/
{
lookup_token(pcb);
if (pcb->t.toktype == TOK_EOF)
	{
	po_say_fatal(pcb, "unexpected end of file");
	return(FALSE);
	}
return(TRUE);
}


void po_redefined(Poco_cb *pcb, char *s)
/*****************************************************************************
 * redefined symbol error - complain and die.
 ****************************************************************************/
{
po_say_fatal(pcb, "%s redefined", s);
}

void po_undefined(Poco_cb *pcb, char *s)
/*****************************************************************************
 * undefined symbol error - complain and die.
 ****************************************************************************/
{
po_say_fatal(pcb, "%s undefined", s);
}

void po_unmatched_paren(Poco_cb *pcb)
/*****************************************************************************
 * unmatched parens error - complain and die.
 ****************************************************************************/
{
po_say_fatal(pcb, "unmatched parenthesis");
}

void po_expecting_lbrace(Poco_cb *pcb)
/*****************************************************************************
 * we wanted to see an open brace - complain and die.
 ****************************************************************************/
{
po_expecting_got(pcb, "{");
}

void po_expecting_rbrace(Poco_cb *pcb)
/*****************************************************************************
 * we wanted to see a closing brace - complain and die.
 ****************************************************************************/
{
po_expecting_got(pcb, "}");
}


/******  MODULE MEMORY Some memory allocation routines **********/
 /*** memory management has been moved to module pocmemry.c ***/


void po_freelist(void *l)
/*****************************************************************************
 * Free any simple singly linked list with next field at offset 0
 ****************************************************************************/
{
Names *cur = *((Names **)l);
Names *next;

while (cur != NULL)
	{
	next = cur->next;
	po_freemem(cur);
	cur = next;
	}
*((Names **)l) = NULL;
}



/****** MODULE SYMBOL Symbol table management ***********/

#ifdef DEADWOOD /* now lives in pocoutil.asm */

int po_hashfunc(UBYTE *s)
/*****************************************************************************
 * typical hashing function.
 ****************************************************************************/
{

int acc;
int c;

acc = *s++;
while ((c = *s++) != 0)
	acc = (acc /* <<1 */ )+c;
return(acc&(HASH_SIZE-1));

}

#endif /* DEADWOOD */

Symbol *po_unlink_el(Symbol *list, Symbol *el)
/*****************************************************************************
 * unlink an element from a singly-linked list.
 ****************************************************************************/
{
Symbol *next;

if (list == el)
	return(el->next);
next = list;
while (next->next != el)
	next = next->next;
next->next = el->next;
return(list);
}

void po_unhash_symbol(Poco_cb *pcb, Symbol *s)
/*****************************************************************************
 * remove a symbol from the current poco_frame's hash table.
 ****************************************************************************/
{
Symbol **hash_slot;

hash_slot = pcb->rframe->hash_table+po_hashfunc(s->name);
*hash_slot = po_unlink_el(*hash_slot, s);
}

static Symbol *new_symbol(Poco_cb *pcb, char *s, SHORT tok_type)
/*****************************************************************************
 * alloc and init Symbol, put name after Symbol in memory, hash sym into table.
 ****************************************************************************/
{
Symbol *new;
Symbol **hash_slot;

hash_slot = pcb->rframe->hash_table+po_hashfunc(s);
new = po_memzalloc(pcb, sizeof(*new) + strlen(s) + 1);
new->name = (char *)(new+1);
strcpy(new->name,s);
new->tok_type = tok_type;
new->next = *hash_slot;
*hash_slot = new;
return(new);
}

int po_rehash(Poco_cb *pcb, Symbol *s)
/*****************************************************************************
 * po_rehash a set of symbols into a hash table.
 * returns zero on success, or if a parm without a name was found, returns
 * the number (first parm is 1, etc) of the bad parm.
 *
 * this is used to copy a set of function parameters from the fuf to
 * the poco_frame, so that the params look like local variables when
 * processing the body of the function.  note that the logic in this routine
 * follows the 'link' pointers, but connects the 'next' pointers.
 ****************************************************************************/
{
int 	counter = 0;
Symbol	**hash_slot;

	while (s != NULL)
		{
		++counter;
		if (s->name[0] == '\0')
			return counter;
		hash_slot = pcb->rframe->hash_table+po_hashfunc(s->name);
		s->next = *hash_slot;
		*hash_slot = s;
		s = s->link;
		}
	return 0;
}

void po_free_symbol(Symbol *s)
/*****************************************************************************
 * free a symbol, and its associated type_info, if there is one.
 ****************************************************************************/
{
poc_gentle_freemem(s->ti);
po_freemem(s);
}

void po_free_symbol_list(Symbol **ps)
/*****************************************************************************
 * free a list of symbols by following the 'link' (not 'next') pointers.
 ****************************************************************************/
{
Symbol *sym, *link;

sym = *ps;
while (sym != NULL)
	{
	link = sym->link;
	po_free_symbol(sym);
	sym = link;
	}
*ps = NULL;
}

Symbol *po_new_symbol(Poco_cb *pcb, char *name)
/*****************************************************************************
 * alloc a new Symbol, and attach it to the current poco_frame.
 ****************************************************************************/
{
Symbol		*s;
Poco_frame	*rf = pcb->rframe;

if ((s = new_symbol(pcb, name, PTOK_UNDEF)) == NULL)
	return(FALSE);
s->link  = rf->symbols;
s->scope = rf->scope;
rf->symbols = s;
return(s);
}

static Symbol *in_symbol_list(register Symbol *l, char *name)
/*****************************************************************************
 * find symbol via linear search of a linked list (using 'next' pointers).
 ****************************************************************************/
{
register char c1 = *name;

while (l != NULL)
	{
	if (l->name[0] == c1) /* quick-check first chars before making call */
		if (po_eqstrcmp(l->name, name) == 0)
			return(l);
	l = l->next;
	}
return(NULL);
}

static Symbol *find_symbol(Poco_cb *pcb, char *name)
/*****************************************************************************
 * find a symbol by search poco_frames from most current scope down to global.
 ****************************************************************************/
{
struct poco_frame *p;
Symbol *s;
int hashval;

p = pcb->rframe;
hashval = po_hashfunc(name);
while (p != NULL)
	{
	if ((s = in_symbol_list(p->hash_table[hashval], name)) != NULL)
		{
		return(s);
		}
	p = p->next;
	}
return(NULL);
}

int po_link_len(Symbol *l)
/*****************************************************************************
 * return the length of (number of items in) a linked list.
 ****************************************************************************/
{
int count = 0;

while (l != NULL)
	{
	count+=1;
	l = l->link;
	}
return(count);
}

void *po_reverse_links(Symbol *el)
/*****************************************************************************
 * reverse the order of the elements in a singly-linked list.
 ****************************************************************************/
{
Symbol *link, *list;

list = NULL;
while (el != NULL)
	{
	link = el->link;
	el->link = list;
	list = el;
	el = link;
	}
return(list);
}

/****** MODULE TOKENFACE ******** interface to tokenizer */

static Tstack *new_token(Poco_cb *pcb)
/*****************************************************************************
 *
 ****************************************************************************/
{
Tstack *t;

	if (NULL == (t = pcb->free_tokens))
		t = po_memalloc(pcb, sizeof(*t));
	else
		pcb->free_tokens = t->next;
	return t;
}

static void free_token(Poco_cb *pcb, Tstack *t)
/*****************************************************************************
 *
 ****************************************************************************/
{
	t->next = pcb->free_tokens;
	pcb->free_tokens = t;
}

static void free_token_lists(Poco_cb *pcb)
/*****************************************************************************
 *
 ****************************************************************************/
{
Tstack *cur;
Tstack *next;

	for (cur = pcb->curtoken; cur != NULL; cur = next)
		{
		next = cur->next;
		po_freemem(cur);
		}
	for (cur = pcb->free_tokens; cur != NULL; cur = next)
		{
		next = cur->next;
		po_freemem(cur);
		}
}

static Tstack *build_token_list(Poco_cb *pcb)
/*****************************************************************************
 * read the next input line and add its tokens to the lookahead list.
 *	items of note...
 *	> we currently guarantee only one token of lookahead.  (that is, you can
 *	  always look at curtoken->next->whatever_field without worrying about a
 *	  NULL next pointer.  if this number is increased, this routine will have
 *	  to be tweaked to loop over multiple input lines, as it is possible to
 *	  get a line that has only one token on it.
 *	> the preprocessor routine po_pp_next_line is now the single source of input
 *	  lines from our point of view.  it will decide whether to get data from
 *	  the list of library prototype lines or from the input file.
 *	> when we hit EOF, we add a couple of TOK_EOF entries to the end of the
 *	  token list.  the number of entries added should be one more than the
 *	  maximum number of lookahead items supported.
 *	> this is the routine responsible for concatenating adjacent string
 *	  litterals.  if a line of input ends on a string token, we'll loop to
 *	  read the next line.
 *	> one day we will support reading a pre-tokenized file. this is probably
 *	  the point at which we would decided whether to read tokens from a file
 *	  or to call po_pp_next_line to get more tokens that way.
 ****************************************************************************/
{
Tstack	*ts;
Tstack	*first_ts;
Tstack	*prev_ts;
Tstack	dummy_ts;
char	*line_pos;
char	*line_start;
char	*strbase;
char	*strwrk;
long	line_count;
short	strlit_len;
short	ctoke_size;

	ts = new_token(pcb);

	first_ts   = ts;
	strbase    = pcb->strlit_work;
	strwrk	   = strbase;
	strlit_len = 0;
	prev_ts    = &dummy_ts;
	prev_ts->type = PTOK_MAX+1;

NEED_MORE:

	line_start = line_pos = po_pp_next_line(pcb);

	if (line_pos == NULL)		/* handle EOF */
		{
		ts->type = TOK_EOF;
		ts->line_num = 0;
		ts->char_num = 0;
		ts->ctoke[0] = '\0';
		prev_ts = ts;
		ts = ts->next = new_token(pcb);
		ts->type = TOK_EOF;
		ts->line_num = 0;
		ts->char_num = 0;
		ts->ctoke[0] = '\0';
		goto ENDFILE;
		}

	line_count = pcb->t.file_stack->line_count;

	for (;;)
		{
		ts->is_symbol = FALSE;
		ts->line_num  = line_count;

		line_pos = (char *)tokenize_word((UBYTE *)line_pos, (UBYTE *)ts->ctoke,
									(UBYTE *)strwrk, &ctoke_size, &ts->type,
									FALSE);
		if (line_pos == NULL)
			goto ENDLINE;

		ts->char_num = 1 + ((line_pos - ctoke_size) - line_start);

		if (prev_ts->type == TOK_QUO && ts->type != TOK_QUO)
			{
			register Names *n;

			n = po_memzalloc(pcb, sizeof(*n) + strlit_len + 1);
			n->name = (char *)(n+1);
			poco_copy_bytes(strbase, n->name, strlit_len+1);
			n->next = pcb->run.literals;
			pcb->run.literals = n;
			prev_ts->val.string = n->name;
			prev_ts->type = PTOK_QUO;
			prev_ts->ctoke_size = strlit_len;
			strncpy(prev_ts->ctoke, prev_ts->val.string, MAX_SYM_LEN-1);
			strlit_len = 0;
			strwrk = strbase;
			}

		ts->ctoke_size = ctoke_size;

		switch (ts->type)
			{
			case TOK_INT:
			case TOK_LONG:

				if (ts->ctoke[1] == 'X')
					ts->val.num = htol(ts->ctoke);
				else
					ts->val.num = atol(ts->ctoke);

				if (ts->val.num > INT_MAX)
					ts->type = TOK_LONG;

				break;

			case TOK_DOUBLE:

				ts->val.dnum = atof(ts->ctoke);
				break;

			case TOK_SQUO:

				if (ctoke_size > 1)
					po_say_fatal(pcb, "invalid character constant '%s'",
								  ts->ctoke);
				ts->val.num = (unsigned char)(ts->ctoke[0]);
				ts->type = TOK_INT;
				break;

			case TOK_QUO:

				strwrk = strbase + (strlit_len += ctoke_size);
				if (strlit_len > MAX_STRLIT_LEN)
					po_say_fatal(pcb, "string literal exceeds length limit of %d characters",
								MAX_STRLIT_LEN);
				break;

			} /* END switch (type) */


		if (prev_ts->type != TOK_QUO)
			{
			prev_ts = ts;
			ts = ts->next = new_token(pcb);
			}

		} /* END  for(;;) */

ENDLINE:

	if (prev_ts == &dummy_ts || prev_ts->type == TOK_QUO)
		goto NEED_MORE;

ENDFILE:

	prev_ts->next = NULL;
	free_token(pcb, ts);

#ifdef DEBUG
	for (ts = first_ts; ts; ts = ts->next)
		printf("%s ",ts->ctoke);
	printf("\n");
#endif

	return first_ts;
}

static SHORT lookahead_type(Poco_cb *pcb)
/*****************************************************************************
 * return the type of the next token.
 ****************************************************************************/
{
Symbol *s;
Tstack *ts = pcb->curtoken->next;
SHORT  ttype = ts->type;

	if (ttype != TOK_UNDEF)
		return ttype;

	if (NULL != (s = find_symbol(pcb, ts->ctoke)))
		{
		ttype = s->tok_type;
		if (ttype == PTOK_ENUMCONST)
			return TOK_INT;
		else
			return ttype;
		}
	return TOK_UNDEF;

}

void po_lookup_freshtoken(Poco_cb *pcb)
/*****************************************************************************
 * get the next token from tokenize_word, handle symbols and constant values.
 * the lookup_token() function was converted to a #define in poco.h, so that
 * the check for the reuse flag (set by pushback_token) is generated inline.
 * thus, this routine will always get a 'fresh' token from the input file.
 * in addition, this routine used to call next_token(), an interface to
 * the tokenizer that would loop if necessary to get more lines of data for
 * the tokenizer; this looping is now handled here.
 ****************************************************************************/
{
Tstack	*ts;
Symbol	*s;

	if (((char *)&pcb) < pcb->stack_bottom)
		po_say_fatal(pcb, "stack overflow (statements too deeply nested)");

	ts = pcb->curtoken->next;
	free_token(pcb, pcb->curtoken);
	pcb->curtoken = ts;
	if (ts->next == NULL)
		ts->next = build_token_list(pcb);

	if (ts->type == TOK_UNDEF)
		{
		if (NULL != (s = find_symbol(pcb, ts->ctoke)))
			{
			ts->type = s->tok_type;
			if (ts->type == PTOK_ENUMCONST)
				{
				ts->val.num = s->symval.i;
				ts->type = TOK_INT;
				}
			else
				{
				ts->val.symbol = s;
				if (ts->type == PTOK_VAR	||
					ts->type == PTOK_LABEL	||
					ts->type == PTOK_UNDEF)
					ts->is_symbol = TRUE;
				}
			}
		else
			{
			s = po_new_symbol(pcb, ts->ctoke);
			ts->type = PTOK_UNDEF;
			ts->val.symbol = s;
			ts->is_symbol = TRUE;
			}
		}
	pcb->t.toktype = ts->type;
	return;
}



/********** MODULE EATSEMI ********************/


Boolean po_is_next_token(Poco_cb *pcb, SHORT ttype)
/*****************************************************************************
 * indicate whether the next token is the indicated type (ie, look ahead).
 ****************************************************************************/
{
Boolean ret = FALSE;

lookup_token(pcb);
if (pcb->t.toktype == ttype || pcb->t.toktype == TOK_EOF)
	ret = TRUE;
pushback_token(&pcb->t);
return(ret);
}

Boolean po_eat_token(Poco_cb *pcb, SHORT ttype)
/*****************************************************************************
 * if the next token is the right type, eat it, else complain and die.
 ****************************************************************************/
{
char buf[2];

if (po_need_token(pcb))
	{
	if (pcb->t.toktype == ttype)
		return(TRUE);
	else
		{
		buf[0] = ttype;
		buf[1] = 0;
		po_expecting_got(pcb, buf);
		}
	}
return(FALSE);
}

Boolean po_eat_rbracket(Poco_cb *pcb)
/*****************************************************************************
 * eat a closing bracket, if wrong token appears next, complain and die.
 ****************************************************************************/
{
return(po_eat_token(pcb, ']'));
}

Boolean po_eat_lparen(Poco_cb *pcb)
/*****************************************************************************
 * eat an opening paren, if wrong token appears next, complain and die.
 ****************************************************************************/
{
return(po_eat_token(pcb, TOK_LPAREN));
}

Boolean po_eat_rparen(Poco_cb *pcb)
/*****************************************************************************
 * eat a closing paren, if wrong token appears next, complain and die.
 ****************************************************************************/
{
return(po_eat_token(pcb, TOK_RPAREN));
}

#ifdef DEADWOOD

Boolean po_check_rparen(Poco_cb *pcb)
/*****************************************************************************
 * ensure a closing paren is next, if not, complain and die.
 * (this was used only by mk_func_call(), it has been nuked.)
 ****************************************************************************/
{
return(po_is_next_token(pcb, TOK_RPAREN));
}

#endif /* DEADWOOD */

/******* MODULE VARIABLE stuff to assign and use variables *******/

/* table to convert from expression type to variable type */
static SHORT inv_ido[] = {TYPE_INT, TYPE_LONG, TYPE_DOUBLE, TYPE_POINTER,};

static void no_assign_void(Poco_cb *pcb)
/*****************************************************************************
 * issue void assignment error message, and die.
 ****************************************************************************/
{
po_say_fatal(pcb, "can't assign to void variable");
}

static void unknown_assignment(Poco_cb *pcb)
/*****************************************************************************
 * issue error message about assignment type, and die.
 ****************************************************************************/
{
po_say_fatal(pcb, "unknown type in assignment");
}

static void no_struct_assign(Poco_cb *pcb)
/*****************************************************************************
 * issue error message about assignment of struct by value, and die.
 ****************************************************************************/
{
po_say_fatal(pcb, "poco can't do struct assignments");
}

SHORT po_find_local_assign(Poco_cb *pcb, Type_info *ti)
/*****************************************************************************
 * return the proper opcode for a local variable assignment.
 ****************************************************************************/
{
SHORT aop;

if (ti->comp_count == 1)
	{
	switch (ti->comp[0])
		{
		case TYPE_VOID:
			no_assign_void(pcb);
			break;
		case TYPE_CHAR:
			aop = OP_LOC_CASS;
			break;
		case TYPE_SHORT:
			aop = OP_LOC_SASS;
			break;
		case TYPE_INT:
			aop = OP_LOC_IASS;
			break;
		case TYPE_LONG:
			aop = OP_LOC_LASS;
			break;
		case TYPE_FLOAT:
			aop = OP_LOC_FASS;
			break;
		case TYPE_DOUBLE:
			aop = OP_LOC_DASS;
			break;
		case TYPE_STRUCT:
			no_struct_assign(pcb);
			break;
#ifdef STRING_EXPERIMENT
		case TYPE_STRING:
			aop = OP_LOC_STRING_ASS;
			break;
#endif /* STRING_EXPERIMENT */
		default:
			unknown_assignment(pcb);
			break;
		}
	}
else
	{
	if (po_is_array(ti))
		po_say_fatal(pcb,"lvalue required");
	else
		aop = OP_LOC_PASS;
	}
return(aop);
}

static SHORT find_global_assign(Poco_cb *pcb, Type_info *ti)
/*****************************************************************************
 * return the proper opcode for a global variable assignment.
 ****************************************************************************/
{
SHORT aop;

if (ti->comp_count == 1)
	{
	switch (ti->comp[0])
		{
		case TYPE_VOID:
			no_assign_void(pcb);
			break;
		case TYPE_CHAR:
			aop = OP_GLO_CASS;
			break;
		case TYPE_SHORT:
			aop = OP_GLO_SASS;
			break;
		case TYPE_INT:
			aop = OP_GLO_IASS;
			break;
		case TYPE_LONG:
			aop = OP_GLO_LASS;
			break;
		case TYPE_FLOAT:
			aop = OP_GLO_FASS;
			break;
		case TYPE_DOUBLE:
			aop = OP_GLO_DASS;
			break;
		case TYPE_STRUCT:
			no_struct_assign(pcb);
			break;
#ifdef STRING_EXPERIMENT
		case TYPE_STRING:
			aop = OP_GLO_STRING_ASS;
			break;
#endif /* STRING_EXPERIMENT */
		default:
			unknown_assignment(pcb);
			break;
		}
	}
else
	{
	if (po_is_array(ti))
		po_say_fatal(pcb,"lvalue required");
	else
		aop = OP_GLO_PASS;
	}
return(aop);
}

SHORT po_find_assign_op(Poco_cb *pcb, Symbol *var, Type_info *ti)
/*****************************************************************************
 * return the right opcode to make a variable assignment.
 ****************************************************************************/
{
return(var->storage_scope == SCOPE_GLOBAL ? find_global_assign(pcb,ti) :
	po_find_local_assign(pcb,ti));
}

static void no_use_void(Poco_cb *pcb)
/*****************************************************************************
 * issue error message about using a void value, and die.
 ****************************************************************************/
{
po_say_fatal(pcb, "can't use void value");
}

static void po_var_too_complex(Poco_cb *pcb)
/*****************************************************************************
 * issue error message that variable type is too complex, and die.
 ****************************************************************************/
{
po_say_fatal(pcb, "variable too complicated to use...");
}

Ido_table po_ido_table[] =
/* Table that lets us quickly determine what operations are legal on
 * a certain IDO_TYPE */
{
/**  is_num can_add ido_type	**/
	{TRUE,	TRUE,	IDO_INT,},
	{TRUE,	TRUE,	IDO_LONG,},
	{TRUE,	TRUE,	IDO_DOUBLE,},
	{FALSE, TRUE,	IDO_POINTER,},
	{FALSE, TRUE,	IDO_CPT,},
	{FALSE, FALSE,	IDO_VOID,},
	{FALSE, FALSE,	IDO_VPT,},
#ifdef STRING_EXPERIMENT
	{FALSE, FALSE,	IDO_STRING,},
#endif /* STRING_EXPERIMENT */
};
static Boolean po_check_ido_table(Poco_cb *pcb)
/*****************************************************************************
 * Run a sanity check on table to correlate IDO-types with instructions
 * to pop result of expression stack.
 ****************************************************************************/
{
#ifdef DEVELOPMENT
int i;

for (i=0; i<Array_els(po_ido_table); i++)
	{
	if (i != po_ido_table[i].ido_type)
		{
		fprintf(pcb->t.err_file, "%d != %d\n",i,po_ido_table[i].ido_type);
		po_say_internal(pcb, "po_ido_table doesn't check");
		return(FALSE);
		}
	}
#endif
return(TRUE);
}

static SHORT find_global_use(Poco_cb *pcb, Type_info *ti)
/*****************************************************************************
 * return the proper opcode to use a global variable.
 ****************************************************************************/
{
SHORT op;

if (ti->ido_type == IDO_POINTER)
	{
	op =  OP_GLO_PVAR;
	}
else if (ti->comp_count == 1)
	{
	switch (ti->comp[0])
		{
		case TYPE_VOID:
			no_use_void(pcb);
			break;
		case TYPE_CHAR:
			op =  OP_GLO_CVAR;
			break;
		case TYPE_SHORT:
			op =  OP_GLO_SVAR;
			break;
		case TYPE_INT:
			op =  OP_GLO_IVAR;
			break;
		case TYPE_LONG:
			op =  OP_GLO_LVAR;
			break;
		case TYPE_FLOAT:
			op =  OP_GLO_FVAR;
			break;
		case TYPE_DOUBLE:
			op =  OP_GLO_DVAR;
			break;
		case TYPE_STRUCT:
			op =  OP_GLO_FVAR; /* filled in later */
			break;
#ifdef STRING_EXPERIMENT
		case TYPE_STRING:
			op = OP_GLO_STRING_VAR;
			break;
#endif /* STRING_EXPERIMENT */
		default:
			goto ERR;
		}
	}
else
	{
ERR:
	po_var_too_complex(pcb);
	}
return(op);
}

SHORT po_find_local_use(Poco_cb *pcb, Type_info *ti)
/*****************************************************************************
 * return the proper opcode to use a local variable.
 ****************************************************************************/
{
SHORT op;

if (ti->ido_type == IDO_POINTER)
	{
	op =  OP_LOC_PVAR;
	}
else if (ti->comp_count == 1)
	{
	switch (ti->comp[0])
		{
		case TYPE_VOID:
			no_use_void(pcb);
			break;
		case TYPE_CHAR:
			op =  OP_LOC_CVAR;
			break;
		case TYPE_SHORT:
			op =  OP_LOC_SVAR;
			break;
		case TYPE_INT:
			op =  OP_LOC_IVAR;
			break;
		case TYPE_LONG:
			op =  OP_LOC_LVAR;
			break;
		case TYPE_FLOAT:
			op =  OP_LOC_FVAR;
			break;
		case TYPE_DOUBLE:
			op =  OP_LOC_DVAR;
			break;
		case TYPE_STRUCT:
			op =  OP_LOC_FVAR; /* filled in later */
			break;
#ifdef STRING_EXPERIMENT
		case TYPE_STRING:
			op = OP_LOC_STRING_VAR;
			break;
#endif /* STRING_EXPERIMENT */
		default:
			goto ERR;
		}
	}
else
	{
ERR:
	po_var_too_complex(pcb);
	}
return(op);
}

static SHORT find_use_op(Poco_cb *pcb, Symbol *var, Type_info *ti)
/*****************************************************************************
 * return the proper opcode to use a variable.
 ****************************************************************************/
{
return(var->storage_scope == SCOPE_GLOBAL ? find_global_use(pcb,ti) :
	po_find_local_use(pcb,ti));
}


static SHORT ref_op(Poco_cb *pcb, Type_info *ti)
/*****************************************************************************
 * return the proper opcode for an indirect use of a variable.
 ****************************************************************************/
{
if (po_is_pointer(ti))
	return(OP_PI_VAR);
else
	{
	if (ti->comp_count != 1)
		return(-1);
	else
		{
		switch (ti->comp[0])
			{
			case TYPE_CHAR:
				return(OP_CI_VAR);
			case TYPE_SHORT:
				return(OP_SI_VAR);
			case TYPE_INT:
				return(OP_II_VAR);
			case TYPE_LONG:
				return(OP_LI_VAR);
			case TYPE_FLOAT:
				return(OP_FI_VAR);
			case TYPE_DOUBLE:
				return(OP_DI_VAR);
			case TYPE_STRUCT:	/* return dummy value, will be patched later */
				return(OP_II_VAR);
#ifdef STRING_EXPERIMENT
			case TYPE_STRING:
				return(OP_STRING_I_VAR);
#endif /* STRING_EXPERIMENT */
			default:
				return(-1);
			}
		}
	}
}

static SHORT ind_op(Poco_cb *pcb, Type_info *ti)
/*****************************************************************************
 * return the proper opcode for an indirect assign of a variable.
 ****************************************************************************/
{
if (po_is_pointer(ti))
	return(OP_PI_ASS);
else
	{
	if (ti->comp_count != 1)
		return(-1);
	else
		{
		switch (ti->comp[0])
			{
			case TYPE_CHAR:
				return(OP_CI_ASS);
			case TYPE_SHORT:
				return(OP_SI_ASS);
			case TYPE_INT:
				return(OP_II_ASS);
			case TYPE_LONG:
				return(OP_LI_ASS);
			case TYPE_FLOAT:
				return(OP_FI_ASS);
			case TYPE_DOUBLE:
				return(OP_DI_ASS);
			case TYPE_STRUCT:	/* return dummy value, will be patched later */
				return(OP_END);
#ifdef STRING_EXPERIMENT
			case TYPE_STRING:
				return(OP_STRING_I_ASS);
#endif /* STRING_EXPERIMENT */
			default:
				return(-1);
			}
		}
	}
}

void po_code_elsize(Poco_cb *pcb, Exp_frame *e, int el_size)
/*****************************************************************************
 * generate radix value to reach an element of an array.
 ****************************************************************************/
{
int bipower;
int i;

if (el_size == 0)
	po_say_fatal(pcb, "size of type is zero (eg, pointer to void)");

if (el_size != 1)
	{
	bipower = 2;
	for (i=1; i<15; i++)		/* try to do it as a shift... */
		{
		if (el_size == bipower)
			{
			if (e->ctc.ido_type == IDO_INT)
				po_code_int(pcb, &e->ecd, OP_ICON, i);
			else
				po_code_long(pcb, &e->ecd, OP_LCON, (long)i);
			po_code_op(pcb, &e->ecd, po_lshift_ops[e->ctc.ido_type]);
			goto GOT_SCALING;
			}
		bipower<<=1;
		}
	/* if it's not shiftable force index to be computed with long multiply */
	po_coerce_numeric_exp(pcb, e, IDO_LONG);
	po_code_long(pcb, &e->ecd, OP_LCON, el_size);
	po_code_op(pcb, &e->ecd, OP_LMUL);
GOT_SCALING:
	po_fold_const(pcb, e);
	}
}

static void get_array(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * generate code to reach a given element in an array.
 ****************************************************************************/
{
Type_info *ti;
Exp_frame iex;
long el_size;
int op;
int end_type = e->ctc.comp[e->ctc.comp_count-1];


if (!any_code(pcb, &e->left))
	{
	po_say_fatal(pcb, "bizarre circumstances for array.");
	return;
	}
if (end_type == TYPE_POINTER)
	{
	/* if it's a pointer move the rval to the lval at this point... */
	po_copy_code(pcb, &e->ecd, &e->left);
	}
else if (end_type == TYPE_ARRAY)
	{
	/* do nothing */
	}
else
	{
	po_say_fatal(pcb, "indexing non-pointer");
	return;
	}
ti = &e->ctc;
ti->comp_count -= 1;
po_set_ido_type(ti);
el_size = po_get_type_size(ti);
po_init_expframe(pcb, &iex);
po_get_expression(pcb, &iex);
if (po_force_num_exp(pcb, &iex.ctc) < 0)
	goto TRASHIT;
if (iex.ctc.ido_type != IDO_INT)
	{
	po_coerce_numeric_exp(pcb, &iex, IDO_LONG);
	}
if (!po_eat_rbracket(pcb))
	goto TRASHIT;
po_code_elsize(pcb, &iex, el_size);
po_code_op(pcb, &iex.ecd, po_add_offset_ops[iex.ctc.ido_type]);
/* have computed parts of array expression common to left and right side... */
po_cat_code(pcb, &e->left, &iex.ecd);	/* this is all for left side */
po_cat_code(pcb, &e->ecd, &iex.ecd);   /* Right side still needs a OP_XREF */
if ((op = ref_op(pcb, ti)) < 0)
	goto TRASHIT;
po_code_op(pcb, &e->ecd, op);
e->left_complex = TRUE;
TRASHIT:
po_trash_expframe(pcb,&iex);
}

void po_make_deref(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * generate code to dereference a pointer.
 ****************************************************************************/
{
SHORT op;

//printf("Entry rval:\n");
//po_dump_codebuf(pcb, &e->ecd);
//printf("Entry lval:\n");
//po_dump_codebuf(pcb, &e->left);


e->left_complex = TRUE;
if (e->ctc.ido_type != IDO_VPT)
	{
	po_copy_code(pcb, &e->ecd, &e->left);
	if ((op = ref_op(pcb, &e->ctc)) < 0)
		{
		if (e->ctc.ido_type == IDO_VOID)
			po_say_fatal(pcb, "cannot dereference a void pointer");
		else if (e->ctc.comp[e->ctc.comp_count-1] == TYPE_ARRAY)
				{
				 /* if we got a negative (bad) refop because the current
					end type is TYPE_ARRAY, then we do nothing, because we
					will generate the refop (if even needed) when we see
					the subscript expression or when we unrecurse and
					generate the code for the '*' in front of the name.
				 */
				}
		else
			po_say_fatal(pcb, "confused pointer dereference");
		}
	else
		{
		po_code_op(pcb, &e->ecd, op);
		}
	}

//printf("Exit rval:\n");
//po_dump_codebuf(pcb, &e->ecd);
//printf("Exit lval:\n");
//po_dump_codebuf(pcb, &e->left);
}

static void not_a_member(Poco_cb *pcb, Struct_info *si, char *mbrname)
/*****************************************************************************
 * issue error message about struct/union access, then die.
 ****************************************************************************/
{
char *structname;
char *type;

structname = (si->name == NULL) ? "\0" : si->name;

type = (si->type == TYPE_UNION) ? "union" : "struct";

if (si->size == 0)
	po_say_fatal(pcb, "elements for %s type %s have not been defined", type, structname);
else
	po_say_fatal(pcb, "%s isn't a member of %s %s", mbrname, type, structname);

}


static void get_pmember(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * generate code to access a struct/union member being accessed via pointer.
 ****************************************************************************/
{
Struct_info *si;
Symbol *msym;
int doff;

if (e->ctc.comp[0] != TYPE_STRUCT || e->ctc.comp_count > 2)
	{
	po_say_fatal(pcb, "using -> on something that isn't a struct");
	goto OUT;
	}
if (e->ctc.comp_count == 1)
	{
	po_say_fatal(pcb, "-> where there should be a . perhaps?");
	goto OUT;
	}
si = e->ctc.sdims[0].pt;
lookup_token(pcb);
if ((msym = in_symbol_list(si->elements, pcb->curtoken->ctoke)) == NULL)
	{
	not_a_member(pcb, si, pcb->curtoken->ctoke);
	goto OUT;
	}
po_copy_type(pcb, msym->ti, &e->ctc);	/* update exp type with member type */
if ((doff = msym->symval.doff) != 0)
	{
	po_code_int(pcb, &e->ecd, OP_ICON, doff);
	po_code_op(pcb, &e->ecd, OP_ADD_IOFFSET);
	}
po_make_deref(pcb, e);
e->left_complex = TRUE;
OUT:

	return;
}

static void get_member(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * generate code to access a struct/union member.
 ****************************************************************************/
{
Struct_info *si;
Symbol *vsym;
Symbol *msym;
int *patch;
int doff;
long elsize;	/* for patching adcon in left buffer */
int op;

vsym = e->var;
if (e->ctc.comp[0] != TYPE_STRUCT)
	{
	goto NOTSTRUCT;
	}
if (e->ctc.comp_count != 1)
	{
	if (po_is_pointer(&e->ctc))
		{
		po_say_fatal(pcb, ". where there should be a -> perhaps?");
		goto OUT;
		}
	else if (po_is_array(&e->ctc))
		{
		po_say_fatal(pcb, "need [] before .");
		goto OUT;
		}
	else
		goto NOTSTRUCT;
	}
si = e->ctc.sdims[0].pt;
lookup_token(pcb);
if ((msym = in_symbol_list(si->elements, pcb->curtoken->ctoke)) == NULL)
	{
	not_a_member(pcb, si, pcb->curtoken->ctoke);
	goto OUT;
	}
po_copy_type(pcb, msym->ti, &e->ctc);	/* update exp type with member type */
doff = msym->symval.doff;
e->doff += doff;
if (!e->left_complex)
	{
	elsize	= po_get_type_size(msym->ti)-1;
	patch	= OPTR(e->left.code_pt, -(sizeof(int)+sizeof(long)));
	*patch += doff; 	/* add var offset to OP_XXX_ADDRESS */
	patch	= OPTR(e->left.code_pt, -(sizeof(long)));
	*(long*)patch = elsize;

	if (po_is_array(msym->ti))	/* move left side (OP_XXX_ADDRESS) */
		{						/* to right side.				   */
		e->ecd.code_pt = OPTR(e->ecd.code_pt, -(OPY_SIZE+sizeof(int)));
		po_cat_code(pcb, &e->ecd, &e->left);
		}
	else
		{
		patch = OPTR(e->ecd.code_pt, -(OPY_SIZE+sizeof(int)));
		*patch = find_use_op(pcb, vsym, &e->ctc);	/* update OP_XXX_XVAR */
		patch = OPTR(e->ecd.code_pt, -sizeof(int));
		*patch += doff; 			/* update var offset in OP_XXX_XVAR */
		}
	}
else
	{
	po_backup_code(pcb, &e->ecd, OPY_SIZE); 	/* get rid of ref_op */
	if (doff)
		{
		po_code_int(pcb, &e->left, OP_ICON, doff);
		po_code_op(pcb, &e->left, OP_ADD_IOFFSET);
		po_code_int(pcb, &e->ecd, OP_ICON, doff);
		po_code_op(pcb, &e->ecd, OP_ADD_IOFFSET);
		}
	if ((op = ref_op(pcb, &e->ctc)) < 0)
		goto OUT;
	po_code_op(pcb, &e->ecd, op);
	}
OUT:
	return;
NOTSTRUCT:
	po_say_fatal(pcb, "using . on something that isn't a struct");
	return;
}


int po_scoped_address_op[2] = {OP_GLO_ADDRESS, OP_LOC_ADDRESS};

static void use_var(Poco_cb *pcb, Exp_frame *e, Symbol *var)
/*****************************************************************************
 * generate code to use a variable.
 ****************************************************************************/
{
Type_info *ti = var->ti;
int op;
long size;


po_copy_type(pcb, ti, &e->ctc); /* update expression type with self */
e->doff = var->symval.doff;

var->flags |= SFL_USED;

op = po_scoped_address_op[var->storage_scope];
size = po_get_type_size(ti)-1;

switch (ti->comp[ti->comp_count-1])
	{
	case TYPE_ARRAY:
		{
		/* code left (address) side of expression 1st */
		po_code_address(pcb, &e->left, op, var->symval.doff, size);
		/* code right (value) side. */
		po_code_address(pcb, &e->ecd, op, var->symval.doff, size);
		break;
		}
	case TYPE_FUNCTION:
		{
		/* code right (value) side. */
		po_code_void_pt(pcb, &e->ecd,
			OP_CODE_ADDRESS,ti->sdims[ti->comp_count-1].pt);
		break;
		}
	default:
		{
		/* code left (address) side of expression 1st */
		po_code_address(pcb, &e->left, op, var->symval.doff, size);
		/* code right (value) side. */
		po_code_int(pcb, &e->ecd,
			find_use_op(pcb, var, &e->ctc), var->symval.doff);
		break;
		}
	}
}


#ifdef SLUFFED
static print_var_sym(Poco_cb *pcb, FILE *f, Symbol *var)
/*****************************************************************************
 *
 ****************************************************************************/
/* Print out name and full type of a variable */
{
Type_info *ti;

if (var->tok_type == PTOK_FUNC)
	fprintf(f, "%s() ", var->name);
if (var->tok_type == PTOK_CFUNC)
	fprintf(f, "C %s() ", var->name);
else
	fprintf(f, "%s: %d\n", var->name, var->tok_type);
if ((ti = var->ti) != NULL)
	po_print_type(pcb, f, ti);
if (var->tok_type != PTOK_FUNC & var->tok_type != PTOK_CFUNC)
	fprintf(f, "  sizeof() = %ld\n", po_get_type_size(var->ti));
}
#endif /* SLUFFED */


void po_new_var_space(Poco_cb *pcb, Symbol *var)
/*****************************************************************************
 * allocate space for a variable on appropriate poco_frame.
 *	 if variable was declared with the 'static' keyword, storage is
 *	 allocated on the global frame, otherwise storage is allocated on
 *	 the current frame.
 ****************************************************************************/
{
Poco_frame *rf	= pcb->rframe;
long	   size = po_get_type_size(var->ti);

if (var->ti->flags & TFL_STATIC)
	{
	while (rf->frame_type != FTY_GLOBAL)
		rf = rf->next;
	var->storage_scope = SCOPE_GLOBAL;
	}
else
	{
	var->storage_scope = (var->scope == SCOPE_GLOBAL) ?
							SCOPE_GLOBAL : SCOPE_LOCAL;
	}

var->symval.doff = (rf->doff -= size);
}

int po_get_temp_space(Poco_cb *pcb, int space)
/*****************************************************************************
 * allocate some local temp space on current poco_frame.
 ****************************************************************************/
{
return(pcb->rframe->doff -= space);
}

/******* MODULE EXPRESSION ************/
/* Here we've got the really recursive part of the recursive descent
   parser - the expression evaluatior */

void po_init_expframe(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * init a pre-allocated expression frame.
 ****************************************************************************/
{
poco_zero_bytes(e,sizeof(*e));
e->pure_const = TRUE;
po_init_code_buf(pcb, &e->ecd); 	/* The right value of expression */
po_init_code_buf(pcb, &e->left);	/* The left value of expression */
e->ctc.comp = e->ctc_comp;			/* e->ctc - the expression type */
e->ctc.sdims = e->ctc_dims;
e->ctc.comp_alloc = MAX_TYPE_COMPS;
}

Exp_frame *po_new_expframe(Poco_cb *pcb)
/*****************************************************************************
 * allocate and init a new expression frame.
 ****************************************************************************/
{
Exp_frame		*new;

new = po_cache_malloc(pcb, &pcb->expf_cache);
po_init_expframe(pcb, new);
return new;
}

void po_trash_expframe(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * free the code buffers from an expression frame.
 ****************************************************************************/
{
po_trash_code_buf(pcb, &e->ecd);
po_trash_code_buf(pcb, &e->left);
}

void po_dispose_expframe(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * free an expression frame allocated earlier.
 ****************************************************************************/
{
po_trash_expframe(pcb, e);
po_freemem(e);
}

SHORT po_force_num_exp(Poco_cb *pcb, Type_info *ti)
/*****************************************************************************
 * ensure an expression is floating point or int, if not, complain and die.
 ****************************************************************************/
{
SHORT nt;

nt = ti->ido_type;
if (!po_ido_table[nt].is_num)
	{
	po_expecting_got(pcb, "numeric expression");
	return(-1);
	}
return(nt);
}

SHORT po_force_int_exp(Poco_cb *pcb, Type_info *ti)
/*****************************************************************************
 * ensure an expression is int, if not, complain and die.
 ****************************************************************************/
{
SHORT nt;

nt = ti->ido_type;
if (!po_is_int_ido(nt))
	{
	po_expecting_got(pcb, "integer expression");
	return(-1);
	}
return(nt);
}

SHORT po_force_ptr_or_num_exp(Poco_cb *pcb, Type_info *ti)
/*****************************************************************************
 * ensure an expression is either numeric or pointer, if not, complain and die.
 ****************************************************************************/
{
SHORT nt;

nt = ti->ido_type;
if (!po_ido_table[nt].can_add)
	po_expecting_got(pcb, "numeric expression or data pointer");
return(nt);
}


static void upgrade_numerical_expression(Poco_cb *pcb
, Exp_frame *e, SHORT ido_type)
/*****************************************************************************
 * make a cast to upgrade an expression to a given type.
 ****************************************************************************/
{
SHORT eido_type;

eido_type = e->ctc.ido_type;
if (ido_type != eido_type)
	{
	switch (ido_type)
		{
		case IDO_CPT:
		case IDO_VPT:
		case IDO_POINTER:
			{
			switch (eido_type)
				{
				case IDO_POINTER:
					break;
				default:
					po_say_fatal(pcb,"cannot do pointer<->number conversion");
					break;
				}
			}
			break;
		case IDO_INT:
			switch (eido_type)
				{
				case IDO_LONG:
					po_code_op(pcb, &e->ecd, OP_LONG_TO_INT);
					goto upgrade_type;
				case IDO_DOUBLE:
					po_code_op(pcb, &e->ecd, OP_DOUBLE_TO_INT);
					goto upgrade_type;
				}
			break;
		case IDO_LONG:
			switch (eido_type)
				{
				case IDO_INT:
					po_code_op(pcb, &e->ecd, OP_INT_TO_LONG);
					goto upgrade_type;
				case IDO_DOUBLE:
					po_code_op(pcb, &e->ecd, OP_DOUBLE_TO_LONG);
					goto upgrade_type;
				}
			break;
		case IDO_DOUBLE:
			switch (eido_type)
				{
				case IDO_INT:
					po_code_op(pcb, &e->ecd, OP_INT_TO_DOUBLE);
					goto upgrade_type;
				case IDO_LONG:
					po_code_op(pcb, &e->ecd, OP_LONG_TO_DOUBLE);
					goto upgrade_type;
				}
			break;
		}
	}
return;
upgrade_type:
e->ctc.comp[0] = inv_ido[ido_type];
e->ctc.ido_type = ido_type;
po_fold_const(pcb, e);
}

void po_coerce_to_boolean(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * make a cast to reduce an expression to a boolean (int) type.
 ****************************************************************************/
{
if (e->ctc.ido_type != IDO_INT)
	{
	switch (e->ctc.ido_type)
		{
		case IDO_LONG:
		case IDO_CPT:
		case IDO_VPT:
			po_code_op(pcb, &e->ecd, OP_LONG_TO_INT);
			break;
		case IDO_DOUBLE:
			po_code_op(pcb, &e->ecd, OP_DOUBLE_TO_INT);
			break;
		case IDO_POINTER:
			po_code_op(pcb, &e->ecd, OP_PPT_TO_CPT);
			po_code_op(pcb, &e->ecd, OP_LONG_TO_INT);
			break;
		default:
			po_say_fatal(pcb, "expecting boolean expression");
			return;
		}
	e->ctc.comp[0] = TYPE_INT;
	e->ctc.comp_count = 1;
	e->ctc.ido_type = IDO_INT;
	}
}

#ifdef STRING_EXPERIMENT
void po_coerce_to_string(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * make a cast to promote an expression to a String type.
 ****************************************************************************/
{
static TypeComp st = TYPE_STRING;
static Type_info string_type_info = {&st,NULL,1,1,IDO_STRING,0};
po_coerce_expression(pcb,e,&string_type_info,TRUE);
}
#endif /* STRING_EXPERIMENT */

void po_coerce_numeric_exp(Poco_cb *pcb, Exp_frame *e, SHORT ido_type)
/*****************************************************************************
 * make cast of expression to given type, if bad starting type, complain & die.
 ****************************************************************************/
{
if (!po_ido_table[e->ctc.ido_type].is_num)
	{
	po_expecting_got(pcb, "numeric expression");
	return;
	}
upgrade_numerical_expression(pcb,e,ido_type);
}

#ifdef STRING_EXPERIMENT
static void cant_convert_to_String(Poco_cb *pcb)
/*****************************************************************************
 * issue error message that string must be a char *, char [], or
 * another string.
 ****************************************************************************/
{
po_say_fatal(pcb, "expression can't be converted to String type");
}
#endif /* STRING_EXPERIMENT */


void po_coerce_expression(Poco_cb *pcb, Exp_frame *e, Type_info *ti, Boolean recast)
/*****************************************************************************
 * make a cast of an expression to a given type.
 ****************************************************************************/
{
TypeComp *pstart_type, start_type;
TypeComp end_type, *pend_type;
int start_count, end_count;

end_count = ti->comp_count;
pend_type = ti->comp + end_count-1;
end_type = *pend_type;
start_count = e->ctc.comp_count;
pstart_type = e->ctc.comp + start_count-1;
start_type = *pstart_type;


if (end_count == 1)
	{
#ifdef STRING_EXPERIMENT
	if (end_type == TYPE_STRING)
	/* The only thing that converts to a String is another String,
	 * a char *, or a char [] */
		{
		if (start_count == 1)
			{
			if (start_type != TYPE_STRING)
				cant_convert_to_String(pcb);
			/* else will end up returning happily - both are strings! */
			}
		else if (start_count == 2)
			{
			if (e->ctc.comp[0] == TYPE_CHAR)
				{
				switch (start_type)
					{
					case TYPE_POINTER:
					case TYPE_ARRAY:
						po_code_op(pcb, &e->ecd, OP_PPT_TO_STRING);
						e->ctc.comp[0] = TYPE_STRING;
						e->ctc.comp_count = 1;
						e->ctc.ido_type = IDO_STRING;
						break;
					case TYPE_CPT:
						po_code_op(pcb, &e->ecd, OP_CPT_TO_STRING);
						e->ctc.comp[0] = TYPE_STRING;
						e->ctc.comp_count = 1;
						e->ctc.ido_type = IDO_STRING;
						break;
					default:
						cant_convert_to_String(pcb);
						break;
					}
				}
			else
				cant_convert_to_String(pcb);
			}
		else
			cant_convert_to_String(pcb);
		}
	else
#endif /* STRING_EXPERIMENT */
	/* Then (hopefully) it's a numerical type of some sort */
		{
		if (recast && start_count != 1)
			po_say_fatal(pcb
			,"cannot recast pointer expression to numeric type");
		po_coerce_numeric_exp(pcb, e, ti->ido_type);
		}
	}
else
	{
	switch (end_type)
		{
		case TYPE_ARRAY:
		case TYPE_POINTER:
			switch (start_type)
				{
				case TYPE_POINTER:
					break;
				case TYPE_CPT:
					po_code_op(pcb, &e->ecd, OP_CPT_TO_PPT);
					start_type = TYPE_POINTER;
					e->ctc.ido_type = IDO_POINTER;
					break;
				case TYPE_ARRAY:
					start_type = TYPE_POINTER;
					e->ctc.ido_type = IDO_POINTER;
					break;
				case TYPE_FUNCTION:
					start_type = TYPE_POINTER;
					pstart_type += 1;
					e->ctc.comp_count  += 1;
					e->ctc.ido_type = IDO_POINTER;
					break;
#ifdef STRING_EXPERIMENT
				case TYPE_STRING:
					po_code_op(pcb, &e->ecd, OP_STRING_TO_PPT);
					*pstart_type++ = TYPE_CHAR; /* Convert type to (char *) */
					e->ctc.comp_count  += 1;
					start_type = TYPE_POINTER;
					e->ctc.ido_type = IDO_POINTER;
					break;
#endif /* STRING_EXPERIMENT */
				default:
					goto WANT_POINTER;
				}
			break;
		case TYPE_CPT:
			switch (start_type)
				{
				case TYPE_CPT:
					break;
				case TYPE_POINTER:
				case TYPE_ARRAY:
					po_code_op(pcb, &e->ecd, OP_PPT_TO_CPT);
					start_type = TYPE_CPT;	/* update expression type */
					e->ctc.ido_type = IDO_CPT;
					break;
				case TYPE_FUNCTION:
					po_code_op(pcb, &e->ecd, OP_PPT_TO_CPT);
					start_type = TYPE_CPT;
					pstart_type += 1;
					e->ctc.comp_count  += 1;
					e->ctc.ido_type = IDO_CPT;
					break;
#ifdef STRING_EXPERIMENT
				case TYPE_STRING:
					/* Here were're almost certainly a parameter to a
					 * C function. */
					po_code_op(pcb, &e->ecd, OP_STRING_TO_CPT);
					*pstart_type++ = TYPE_CHAR; /* Convert type to (char *) */
					e->ctc.comp_count  += 1;
					start_type = TYPE_CPT;
					e->ctc.ido_type = IDO_CPT;
					break;
#endif /* STRING_EXPERIMENT */
				default:
					goto WANT_POINTER;
				}
			break;
		default:
			goto WANT_POINTER;
		}
	*pstart_type = start_type;
	if (recast)
		{
		po_copy_type(pcb, ti, &e->ctc);
		}
	else if (!(ti->comp[0] == TYPE_VOID || e->ctc.comp[0] == TYPE_VOID))
		{
		if (!po_types_same(ti, &e->ctc, 0))
			{
#ifdef DEBUG
			fprintf(pcb->t.err_file, "s is ");
			po_print_type(pcb, pcb->t.err_file, &e->ctc);
			fprintf(pcb->t.err_file, "\nd is ");
			po_print_type(pcb, pcb->t.err_file, ti);
			fprintf(pcb->t.err_file, "\n");
#endif
			po_say_fatal(pcb, "type mismatch in pointer evaluation");
			}
		}
	}
return;
WANT_POINTER:
	{
	if (recast)
		po_say_fatal(pcb,"cannot recast numeric expression to pointer type");
	else
		po_expecting_got(pcb, "pointer expression");
	}
}


void po_get_prim(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * get primitive - a number, variable, or expression in parenthesis
 ****************************************************************************/
{
switch (pcb->t.toktype)
	{
	case TOK_LPAREN:
		{
		po_get_expression(pcb, e);
		lookup_token(pcb);
		if (pcb->t.toktype != TOK_RPAREN)
			{
			po_say_fatal(pcb, "missing right parenthesis");
			}
		break;
		}
	case TOK_INT:
		{
		po_code_int(pcb, &e->ecd, OP_ICON, pcb->curtoken->val.num );
		po_set_base_type(pcb, &e->ctc, TYPE_INT, 0, NULL);
		clear_code_buf(pcb, &e->left);
		break;
		}
	case TOK_LONG:
		{
		po_code_long(pcb, &e->ecd, OP_LCON, pcb->curtoken->val.num );
		po_set_base_type(pcb, &e->ctc, TYPE_LONG, 0, NULL);
		clear_code_buf(pcb, &e->left);
		break;
		}
	case TOK_DOUBLE:
		{
		po_code_double(pcb, &e->ecd, OP_DCON, pcb->curtoken->val.dnum);
		po_set_base_type(pcb, &e->ctc, TYPE_DOUBLE, 0, NULL);
		clear_code_buf(pcb, &e->left);
		break;
		}
	case PTOK_QUO:
		{
		po_code_popot(pcb, &e->ecd, OP_PCON,
			pcb->curtoken->val.string,
			pcb->curtoken->val.string+strlen(pcb->curtoken->val.string),
			pcb->curtoken->val.string);
		po_set_base_type(pcb, &e->ctc, TYPE_CHAR, 0, NULL);
		po_append_type(pcb, &e->ctc, TYPE_POINTER, 0, NULL);
		clear_code_buf(pcb, &e->left);
		break;
		}
	case PTOK_NULL:
		{
		po_code_popot(pcb, &e->ecd, OP_PCON, (void *)1, NULL, NULL);
		po_set_base_type(pcb, &e->ctc, TYPE_VOID, 0, NULL);
		po_append_type(pcb, &e->ctc, TYPE_POINTER, 0, NULL);
		clear_code_buf(pcb, &e->left);
		break;
		}
	case PTOK_VAR:
		{
		use_var(pcb, e, e->var = pcb->curtoken->val.symbol);
		e->pure_const = FALSE;
		break;
		}
	case PTOK_UNDEF:
		{
		po_undefined(pcb, pcb->curtoken->val.symbol->name);
		break;
		}
	default:
		{
		po_expecting_got(pcb, "a number, variable, or string");
		break;
		}
	}
}

static void get_prec1(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * do precedence-1 right end operators. ('->'  '['  ']'  '.')
 ****************************************************************************/
{
SHORT ttype;

po_get_prim(pcb,e);
for (;;)
	{
	lookup_token(pcb);
	ttype = pcb->t.toktype;
	switch (ttype)
		{
		case '[':
			get_array(pcb,e);
			break;
		case '(':
			po_get_function(pcb,e);
			break;
		case '.':
			get_member(pcb,e);
			break;
		case TOK_ARROW:
			get_pmember(pcb,e);
			break;
		default:
			pushback_token(&pcb->t);
			return;
		}
	po_fold_const(pcb, e);
	}
}

static void use_sizeof(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * code a sizeof(something) value.
 ****************************************************************************/
{
extern dcl();

Exp_frame	vexp;
long		size = 0;

po_init_expframe(pcb,&vexp);

if (!po_is_next_token(pcb, TOK_LPAREN))
	{
	po_get_unop_expression(pcb, &vexp);  /* no parens, we have sizeof unary_expression */
	}
else
	{
	switch (lookahead_type(pcb))	/* lookahead to token after paren */
		{
		case PTOK_TYPE:
		case PTOK_USER_TYPE:

			po_get_typename(pcb, &vexp.ctc);
			break;

		default:

			po_get_unop_expression(pcb, &vexp);
			break;
		}
	}

size = po_get_type_size(&vexp.ctc);

po_code_long(pcb, &e->ecd, OP_LCON, size );
po_set_base_type(pcb, &e->ctc, TYPE_LONG, 0, NULL);

clear_code_buf(pcb, &e->left);
po_trash_expframe(pcb,&vexp);
}


static void get_sizeof(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * decide whether a sizeof() is being handled.
 ****************************************************************************/
{
if (pcb->t.toktype == PTOK_SIZEOF)
	{
	use_sizeof(pcb,e);
	}
else
	{
	get_prec1(pcb,e);
	}
}

static void get_cast(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * handle a recast.
 ****************************************************************************/
{
Itypi		tip;
Type_info	*ti;

	pushback_token(&pcb->t);
	ti = po_typi_type(&tip);
	po_get_typename(pcb, ti);
	po_get_unop_expression(pcb, e);
	po_coerce_expression(pcb, e, ti, TRUE);
	return;

}


static void code_one(Poco_cb *pcb, Code_buf *cb, Type_info *ti)
/*****************************************************************************
 * code a constant value of 1 in a given type.
 ****************************************************************************/
{
switch (ti->ido_type)
	{
	case IDO_INT:
		po_code_int(pcb, cb, OP_ICON, 1);
		break;
	case IDO_LONG:
		po_code_long(pcb, cb, OP_LCON, 1L);
		break;
	case IDO_DOUBLE:
		po_code_double(pcb, cb, OP_DCON, (double)1.0);
		break;
	case IDO_POINTER:
		po_code_int(pcb, cb, OP_ICON, (int)po_get_subtype_size(pcb,ti));
		break;
	}
}

static void get_post_increment(Poco_cb *pcb, Exp_frame *e,
								Op_type op_group[NUM_IDOS])
/*****************************************************************************
 * code a post-increment or post-decrement, depending of value of op_group.
 ****************************************************************************/
{
Symbol *v;
SHORT ido_type;

if (any_code(pcb, &e->left))
	{
	v = e->var;
	if ((ido_type = po_force_ptr_or_num_exp(pcb, &e->ctc)) >= 0)
		{
		po_code_op(pcb, &e->ecd, po_dupe_ops[ido_type]);
		code_one(pcb, &e->ecd, &e->ctc);
		po_code_op(pcb, &e->ecd, op_group[ido_type]);
		assign_after_value(pcb, e, v);
		po_code_op(pcb, &e->ecd, po_find_clean_op(pcb, &e->ctc));
		/* clear_code_buf(pcb, &e->left); */
		}
	}
else
	{
	po_say_fatal(pcb,"trying to increment a non-variable");
	}
}

static void get_pre_increment(Poco_cb *pcb, Exp_frame *e,
								Op_type op_group[NUM_IDOS])
/*****************************************************************************
 * code a pre-increment or pre-decrement, depending on value of op_group.
 ****************************************************************************/
{
Symbol *v;
SHORT ido_type;

po_get_unop_expression(pcb, e);
if (any_code(pcb, &e->left))
	{
	v = e->var;
	if ((ido_type = po_force_ptr_or_num_exp(pcb, &e->ctc)) >= 0)
		{
		code_one(pcb, &e->ecd, &e->ctc);
		po_code_op(pcb, &e->ecd, op_group[ido_type]);
		assign_after_value(pcb, e, v);
		}
	}
else
	{
	po_say_fatal(pcb,"trying to increment a non-variable");
	}
}

static void get_dereference(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * check for a dereference, call the coder routine if so.
 ****************************************************************************/
{
	if (any_code(pcb, &e->left))
		{
		if (po_is_pointer(&e->ctc) || po_is_array(&e->ctc))
			{
			e->ctc.comp_count -= 1;
			po_set_ido_type(&e->ctc);
			po_make_deref(pcb,e);
			return;
			}
		}

	po_say_fatal(pcb, " '*' on non-pointer expression");

}

static void get_address(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * code an address-of op.
 ****************************************************************************/
{
Exp_frame ex;

po_init_expframe(pcb, &ex);
po_get_unop_expression(pcb,&ex);
if (!any_code(pcb, &ex.left))
	{
	po_say_fatal(pcb, "trying to take address of non-variable");
	goto OUT;
	}
po_copy_type(pcb, &ex.ctc, &e->ctc);
po_append_type(pcb, &e->ctc, TYPE_POINTER, 0, NULL);
po_cat_code(pcb, &e->ecd, &ex.left);
e->pure_const &= ex.pure_const;
clear_code_buf(pcb, &e->left);
OUT:
po_trash_expframe(pcb, &ex);
}


void po_get_unop_expression(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * get signed primitive - a primitive with optional preceeding unary op.
 ****************************************************************************/
{
TypeComp	   t1;
Op_type 		*op_group;
int 			fok = FALSE;	/* float ok? */
int 			temp;
register SHORT	ttype;
SHORT			ntype;

if (!po_need_token(pcb))
	return;

ttype = pcb->t.toktype;
ntype = lookahead_type(pcb);

if (ttype == '-')
	{
	po_get_unop_expression(pcb,e);
	op_group = po_neg_ops;
	fok = TRUE;
	}
else if (ttype == '!')
	{
	po_get_unop_expression(pcb,e);
	op_group = po_not_ops;
	}
else if (ttype == '~')
	{
	po_get_unop_expression(pcb,e);
	op_group = po_comp_ops;
	}
else if (ttype == '+')
	{
	po_get_unop_expression(pcb,e);
	op_group = NULL;
	}
else if (ttype == '*')
	{
	po_get_unop_expression(pcb,e);
	get_dereference(pcb,e);
	return;
	}
else if (ttype == '&')
	{
	get_address(pcb,e);
	return;
	}
else if (ttype == TOK_PLUS_PLUS)
	{
	get_pre_increment(pcb,e, po_add_ops);
	return;
	}
else if (ttype == TOK_MINUS_MINUS)
	{
	get_pre_increment(pcb,e, po_sub_ops);
	return;
	}
else if (ttype == TOK_LPAREN && (ntype == PTOK_TYPE || ntype == PTOK_USER_TYPE))
	{
	get_cast(pcb, e);
	return;
	}
else
	{
	get_sizeof(pcb, e);
	if (po_need_token(pcb))
		{
		if (pcb->t.toktype == TOK_PLUS_PLUS)
			get_post_increment(pcb,e,po_add_ops);
		else if (pcb->t.toktype == TOK_MINUS_MINUS)
			get_post_increment(pcb,e,po_sub_ops);
		else
			pushback_token(&pcb->t);
		}
	return;
	}

t1 = temp = po_force_num_exp(pcb, &e->ctc);
if (temp < 0)
	return;
if (op_group != NULL)
	{
	if ((t1 == IDO_DOUBLE) && !fok)
		{
		po_say_fatal(pcb,
			"~ and ! operators can't be used with floats or doubles");
		return;
		}
	po_code_op(pcb, &e->ecd, op_group[t1]);
	po_fold_const(pcb, e);
	}
clear_code_buf(pcb, &e->left);
}

static void assign_after_value(Poco_cb *pcb, Exp_frame *e, Symbol *var)
/*****************************************************************************
 * code an assignment of an expression value.
 ****************************************************************************/
{
if (e->left_complex)
	{
	po_cat_code(pcb, &e->ecd, &e->left);
	po_code_op(pcb, &e->ecd, ind_op(pcb,&e->ctc) );
	}
else
	{
	po_code_int(pcb, &e->ecd, po_find_assign_op(pcb,var,&e->ctc), e->doff);
	}
e->includes_assignment++;
}

static void make_assign(Poco_cb *pcb,
	Exp_frame *e, Exp_frame *val_exp, Symbol *var)
/*****************************************************************************
 * drive process of coding an assignment.
 *****************************************************************************/
{
TypeComp obase = e->ctc.comp[0];

po_coerce_expression(pcb, val_exp, &e->ctc, FALSE);
po_cat_code(pcb, &e->ecd, &val_exp->ecd);
e->ctc.comp[0] = obase;
assign_after_value(pcb,e,var);
clear_code_buf(pcb, &e->left);
e->includes_assignment += val_exp->includes_assignment;
e->includes_function += val_exp->includes_function;
e->pure_const &= val_exp->pure_const;
}

Boolean po_assign_after_equals(Poco_cb *pcb, Exp_frame *e, Symbol *var,
	Boolean must_be_init_constant)
/*****************************************************************************
 * drive process of getting an expression after '=' and coding the assign.
 *
 *	if this is called from the variable init parser, and the variable's
 *	storage class is static, then the expression must be a constant.  we
 *	have to check for const-ness before doing the make_assign(), since it
 *	adds code to the buffer which makes the expression look non-constant.
 *	if the pure_const flag in the expression is true, we're in fine shape,
 *	but if it's false, it could be because of taking the address of a static
 *	data item.	taking such an address is constant in terms of init
 *	expressions, but not in terms of constant-folding, so we have a special
 *	routine (in fold.c) for checking a !pure_const expression to see if it
 *	qualifies as constant for an init expression.
 ****************************************************************************/
{
Exp_frame val_eee;

po_init_expframe(pcb, &val_eee);
po_get_expression(pcb, &val_eee);

if (must_be_init_constant) {
	if (!val_eee.pure_const) {
		if (!po_is_static_init_const(pcb, &val_eee.ecd)) {
			po_say_fatal(pcb, "constant expression required for static initializer");
		}
	}
}

make_assign(pcb, e, &val_eee, var);
po_trash_expframe(pcb, &val_eee);
return(TRUE);
}

static void plus_equals(Poco_cb *pcb, Exp_frame *e, Symbol *var,
	Op_type op_group[NUM_IDOS],
	SHORT (*enforcer)(Poco_cb *pcb, Type_info *ti))
/*****************************************************************************
 * code a '+=' type op (eg, *= <<=, etc).
 ****************************************************************************/
{
Exp_frame val_eee;
int is_pt = po_is_pointer(&e->ctc);

po_init_expframe(pcb, &val_eee);
po_get_expression(pcb, &val_eee);
if ((*enforcer)(pcb, &e->ctc) < 0)
	goto TRASH;
if (is_pt)
	{
	po_coerce_numeric_exp(pcb, &val_eee, IDO_INT);
	po_code_elsize(pcb, &val_eee, po_get_subtype_size(pcb, &e->ctc) );
	if (val_eee.ctc.ido_type != IDO_INT)
		{
		po_coerce_numeric_exp(pcb, &val_eee, IDO_INT);
		po_fold_const(pcb, e);
		}
	po_code_op(pcb, &val_eee.ecd, op_group[e->ctc.ido_type]);
	po_cat_code(pcb,&e->ecd,&val_eee.ecd);
	assign_after_value(pcb, e, var);
	}
else
	{
	po_coerce_numeric_exp(pcb, &val_eee, e->ctc.ido_type);
	po_code_op(pcb, &val_eee.ecd, op_group[e->ctc.ido_type]);
	make_assign(pcb, e, &val_eee, var);
	}
TRASH:
po_trash_expframe(pcb, &val_eee);
}


void po_get_expression(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * drive the process of expression parsing and code generation.
 ****************************************************************************/
{
Symbol *var;

po_get_binop_expression(pcb, e);
if (any_code(pcb, &e->left))
	{
	var = e->var;
	lookup_token(pcb);
	switch (pcb->t.toktype)
		{
		case TOK_PLUS_EQUALS:
			plus_equals(pcb, e, var, po_add_ops, po_force_ptr_or_num_exp);
			break;
		case TOK_MINUS_EQUALS:
			plus_equals(pcb, e, var, po_sub_ops, po_force_ptr_or_num_exp);
			break;
		case TOK_DIV_EQUALS:
			plus_equals(pcb, e, var, po_div_ops, po_force_num_exp);
			break;
		case TOK_MUL_EQUALS:
			plus_equals(pcb, e, var, po_mul_ops, po_force_num_exp);
			break;
		case TOK_MOD_EQUALS:
			plus_equals(pcb, e, var, po_mod_ops, po_force_int_exp);
			break;
		case TOK_LSHIFT_EQUALS:
			plus_equals(pcb, e, var, po_lshift_ops,po_force_int_exp);
			break;
		case TOK_RSHIFT_EQUALS:
			plus_equals(pcb, e, var, po_rshift_ops,po_force_int_exp);
			break;
		case TOK_AND_EQUALS:
			plus_equals(pcb, e, var, po_band_ops, po_force_int_exp);
			break;
		case TOK_OR_EQUALS:
			plus_equals(pcb, e, var, po_bor_ops, po_force_int_exp);
			break;
		case TOK_XOR_EQUALS:
			plus_equals(pcb, e, var, po_xor_ops, po_force_int_exp);
			break;
		case '=':
			/* oops, didn't mean to code that... */
			clear_code_buf(pcb, &e->ecd);
			po_assign_after_equals(pcb, e, var, FALSE);
			break;
		default:
			pushback_token(&pcb->t);
			break;
		}
	}
}


/**************** MODULE COMPILE **********************************/


Boolean po_new_frame(Poco_cb *pcb, int scope, char *name, int type)
/*****************************************************************************
 * alloc & init a poco_frame (and the structures that attach to it, if needed).
 *
 * a poco_frame is a place where code, symbols, and struct_info's live.
 * the poco_frame is used during compilation, but is reduced to a function
 * frame or struct_info once the appropriate point has been reached in the
 * parsing.  (When the prototype, function block, or struct definition has
 * been parsed.)
 *
 * FTY_STRUCT type frames don't get code buffer and line_data structures,
 * since these don't mean anything in the context of parsing a prototype
 * or structure definition (this also lightens up on malloc() a bit).
 ****************************************************************************/
{
Poco_frame *pf;

	pf = po_cache_malloc(pcb, &pcb->pocf_cache);

	poco_zero_bytes(pf, sizeof(Poco_frame) + (HASH_SIZE * sizeof(Symbol *)));

	if (type != FTY_STRUCT) /* no line data or code buf for struct/proto */
		{
		pf->ld = po_new_line_data(pcb);
		po_init_code_buf(pcb,&pf->fcd);
		}

	pf->hash_table = (Symbol **)(pf+1);
	pf->name = name;
	pf->next = pcb->rframe;
	pcb->rframe = pf;
	pf->scope = scope;
	pf->frame_type = type;

	return(TRUE);

}

void po_old_frame(Poco_cb *pcb)
/*****************************************************************************
 * return to parent frame, free child frame and any structures attached to it.
 ****************************************************************************/
{
struct poco_frame *rf;

if ((rf = pcb->rframe) != NULL)
	{
	pcb->rframe = rf->next;

	if (rf->symbols)
		po_free_symbol_list(&rf->symbols);
	if (rf->fsif)
		po_free_sif_list(&rf->fsif);

	if (rf->frame_type != FTY_STRUCT)
		{
		po_trash_code_buf(pcb,&rf->fcd);
		po_free_line_data(rf->ld);
		}
	po_freemem(rf);
	}
}

static Boolean init_reserved_words(Poco_cb *pcb)
/*****************************************************************************
 * Make up symbolic tokens for reserved words, add symbols to root poco_frame.
 ****************************************************************************/
{
int i;
Symbol *n;
Poco_frame *rf = pcb->rframe;

static struct rwinit {char *string; SHORT type; SHORT val;} rwi[] = {
	{"void",        PTOK_TYPE,      TYPE_VOID,},
	{"char",        PTOK_TYPE,      TYPE_CHAR,},
	{"short",       PTOK_TYPE,      TYPE_SHORT,},
	{"int",         PTOK_TYPE,      TYPE_INT,},
	{"long",        PTOK_TYPE,      TYPE_LONG,},
	{"float",       PTOK_TYPE,      TYPE_FLOAT,},
	{"double",      PTOK_TYPE,      TYPE_DOUBLE},
	{"signed",      PTOK_TYPE,      TYPE_SIGNED},
	{"unsigned",    PTOK_TYPE,      TYPE_UNSIGNED},
	{"Screen",      PTOK_TYPE,      TYPE_SCREEN},
#ifdef STRING_EXPERIMENT
	{"String",      PTOK_TYPE,      TYPE_STRING},
#endif /* STRING_EXPERIMENT */
	{"ErrCode",     PTOK_TYPE,      TYPE_INT},
	{"Errcode",     PTOK_TYPE,      TYPE_INT},
	{"Boolean",     PTOK_TYPE,      TYPE_CHAR},
	{"FILE",        PTOK_TYPE,      TYPE_FILE},
	{"struct",      PTOK_TYPE,      TYPE_STRUCT},
	{"union",       PTOK_TYPE,      TYPE_UNION},
	{"enum",        PTOK_TYPE,      TYPE_ENUM},
	{"const",       PTOK_TYPE,      TYPE_CONST},
	{"volatile",    PTOK_TYPE,      TYPE_VOLATILE},
	{"extern",      PTOK_TYPE,      TYPE_EXTERN},
	{"static",      PTOK_TYPE,      TYPE_STATIC},
	{"auto",        PTOK_TYPE,      TYPE_AUTO},
	{"register",    PTOK_TYPE,      TYPE_REGISTER},
	{"typedef",     PTOK_TYPEDEF,   0},
	{"...",         PTOK_ELLIPSIS,  TYPE_ELLIPSIS,},
	{"for",         PTOK_FOR,       0},
	{"if",          PTOK_IF,        0},
	{"while",       PTOK_WHILE,     0},
	{"return",      PTOK_RETURN,    0},
	{"switch",      PTOK_SWITCH,    0},
	{"goto",        PTOK_GOTO,      0},
	{"do",          PTOK_DO,        0},
	{"else",        PTOK_ELSE,      0},
	{"break",       PTOK_BREAK,     0},
	{"continue",    PTOK_CONTINUE,  0},
	{"sizeof",      PTOK_SIZEOF,    0},
	{"NULL",        PTOK_NULL,      0},
	{"case",        PTOK_CASE,      0},
	{"default",     PTOK_DEFAULT,   0},
	};

for (i=0; i<Array_els(rwi); i++)
	{
	if ((n = new_symbol(pcb, rwi[i].string, rwi[i].type)) == NULL)
		return(Err_no_memory);
	n->symval.i = rwi[i].val;
	n->link = rf->parameters;
	rf->parameters = n;
	}
return(Success);
}

static void free_cframes(Poco_cb *pcb, C_frame **cframes)
/*****************************************************************************
 * free a list of c_frames, and the list of symbols attached to each.
 * (Note to self: Does this routine do anything?  A C_frame is typedef'd as
 * a fuf in poco.h, look into this.)
 * -jk - looks dead to me.	I believe pcb->cframes is always NULL now....
 * This is probably a relic from before library functions had ascii
 * prototypes.
 ****************************************************************************/
{
C_frame *next, *cf;

cf = *cframes;
while (cf != NULL)
	{
	next = cf->next;
	po_free_symbol_list(&cf->parameters);
	po_freemem(cf);
	cf = next;
	}
*cframes = NULL;
}

Boolean po_check_undefined_funcs(Poco_cb *pcb, Symbol *sl)
/*****************************************************************************
 * walk symbol list of root frame, look for referenced functions with no code.
 ****************************************************************************/
{
Type_info *ti;
Func_frame *fuf;
Boolean ok = TRUE;

while (sl != NULL)
	{
	if (sl->flags & SFL_USED)
		{
		if (po_is_func(sl->ti))
			{
			ti = sl->ti;
			fuf = ti->sdims[ti->comp_count-1].pt;
			if (!fuf->got_code)
				{
				po_say_fatal(pcb, "function '%s' not found in source code or builtin library", sl->name);
				}
			}
		}
	sl = sl->link;
	}
return(ok);
}


Boolean po_compile_file(Poco_cb *pcb, char *name)
/*****************************************************************************
 * compile pcb->file into pcb->run.fff.
 ****************************************************************************/
{
Tstack		dummy_token;
Func_frame	*fuf = NULL;
Poco_frame	*pf = NULL;

#ifdef DEVELOPMENT
  if (!po_check_instr_table(pcb))
	  {
	  po_say_internal(pcb, "instruction table failed self-check\n");
	  }
  if (!po_check_type_names(pcb))
	  {
	  po_say_internal(pcb, "type_names table failed self-check\n");
	  }
  if (!po_check_ido_table(pcb))
	  {
	  po_say_internal(pcb, "ido_table table failed self-check\n");
	  }
#endif

po_init_qbop_table(pcb);

po_init_pp(pcb, name);

if (po_new_frame(pcb, SCOPE_GLOBAL, name, FTY_GLOBAL))
	{
	pf = pcb->rframe;
	if (init_reserved_words(pcb) < Success)
		goto BADOUT;

	memset(&dummy_token, 0, sizeof(dummy_token));
	pcb->curtoken	 = &dummy_token;
	pcb->free_tokens = NULL;
	dummy_token.next = build_token_list(pcb);

	po_get_statements(pcb, pf); /* returns on EOF or unexpected RBRACE */
	lookup_token(pcb);
	if (pcb->t.toktype != TOK_EOF)
		po_say_fatal(pcb, "unexpected '}'");

	po_code_op(pcb, &pf->fcd, OP_END);
	fuf = po_memzalloc(pcb, sizeof(*fuf));
	fuf->name = po_clone_string(pcb, name);
	fuf->mlink = pcb->run.protos;
	pcb->run.protos = fuf;
	if (!po_compress_func(pcb, pf, fuf))
		goto BADOUT;
	if (!po_check_undefined_funcs(pcb, pf->symbols))
		goto BADOUT;
	po_dump_file(pcb);
	pcb->run.data_size = -pf->doff;
	}

BADOUT:

free_token_lists(pcb);

po_free_symbol_list(&pf->parameters);	/* free res. words */

if (fuf != NULL)
	fuf->parameters = NULL; 	/* we just freed these above! */

po_old_frame(pcb);

free_cframes(pcb, &pcb->cframes);

po_free_pp(pcb);

return(Success);
}

static void free_fuf_list(Func_frame **pff)
/*****************************************************************************
 * free list of func_frames, and structures attached to each.
 ****************************************************************************/
{
Func_frame *ff, *next;

ff = *pff;
while (ff != NULL)
	{
	next = ff->mlink;
	po_free_symbol_list(&ff->parameters);
	poc_gentle_freemem(ff->name);
	if (ff->type == CFF_POCO)
		{
		po_free_line_data(ff->ld);
		poc_gentle_freemem(ff->code_pt);
		}
	poc_gentle_freemem(ff->return_type);
	po_freemem(ff);
	ff = next;
	}
*pff = NULL;
}

void po_free_run_env(Poco_run_env *pev)
/*****************************************************************************
 * free the run environement, and the literals and functions attached to it.
 ****************************************************************************/
/* Free the run-environment */
{
po_freelist(&pev->literals);
free_fuf_list(&pev->protos);
}
