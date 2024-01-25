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
