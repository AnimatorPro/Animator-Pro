/*****************************************************************************
 *
 * declare.c - Routines to parse a declarator, and handle prototypes.
 *
 * MAINTENANCE
 *	08/19/90	(Ian)
 *				Fixed check_dupe_proto().  When a duplicate proto is
 *				detected, and the old fuf has no code (it is truly a proto)
 *				and the param types are the same between the old and the new
 *				proto, then we attach the symbol list from the new proto
 *				to the fuf of the old.	This is only half a fix; a real
 *				fix would be to delete the old fuf entirely, and link the
 *				new fuf with the old symbols.  But, that's a big change
 *				because the fuf's are a singly-linked list, and there is
 *				no routine right now to delete just one of them.
 *	08/25/90	(Ian)
 *				Changed all occurances of freemem() and gentle_freemem() to
 *				po_freemem() and poc_gentle_freemem().
 *	09/01/90	(Ian)
 *				Combined reverse_comps() and reverse_dims() into one routine.
 *	09/19/90	(Ian)
 *				Added handling of prototypes with just the word 'void' inside
 *				the parens (eg, int xyz(void);) to get_param & gather_params.
 *	10/01/90	(Ian)
 *				Changed coding of OP_LEAVE; it no longer has immediate data
 *				as part of the instruction.
 *	10/04/90	(Ian)
 *				Added support of static vars, and enforcement of the ANSI
 *				requirement that array/struct/static initializers be
 *				constant expressions.  This required changing some occurances
 *				of var->scope to var->storage_scope.  A change was made to
 *				gather_params() to ensure that function params are always
 *				classified as local storage scope.	A change was made to
 *				based_decs() to propogate the new flags element from the base
 *				type_info to the new one being built for the variable. Changes
 *				were made in po_var_init() to enforce the need for constant init
 *				expressions().	This relies, in part, on a fix made in the
 *				make_assign() routine in poco.c that propogates the pure_const
 *				element from the value expression to the assignment expression.
 *	10/04/90	(Ian)
 *				Fixed a bug in check_dupe_proto() that was reporting a return
 *				type mismatch when the problem was actually a parameter
 *				mismatch.  We now simply check parameters before return type.
 *				Also, converted sprintf/po_say_fatal to new formatted po_say_fatal.
 *	10/07/90	(Ian)
 *				Removed NULL checks from calls to memory allocation.
 *	10/20/90	(Ian)
 *				Added support for 'typenames' as defined in K&R2 A.8.8.
 *				This involving adding a new entry point to the type parser,
 *				called po_get_typename(), and kluging up some of the other
 *				routines.  Notably, the dirdcl() routine has some checks in
 *				it now for a null pointer to a symbol, this being the key
 *				that says we are looking for a typename, which ironically
 *				enough means we don't expect to find a name, just the type
 *				information.
 *	10/21/90	(Ian)
 *				Added logic to the func_proto() routine to make the connection
 *				between a library function prototype and the code address of
 *				the function.  See comment block within func_proto() for
 *				more details.
 *	10/22/90	(Ian)
 *				Changed t.ctoke to curtoken.val.string in po_var_init(), part of
 *				supporting multi-token lookaheads (t.ctoke is no more).
 *	10/27/90	(Ian)
 *				Removed discard_to_rparen() routine; typename parsing now
 *				processed prototypes that it finds in full, so that proper
 *				type checking can be done during recast processing.
 *				Added 'magic' field to func_frame structure, so that RUNOPS
 *				can detect wild function pointers at runtime.  That field
 *				is set to its proper magic number value herein, at the same
 *				time we set the got_code field to TRUE.
 *	10/29/90	(Ian)
 *				Changed array_decl() so that zero-dimension arrays are
 *				disallowed except within function prototypes.  When found
 *				there, the type is set up as a pointer instead of an array.
 *				Also, added logic so that prototypes without a code block
 *				following them may have typenames only, without actual
 *				parameter names. (We build dummy symbols with a nullstring
 *				for a name, so that we have someplace to hang the type infos
 *				off of for the prototype.) The way we detect missing parm
 *				names when there is a code block is via the po_rehash()
 *				function; it will return non-zero if it finds a nullstring
 *				name during the rehasing, and get_body() herein will report
 *				the error.
 *	05/12/91	(Ian)
 *				Added check in check_dupe_proto to is a symbol-redefined
 *				error message for a function name that duplicates a library
 *				function name.	(This only affected functions from POE libs
 *				that duplicated builtin lib functions).
 *	09/06/91	(Jim)
 *		o		Made coding of OP_POP's to clean off expression stack
 *				go through po_find_pop_op() instead of the
 *				somewhat wierd (OP_IPOP+ido_type) technique.
 *	05/22/92	(Ian)
 *				Moved var_init() function to its own module, VARINIT.C.
 ****************************************************************************/

