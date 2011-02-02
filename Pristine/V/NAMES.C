
/* names.c - simple string and list management utility routines */

#include "jimk.h"
#include <ctype.h>

#ifdef SLUFFED
string_in(pat, string)
char *pat;
char *string;
{
char c, s;

for (;;)
	{
	if ((c = *pat++) == 0)	/* empty pattern matches all */
		return(1);
	if (c == '*')
		{
		for (;;)
			{
			if (string_in(pat, string))
				return(1);
			if ((s = *string++) == 0)
				return(0);
			}
		}
	if ((s = *string++) == 0)	/* end of string before end of pattern, no*/
		return(0);
	if (c == '?')
		continue;
	if (islower(c))
		c = _toupper(c);
	if (islower(s))
		s = _toupper(s);
	if (c != s)
		return(0);
	}
}
#endif /* SLUFFED */

to_upper(s)
register UBYTE *s;
{
register UBYTE c;

while ((c = *s++) != 0)
	{
	if (islower(c))
		*(s-1) = _toupper(c);
	}
}

/* string compare ignoring case */
ustrcmp(as, bs)
register char *as, *bs;
{
register char a, b;

for (;;)
	{
	a = *as++;
	b = *bs++;
	if (isupper(a))
		a = _tolower(a);
	if (isupper(b))
		b = _tolower(b);
	if (a != b)
		return(a-b);
	if (a == 0)
		return(0);
	}
}

#ifdef SLUFFED
ustrncmp(as, bs, len)
register char *as, *bs;
register WORD len;
{
register char a, b;

while (--len >= 0)
	{
	a = *as++;
	b = *bs++;
	if (isupper(a))
		a = _tolower(a);
	if (isupper(b))
		b = _tolower(b);
	if (a != b)
		return(a-b);
	}
return(0);
}
#endif /* SLUFFED */



#ifdef SLUFFED
upc(s)
register char *s;
{
register char c;

while (c = *s)
	{
	*s++ = toupper(c);
	}
}

#endif /* SLUFFED */

tr_string(string, in, out)
register char *string;
register char in, out;
{
register char c;

while ((c = *string)!=0)
	{
	if ( c == in )
		*string = out;
	string++;
	}
}

cut_suffix(title, suffix)
char *title, *suffix;
{
title[strlen(title) - strlen(suffix)] = 0;
}

suffix_in(string, suff)
char *string, *suff;
{
string += strlen(string) - strlen(suff);
return( ustrcmp(string, suff) == 0);
}

#ifdef SLUFFED
lines_size(lines)
char *lines[];
{
int count = 0;

while (*lines++ != 0L)
	count++;
return(count);
}
#endif /* SLUFFED */

el_ix(list, el)
Name_list *list;
Name_list *el;
{
int i = 0;

while (list)
	{
	if (list == el)
		return(i);
	i++;
	list = list->next;
	}
return(-1);
}

void *
list_el(list, ix)
register Name_list *list;
int ix;
{
while (list && --ix>= 0)
	{
	list = list->next;
	}
return(list);
}

#ifdef SLUFFED
list_ix(list, el)
Name_list *list, *el;
{
int ix = 0;

while (list)
	{
	if (list == el)
		break;
	list = list->next;
	ix++;
	}
return(ix);
}
#endif SLUFFED

els_in_list(list)
register Name_list *list;
{
register count = 0;

while (list)
	{
	count++;
	list = list->next;
	}
return(count);
}

#ifdef SLUFFED
void *
remove_el(list,el)
Name_list *list;
Name_list *el;
{
Name_list *last;
register Name_list *next;

if (list == el)
	return(list->next);
next = last = list;
while ((next = next->next) != NULL)
	{
	if (next == el)
		{
		last->next = next->next;
		return(list);
		}
	last = next;
	}
return(list);
}
#endif SLUFFED

static
cmp_name_list(l1,l2)
Name_list *l1, *l2;
{
return(strcmp(l1->name, l2->name) );
}


