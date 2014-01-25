#include <string.h>
#include "linklist.h"
#include "errcodes.h"
#include "memory.h"

void free_dl_list(Dlheader *list)
/* free up a double-linked lists nodes. (If the nodes are simple) */
{
Dlnode *lnode, *nextnode;

if (list->tails_prev == NULL) /* not initialized */
	return;
for(lnode = list->tails_prev;
    NULL != (nextnode = lnode->prev);
	lnode = nextnode)
	{
	pj_free(lnode);
	}
init_list(list);
}

Errcode clone_dl_list(Dlheader *source, Dlheader *dest, int node_size)
/* Clone a double-linked lists noded. (If the nodes are simple) */
{
Dlnode *lnode, *nextnode, *new;

init_list(dest);
if (source->tails_prev->prev == NULL)
	return(Success);
for(lnode = source->tails_prev;
    NULL != (nextnode = lnode->prev);
	lnode = nextnode)
	{
	if ((new = pj_malloc(node_size)) == NULL)
		return(Err_no_memory);
	memcpy(new,lnode,node_size);
	add_head(dest,new);
	}
return(Success);
}