#include <string.h>
#include "poco.h"

static void 		dirdcl(Poco_cb *pcb, Poco_frame *pf, Type_info *ti,
						Symbol **name, Symbol **osym);
static void 		dcl(Poco_cb *pcb, Poco_frame *pf, Type_info *ti,
						Symbol **name, Symbol **osym);
static Errcode		func_proto(Poco_cb *pcb, Poco_frame *pf, Type_info *ti,
						char *name);
static void 		array_decl(Poco_cb *pcb, Type_info *ti);
static void 		one_dec(Poco_cb *pcb, Poco_frame *pf, Type_info *base_ti,
						Symbol **pvar);
static Type_info	*rev_type_info(Poco_cb *pcb, Type_info *old);


static void dcl(Poco_cb *pcb, Poco_frame *pf, Type_info *ti,
	Symbol **name, Symbol **osym)
/*****************************************************************************
 * parse a declarator (this routine responsible for handling stars)
 ****************************************************************************/
{
int ns = 0;

for (;;)	/* count stars */
	{
	lookup_token(pcb);
	if (pcb->t.toktype == '*')
		ns += 1;
	else
		break;
	}
dirdcl(pcb,pf,ti,name,osym);
while (ns-- > 0)
	po_append_type(pcb, ti, TYPE_POINTER, 0, NULL);
}

static Symbol *force_local_symbol(Poco_cb *pcb, Symbol *s, Symbol **osym)
/*****************************************************************************
 * if symbol exists at a more global scope, make a new locally-scoped symbol.
 *
 * (Note to self: this routine actually creates duplicate symbols in a frame,
 * and it is the responsibility of the callers to detect this later and get
 * the duplicate back out of the linked list before someone sees it. Strange.)
 * jk - horrible kludge #103.
 ****************************************************************************/
{
*osym = NULL;
if (s->scope < pcb->rframe->scope)
	{
	s = po_new_symbol(pcb, s->name);
	}
else
	{
	if (s->tok_type != PTOK_UNDEF)
		{
		*osym = s;
		s = po_new_symbol(pcb, s->name);
		}
	}
if (s != NULL)
	{
	s->tok_type = PTOK_VAR;
	}
return(s);
}

static void dirdcl(Poco_cb *pcb, Poco_frame *pf, Type_info *ti,
	Symbol **name, Symbol **osym)
