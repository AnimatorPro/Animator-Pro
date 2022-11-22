
/*****************************************************************************
 *
 * pocotype.c - Type checking, and some of the type parsing routines.
 *
 * MAINTENANCE
 *	09/18/90	(Ian)
 *				Added support for union.
 *	09/19/90	(Ian)
 *				Added support for all ANSI type keywords (const, auto, etc).
 *	10/02/90	(Ian)
 *				Added support for enum.
 *	10/03/90	(Ian)
 *				Big changes to the po_get_base_type() routine.	A new flags
 *				element was added to the type_info structure to track
 *				modifiers (such as long, short, static, etc) to the base
 *				type.  Right now, the only flag that is exported from the
 *				routine is TFL_STATIC.	All other modifications to the base
 *				type are handled within the routine, and the type_comp will
 *				reflect that without the need for the flags that lead to it
 *				to be seen elsewhere.  (IE, when 'long int' is seen, the
 *				base type comp is set to TYPE_LONG, and the TFL_LONG flag
 *				will NOT be set in the type_info.)
 *	10/07/90	(Ian)
 *				Removed NULL checks from calls to memory allocation.
 *	10/19/90	(Ian)
 *				Fixed po_get_base_type so that size of a short is 2 bytes again.
 *	10/23/90	(Ian)
 *				Added TYPE_CHAR to the see-and-ignore list for signed/unsigned.
 *	10/24/90	(Ian)
 *				Changed po_types_same() so that pointers and arrays are
 *				intermixable (eg, char *a, b[]; // a and b are the same type)
 *	10/27/90	(Ian)
 *				Fixed a type in po_types_same() that was allowing type
 *				mismatches.
 *	07/06/91	(Ian)
 *				Tweaked po_types_same() to fix pointer/array ambiguity
 *				glitches.  See comments in that routine's header block.
 *  09/06/91	(Jim)
 *				Started to add in String type.  Converted unused TYPE_RAST
 *				into TYPE_STRING for starters.  A string is a base type,
 *				like int, etc.   It gets converted back and forth from
 *				(char *)pretty easily.
 *			>	Folded po_is_num_ido() into a macro on the po_ido_table.
 ****************************************************************************/

#include <string.h>
#include "poco.h"


struct type_table
	{
	char *name;
	SHORT val;
	long size;
	Boolean is_int;
	};

static struct type_table base_type_names[] = {
/*		 name			  val			  size			is_int */
/*	-----------------	-------------	-------------	-------*/
	{ "TYPE_END",       TYPE_END,       0,              FALSE,},
	{ "TYPE_CHAR",      TYPE_CHAR,      sizeof(char),   TRUE,},
	{ "TYPE_UCHAR",     TYPE_UCHAR,     sizeof(char),   TRUE,},
	{ "TYPE_SHORT",     TYPE_SHORT,     sizeof(short),  TRUE,},
	{ "TYPE_USHORT",    TYPE_USHORT,    sizeof(short),  TRUE,},
	{ "TYPE_INT",       TYPE_INT,       sizeof(int),    TRUE,},
	{ "TYPE_UINT",      TYPE_UINT,      sizeof(int),    TRUE,},
	{ "TYPE_LONG",      TYPE_LONG,      sizeof(long),   TRUE,},
	{ "TYPE_ULONG",     TYPE_ULONG,     sizeof(long),   TRUE,},
	{ "TYPE_FLOAT",     TYPE_FLOAT,     sizeof(float),  FALSE,},
	{ "TYPE_DOUBLE",    TYPE_DOUBLE,    sizeof(double), FALSE,},
	{ "TYPE_POINTER",   TYPE_POINTER,   sizeof(Popot),  FALSE,},
	{ "TYPE_FUNCTION",  TYPE_FUNCTION,  0,              FALSE,},
	{ "TYPE_VOID",      TYPE_VOID,      0,              FALSE,},
	{ "TYPE_ARRAY",     TYPE_ARRAY,     0,              FALSE,},
	{ "TYPE_ELLIPSIS",  TYPE_ELLIPSIS,  0,              FALSE,},
	{ "TYPE_SCREEN",    TYPE_SCREEN,    sizeof(Popot),  FALSE,},
#ifdef STRING_EXPERIMENT
	{ "TYPE_STRING",    TYPE_STRING,    sizeof(PoString), FALSE,},
#endif /* STRING_EXPERIMENT */
	{ "TYPE_FILE",      TYPE_FILE,      0,              FALSE,},
	{ "TYPE_UNUSED0",   TYPE_UNUSED0,   0, 				FALSE,},
	{ "TYPE_CPT",       TYPE_CPT,       sizeof(void *), FALSE,},
	{ "TYPE_UNUSED1",   TYPE_UNUSED1,   0,              FALSE,},
	{ "TYPE_STRUCT",    TYPE_STRUCT,    0,              FALSE,},
	};

