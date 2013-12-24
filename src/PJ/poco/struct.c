/*****************************************************************************
 *
 * struct.c - Keeps track of struct tags and member names/offsets/types.
 *
 *	08/25/90	(Ian)
 *				Changed all occurances of freemem() and gentle_freemem() to
 *				po_freemem() and poc_gentle_freemem().
 *	09/18/90	(Ian)
 *				Added po_move_sifs_to_parent() function to fix handling of
 *				nested structures.	Added support for unions.
 *	09/20/90	(Ian)
 *				Implemented the ANSI special rule for structure scoping: a
 *				definition of the form "struct tagname;" will build a new
 *				incomplete definition at the local scope even if the tag
 *				already exists at an outer scope.
 *				Also, fixed a 'do forever' loop bug in pf_to_sif() that
 *				would occur when struct tags were defined in nested struct
 *				defintions.
 *	10/01/90	(Ian)
 *				Added support for enum as a bastardized form of structure.
 *				Note that we are not 100% compliant with ANSI on enums: you
 *				should not be able to declare an incomplete enum defintion,
 *				and we allow it, since enums are treated like structs.	I've
 *				noticed that a couple other major compilers act this way.
 *	10/05/90	(Ian)
 *				Converted sprintf/po_say_fatal pairs to po_say_fatal w/formatting.
 *	10/07/90	(Ian)
 *				Removed NULL checks from calls to memory allocation.
 ****************************************************************************/

#include <string.h>
#include "poco.h"

static Struct_info *in_sif_list(register Struct_info *l, char *name)
/*****************************************************************************
 * find structure in linked list, return pointer to struct_info.
 ****************************************************************************/
{
while (l != NULL)
	{
	if (l->name[0] == *name)	/* quick-check 1st char before strcmp call */
		if (po_eqstrcmp(l->name,name) == 0)
			return(l);
	l = l->next;
	}
return(NULL);
}

static Struct_info *find_sif(Poco_cb *pcb, Poco_frame *pf, char *name)
/*****************************************************************************
 * find structure by searching from innermost scope outwards until found.
 ****************************************************************************/
{
Struct_info *sif;

while (pf != NULL)
	{
	if ((sif = in_sif_list(pf->fsif, name)) != NULL)
		return(sif);
	pf = pf->next;
	}
return(NULL);
}

static Struct_info *new_sif(Poco_cb *pcb, Poco_frame *pf, char *name)
/*****************************************************************************
 * alloc and init a new struct_info, attach it to the parent poco_frame.
 ****************************************************************************/
{
Struct_info *new;

new = po_memzalloc(pcb, sizeof(*new) + strlen(name) + 1);
new->name = (char *)(new+1);
strcpy(new->name, name);
new->next = pf->fsif;
pf->fsif = new;
return(new);
}

static void free_sif(Struct_info *sif)
/*****************************************************************************
 * free struct_info, and symbols (member names) attached to it.
 ****************************************************************************/
{
if (sif != NULL)
	{
	po_free_symbol_list(&sif->elements);
	po_freemem(sif);
	}
}

void po_free_sif_list(Struct_info **psif)
/*****************************************************************************
 * free a linked list of stuct_infos.
 ****************************************************************************/
{
Struct_info *sif, *next;

next = *psif;
while ((sif=next) != NULL)
	{
	next = next->next;
	free_sif(sif);
	}
*psif = NULL;
}

static void pf_to_sif(Poco_cb *pcb, Poco_frame *pf, Struct_info *sif, SHORT ttype)
/*****************************************************************************
 * move symbols from poco_frame to members list of struct_info.  calc sizes.
 ****************************************************************************/
{
Symbol *s, *link;
long size;

s = pf->symbols;
while (s != NULL)
	{
	link = s->link;
	if (s->tok_type == PTOK_VAR)
		{
		s->next = s->link = sif->elements;
		sif->elements = s;
		}
	else
		{
		po_free_symbol(s);
		}
	s = link;
	}
pf->symbols = NULL;
s = sif->elements;
while (s != NULL)
	{
	sif->el_count+=1;
	size = po_get_type_size(s->ti);
	if (size == 0)
		{
		po_say_fatal(pcb, "element %s in structure/union is of unknown size/type", s->name);
		}
	if (ttype == TYPE_STRUCT)
		{
		s->symval.doff = sif->size;
		sif->size += size;
		}
	else
		{
		s->symval.doff = 0;
		if (sif->size < size)
			sif->size = size;
		}
	s = s->next;
	}
}

