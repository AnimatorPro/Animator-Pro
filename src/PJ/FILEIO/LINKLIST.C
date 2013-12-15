/* linklist.c doubly linked list utilities */

#include "linklist.h"
#include "memory.h"

#define SAFETY  

/***** single linked list calls ******/

/*********************************************/
void *slist_el(register Slnode *list, int ix)

/* given pointer to first element of a list and index returns pointer to 
 * element. NULL if not there */
{
	while(list && --ix >= 0)
		list = list->next;
	return(list);
}
/***********************************************************/
int slist_ix(register Slnode *list, Slnode *el)

/* given pointer to element and pointer to first in list returns
 * "index" or count away from list element is -1 if not found */
{
int ix = 0;

	while(list != NULL)
	{
		if (list == el)
			return(ix);
		++ix;
		list = list->next;
	}
	return(-1);
}
/***********************************************************/
int slist_len(register Slnode *list)
{
register int count = 0;

	while (list)
	{
		count++;
		list = list->next;
	}
	return(count);
}
/***********************************************************/
void *join_slists(Slnode *s1, Slnode *s2)
{
Slnode *t, *next;

if (s1 == NULL)
	return(s2);
if (s2 == NULL)
	return(s1);
t = s1;
while ((next = t->next) != NULL)	/* seek to end of s1 */
	t = next;
t->next = s2;
return(s1);
}

/***********************************************************/
/* DOUBLY linked list (dl) calls */
/***********************************************************/
void init_list(list)

Dlheader *list;
{
	list->head = (Dlnode *)&(list->tail);
	list->tail = NULL;
	list->tails_prev = (Dlnode *)list;
}
/***********************************************************/
void add_head(list,node)

Dlheader *list;
Dlnode *node;
{
	node->prev = (Dlnode *)(&list->head);
	node->next = list->head;
	node->next->prev = list->head = node;
}
/***********************************************************/
void add_tail(list,node)

Dlheader *list;
Dlnode *node;
{
	node->next = (Dlnode *)(&list->tail);
	node->prev = list->tails_prev;
	node->prev->next = list->tails_prev = node;
}
/***********************************************************/
void insert_after(node,lnode)

register Dlnode *node; /* node to insert after */
register Dlnode *lnode; /* attached node */
{
	node->prev = lnode;
	node->next = lnode->next;
	node->next->prev = lnode->next = node;
}
/**********************************************************/
void insert_before(node,lnode)

register Dlnode *node; /* node to insert before */
register Dlnode *lnode; /* attached node */
{
	node->next = lnode;
	node->prev = lnode->prev;
	node->prev->next = lnode->prev = node;
}
/**********************************************************/
Dlnode *get_head(register Dlheader *list)
{
register Dlnode *head;

	if((Dlnode *)list == list->tails_prev)
		return(NULL);

	head = list->head;
	list->head = head->next;
	list->head->prev = (Dlnode *)(&list->head);
#ifdef SAFETY  
	head->next = NULL;
#endif /* SAFETY */
	return(head);
}
/**********************************************************/
Dlnode *see_head(register Dlheader *list)
{
	if((Dlnode *)list == list->tails_prev)
		return(NULL);
	return(list->head);
}
/**********************************************************/
Dlnode *get_tail(register Dlheader *list)
{
register Dlnode *tail;

	if((Dlnode *)list == (tail = list->tails_prev))
		return NULL;

	list->tails_prev = tail->prev;
	list->tails_prev->next = (Dlnode *)(&list->tail);
#ifdef SAFETY  
	tail->next = NULL;
#endif /* SAFETY */
	return(tail);
}
/**********************************************************/
Dlnode *see_tail(register Dlheader *list)
{
	if((Dlnode *)list == list->tails_prev)
		return(NULL);
	return(list->tails_prev);
}
/**********************************************************/
void rem_node(register Dlnode *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
#ifdef SAFETY  
	node->next = NULL;
#endif /* SAFETY */
}
/**********************************************************/
#ifdef SAFETY  
void safe_rem_node(register Dlnode *node)
{
	if(node->next == NULL)
		return;
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->next = NULL;
}
#endif /* SAFETY */
/**************************************************************/
/* moves adds all of the fromlists nodes to
 * the head of the tolist the nodes remain in the same order
 * as in the fromlist */