/*****************************************************************************
 * parse a direct declarator.
 * (direct implies stars have been removed at this level of paren nesting).
 *
 * this routine has been hacked up to process typenames.  if the 'name'
 * parameter is NULL, that is our key that a typename is being processed,
 * and instead of expecting the name at a given point in the declaration,
 * we ensure that there is NO name at that point.  also, we do a little
 * strangeness involving pushback_token when doing a typename, due to the
 * difference in the call paths for normal and typename processing.
 ****************************************************************************/
{
if (pcb->t.toktype == TOK_LPAREN)	 /* ( dcl ) */
	{
	dcl(pcb,pf,ti,name,osym);
	if (pcb->t.toktype == TOK_RPAREN)	 /* ( dcl ) */
		pcb->t.reuse = FALSE; /* for typenames: eat it if a reuse is pending. */
	else
		po_unmatched_paren(pcb);
	}
else if (pcb->curtoken->is_symbol)
	{
	if (name == NULL)						/* for typenames... */
		po_say_fatal(pcb, "declaration of '%s' not allowed in this expression",
					pcb->curtoken->val.symbol->name);
	else									/* for regular declarations... */
		*name = force_local_symbol(pcb,pcb->curtoken->val.symbol,osym);
	}
else
	{
	if (name == NULL)						/* for typenames... */
		pushback_token(&pcb->t);
	else if (pcb->rframe->is_proto_frame)	/* for prototypes...*/
		{
		pushback_token(&pcb->t);
		*name = po_new_symbol(pcb, "");
		(*name)->tok_type = PTOK_VAR;
		*osym = NULL;
		}
	else									/* for regular declarations... */
		po_expecting_got(pcb, "name in declaration");
	}

for (;;)
	{
	lookup_token(pcb);
	if (pcb->t.toktype == TOK_LPAREN)
		{
		if (name == NULL)	/* for typenames, we just eat the prototype */
			func_proto(pcb, pf, ti, "noname");
		else			/* for regular declarations, we build a prototype... */
			func_proto(pcb,pf,ti,(*name)->name);
		}
	else if (pcb->t.toktype == '[')
		array_decl(pcb,ti);
	else
		{
		if (name == NULL) /* for typenames...*/
			pushback_token(&pcb->t);
		break;
		}
	}
}

static void array_decl(Poco_cb *pcb, Type_info *ti)
/*****************************************************************************
 * figure out the dimension of an array declaration.
 ****************************************************************************/
{
int 		dim;
Exp_frame	ef;
short		datatype = TYPE_ARRAY;

	lookup_token(pcb);
	if (pcb->t.toktype == ']')
		{
		if (pcb->rframe->is_proto_frame)
			datatype = TYPE_POINTER;
		else if (!po_is_next_token(pcb, '='))
			po_say_fatal(pcb, "size of array cannot be zero");
		dim = 0;
		}
	else
		{
		if (pcb->rframe->is_proto_frame)
			datatype = TYPE_POINTER;
		pushback_token(&pcb->t);
		po_init_expframe(pcb, &ef);
		po_get_expression(pcb, &ef);
		if (!ef.pure_const)
			po_say_fatal(pcb, "expecting constant expression for array dimension.");
		po_force_num_exp(pcb, &ef.ctc);
		po_coerce_numeric_exp(pcb, &ef, IDO_INT);
		po_eat_rbracket(pcb);
		dim = ((int *)(ef.ecd.code_buf+OPY_SIZE))[0];
		po_trash_expframe(pcb, &ef);

		}

	po_append_type(pcb, ti, datatype, dim, NULL);
}

static Symbol *get_param(Poco_cb *pcb, Poco_frame *pf, SHORT pcount, long *poff)
/*****************************************************************************
 * get one param (type and name) from a function prototype.
 * the size of the param is also obtained, so our caller can calc offsets.
 ****************************************************************************/
{
Itypi tip;
Type_info *ti;
Symbol *var;

ti = po_typi_type(&tip);
if (pcb->t.toktype == PTOK_ELLIPSIS)
	{
	if ((var = po_new_symbol(pcb, "...")) == NULL)
		return(NULL);
	var->tok_type = PTOK_VAR;
	var->flags |= SFL_ELLIP;
	po_append_type(pcb, ti, TYPE_ELLIPSIS, 0L, NULL);
	if ((var->ti = po_new_type_info(pcb, ti, 0)) == NULL)
		return(NULL);
	return(var);
	}
if (po_get_base_type(pcb,pf,ti))
	{
	/*
	 * hack-ola dept:
	 *	if the only thing inside the parens of the prototype is the word
	 *	'void', return NULL as a signal that no vars are defined here. our
	 *	 caller (gather_params) recognizes the NULL return as an indication
	 *	 that no params are specified in the prototype.  we have to handle
	 *	 this here instead of in gather_params because it takes two tokens
	 *	 worth of lookahead from there to figure out (void) was specified.
	 */
	if (pcount == 0 && ti->comp_count == 1 && ti->comp[0] == TYPE_VOID)
		{
		lookup_token(pcb);
		if (pcb->t.toktype == TOK_RPAREN)
			return NULL;
		else
			pushback_token(&pcb->t);
		}
	one_dec(pcb,pf,ti,&var);
	var->symval.doff = *poff;
	var->storage_scope = SCOPE_LOCAL;
	*poff += po_get_param_size(pcb, var->ti->ido_type);
	return(var);
	}
else
	{
	po_expecting_got(pcb,"type in function parameter list");
	return NULL;
	}
}

