
/* names.c - simple string and list management utility routines */

#include "errcodes.h"
#include "jimk.h"
#include <ctype.h>

Names *array_to_names(char **array, int size)
{
	Names *list = NULL;
	Names *name;

	while (--size >= 0)
		{
		if ((name = begmem(sizeof(*name))) == NULL)
			break;
		name->name = *array++;
		name->next = list;
		list = name;
		}
	return reverse_slist(list);
}

#ifdef SLUFFED
int string_in(char *pat, char *string)
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



#ifdef SLUFFED

int lines_size(char **lines)
{
int count = 0;

while (*lines++ != NULL)
	count++;
return(count);
}
#endif /* SLUFFED */





#ifdef SLUFFED
int jstrcmp(register char *a, register char *b)
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





#ifdef SLUFFED
unsigned char bitmasks[8] =
	{
	0x80, 0x40, 0x20, 0x10,
	0x8, 0x4, 0x2, 0x1,
	};

void getbit(UBYTE *table, register int ix)
{
return(table[(ix>>3)]&bitmasks[ix&7]);
}

void setbit(UBYTE *table, register int ix)
{
table[(ix>>3)] |= bitmasks[ix&7];
}
#endif /* SLUFFED */




#ifdef SLUFFED
void free_names(register Names *lst)
{
register Names *next;

while (lst)
	{
	next = lst->next;
	pj_free(lst->name);
	pj_free(lst);
	lst = next;
	}
}
#endif


#ifdef SLUFFED
int longest_string(char **strings, int count)
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
#endif
