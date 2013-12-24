/* namestr.c - deals with soft menu items of form:
	NameStr symbol
		{
		name1, "Some string 1"
		name2, "Some really long string that has to go"
			   "over a couple of lines"
		name3, "another string"
		}
   We use these to group together messages where we don't want to
   put a million of them in individual text things.
 */
#include "softmenu.h"
#include "errcodes.h"
#include "ptrmacro.h"
#include "memory.h"



static Errcode smu_do_name_things(struct softmenu *sm, 
						   char *symname,
						   Errcode (*dothing)(Swork *swork,void *data),
						   void *data)
/*************************************************************************
 * Scan through name/data pairs calling dodata for each name until we reach
 * the terminal '}' or dodata returns non zero.
 ************************************************************************/
{
Errcode err;
Stok *t;
Swork rswork;
Smu_symbol *sym;

	if ((err = smu_lookup(sm,&sym,SMU_NAMESTRING_CLASS,symname)) < Success)
		goto OUT;
	swork_init(&rswork, sm->sf, sym->foff, sym->fline);

	for(;;)
	{
		if(swork_topt(&rswork) == TOK_RBRACE)	/* end of line and no match */
		{
			err = Err_not_enough_fields;
			goto error;
		}

		t = swork_top(&rswork);
		if (t->ttype != TOK_UNDEF)
			return(Err_expecting_name);
		if((err = (*dothing)(&rswork,data)) != 0)
			goto error;
	}

error:
	swork_end(&rswork,sm);
OUT:
	return(err);
}

static Stok *smu_get_next_string(Swork *swork)
{
Stok *t;

	swork_advance_over(swork,',');	/* skip optional comma */
	t = swork_top(swork);
	if (t->ttype == TOK_QUO)
		return(t);
	return(NULL);
}
static Errcode nametext_err(Errcode err, char *symname, char *strname)
{
	if (err >= Success)
		return(err);
	return(errline(err, "?? %s %s %s ??", 
				   smu_class_names[SMU_NAMESTRING_CLASS],symname, strname));
}

struct dsload {
	Dstring ds;
	char *name;
};
static Errcode found_string_to_dstring(Swork *swork,struct dsload *dsl)
{
Errcode err;
Stok *t;
Boolean match;

	t = swork_top(swork);
	match = strcmp(t->d.buf, dsl->name) == 0;

	while((t = smu_get_next_string(swork)) != NULL)
	{
		if(match)
		{
			if((err = dstring_strcat(&(dsl->ds),&(t->d))) < Success)
				return(err);
		}
	}
	return(match);
}


Errcode smu_load_name_text(struct softmenu *sm,	/* read in a named string */
	char *symname,					/* name of symbol in resource file */
	char *strname,					/* name of string in symbol */
	char **ptext)					/* place to put text */
{
Errcode err;
struct dsload sdsl;

	*ptext = NULL; /* in case we don't find anything... */
	sdsl.name = strname;
	dstring_init(&sdsl.ds);
	if((err = smu_do_name_things(sm,symname,
						found_string_to_dstring,&sdsl)) < Success)
	{
		goto error;	
	}

	err = dstring_get_clone(&sdsl.ds,ptext);
error:
	dstring_cleanup(&sdsl.ds);
	return(err); /* note this does NOT report errors */
}

struct bufload {
	char *buf;
	char *name;
	int size;
};

static Errcode found_string_to_buf(Swork *swork,struct bufload *bl)
{
Stok *t;
Boolean match;
int len_copy;

	t = swork_top(swork);
	match = strcmp(t->d.buf, bl->name) == 0;

	while((t = smu_get_next_string(swork)) != NULL)
	{
		if(match && (bl->size > 0))
		{
			/* we have room for more in pre allocd buffer */
			if ((len_copy = (t->d.blen-1)) > bl->size) /* will it all fit? */
				len_copy = bl->size;     /* truncate to what will */
		    memcpy(bl->buf,t->d.buf,len_copy); /* copy token to end */
			bl->buf += len_copy;				
			bl->size -= len_copy;
		}
	}
	return(match);
}

int smu_name_string(struct softmenu *sm,	/* read in a named string */
	char *symname,					/* name of symbol in resource file */
	char *strname,					/* name of string in symbol */
	char *strbuf,					/* place to put string */
	int bufsize)					/* length of place to put string */
{
Errcode err;
struct bufload sbl;

	strbuf[0] = 0;		/* in case we don't find anything... */
	if(bufsize <= 0)
		return(0);

	sbl.size = bufsize-1;
	sbl.buf = strbuf;
	sbl.name = strname;
	if((err = smu_do_name_things(sm,symname,found_string_to_buf,&sbl)) < 0)
		goto error;
	*sbl.buf = 0;
	return(sbl.buf - strbuf); 
error:
	return(nametext_err(err, symname, strname));
}

typedef struct sctlist {
	struct sctlist *next;
	Smu_name_scats *sct;
};

struct sctload {
	Dstring ds;
	struct sctlist *empty;
	struct sctlist *filled;
	int num_filled;
	char *origin;
	struct sctlist *item; /* for error report */
	USHORT flags;
};