static void gather_params(Poco_cb *pcb, Func_frame *proto)
/*****************************************************************************
 * get all params (types and names) from a function prototype.
 * assign stack offsets to params as we go.
 ****************************************************************************/
{
#define STACK_EL_SIZE (sizeof(void *))
#define HARD_FRAME_SIZE (2*STACK_EL_SIZE)

long poff = HARD_FRAME_SIZE;
Symbol *var;

	if (pcb->t.toktype == TOK_RPAREN)
		return;
	for (;;)
		{
		if (NULL == (var = get_param(pcb, pcb->rframe, proto->pcount, &poff)))
			goto DONE;	/* only param was the word 'void', ie, no params */

		proto->pcount += 1;
		if (var->ti->comp[0] == TYPE_ELLIPSIS)
			{
			po_eat_rparen(pcb);
			goto DONE;
			}

		switch (pcb->t.toktype)
			{
			case TOK_RPAREN:
				goto DONE;
			case ',':
				break;
			default:
				po_expecting_got(pcb, ", or ) in function parameter list");
			}
		lookup_token(pcb);
		}

DONE:
	return;

#undef STACK_EL_SIZE
#undef HARD_FRAME_SIZE
}

static Errcode transfer_params(Poco_cb *pcb, Poco_frame *pf, Func_frame *ff)
/*****************************************************************************
 * Parameters symbols get transfered from pf to ff.  Other symbols stay.
 * Side effect of reversing order of symbols that stay.
 *
 * (Note to self:  The only symbols that could be in pf besides params
 * would be struct tags, right?  This needs investigating, might be able
 * to use that fact to speed things up.)
 * jk - I think you're correct.  I'm not proud of this piece of code, but
 * couldn't think of another way without redoing a lot of other stuff.
 * Probably best to think about it for a while before taking action.
 ****************************************************************************/
{
Symbol *plist = NULL;		/* new pf->symbols */
Symbol *flist = NULL;		/* new ff->parameters */
Symbol *pt, *link;
int pcount = 0;
Errcode err = Success;

link = pf->symbols;
while ((pt = link) != NULL)
	{
	link = pt->link;
	if (pt->tok_type == PTOK_VAR)
		{
		pt->link = flist;
		flist = pt;
		pcount += 1;
		}
	else
		{
		pt->link = plist;
		plist = pt;
		}
	}
ff->pcount = pcount;
ff->parameters = flist;
pf->symbols = plist;
return(err);
}


static Errcode func_proto(Poco_cb *pcb, Poco_frame *pf,
	Type_info *ti, char *name)