Boolean po_check_type_names(Poco_cb *pcb)
/*****************************************************************************
 * Run a sanity check on base_type_names array.
 ****************************************************************************/
{

#ifdef DEVELOPMENT
int i;

for (i=0; i<Array_els(base_type_names); i++)
	{
	if (i != base_type_names[i].val)
		{
		fprintf(pcb->t.err_file, "%d != %d at %s\n", i,
			base_type_names[i].val, base_type_names[i].name);
		po_say_internal(pcb, "base_type_names doesn't check");
		return(FALSE);
		}
	}
#endif
return(TRUE);
}


Type_info *po_new_type_info(Poco_cb *pcb, Type_info *old, int extras)
/*****************************************************************************
 * allocate and init a new type_info structure, based on an old one.
 * typically, the old one will be allocated on somebody's local stack; they
 * call here when ready to allocate a permenant type_info to tie to a symbol.
 ****************************************************************************/
{
Type_info *newt;
int lsize,csize;
int noff;
int ocount;

ocount = old->comp_count + extras;
lsize = ocount*sizeof(Pt_long);
csize = ocount*sizeof(TypeComp);
newt = po_memzalloc(pcb, sizeof(*newt) + lsize + csize);
*newt = *old;
newt->comp_count = old->comp_count;
newt->comp_alloc = ocount;
noff = sizeof(*newt);
newt->sdims = OPTR(newt,noff);
poco_copy_bytes(old->sdims, newt->sdims, lsize);
noff += lsize;
newt->comp = OPTR(newt,noff);
poco_copy_bytes(old->comp, newt->comp, csize);
return(newt);
}




Boolean po_is_int_ido(SHORT ido)
/*****************************************************************************
 * indicate whether IDO type is int.
 ****************************************************************************/
{
switch (ido)
	{
	case IDO_INT:
	case IDO_LONG:
		return(TRUE);
	}
return(FALSE);
}

#ifdef STRING_EXPERIMENT
Boolean po_is_string(Type_info *ti)
/*****************************************************************************
 * indicate whether type is string.
 ****************************************************************************/
{
return(ti->comp_count == 1 && ti->comp[0] == TYPE_STRING);
}
#endif /* STRING_EXPERIMENT */

Boolean po_is_pointer(Type_info *ti)
/*****************************************************************************
 * indicate whether type is pointer.
 ****************************************************************************/
{
if (ti->comp_count < 2)
	return(FALSE);
return(ti->comp[ti->comp_count-1] == TYPE_POINTER);
}

Boolean po_is_array(Type_info *ti)
/*****************************************************************************
 * indicate whether type is array or not.
 ****************************************************************************/
{
if (ti->comp_count < 2)
	return(FALSE);
return(ti->comp[ti->comp_count-1] == TYPE_ARRAY);
}

Boolean po_is_struct(Type_info *ti)
/*****************************************************************************
 * indicate whether type is a structure or not.
 ****************************************************************************/
{
return(ti->comp_count == 1 && ti->comp[0] == TYPE_STRUCT);
}

Boolean po_is_func(Type_info *ti)
/*****************************************************************************
 * indicate whether type is a function or not.
 ****************************************************************************/
{
if (ti->comp_count < 2)
	return(FALSE);
return(ti->comp[ti->comp_count-1] == TYPE_FUNCTION);
}