void smu_free_scatters(void **buf)
{
	pj_freez(buf);
}
static struct sctlist *sct_in_list(struct sctlist *lst, char *name, 
								   struct sctlist **pprev)
{
struct sctlist *prev;

	prev = NULL;
	while(lst)
	{
		if(strcmp(name, lst->sct->name) == 0)
		{
			*pprev = prev;
			break;
		}
		prev = lst;
		lst = lst->next;
	}
	return(lst);
}

static Errcode load_name_scatter(Swork *swork,struct sctload *sl)
{
Errcode err;
Stok *t;
struct sctlist *prev;
struct sctlist *empty;
Errcode (*add_str)(Dstring *d,Dstring *s);
int dstart;
char **toload;


	t = swork_top(swork);

	if((empty = sct_in_list(sl->empty, t->d.buf, &prev)) == NULL)
	{
		if((sl->item = sct_in_list(sl->filled,t->d.buf, &prev)) != NULL)
			return(Err_duplicate_name);
	}
	else
	{
		dstart = sl->ds.blen;
		add_str = dstring_memcat;
	}

	while((t = smu_get_next_string(swork)) != NULL)
	{
		if(empty)
		{
			if((err = (*add_str)(&(sl->ds),&(t->d))) < Success)
				return(err);
			add_str = dstring_strcat;
		}
	}

	if(empty)
	{
		toload = &(empty->sct->toload.s);
		if(!(sl->flags & SCT_DIRECT))
			toload = (char **)(*toload);
		*toload = sl->origin + dstart;


		if(prev)
			prev->next = empty->next;
		else
			sl->empty = empty->next;
		empty->next = sl->filled;
		sl->filled = empty;
		++sl->num_filled;
	}
	if(sl->empty != NULL)
		return(Success);
	else
		return(Success+1);
}
int smu_name_scatters(struct softmenu *sm,	/* read in a named string */
				  char *symname,	/* name of symbol in resource file */
			  Smu_name_scats *scts,	/* keys and where to put
			  							 * results */
			  int num_scatters, /* how many of them we want to load */
			  void **allocd,    /* pointer to what one needs to free.
			  				     * To get rid of them use smu_free_scatters()
								 * this should not be accessed otherwise */
			  USHORT flags) /* if SCT_OPTIONAL  only items found are loaded.
			  				 * If not there must be an item
			  				 * found for every item in the list. The
							 * first symbol not found will be reported
							 * in the error message. If not true only
							 * items found are loaded.
							 *
							 * if SCT_DIRECT the list is a list of char*
							 * to load s else it is char **ps to load */

/* Scatters strings into pointers for names, searches for named strings in
 * a NameString list. Returns the number of items loaded.  Leaves items 
 * unloaded, unaltered.  Puts a pointer to the buffer to free in *allocd.
 * Returns an error if a string is found more than once in text file.
 * All keys in the list should be unique. All keys in the text file should
 * be unique 
 *
 * Currently limited to 150 items uses about 1k of stack
 *
 * If an error is detected loaded string pointers will be NULLED out */
{
Errcode err;
struct sctload sl;
struct sctlist list[150];
struct sctlist *next;
char **toload;

	if(num_scatters > Array_els(list))
		return(Err_too_big);
	*allocd = NULL;

	clear_struct(&sl);
	sl.flags = flags;
	next = list;

	/* move array into linked list */

	while(num_scatters-- > 0)
	{
		next->sct = &scts[num_scatters];
		next->next = sl.empty;
		sl.empty = next++;
	}

	dstring_init(&sl.ds);

	/* get all strings that match one in the list */
	if((err = smu_do_name_things(sm,symname,
								  load_name_scatter,&sl)) < Success)
	{
		if(err != Err_not_enough_fields) /* this is success in this case */
			goto error;	
	}

	if(!(flags & SCT_OPTIONAL) && sl.empty)
	{
		sl.item = sl.empty;
		err = Err_not_found;
		goto error;
	}

	/* allocate cloned text buffer */
	err = dstring_get_clone(&sl.ds,allocd);

error: /* note success gets you here too */

	while(sl.filled)
	{
		toload = &(sl.filled->sct->toload.s);
		if(!(flags & SCT_DIRECT))
			toload = (char **)(*toload);

		if(err < Success) /* error NULLs out loaded pointers */
		{
			*toload = NULL;
		}
		else /* If Success adjust pointers for allocated buffer origin */
		{	
			*toload += ((char *)(*allocd) - sl.origin); 
		}
		sl.filled = sl.filled->next;
	}

	if(err < Success)
	{
		if(flags && SCT_STRINIT)
			default_common_str();

		err = softerr(err, "!%s%s%s", "no_smu_class_item", 
					  smu_class_names[SMU_NAMESTRING_CLASS],symname,
					  sl.item?sl.item->sct->name:"");
	}
	else
	{
		/* return number of items filled */
		err = sl.num_filled;
	}

	dstring_cleanup(&sl.ds);
	return(err);
}