void po_move_sifs_to_parent(Poco_cb *pcb)
/*****************************************************************************
 * move any struct_infos from current poco_frame to parent (global/func) frame.
 * call this function only if:
 *	the current frame is an FTY_STRUCT type frame
 *	it is known that at least one sif is tied to the current frame
 ****************************************************************************/
{
Struct_info *sifs;			/* -> last sif in chain tied to current frame */
Poco_frame	*pf;			/* -> current frame */
Poco_frame	*rf;			/* -> parent frame */

rf = pf = pcb->rframe;
while(rf->frame_type == FTY_STRUCT) /* find parent func/global frame */
	rf = rf->next;

sifs = pf->fsif;
while(sifs->next != NULL)	/* find end of sif chain */
	sifs = sifs->next;

sifs->next = rf->fsif;		/* concatenate sif chain of current 	*/
rf->fsif = pf->fsif;		/* frame ahead of chain on parent frame 	*/

pf->fsif = NULL;			/* prevent chain from being free'd      */
}

static void get_enum_block(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * process enum constant declarations until we see a closing brace.
 ****************************************************************************/
{
Exp_frame	ef;
Symbol		*s;
PToken_t	ttype;
int 		curval = 0;

do	{
	po_need_token(pcb);
	ttype = pcb->t.toktype;
	switch (ttype)
		{
		case PTOK_UNDEF:
			s = pcb->curtoken->val.symbol;
			s->tok_type = PTOK_ENUMCONST;
			po_need_token(pcb);
			if (pcb->t.toktype != '=')
				{
				pushback_token(&pcb->t);
				}
			else
				{
				po_init_expframe(pcb, &ef);
				po_get_expression(pcb, &ef);
				curval = po_eval_const_expression(pcb, &ef);
				po_trash_expframe(pcb, &ef);
				}
			s->symval.i = curval++;
			ttype = po_need_comma_or_brace(pcb);
			break;
		case TOK_RBRACE:
			break;
		case PTOK_VAR:
			po_say_fatal(pcb, "enum constant name redefined");
			break;
		default:
			po_expecting_got(pcb, "name of enum constant or }");
			break;
		}
	} while (ttype != TOK_RBRACE);
}

Struct_info *po_get_struct(Poco_cb *pcb, Poco_frame *pf, SHORT struct_union_ttype)
/*****************************************************************************
 * parse a struct/union statement ('struct' has been seen, this handles it).
 ****************************************************************************/
{
Struct_info *sif = NULL;
SHORT ttype;
char *name;

lookup_token(pcb);
ttype = pcb->t.toktype;

switch (ttype)
	{
	case TOK_LBRACE:

		sif = new_sif(pcb, pf, "");
		pushback_token(&pcb->t);
		break;

	case PTOK_UNDEF:
	case PTOK_VAR:
	case PTOK_LABEL:

		name = pcb->curtoken->val.symbol->name;
		sif  = find_sif(pcb, pf, name);
		if (sif != NULL)							/* Handle strange ANSI	*/
			{										/* rule:  if struct tag */
			lookup_token(pcb);						/* exists at another	*/
			if (pcb->t.toktype == ';')             /* scope, and this def  */
				{									/* is 'struct name;',   */
				sif = in_sif_list(pf->fsif, name);	/* then we build a new	*/
				}									/* incomplete definition*/
			pushback_token(&pcb->t);				/* at the current scope.*/
			}
		if (sif == NULL)
			sif = new_sif(pcb, pf, name);
		break;

	default:
		po_say_fatal(pcb, "malformed name for struct/union/enum...");
		goto ERROR;
	}

lookup_token(pcb);

if (pcb->t.toktype == TOK_LBRACE)
	{
	if (sif->size != 0)
		{
		po_say_fatal(pcb, "struct/union/enum tag redefined");
		goto ERROR;
		}
	if (struct_union_ttype == TYPE_ENUM)
		{
		get_enum_block(pcb, pf);
		sif->size = -1; 		/* used only to help catch dup tags */
		}
	else
		{
		po_new_frame(pcb, pf->scope+1, sif->name, FTY_STRUCT);
		po_get_block(pcb, pcb->rframe);
		pf_to_sif(pcb, pcb->rframe, sif, struct_union_ttype);
		if (pcb->rframe->fsif != NULL)
			po_move_sifs_to_parent(pcb);
		po_old_frame(pcb);
		}
	sif->type = struct_union_ttype;
	}
else
	pushback_token(&pcb->t);

return(sif);

ERROR:
return(NULL);
}