void po_set_ido_type(Type_info *ti)
/*****************************************************************************
 * figure out internal ('I do') type from variable/expression type.
 ****************************************************************************/
{
TypeComp t;

ti->ido_type = IDO_BAD;
if (ti->comp_count == 1)
	{
	t = ti->comp[0];
	switch (t)
		{
		case TYPE_CHAR:
		case TYPE_SHORT:
		case TYPE_INT:
			ti->ido_type = IDO_INT;
			break;
		case TYPE_LONG:
			ti->ido_type = IDO_LONG;
			break;
		case TYPE_DOUBLE:
		case TYPE_FLOAT:
			ti->ido_type = IDO_DOUBLE;
			break;
#ifdef STRING_EXPERIMENT
		case TYPE_STRING:
			ti->ido_type = IDO_STRING;
			break;
#endif /* STRING_EXPERIMENT */
		case TYPE_VOID:
			ti->ido_type = IDO_VOID;
			break;
		}
	}
else
	{
	t = ti->comp[ti->comp_count-1];
	switch (t)
		{
		case TYPE_POINTER:
		case TYPE_ARRAY:
			ti->ido_type = IDO_POINTER;
			break;
		case TYPE_FUNCTION:
			ti->ido_type = IDO_VPT;
			break;
		default:
			ti->ido_type = IDO_CPT;
			break;
		}
	}
}


Boolean po_append_type(Poco_cb *pcb, Type_info *ti, TypeComp tc, long dim,
	void *sif)
/*****************************************************************************
 * append new type onto type_info, complain & die if variable is too complex.
 ****************************************************************************/
{
if (ti->comp_count >= ti->comp_alloc)
	{
	po_say_fatal(pcb, "variable type too complex");
	return(FALSE);
	}
ti->comp[ti->comp_count] = tc;
if (tc == TYPE_STRUCT || tc == TYPE_FUNCTION)
	ti->sdims[ti->comp_count].pt = sif;
else
	ti->sdims[ti->comp_count].l = dim;
ti->comp_count++;
po_set_ido_type(ti);
return(TRUE);
}

Boolean po_set_base_type(Poco_cb *pcb, Type_info *ti, TypeComp tc,
	long dim, Struct_info *sif)
/*****************************************************************************
 * blast any existing type info and set the base type into the type_info.
 ****************************************************************************/
{
ti->comp_count = 0;
return(po_append_type(pcb,ti,tc,dim,sif));
}

Boolean po_copy_type(Poco_cb *pcb, Type_info *s, Type_info *d)
/*****************************************************************************
 * copy type info data to another type_info structure.
 ****************************************************************************/
{

#ifdef DEVELOPMENT
if (s->comp_count > d->comp_alloc)
	{
	po_say_internal(pcb, "variable type too complex in po_copy_type");  /* should never happen */
	return(FALSE);
	}
#endif

poco_copy_bytes(s->comp, d->comp, s->comp_count*sizeof(*(d->comp)) );
poco_copy_bytes(s->sdims, d->sdims, s->comp_count*sizeof(*(d->sdims)) );
d->ido_type = s->ido_type;
d->comp_count = s->comp_count;
return(TRUE);
}

Boolean po_cat_type(Poco_cb *pcb, Type_info *d, Type_info *s)
/*****************************************************************************
 * concatenate type info data onto another type_info structure.
 ****************************************************************************/
{
if (d->comp_count+s->comp_count > d->comp_alloc)
	{
	po_say_fatal(pcb, "variable type too complex");
	return(FALSE);
	}
poco_copy_bytes(s->comp, d->comp+d->comp_count,
	s->comp_count*sizeof(*(s->comp)) );
poco_copy_bytes(s->sdims, d->sdims+d->comp_count,
	s->comp_count*sizeof(*(s->sdims)) );
d->comp_count += s->comp_count;
return(TRUE);
}

Boolean po_is_void_ptr(Type_info *ti)
/*****************************************************************************
 * indicate whether type is a void pointer or not.
 ****************************************************************************/
{
return(ti->comp_count == 2	&& ti->comp[0] == TYPE_VOID &&
	(ti->comp[1] == TYPE_CPT || ti->comp[1] == TYPE_POINTER) );
}