/*****************************************************************************
 * process a function prototype.
 * (Note to self, for future investigation...
 * this routine doesn't know whether code will follow, or if this is truly
 * a prototype.  this routine builds the fuf for the function, unfortunately,
 * it does so even if a fuf already exists (could this be fixed?)  It
 * watches for new structure tags, and moves them to the root frame's fsif,
 * a process which might be better handled in transfer_params, above.)
 * jk - Putting the move-to-root of struct tags in transfer_params sounds
 * like a good idea and relatively easy.  On the other hand I'm not sure
 * if it's avoidable to build the prototype.  Certainly we do have to
 * check that the proto's are logically the same.
 ****************************************************************************/
{
Func_frame *proto = NULL;
Poco_frame *rf;

lookup_token(pcb);
proto = po_memzalloc(pcb, sizeof(*proto));

/*
 * if we are doing a library proto (indicated by a non-NULL libfunc pointer
 * in the pcb structure), make the connection now between the
 * library function's code address and the proto we are processing.  note
 * that this is only done if we are at the root level -- a proto within a
 * proto won't grab an address (since that's a meaningless concept).
 *
 * note that this MUST be done before we start processing the prototype,
 * because by the time we reach the end of the prototype, the multi-token
 * lookahead logic will have advanced us to the next prototype from the
 * library, and we would end up pointing to the wrong function.
 */

if (pcb->libfunc != NULL && pf->frame_type == FTY_GLOBAL)
	{
	proto->code_pt	= pcb->libfunc;
	proto->got_code = TRUE;
	proto->type 	= CFF_C;
	proto->magic	= FUNC_MAGIC;  /* helps detect wild pointers at runtime */
	}
else
	{
	proto->code_pt	= NULL;
	proto->got_code = FALSE;
	proto->type 	= CFF_POCO;
	}

if (pcb->t.toktype != TOK_RPAREN)
	{
	po_new_frame(pcb, pf->scope+1, name, FTY_STRUCT);
	rf = pcb->rframe;
	rf->is_proto_frame = TRUE;
	gather_params(pcb, proto);
	transfer_params(pcb, rf, proto);
	if (rf->fsif != NULL)
		po_move_sifs_to_parent(pcb);

#ifdef DEADWOOD
			/* move any new structure declarations to root frame */
			if ((sifs = rf->fsif) != NULL)
				{
				while (sifs->next != NULL)
					sifs = sifs->next;
				/* put root frame in rr */
				rr = rf;
				while (rr->next != NULL)
					rr = rr->next;
				/* and concatinate root's struct-info list with ours */
				sifs->next = rr->fsif;
				rr->fsif = rf->fsif;
				rf->fsif = NULL;
				}
#endif
	po_old_frame(pcb);
	}

proto->name = po_clone_string(pcb, name);
po_append_type(pcb, ti, TYPE_FUNCTION, 0L, proto);
proto->mlink = pcb->run.protos;
pcb->run.protos = proto;

return(Success);
}

static void reverse_comps(TypeComp *oldc, TypeComp *newc,
						  Pt_long	*oldd, Pt_long	 *newd,
						  int count)
/*****************************************************************************
 * reverse elements in type_info type_comps and sdims arrays.
 ****************************************************************************/
{
newc += count;
newd += count;
while (--count >= 0)
	{
	*(--newc) = *oldc++;
	*(--newd) = *oldd++;
	}
}

static Type_info *rev_type_info(Poco_cb *pcb, Type_info *old)
/*****************************************************************************
 * alloc new type_info, init it, copy comp and sdims arrays to it in reverse.
 * this routine is the main allocator of type_comps for others.  after a
 * type is all parsed out, it lives in backwards order in a stack-alloc'd
 * Itype_i in another routine.	that routine will call this one to allocate
 * the real type_info struct (and associated arrays) that will be linked to
 * the symbol which is being processed.
 ****************************************************************************/
{
Type_info *newt;
int lsize,csize;

lsize = old->comp_count*sizeof(Pt_long);
csize = old->comp_count*sizeof(TypeComp);
newt = po_memzalloc(pcb, sizeof(*newt) + lsize + csize);
*newt = *old;
newt->comp_count = newt->comp_alloc = old->comp_count;
newt->sdims = OPTR(newt,(sizeof(*newt)));
newt->comp	= OPTR(newt,(sizeof(*newt)+lsize));
reverse_comps(old->comp,  newt->comp,
			  old->sdims, newt->sdims,
			  old->comp_count);
po_set_ido_type(newt);
return(newt);
}

Func_frame *po_sym_to_fuf(Symbol *s)
/*****************************************************************************
 * return a pointer to the fuf associated with a TYPE_FUNCTION symbol.
 ****************************************************************************/
{
Type_info *ti;

ti = s->ti;
return(ti->sdims[ti->comp_count-1].pt);
}