void list_tohead( register Dlheader *fromlist, register Dlheader *tolist)
{
Dlnode *fromtail;
extern Dlnode *see_tail();

	if(NULL == (fromtail = see_tail(fromlist))) /* nothing to move */
		return;

	/* link fromtail to tohead */
	fromtail->next = tolist->head;
	tolist->head->prev = fromtail;

	/* link fromhead onto tolist->head */
	tolist->head = fromlist->head;
	tolist->head->prev = (Dlnode *)&tolist->head;

	/* clear fromlist */
	init_list(fromlist);
}
/*************************************************************/
/* moves adds all of the fromlists nodes to
 * the tail of the tolist the nodes remain in the same order
 * as in the fromlist */

void list_totail(Dlheader *fromlist, Dlheader *tolist)
{
Dlnode *fromhead;
extern Dlnode *see_head();

	if(NULL == (fromhead = see_head(fromlist))) /* nothing to move */
		return;

	/* link fromhead to totail */
	fromhead->prev = tolist->tails_prev;  /* point fromhead back to totail */
	fromhead->prev->next = fromhead;      /* point totail to fromhead */

	/* link fromtail to tolist->tail */
	tolist->tails_prev = fromlist->tails_prev;
	tolist->tails_prev->next = (Dlnode *)&tolist->tail; 	

	/* clear fromlist */
	init_list(fromlist);
}
/***********************************************************/
/* returns length of doubly linked list */

LONG listlen(Dlheader *list)
{
register Dlnode *node;
register Dlnode *next;
register LONG len;

	for(node = list->head, len = 0;
		NULL != (next = node->next);
		node = next)
	{
		++len;
	}
	return(len);
}
/***********************************************************/
/* returns header node is attached to 
 * (if SAFETY returns NULL if not attached) */

Dlheader *find_header(register Dlnode *node)
{
#ifdef SAFETY
	if(node->next == NULL)
		return(NULL);
#endif /* SAFETY */

	while(node->prev != NULL)
		node = node->prev;
	return((Dlheader *)node);
}
/****************************************************************/
Boolean isin_list(register Dlnode *testnode,Dlheader *list)

/* returns 1 if node found in list 0 if not */
{
register Dlnode *node;

	node = list->head;
	while(node->next != NULL)
	{
		if(testnode == node)
			return(1);
		node = node->next;
	}
	return(0);
}
/****************************************************************/
/* inserts a node in a list given compare function and the list
 * only works if list is in sorted order using this compare function */

void insert_compare(register Dlnode *node,
					Dlheader *list,
					FUNC compare)
/* FUNC a function that is called given *node A and *node B
 * compare(Dlnode *nodea Dlnode *nodeb) returns 0 if
 * A == B, < 0 if A < B, > 0 if A > B */
{
register Dlnode *lnode; /* current list node */
register Dlnode *nextnode;

	for(lnode = list->tails_prev;
	    NULL != (nextnode = lnode->prev);
		lnode = nextnode)
	{
		if((*compare)(node,lnode) > 0)
			break;
	}
	insert_after(node,lnode);
}
/*************************************************************/
/* insert sorts a list given compare function and the list
 * list will be in ascending order based on compare() */

void isort_list(Dlheader *list, FUNC compare)

/* a function that is called given *node A and *node B
 * compare(Dlnode *nodea Dlnode *nodeb) returns 0 if
 * A == B, < 0 if A < B, > 0 if A > B */
{
Dlheader tlist; /* tempory list */
Dlnode *node;

	init_list(&tlist);
	list_tohead(list,&tlist);

	while(NULL != (node = get_head(&tlist)))
		insert_compare(node,list,compare);
}

/* some sort routines.  Why here??? Why not??? */
void sort_indarray(void **array, LONG count, FUNC cmp, void *cmpdat)

/* a little shell on an array of indirect pointers to things.
 * takes a function that is like strcmp() to compare things pointed to */
{
register void **pt1, **pt2;
register void *swap;
register LONG swaps;
register LONG space, ct;

	if (count < 2)  /*very short arrays are already sorted*/
		return;

	space = count/2;
	--count; /* since look at two elements at once...*/
	for (;;)
	{
		swaps = 1;
		while (swaps)
		{
			pt1 = array;
			pt2 = array + space;
			ct = count - space + 1;
			swaps = 0;
			while (--ct >= 0)
			{
				if ((*cmp)(*pt1, *pt2, cmpdat) < 0)
				{
					swaps = 1;
					swap = *pt1;
					*pt1 = *pt2;
					*pt2 = swap;
				}
				pt1++;
				pt2++;
			}
		}
		if ( (space /= 2) == 0)
			break;
	}
}