#ifdef DEADWOOD
Boolean po_ptypes_same(Type_info *st, Type_info *dt)
/*****************************************************************************
 * indicate whether 2 pointers have legally-matching types or not.
 * (why doesn't that description seem right?)
 * (er, because it's not, I guess.  anyway, this seems to be deadwood.)
 ****************************************************************************/
{
	if (!(po_is_void_ptr(st) || po_is_void_ptr(dt)))
		return(FALSE);
	else
		return(TRUE);
}
#endif /* DEADWOOD */

Boolean po_fuf_types_same(Func_frame *sf, Func_frame *df)
/*****************************************************************************
 * check parameter types are compatible (identical except for mixing
 * pointers and void pointers, or ellipsis)
 ****************************************************************************/
{
Symbol *ss, *ds;
Type_info *st, *dt;
TypeComp sc, dc;

ss = sf->parameters;
ds = df->parameters;
while (ss != NULL && ds != NULL)
	{
	if (!po_types_same(st = ss->ti, dt = ds->ti, 0) )
		{
		sc = st->comp[0];
		dc = dt->comp[0];
		if (sc == TYPE_ELLIPSIS || dc == TYPE_ELLIPSIS )
			return(TRUE);		/* ... will match anything */
		if (!(po_is_void_ptr(st) || po_is_void_ptr(dt)))
			return(FALSE);
		}
	ss = ss->link;
	ds = ds->link;
	}
/* if have come to end of both parameter lists at same time then
   the comparison is simply true */
if (ss == NULL && ds == NULL)
	return(TRUE);
/* check if the remaining paremeter is an ellipsis. If so return TRUE */
if (ss == NULL)
	dc = ds->ti->comp[0];
else
	dc = ss->ti->comp[0];
if (dc == TYPE_ELLIPSIS)
	return(TRUE);
return(FALSE);
}

Boolean po_types_same(Type_info *s, Type_info *d, int start)
/*****************************************************************************
 * indicate whether 2 type_infos are the same.
 * 07/06/91: this routine has taken a turn for the weird...
 *	 we used to allow total interchangibility between pointers and arrays at
 *	 the same level.  this would break because it made char a[5][2] look
 *	 equivelent to char *a[].  so now, we allow interchangibility at any
 *	 given level, but when one of the types involved is a pointer, we check
 *	 the prior level to make sure they're pointers to the same kind of thing.
 *	 also, things get weird because in the case of char a[5][2], our caller
 *	 (coerce_expression) has already changed the [2] to a TYPE_POINTER, so
 *	 you end up with a[2][2] looking like TYPE_CHAR [2] TYPE_POINTER and
 *	 *a[2] looks like TYPE_CHAR TYPE_POINTER TYPE_POINTER.	As we cruise
 *	 along, the [2] and the TYPE_POINTER at level two are allowed to be
 *	 interchangible, but when we get to the next level, where we have
 *	 TYPE_POINTER for both elements, we still have to look back a level
 *	 to see that they are pointers to different things.
 ****************************************************************************/
{
int i;
int count;
TypeComp stc;
TypeComp dtc;

count = s->comp_count;
if (count != d->comp_count)
	return(FALSE);
for (i=start; i<count; i++)
	{
	if ((stc = s->comp[i]) != (dtc = d->comp[i]))
		{
		if (! ((stc == TYPE_POINTER && dtc == TYPE_ARRAY)  ||
			   (stc == TYPE_ARRAY	&& dtc == TYPE_POINTER)) )
			return FALSE; /* not array/ptr intermixing, just a plain mismatch */
		}

	if (stc == TYPE_POINTER || dtc == TYPE_POINTER)
		if (i > start)	/* should never be <=, but hate to hit -1 element */
			if (s->comp[i-1] != d->comp[i-1])
				return FALSE;

	if (stc == TYPE_FUNCTION)
		{
		if (!po_fuf_types_same(s->sdims[i].pt, d->sdims[i].pt))
			return(FALSE);
		}
	}
return(TRUE);
}


