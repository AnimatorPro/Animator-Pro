/* softmenu.c - stuff to read in pulldowns and numbered item menus from
 * ascii resource file.
 *
 * The format of the resource file is:
 * 		Class item
 *		{
 *		stuff specific to class
 *		}
 *
 * The only restriction to 'stuff specific to class' is that it
 * must have a matching number of {'s and }'s except in strings,
 * character constants, and line comments (anything after //)
 *
 * Softmenu will generate a list of items for each class from the
 * file it reads in.  All it saves at this point is the item name and file
 * offset of the byte past the opening brace.
 * 
 * Typical use is something like:
 *
 * Softmenu sm;
 * 
 * smu_init(&sm,resource_file_name);
 * smu_load_XXX(&sm, "item");
 * smu_cleanup(&sm);
 *
 * An smu_load_XXX must be written for each class.  Normally this will
 * be done using the swork_ family of routines to tokenize.  Look for
 * their prototypes in softok.h.
 *
 */

#include <string.h>
#include "commonst.h"
#include "errcodes.h"
#include "linklist.h"
#include "memory.h"
#include "softmenu.h"

static Errcode new_smu_symbol(Smu_symbol **psym, Smu_class *class, 
	char *name)
{
Smu_symbol *newsym;
int len = strlen(name)+1;

if ((*psym = newsym = pj_malloc(sizeof(*newsym)+len)) == NULL)
	return(Err_no_memory);
newsym->next = class->symbols;
class->symbols = newsym;
strcpy(newsym->name = (char *)(newsym+1), name);
newsym->foff = newsym->fsize = 0;
return(Success);
}


static int class_array_ix(Smu_class *array, int asize, char *name)
{
int i;
for (i=0; i<asize; i+=1)
	{
	if (strcmp(array[i].name,name) == 0)
		return(i);
	}
return(Err_not_found);
}

#ifdef SLUFFED
static Errcode skip_past(Swork *swork, int ttype)
{
SHORT tp;

for (;;)
	{
	swork_advance(swork,1);
	tp = swork_top(swork)->ttype;
	if (tp == ttype)
		{
		swork_advance(swork,1);
		return(Success);
		}
	if (tp == TOK_EOF)
		return(Err_eof);
	}
}
#endif /* SLUFFED */

static Errcode skip_to_match(Swork *swork, int uptype, int downtype,
	int ipos)
{
SHORT ts;

for (;;)
	{
	ts = swork_top(swork)->ttype;
	if (ts == uptype)
		ipos+=1;
	else if (ts == downtype)
		ipos-=1;
	else if (ts == TOK_EOF)
		return(Err_unmatched_brace);
	if (ipos <= 0)
		break;
	swork_advance(swork,1);
	}
return(Success);
}

static Errcode smu_parse_one(Swork *swork, Softmenu *s)
{
Errcode err = Success;
int classix;
Smu_symbol *sym;

if (swork_top(swork)->ttype == TOK_EOF)
	return(Err_eof);
if (swork_top(swork)->ttype == TOK_UNDEF 
	&& swork_tok(swork,1)->ttype == TOK_UNDEF
	&& swork_tok(swork,2)->ttype == TOK_LBRACE)
	{
	if ((err = classix = 
		class_array_ix(s->classes, s->class_count, swork_top(swork)->d.buf)) 
		< Success)
		{
		goto ERR;
		}
	if ((err = new_smu_symbol(&sym, &s->classes[classix], 
		swork_tok(swork,1)->d.buf))
		< Success)
		goto ERR;
	swork_advance(swork,3);
	sym->foff = swork_top(swork)->tpos;
	sym->fline = swork_top(swork)->tline;
	if ((err = skip_to_match(swork,TOK_LBRACE,TOK_RBRACE,1))<Success)
		goto ERR;
	sym->fsize = swork_top(swork)->tpos - sym->foff;
	swork_advance(swork,1);
	}
else
	{
	err = Err_mu_syntax;
	}
ERR:
	{
	return(err);
	}
}


void swork_end(Swork *swork, Softmenu *sm)
	/* like swork_cleanup but transfers error line # to sm */
{
sm->err_line = swork_top(swork)->tline;
swork_cleanup(swork);
}

static Errcode smu_parse(struct softmenu *sm)
{
Errcode err;
Swork rswork;

swork_init(&rswork, sm->xf, 0L, 0L);
while ((err = smu_parse_one(&rswork,sm)) >= Success)
	;
swork_end(&rswork,sm);
return(err);
}

static Errcode smu_init_classes(struct softmenu *smu,	/* constructor */
	char **class_names,				/* names of all classes */
	unsigned int class_count,		/* total # of classes */
	char *resource_file)			/* resource file name */
/* initialize softmenu with a class-list */
{
Errcode err = Success;
unsigned int i;

clear_struct(smu);
if ((smu->classes = pj_zalloc(class_count*sizeof(*smu->classes))) == NULL)
	{
	err = Err_no_memory;
	goto OUT;
	}
smu->class_count = class_count;
for (i=0; i<class_count; i++)
	smu->classes[i].name = class_names[i];
if ((err = xffopen(resource_file, &smu->xf, XREADONLY)) < Success)
	goto OUT;
if ((err = smu_parse(smu)) < Success)
	goto OUT;
OUT:
if (err == Err_eof)
	err = Success;
if (err < Success)
	smu_cleanup(smu);
return(err);
}

char *smu_class_names[] =
	{
	"Strings",
	"Qchoice",
	"NumString",
	"Pull",
	"Text",
	"NameString",
	};

Errcode smu_init(struct softmenu *smu,	/* constructor */
	char *resource_file)			/* resource file name */
{
return(smu_init_classes(smu, 
	smu_class_names, Array_els(smu_class_names), resource_file));
}

void smu_cleanup(struct softmenu *sm)	/* destructor */
{
int i;
Smu_class *cl = sm->classes;

xffclose(&sm->xf);
i = sm->class_count;
while (--i >=0)
	{
	free_slist((Slnode *)(cl->symbols));
	cl += 1;
	}
pj_freez(&sm->classes);
sm->class_count = 0;
return;
}

Errcode smu_lookup(struct softmenu *sm,	/* Find symbol */ 
	struct smu_symbol **psym,		/* return symbol here */
	unsigned class,					/* symbol class */
	char *name) 					/* symbol name */
{
if (class >= sm->class_count)
	return(Err_no_such_class);
if ((*psym = (Smu_symbol *)name_in_list(name, 
	(Names *)(sm->classes[class].symbols))) == NULL)
	return(Err_not_found);
return(Success);
}

