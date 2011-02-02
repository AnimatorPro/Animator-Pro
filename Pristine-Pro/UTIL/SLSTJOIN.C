#include "linklist.h"

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
