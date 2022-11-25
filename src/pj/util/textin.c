#include "linklist.h"
#include "util.h"

Names *text_in_list(char *name, Names *list)

/* does case independent search for "name" in list */
{
	while(list != NULL)
	{
		if(txtcmp(list->name, name) == 0)
			break;
		list = list->next;
	}
	return(list);
}