static Errcode check_params_same(Poco_cb *pcb, Func_frame *f1, Func_frame *f2)
/*****************************************************************************
 * compare the type_info for each param in a pair of prototypes.
 * note that this is used only in handling prototype declarations, it is not
 * used in parsing function calls at code-gen time (that is handled by
 * the routine po_fuf_types_same() in pocotype.c).	a single routine can't be
 * used because at code-gen time we have to handle the ellipsis differently.
 ****************************************************************************/
{
int count;
int i;
Symbol *p1, *p2;
Errcode err = Success;

if ((count=f1->pcount) != f2->pcount)
	{
	po_say_fatal(pcb,"argument count disagrees in redeclaration of %s", f1->name);
	}
else
	{
	p1 = f1->parameters;
	p2 = f2->parameters;
	for (i=0; i<count; i++)
		{
		if (!po_types_same(p1->ti, p2->ti, 0))
			{
			po_say_fatal(pcb, "type mismatch for argument %d in redeclaration of %s",
				i+1, f1->name);
			}
		p1 = p1->link;
		p2 = p2->link;
		}
	}
return(err);
}

static void part_free_proto(Poco_cb *pcb, Symbol *s, Func_frame *proto)
/*****************************************************************************
 * free symbols from a duplicate proto's fuf, and remove the fuf's symbol
 * from the global poco_frame.	(The symbol will actually be a duplicate
 * of a symbol already in the global frame, because of the way func_proto()
 * works.  If func_proto() is fixed, this routine needs fixing too.)
 ****************************************************************************/
{
po_free_symbol_list(&proto->parameters);
po_unhash_symbol(pcb,s);
}

static void check_dupe_proto(Poco_cb *pcb, Symbol *osym, Symbol *nsym)
/*****************************************************************************
 * check that 2 prototypes are identical in all respects execpt param names.
 * if not, complain and die.  if so, keep the fuf from the old prototype,
 * but attach the symbol list from the new prototype (freeing the old symbol
 * list.)  this allows a proto to be redefined any number of times, but
 * ensures that the param names from the most recent definition are used.
 * (a concept which is significant when the proto has a code block after it!)
 ****************************************************************************/
{
Func_frame *ofuf = po_sym_to_fuf(osym);
Func_frame *nfuf = po_sym_to_fuf(nsym);

if (check_params_same(pcb,ofuf,nfuf) >= Success)
	{
	if (!po_types_same(osym->ti, nsym->ti, 0))
		{
		po_say_fatal(pcb, "return type mismatch in redeclaration of %s", osym->name);
		}
	else if (ofuf->got_code &&
				(ofuf->type == CFF_C || pcb->t.toktype == TOK_LBRACE))
		{
		po_redefined(pcb, nsym->name);
		}
	else
		{
		part_free_proto(pcb, nsym, ofuf);
		ofuf->parameters = nfuf->parameters;
		nfuf->parameters = NULL;
		}
	}
}

static void one_dec(Poco_cb *pcb, Poco_frame *pf, Type_info *base_ti,
	Symbol **pvar)
/*****************************************************************************
 * process one declaration (er, everything up to a ',' or ';', I think).
 ****************************************************************************/
{
Symbol *var;
Itypi tip;
Type_info *ti;
Symbol *osym;
Boolean po_is_func;

ti = po_typi_type(&tip);
dcl(pcb, pf,ti, &var, &osym);

if (!po_cat_type(pcb,ti,base_ti))
	return;
ti->flags = base_ti->flags;

ti = var->ti = rev_type_info(pcb, ti);
if ((po_is_func = (ti->comp[ti->comp_count-1] == TYPE_FUNCTION)) == TRUE)
	{
	if (pf->frame_type != FTY_GLOBAL)
		{
		po_say_fatal(pcb,
			"function prototypes only allowed outside function declarations.");
		}
	}
if (osym != NULL)
	{
	if (!po_is_func)
		{
		po_redefined(pcb, var->name);
		}
	else	/* check prototypes the same... */
		{
		check_dupe_proto(pcb,osym,var);
		var = osym;
		}
	}
*pvar = var;
}