void po_print_type(Poco_cb *pcb, FILE *f, Type_info *ti)
/*****************************************************************************
 * print contents of type_info, used by tracer routines.
 * does not handle members of structs, but does do params from protos.
 * (Note to self: it could do structs easily enough, if there's a need.)
 ****************************************************************************/
{
int i;
TypeComp tc;
Func_frame *fuf;
Symbol *param;

for (i=0; i<ti->comp_count; i++)
	{
	switch (tc = ti->comp[i])
		{
		case TYPE_ARRAY:
			fprintf(f, "[%ld] ", ti->sdims[i].l);
			break;
		case TYPE_FUNCTION:
			fprintf(f, "%s(", base_type_names[tc].name);
			fuf = ti->sdims[i].pt;
			param = fuf->parameters;
			while (param != NULL)
				{
				po_print_type(pcb, f, param->ti);
				param = param->link;
				if (param != NULL)
					fprintf(f, ",");
				}
			fprintf(f, ") ");
			break;
		default:
			fprintf(f, "%s ", base_type_names[tc].name);
			break;
		}
	}
}


long po_get_type_size(Type_info *ti)
/*****************************************************************************
 * return the size of the data associated with a given type. does struct/array.
 ****************************************************************************/
{
int i;
TypeComp tc;
long size = 0;
long elsize;

for (i=0; i<ti->comp_count; i++)
	{
	tc = ti->comp[i];
	elsize = base_type_names[tc].size;
	if (elsize == 0)
		{
		switch (tc)
			{
			case TYPE_ARRAY:
				elsize = size*ti->sdims[i].l;
				break;
			case TYPE_STRUCT:
				elsize =
					((Struct_info *)(ti->sdims[i].pt))->size;
				break;
			}
		}
	size = elsize;
	}
return(size);
}

long po_get_subtype_size(Poco_cb *pcb, Type_info *ti)
/*****************************************************************************
 * get size of data associated with the sub-type of the given type_info.
 ****************************************************************************/
{
long ret;

ti->comp_count -= 1;
ret = po_get_type_size(ti);
ti->comp_count += 1;
return(ret);
}


/******** MODULE TYPE_PARSER *****************************/


