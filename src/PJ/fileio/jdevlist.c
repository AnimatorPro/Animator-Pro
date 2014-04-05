#include <stdio.h>
#include <string.h>
#include "errcodes.h"
#include "jfile.h"
#include "memory.h"
#include "reqlib.h"
#include "util.h"
#include "wildlist.h"

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

static void unslash_dir(char *dir, char *unslash)
/* Pass in pointer to directory that might have an extra slash at the end and
 * pointer to where to put unslashed version. */
{
int sz;

strcpy(unslash, dir);
if ((sz = strlen(unslash)) <= 1)	/* special case for \  */
	return;
if (unslash[sz-1] == DIR_DELIM) /* ends in slash */
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