void *
sort_list(list, cmp)
register Name_list *list;
Vector cmp;
{
register Name_list **array, *pt, **array_pt;
register int elements, i;

elements = els_in_list(list);
if (elements <= 1)
	return(list);	/* length 0 or 1 lists already sorted */

array = (Name_list **)askmem( elements * sizeof(Name_list *) );
if (array)
	{
	pt = list;
	array_pt = array;
	while ( pt )
		{
		*array_pt++ = pt;
		pt = pt->next;
		}
	sort_array(array, elements, cmp);
	array_pt = array;
	list = NULL;
	i = elements;
	while (--i >= 0)
		{
		pt = *array_pt++;
		pt->next = list;
		list = pt;
		}
	freemem( array );
	}
return(list);
}

Name_list *
sort_name_list(list)
register Name_list *list;
{
return(sort_list(list, cmp_name_list));
}

#ifdef SLUFFED
jstrcmp(a, b)
register char *a, *b;
{
register char aa, bb;

if (a == b)
	return(0);
if (a == NULL)
	return(1);
if (b == NULL)
	return(-1);
for (;;)
	{
	aa = *a++;
	bb = *b++;
	if (aa == bb)
		{
		if (!aa)
			return(0);
		}
	else
		return( aa - bb);
	}
}
#endif /* SLUFFED */

/* a little shell sort on an array of strings */
sort_array(array, count, cmp)
Name_list **array;
int count;
Vector cmp;
{
register Name_list **pt1, **pt2;
register Name_list *swap;
register short swaps;
register int space, ct;

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
			if ((*cmp)(*pt1, *pt2) < 0)
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
	if ( (space = space/2) == 0)
		break;
	}

}

Name_list *
name_in_list(name, list)
char *name;
Name_list *list;
{
while (list != NULL)
	{
	if (strcmp(list->name, name) == 0)
		break;
	list = list->next;
	}
return(list);
}



char *
clone_string(s)
char *s;
{
char *d;

if ((d = (char *)askmem(strlen(s)+1)) != NULL)
	strcpy(d, s);
return(d);
}

#ifdef SLUFFED
unsigned char bitmasks[8] =
	{
	0x80, 0x40, 0x20, 0x10,
	0x8, 0x4, 0x2, 0x1,
	};

getbit(table, ix)
unsigned char *table;
register int ix;
{
return(table[(ix>>3)]&bitmasks[ix&7]);
}

setbit(table,ix)
unsigned char *table;
register int ix;
{
table[(ix>>3)] |= bitmasks[ix&7];
}
#endif SLUFFED


free_list(lst)
register Name_list *lst;
{
register Name_list *next;

while (lst)
	{
	next = lst->next;
	freemem(lst);
	lst = next;
	}
}

free_name_list(lst)
register Name_list *lst;
{
register Name_list *next;

while (lst)
	{
	next = lst->next;
	freemem(lst->name);
	freemem(lst);
	lst = next;
	}
}

longest_string(strings, count)
char **strings;
int count;
{
int acc, len;

acc = 0;
while (--count >= 0)
	{
	len = strlen(*strings++);
	if (len > acc)
		acc = len;
	}
return(acc);
}

intabs(a)
int a;
{
return(a >= 0 ? a : -a);
}

intmin(a, b)
int a,b;
{
return( a < b ? a : b);
}

/* reverse order of bytes in buffer */
reverse_bytes(b,count)
char *b;
int count;
{
char swap;
char *end;

end = b + count - 1;
count >>= 1;
while (--count >= 0)
	{
	swap = *end;
	*end = *b;
	*b++ = swap;
	end -= 1;
	}
}

/* swap two buffers */
exchange_buf(s1, s2, count)
register char *s1, *s2;
WORD count;
{
register char swap;

while (--count >= 0)
	{
	swap = *s1;
	*s1++ = *s2;
	*s2++ = swap;
	}
}

/* copy bytes starting at hi end */
back_copy_bytes(s,d,count)
register char *s, *d;
WORD count;
{
s += count;
d += count;
while (--count >= 0)
	*(--d) = *(--s);
}

