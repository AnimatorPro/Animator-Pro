/****************************************************************************
*
* postring.c - This file supports the String type in Poco programs.
* Strings in Poco are (String_ref *) here.  Each string has a reference
* count associated with it.  Most string expressions just involve
* bumping the reference count up and down though some, like string
* concatenation, will create new String_ref's.  When the reference
* count goes to zero the String_ref is freed.
*****************************************************************************/
#include "poco.h"
#include "linklist.h"

#ifdef STRING_EXPERIMENT

#ifdef SHOW_REF_COUNT
#define DDT(a) {printf a;}
#else
#define DDT(a)
#endif /* SHOW_REF_COUNT */


void po_add_local_string(Poco_cb *pcb, Poco_frame *pf, Symbol *symbol)
/*****************************************************************************
 * Allocate and initialize a string-list if symbol is a simple string type.
 ****************************************************************************/
{
Local_string *new;
Type_info *ti;

ti = symbol->ti;
if (ti->comp[0] == TYPE_STRING && ti->comp_count == 1)
	{
	new = po_memalloc(pcb, sizeof(*new));
	new->string_symbol = symbol;
	new->next = pf->local_string_list;
	pf->local_string_list = new;
	}
}

void po_free_local_string_list(Poco_cb *pcb, Poco_frame *pf)
{
Local_string *list, *next;

next = pf->local_string_list; 
while ((list = next) != NULL)
	{
	next = list->next;
	po_freemem(list);
	}
}

