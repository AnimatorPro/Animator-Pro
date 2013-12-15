/* defpull.c - generate a bunch of:
 *		#define XXX_YYY_PULL nnn
 * statements into a file based on the names and id's of the Pull you
 * pass in.
 *
 * This one shouldn't be linked into the final.
 */
#include "lstdio.h"
#include "dstring.h"
#include "menus.h"
#define CHARS 3

char *skip_to_real(char *s)
{
while (s[0] == ' ' || s[0] == '*')
	s += 1;
return(s);
}

static Errcode write_leaf_defs(Pull *p, FILE *f)
{
char leafn[CHARS+1], itemn[CHARS+1];

strncpy(leafn, skip_to_real(p->data), sizeof(leafn));
leafn[CHARS] = 0;
upc(leafn);
fprintf(f, "#define %s_PUL\t%d\n", leafn, p->id);
p = p->children->children;
while (p != NULL)
	{
	if (p->data != NULL && strncmp(p->data, "----", 4) != 0)
		{
		strncpy(itemn, skip_to_real(p->data), sizeof(itemn));
		itemn[CHARS] = 0;
		upc(itemn);
		fprintf(f, "#define %s_%s_PUL\t%d\n", leafn, itemn, p->id);
		}
	p = p->next;
	}
return(Success);
}

Errcode write_pull_defs(Pull *p, char *fname)
{
FILE *f;
Errcode err;

if ((err = ffopen(fname, &f, "w")) < Success)
	goto OUT;
while (p != NULL)
	{
	if ((err = write_leaf_defs(p,f)) < Success)
		goto OUT;
	p = p->next;
	}
OUT:
ffclose(&f);
return(err);
}



