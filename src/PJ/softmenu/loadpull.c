/* loadpull.c - Parse SoftMenu Pull objects.  They have the following
 * format:
 * 		{
 *		"leaf name1",leaf_id,[2 key equiv]
 *			{
 *			"item name1",item_id,[2 key equiv],[1 key equiv]
 *			"item name2",item_id,[2 key equiv],[1 key equiv]
 *			...
 *			}
 *		"leaf name1",leaf_id,[2 key equiv]
 *			{
 *			...
 *			}
 *		...
 *		}
 * The two key equivalents are not given, the first letter of the
 * name string is used.  If you wish to include a 1 key equivalent,
 * you have to explicitly state the 2 key equiv. as well so I can
 * do the parsing.
 *
 * As with most SoftMenu things (hopefully) the commas can be
 * replaced with white space with the same result.
 */

#include "menus.h"
#include "softmenu.h"
#include "ptrmacro.h"
#include "memory.h"
#include "linklist.h"
#include "errcodes.h"


static void rfree_pull(Pull *p)
/* recursively free a pull and all it's siblings and children. */
{
if (p == NULL)
	return;
rfree_pull(p->next);
rfree_pull(p->children);
pj_free(p);
}

Errcode new_pull(Pull **ppull, char *inits)
/* allocate and initialize a Pull. 
 * If the string inits is non-NULL, then
 * allocate extra space at end of pull for 
 * string, and set the Pull data pointer. */
{
int slen = 0;
Errcode err;
Pull *p = NULL;

if (inits != NULL)
	slen = strlen(inits)+1;
if ((err = ealloc(&p,sizeof(Pull)+slen)) < Success)
	goto OUT;
if (inits != NULL)
	strcpy(p->data = (p+1), inits);
OUT:
*ppull = p;
return(err);
}

static void smu_read_opt_short(Swork *swork, SHORT *pt)
/* Check to see if next token is a number, and if so read it and put it
 * into *pt  */
{
if (swork_topt(swork) == TOK_LONG)	/* if got number it's 2 key equiv */
	{
	*pt = swork_top(swork)->longval;
	swork_advance_over(swork,',');	/* skip over a comma if it's there */
	}
}

static Errcode sp_pull_from_tops(Swork *swork, Pull **pp)
/* initialize a pull from the next string in input.  Look for optional
 * numbers after the string. */
{
Pull *p;
Errcode err = Success;

if (swork_topt(swork) != TOK_QUO)
	return(Err_expecting_string);
if ((err = new_pull(pp,swork_tops(swork))) < Success)
	goto OUT;
p = *pp;
swork_advance_over(swork,',');	/* skip over string & a comma if it's there */
if (swork_topt(swork) != TOK_LONG)
	return(Err_expecting_id);
p->id = swork_top(swork)->longval;
swork_advance_over(swork,',');	/* skip over id & a comma if it's there */
p->key2 = ((char *)(p->data))[0];		/* set up default 2 key equiv */
if (p->key2 == '*' || p->key2 == ' ')
	p->key2 = ((char *)(p->data))[1];
smu_read_opt_short(swork,&p->key2);
smu_read_opt_short(swork,&p->key_equiv);
OUT:
return(err);
}

static Errcode sp_items(Swork *swork, Pull **pitem)
/* Read in successive Pull's until closing brace. 
 * Put result in singly linked list *pitem */
{
Errcode err = Success;

for (;;)
	{
	if ((err = sp_pull_from_tops(swork,pitem)) < Success)
		goto OUT;
	if (swork_topt(swork) == TOK_RBRACE)
		goto OUT;
	pitem = &(*pitem)->next;
	}
OUT:
return(err);
}

static Errcode sp_get_leaf(Swork *swork,  Pull **leaf)
/* Read in the leaf title pull.  Make up the dummy pull that draws the
 * solid box behind all the items, and then read in the items. */
{
Errcode err = Success;
Pull *rootp;
Pull *boxp;

/* get the struct for the word on the menu bar, and the box behind
   all the items */
if ((err = sp_pull_from_tops(swork,leaf)) < Success)
	goto OUT;
rootp = *leaf;
if ((err = new_pull(&boxp,NULL)) < Success)
	goto OUT;
rootp->children = boxp;
if (swork_topt(swork) != TOK_LBRACE)
	{
	err = Err_expecting_lbrace;
	goto OUT;
	}
swork_advance(swork,1);
if ((err = sp_items(swork, &boxp->children)) < Success)
	goto OUT;
if (swork_topt(swork) != TOK_RBRACE)
	{
	err = Err_expecting_rbrace;
	goto OUT;
	}
swork_advance_over(swork,',');
OUT:
return(err);
}

static Errcode sp_parse(Swork *swork, Pull **pitem)
/* Read a list of Pull-leafs until closing brace.  Put result in
 * *pitem */
{
Errcode err = Success;
SHORT tt;

for (;;)
	{
	tt = swork_top(swork)->ttype;
	if (tt == TOK_RBRACE)
		break;
	if (tt == TOK_EOF)
		{
		err = Err_unmatched_brace;
		break;
		}
	if ((err = sp_get_leaf(swork,pitem)) < Success)
		break;
	pitem = &(*pitem)->next;
	}
return(err);
}

Errcode smu_load_pull(struct softmenu *sm,			/* read in a pulldown */
	char *symname,					/* name of symbol */
	struct menuhdr *pullhdr)		/* place to put loaded pulldown */
/* Allocates and reads in a pull-down from a file.  Does just about
 * everything except calculate the pixel coordinates (see pullfmt.c)
 * and install the dodata and domenu functions */
{
Errcode err = Success;
Swork rswork;
Smu_symbol *sym;

if ((err = smu_lookup(sm,&sym,SMU_PULL_CLASS,symname)) < Success)
	return(err);
swork_init(&rswork, sm->sf, sym->foff, sym->fline);
clear_struct(pullhdr);
if ((err = sp_parse(&rswork, &pullhdr->mbs)) < Success)
	goto OUT;
pullhdr->type = PULLMENU;
pullhdr->font = SCREEN_FONT;
pullhdr->cursor = SCREEN_CURSOR;
pullhdr->seebg = seebg_ulwhite;
pullhdr->ioflags =  (KEYHIT|MMOVE|MBRIGHT);  /* input wanted flags */
pullhdr->flags = MENU_KEYSONHIDE;
OUT:
if (err < Success)
	smu_free_pull(pullhdr);
swork_end(&rswork,sm);
return(err);
}

void smu_free_pull(struct menuhdr *pullhdr)
{
rfree_pull(pullhdr->mbs);
pullhdr->mbs = NULL;
}