void po_code_free_string_ops(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * Generate an OP_FREE_STRING for each local_string in Poco_frame.
 ****************************************************************************/
{
Local_string *list;

for (list = pf->local_string_list; list != NULL; list = list->next)
	{
	po_code_int(pcb, &pf->fcd, OP_FREE_STRING
	,list->string_symbol->symval.doff);
	}
}
/******* Start of run-time code ********/

String_ref *po_sr_new(int len)
/*****************************************************************************
 * Allocate a new reference for a buffer
 ****************************************************************************/
{
Popot popot_ref;
Popot popot_string;
String_ref *ref;

DDT(("po_sr_new(%d)\n", len));
popot_ref = po_malloc(sizeof(String_ref));
if ((ref = popot_ref.pt) == NULL)
	{
	builtin_err = Err_poco_out_of_string_space;
	return(NULL);
	}
ref->string = popot_string = po_malloc(len);
if (popot_string.pt == NULL)
	{
	builtin_err = Err_poco_out_of_string_space;
	return(NULL);
	}
ref->ref_count = 1;
return(ref);
}

String_ref *po_sr_new_copy(char *pt, int len)
/*****************************************************************************
 * Allocate a new reference for a buffer and initialize it from pt.
 ****************************************************************************/
{
String_ref *ret;

DDT(("po_sr_new_copy(\"%s\", %d)\n", pt, len));
if ((ret = po_sr_new(len)) != NULL)
	poco_copy_bytes(pt, ret->string.pt, len);
return(ret);
}

String_ref *po_sr_new_string(char *pt)
/*****************************************************************************
 * Allocate a new reference for a null terminated character array  
 ****************************************************************************/
{
return(po_sr_new_copy(pt, strlen(pt)+1));
}

String_ref *po_sr_cat(String_ref *a, String_ref *b)
/*****************************************************************************
 * Allocate a new string reference for the concatenation of a & b
 ****************************************************************************/
{
String_ref *ret;
int lena, lenb;
char *ret_buf;
#define Popot_bufsize(p)			((char *)((p)->max)-(char *)((p)->pt)+1)

/* Concatenation of two NULL strings is NULL */
if (a == NULL && b == NULL)
	return(NULL);
/* Concatenation of a NULL and a non-null is the non-null */
if (a == NULL)
	{
	po_sr_inc_ref(b);
	return(b);
	}
if (b == NULL)
	{
	po_sr_inc_ref(a);
	return(a);
	}
/* Concatenation of two non-null strings takes a little work... */
lena = strlen(a->string.pt);
lenb = strlen(b->string.pt);
if ((ret = po_sr_new(lena+lenb+1)) != NULL)
	{
	ret_buf = ret->string.pt;
	poco_copy_bytes(a->string.pt, ret_buf, lena);
	ret_buf += lena;
	poco_copy_bytes(b->string.pt, ret_buf, lenb);
	ret_buf[lenb] = 0;			/* add zero tag at end */
	}
return(ret);
}


void po_sr_inc_ref(String_ref *ref)
{
if (ref != NULL)
	{
	++ref->ref_count;
	DDT(("po_sr_inc_ref(\"%s\") ref_count = %d\n", ref->string.pt, ref->ref_count));
	}
else
	DDT(("po_sr_inc_ref(NULL)\n"));
}

void po_sr_destroy(String_ref *ref)
/*****************************************************************************
 * Destroy string ref and the string data too.
 ****************************************************************************/
{
Popot popot_ref;

po_free(ref->string);
popot_ref.pt = ref;	/* Po_free wants a popot, not a pointer */
po_free(popot_ref);
}

Boolean po_sr_clean_ref(String_ref *ref)
/* Decrement the reference count and if down to zero destroy string. 
 * Return TRUE if string destroyed. */
{
DDT(("po_sr_clean_ref(\"%s\")\n", (ref==NULL ? "(NULL)" : ref->string.pt)));
if (ref != NULL)
	{
	po_sr_dec_ref(ref);
	if (ref->ref_count <= 0)
		{
		po_sr_destroy(ref);
		return(TRUE);
		}
	}
}


void po_sr_dec_ref(String_ref *ref)
{
if (ref != NULL)
	{
	--ref->ref_count;
	DDT(("po_sr_dec_ref(\"%s\") ref = %d\n", ref->string.pt, ref->ref_count));
	}
else
	DDT(("po_sr_dec_ref(NULL)\n"));
}

Boolean po_sr_eq(String_ref *a, String_ref *b)
/* Are two strings the same */
{
if (a == b)
	return TRUE;
else if (a == NULL || b == NULL)
	return FALSE;
else
	return(strcmp(a->string.pt,b->string.pt) == 0);
}


Boolean po_sr_ge(String_ref *a, String_ref *b)
/* return a >= b */
{
if (a == b)
	return(FALSE);
else if (a == NULL)	/* treat NULL as very small */
	return(FALSE);
else if (b == NULL)
	return(TRUE);
else
	return(strcmp(a->string.pt, b->string.pt) >= 0);
}


Boolean po_sr_le(String_ref *a, String_ref *b)
/* return a <= b */
{
if (a == b)
	return(FALSE);
else if (a == NULL)	/* treat NULL as very small */
	return(TRUE);
else if (b == NULL)
	return(FALSE);
else
	return(strcmp(a->string.pt, b->string.pt) <= 0);
}

Boolean po_sr_eq_and_clean(String_ref *a, String_ref *b)
{
Boolean ret = po_sr_eq(a,b);
po_sr_clean_ref(a);
po_sr_clean_ref(b);
return(ret);
}

Boolean po_sr_ge_and_clean(String_ref *a, String_ref *b)
{
Boolean ret = po_sr_ge(a,b);
po_sr_clean_ref(a);
po_sr_clean_ref(b);
return(ret);
}

Boolean po_sr_le_and_clean(String_ref *a, String_ref *b)
{
Boolean ret = po_sr_le(a,b);
po_sr_clean_ref(a);
po_sr_clean_ref(b);
return(ret);
}

String_ref *po_sr_cat_and_clean(String_ref *a, String_ref *b)
{
String_ref *ret = po_sr_cat(a,b);
po_sr_clean_ref(a);
po_sr_clean_ref(b);
return(ret);
}
#endif /* STRING_EXPERIMENT */
