#include "linklist.h"

/**********************************************************/
void rem_node(register Dlnode *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
#ifdef DLL_SAFETY  
	node->next = NULL;
#endif /* DLL_SAFETY */
}
/**********************************************************/
#ifdef DLL_SAFETY  
void safe_rem_node(register Dlnode *node)
{
	if(node->next == NULL)
		return;
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->next = NULL;
}
#endif /* DLL_SAFETY */