Func_frame *po_get_proto(Type_info *ti)
/*****************************************************************************
 * given a pointer to a type_info, return the pointer to a fuf.
 ****************************************************************************/
{
return(ti->sdims[ti->comp_count-1].pt);
}

static void get_body(Poco_cb *pcb, Poco_frame *pf, Symbol *fvar)
/*****************************************************************************
 * parse and code-gen from just past opening '{' of function until '}'.
 ****************************************************************************/
{
Poco_frame *rf;
Func_frame *proto;
long enter_fixup;
int local_space;
int errparm;
#ifdef STRING_EXPERIMENT
Symbol *params;
#endif /* STRING_EXPERIMENT */

if (po_new_frame(pcb,pf->scope+1,fvar->name,FTY_FUNC))
	{
	proto = po_get_proto(fvar->ti);
	rf = pcb->rframe;
	rf->parameters = proto->parameters;
	/* Put parameter strings in list to clean up on function exit.
	 * (This doesn't handle parameters past the ... in variable
	 * argument functions - that must be coped with in funccall.c */
#ifdef STRING_EXPERIMENT
	params = rf->parameters;
	while (params != NULL)
		{
		po_add_local_string(pcb,rf,params);
		params = params->link;
		}
#endif /* STRING_EXPERIMENT */
	rf->pcount = proto->pcount;
	rf->return_type = proto->return_type;
	/* Transfer parameters to local symbol table */
	if (0 != (errparm = po_rehash(pcb, proto->parameters)))
		po_say_fatal(pcb, "missing name for parameter %d of function %s",
						errparm, fvar->name);
	enter_fixup = po_code_int(pcb, &rf->fcd, OP_ENTER, 0);
	po_get_block(pcb, rf);
	local_space = -rf->doff;
	po_int_fixup(&rf->fcd, enter_fixup, local_space);
#ifdef STRING_EXPERIMENT
	po_code_free_string_ops(pcb, rf);
#endif /* STRING_EXPERIMENT */
	po_code_op(pcb, &rf->fcd, OP_LEAVE);
	po_code_op(pcb, &rf->fcd, OP_RET);
	po_compress_func(pcb, rf, proto);
	proto->got_code = TRUE;
	proto->magic	= FUNC_MAGIC;  /* helps detect wild pointers at runtime */
	po_old_frame(pcb);
	}
}

void po_pop_off_result(Poco_cb *pcb, Exp_frame *e)
/*****************************************************************************
 * gen code to pop off result of current expression.
 ****************************************************************************/
{
if (e->ctc.ido_type != IDO_VOID)
	po_code_pop(pcb, &e->ecd
	,po_find_clean_op(pcb, &e->ctc), po_find_push_op(pcb, &e->ctc));
}

