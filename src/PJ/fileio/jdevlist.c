#include "errcodes.h"
#include "jfile.h"
#include "memory.h"
#include "util.h"
#include "wildlist.h"

typedef struct wild_data {
	Names **plist; 	/* pointer to name list start */
	char prefix[4];
	int min_name_size;
	Nameload load_name;
	struct fndata fn;
} Wild_data;


void free_wild_list(Names **pwild_list)
{
Names *l, *n;

	l = *pwild_list;
	while (l != NULL)
	{
		n = l->next;
		pj_free(l);
		l = n;
	}
	*pwild_list = NULL;
}
static Errcode add_wild(Wild_data *wd)
{
Wild_entry *next;
char buf[16];
int c2;
int len;

	if (wd->fn.name[0] == '.')	/* filter out '.' and '..' */
	{
		c2 = wd->fn.name[1];
		if (c2 == '.' || c2 == 0)
			return(Success);
	}
	len = sizeof(Wild_entry) + sprintf(buf, "%s%s", wd->prefix, wd->fn.name);
	if((next = pj_malloc(Max(len,wd->min_name_size))) == NULL)
		return(Err_no_memory);
	next->hdr.name = next->name_buf;
	(*(wd->load_name))(next, buf);
	next->hdr.next = *(wd->plist);
	*(wd->plist) = &(next->hdr);
	return(Success);
}

static Errcode attr_wild_list(int attr, char *pat, Wild_data *wd) 

/* will return Success if nothing is found */
{
Errcode err;

	/* set the 'DTA' area for directory search */
	pj_dset_dta(&wd->fn);

	/* now do the find first... */
	if(pj_dfirst(pat, attr) )
	{
		for(;;)
		{
			if ((wd->fn.attribute&16) == attr)
				if((err = add_wild(wd)) < Success)
					return(err);
			if (!pj_dnext())
				break;
		}
	}
	return(Success);
}
Errcode alloc_wild_list(Names **pwild_list, char *pat,Boolean get_dirs,
							   int min_name_size, 
							   Nameload load_name )

/* allocate list of files from current directory insuring that each buffer 
 * is the size requested and copying in name with input function does not
 * sort list */
{
Errcode err;
Wild_data wd;

	*pwild_list = NULL;
	wd.plist = pwild_list;
	wd.min_name_size = min_name_size;
	wd.load_name = load_name;

	if (pat[0] == '#' && pat[1] == ':')
	{
boxf("Oooops this could crash! see \\paa\\fileio\\jdevlist.c");
#ifdef WONT_LINK
		rget_dir(pwild_list);
#endif
	}
	else
	{
		/* get all directories */
		if (get_dirs)
		{
			wd.prefix[0] = '\\';
			wd.prefix[1] = 0;
			if((err = attr_wild_list(16, "*.*",&wd)) < Success)
				goto error;	
		}
		/* and other files matching wild */
		wd.prefix[0] = 0;
		if((err = attr_wild_list(0,pat,&wd)) < Success)
			goto error;		
	}
	return(Success);
error:
	free_wild_list(pwild_list);
	return(err);
}
static void load_wild_name(Wild_entry *entry, char *name)
{
	entry->hdr.name = entry->name_buf; 
	strcpy(entry->name_buf,name);
}
Names *merge_wild_lists(Names *l1, Names *l2)
{
Names first = { NULL, NULL };
Names *out = &first;

	for(;;)
	{
		if(l1 == NULL)
		{
			out->next = l2;
			break;
		}
		if(l2 == NULL)
		{
			out->next = l1;
			break;
		}
		/* aack we have to keep dirs at end */

		if(txtcmp(l1->name,l2->name) > 0)
		{
			out->next = l2;
			out = l2;
			l2 = l2->next;
		}
		else /* we could free the redundant name if txtcmp returns 0 
		      * but I don't. It really sould have a bigger build wild list
			  * function for that */
		{
			out->next = l1;
			out = l1;
			l1 = l1->next;
		}
	}
	return(first.next);
}
Errcode build_wild_list(Names **pwild_list, char *pat, Boolean get_dirs)
{
Errcode err;

	if((err = alloc_wild_list(pwild_list,pat,get_dirs,0,
							  load_wild_name)) >= Success)
	{
		*pwild_list = sort_names(*pwild_list);
	}
	return(err);
}

void unslash_dir(char *dir, char *unslash)
/* Pass in pointer to directory that might have an extra slash at the end and
 * pointer to where to put unslashed version. */
{
int sz;

strcpy(unslash, dir);
if ((sz = strlen(unslash)) <= 1)	/* special case for \  */
	return;
if (unslash[sz-1] == '\\')		/* ends in slash */
	{
	if (unslash[sz-2] != ':')	/* special case for X:\ */
		unslash[sz-1] = 0;
	}
}

Errcode build_dir_list(Names **pwild_list, char *pat, 
					   Boolean get_dirs, char *dir)
/* get list of files from a specified directory */
{
Errcode err;
char odir[PATH_SIZE];
char ndir[PATH_SIZE];

	get_dir(odir);
	unslash_dir(dir,ndir);
	if((err = change_dir(ndir)) <  Success)
	{
		*pwild_list = NULL;
		goto error;
	}
	err = build_wild_list(pwild_list, pat, get_dirs);
	change_dir(odir);
error:
	return(err);
}