Boolean po_get_base_type(Poco_cb *pcb, Poco_frame *pf, Type_info *ti)
/*****************************************************************************
 * get the base type of a declaration.
 ****************************************************************************/
{
static char signed_and_unsigned[] = "cannot specify both signed and unsigned.";
static char long_and_short[]	  = "cannot specify both long and short";
SHORT		type_token;
Struct_info *sif;
UBYTE		flags = 0;
UBYTE		comp;

ti->comp_count = 1;
comp = TYPE_BAD;

for (;;)
	{
	if (pcb->t.toktype == PTOK_USER_TYPE)
		{
		po_copy_type(pcb, pcb->curtoken->val.symbol->ti, ti);
		comp = ti->comp[ti->comp_count-1];
		break;
		}
	else if (pcb->t.toktype != PTOK_TYPE)
		{
		pushback_token(&pcb->t);
		break;
		}
	else
		{
		type_token = pcb->curtoken->val.symbol->symval.i;
		switch (type_token)
			{
			case TYPE_REGISTER: 			/* We ignore all these... */
			case TYPE_AUTO:
			case TYPE_EXTERN:
			case TYPE_CONST:
			case TYPE_VOLATILE:
				break;

			case TYPE_STATIC:				/* set static flag for later */
				flags |= TFL_STATIC;
				break;

			case TYPE_SIGNED:				/* set signed flag if not unsigned */
				if (flags & TFL_UNSIGNED)
					po_say_fatal(pcb, signed_and_unsigned);
				flags |= TFL_SIGNED;
				break;

			case TYPE_UNSIGNED: 			/* set unsigned flag if not signed */
				if (flags & TFL_SIGNED)
					po_say_fatal(pcb, signed_and_unsigned);
				flags |= TFL_UNSIGNED;
				break;

			case TYPE_LONG: 				/* set long flag if not short	*/
				if (flags & TFL_SHORT)
					po_say_fatal(pcb, long_and_short);
				flags |= TFL_LONG;
				break;

			case TYPE_SHORT:			   /* set short flag if not long	*/
				if (flags & TFL_SHORT)
					po_say_fatal(pcb, long_and_short);
				flags |= TFL_SHORT;
				break;

			case TYPE_STRUCT:				/* handle structs and such		*/
			case TYPE_UNION:
			case TYPE_ENUM:
				sif = po_get_struct(pcb, pf, type_token);
				if (type_token == TYPE_ENUM)
					{
					comp = TYPE_INT;
					}
				else
					{
					ti->sdims[0].pt = sif;
					comp = TYPE_STRUCT;
					}
				break;

			default:					/* else it's an integral type   */
				comp = type_token;
				break;
			}
		po_need_token(pcb);
		}
	}

/*
 * if we saw the word 'long'...
 *	 if type is unknown or int, it becomes long int
 *	 if type is double, we don't complain about seeing long double
 *	 else complain and die (eg, we got 'long struct' or other nonsense)
 */

if (flags & TFL_LONG)
	{
	switch (comp)
		{
		case TYPE_BAD:
		case TYPE_INT:
			comp = TYPE_LONG;
			break;
		case TYPE_DOUBLE:
			break;
		default:
			po_say_fatal(pcb, "long cannot be specified for this type");
			break;
		}
	}
else
	{
/*
 * if we saw the word 'short'...
 *	 if the type is unknown or int, it becomes short
 *	 else we complain and die
 */
	if (flags & TFL_SHORT)
		{
		switch (comp)
			{
			case TYPE_BAD:
			case TYPE_INT:
				comp = TYPE_SHORT;
				break;
			default:
				po_say_fatal(pcb, "short cannot be specified for this type");
				break;
			}
		}
	}

/*
 * if we saw the words 'signed' or 'unsigned'...
 *	 if the type was unknown, it becomes int
 *	 if the type was char, int, or long int, it doesn't change
 *	 else we complain and die
 */

if (flags & (TFL_SIGNED|TFL_UNSIGNED))
	{
	switch (comp)
		{
		case TYPE_BAD:
			comp = TYPE_INT;
			break;
		case TYPE_CHAR:
		case TYPE_SHORT:
		case TYPE_INT:
		case TYPE_LONG:
			break;
		default:
			po_say_fatal(pcb, "signed/unsigned cannot be specified for this type");
			break;
		}
	}

ti->flags = flags & TFL_STATIC; 	/* we export only the static flag */
ti->comp[ti->comp_count-1] = comp;	/* save any mods to the base type */

if (comp != TYPE_BAD)
	{
	po_set_ido_type(ti);
	return TRUE;
	}
else
	{
	return FALSE;
	}
}

Symbol *po_need_local_symbol(Poco_cb *pcb)
/*****************************************************************************
 * ensure next token is a local symbol, make one if token is PTOK_UNDEF.
 * this is used in handling labels for gotos, regular symbols are handled
 * by po_new_symbol().
 * (Note to self:  maybe labels and regular symbols can be handled one way?)
 ****************************************************************************/
{
Symbol *var;
short type;

if (po_need_token(pcb))
	{
	type = pcb->t.toktype;
	switch (type)
		{
		case PTOK_VAR:
			if (pcb->curtoken->val.symbol->scope < pcb->rframe->scope)
				/* make new sybol */
				{
				if ((var = po_new_symbol(pcb,
					pcb->curtoken->val.symbol->name)) == NULL)
					return(NULL);
				return(var);
				}
			else
				{
				po_redefined(pcb, pcb->curtoken->val.symbol->name);
				return(NULL);
				}
		case PTOK_UNDEF:
			if (pcb->curtoken->val.symbol->scope < pcb->rframe->scope)
				{
				if ((var = po_new_symbol(pcb,
					pcb->curtoken->val.symbol->name)) == NULL)
					return(NULL);
				return(var);
				}
			else
				return(pcb->curtoken->val.symbol);
		default:
			po_expecting_got(pcb, "label name");
		}
	}
return(NULL);
}

Type_info *po_typi_type(Itypi *tip)
/*****************************************************************************
 * init the type_info embedded in an Itypei structure, return ptr to it.
 ****************************************************************************/
{
tip->iti.comp = tip->typec;
tip->iti.sdims = tip->dimc;
tip->iti.comp_count = 0;
tip->iti.comp_alloc = MAX_TYPE_COMPS;
tip->iti.flags = 0;
return(&tip->iti);
}