void po_get_typedef(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * parse type_info for typedef, add sym to global frame as PTOK_USER_TYPE.
 ****************************************************************************/
{
Symbol *var;
Itypi tip;
Type_info *ti;
Type_info *rti;


lookup_token(pcb);
ti = po_typi_type(&tip);
if (!po_get_base_type(pcb,pf,ti))
	{
	po_say_fatal(pcb, "no type in typedef");
	goto OUT;
	}
one_dec(pcb,pf,ti,&var);
var->tok_type = PTOK_USER_TYPE;
/* reverse type info so it will read as if it were being parsed */
if ((rti = rev_type_info(pcb, var->ti)) == NULL)
	goto OUT;
po_freemem(var->ti);
var->ti = rti;
OUT:
return;
}

static void fill_in_return_type(Poco_cb *pcb, Symbol *var)
/*****************************************************************************
 * fill in the type_info for the return value of a function.
 ****************************************************************************/
{
Type_info *ti;
Func_frame *fuf;

ti = var->ti;
fuf = ti->sdims[ti->comp_count-=1].pt;
if (fuf->return_type == NULL)
	{
	fuf->return_type = po_new_type_info(pcb, ti, 0);
	po_set_ido_type(fuf->return_type);
	}
ti->comp_count += 1;
}

static void based_decs(Poco_cb *pcb, Poco_frame *pf, Type_info *base_ti)
/*****************************************************************************
 * get declarations of the same base type. loop until ';' or '{stuff}' seen.
 * (handles 'int a,b,*c;', etc)
 ****************************************************************************/
{
Poco_frame	*rf;
Symbol		*var;
Exp_frame	eee;
SHORT		frame_type;
Boolean 	is_fu;

loop:
	{
	one_dec(pcb,pf,base_ti, &var);
	if ((is_fu = po_is_func(var->ti)) == TRUE)
		{
		fill_in_return_type(pcb, var);
		}
	else
		{
		po_new_var_space(pcb, var);
#ifdef STRING_EXPERIMENT
		if (pf->frame_type == FTY_FUNC)
			po_add_local_string(pcb, pf, var);
#endif /* STRING_EXPERIMENT */
		}
	if (pcb->t.toktype == '=')  /* it's an assignment, whoopie */
		{
		if ((frame_type = pf->frame_type) == FTY_STRUCT)
			{
			po_say_fatal(pcb, "= not allowed inside structure definitions");
			goto end;
			}
		rf = pf;
		if (var->storage_scope == SCOPE_GLOBAL)
			{
			while (rf->frame_type != FTY_GLOBAL)
				rf = rf->next;
			frame_type = FTY_GLOBAL;
			}
		po_init_expframe(pcb, &eee);
		po_copy_type(pcb,var->ti, &eee.ctc);
		po_var_init(pcb, &eee, var, frame_type);
		po_cat_code(pcb, &rf->fcd, &eee.ecd);
		po_trash_expframe(pcb, &eee);
		lookup_token(pcb);
		}
	switch (pcb->t.toktype)
		{
		case ',':
			goto loop;
		case ';':
			pushback_token(&pcb->t);
			goto end;
		case TOK_LBRACE:
			if (pf->scope != SCOPE_GLOBAL || !is_fu)
				{
				goto wanna;
				}
			get_body(pcb, pf, var);
			goto end;
		default:
wanna:
			po_expecting_got(pcb, ", or ;");
			goto end;
		}
	}
end:
return;
}

void po_get_typename(Poco_cb *pcb, Type_info *ti)
/*****************************************************************************
 * get a typename (per K&R2 A.8.8).  called from use_sizeof() and make_cast().
 ****************************************************************************/
{
Itypi		tip;
Type_info	*ti_temp;
Poco_frame *pf = pcb->rframe;

po_eat_lparen(pcb);
po_need_token(pcb);

#ifdef DEVELOPMENT
	if (FALSE == po_get_base_type(pcb,pf,ti))
		po_say_internal(pcb, "bad return from po_get_base_type detected in po_get_typename");
#else
	po_get_base_type(pcb, pf, ti);
#endif

	if (po_is_next_token(pcb, ')'))
		{
		po_eat_rparen(pcb);
		return;
		}

	ti_temp = po_typi_type(&tip);

	dcl(pcb, pf, ti_temp, NULL, NULL);

	po_eat_rparen(pcb);

	po_cat_type(pcb, ti_temp, ti);
	ti->comp_count = ti_temp->comp_count;

	reverse_comps(ti_temp->comp,  ti->comp,
				  ti_temp->sdims, ti->sdims,
				  ti_temp->comp_count);
	po_set_ido_type(ti);

}

void po_get_declaration(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * main entry point to declaration parser. called from statement().
 ****************************************************************************/
{
Itypi tip;
Type_info *ti;


ti = po_typi_type(&tip);
if (po_get_base_type(pcb,pf,ti))
	{
	if (po_is_next_token(pcb, ';'))
		{
		return;
		}
	based_decs(pcb, pf, ti);
	}
else
	{
	po_set_base_type(pcb, ti, TYPE_INT, 0L, NULL);
	based_decs(pcb, pf, ti);
	}
}

