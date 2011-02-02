/* savepull.c - write out pulls from memory into SoftMenu file.
 * Currently not linked in, and doesn't even compile, but here
 * for reference in case I have to do it again.... */

#include "menus.h"
#include <ctype.h>

static void stringout(FILE *f, char *s)
{
fprintf(f, "\"%s\"", s);
}

static Errcode sleaf(FILE *f, Pull *p, int pullval)
{
SHORT key;
int leafval = 1;

stringout(f, p->data);
fprintf(f,",%d",pullval);
fprintf(f,"\n\t{\n");
p = p->children->children;
while (p != NULL)
	{
	fprintf(f, "\t");
	if (p->see == pull_midline)
		stringout(f, "----");
	else
		stringout(f, p->data);
	fprintf(f,",%d",pullval+leafval);
	leafval+=1;
	if ((key = p->key_equiv) != 0)
		{
		fprintf(f,",'%c'", tolower(((char *)(p->data))[0]));
		if (isalnum(key) || ispunct(key))
			fprintf(f,",'%c'", key);
		else
			fprintf(f,",0x%x", key);
		}
	fprintf(f, "\n");
	p = p->next;
	}
fprintf(f, "\t\}\n");
return(Success);
}

static Errcode spull(FILE *f, Pull *p, char *pname)
{
Errcode err = Success;
int leafval = 100;

fprintf(f, "Pull %s\n{\n", pname);
while (p != NULL)
	{
	if ((err = sleaf(f, p, leafval)) < Success)
		break;
	leafval += 100;
	p = p->next;
	}
fprintf(f, "}\n\n\n");
return(err);
}

typedef struct 
	{
	char *name;
	Pull *p;
	} PullList;
extern Pull cel_pull, presets_pull, pal_pull, sys_pull, twe_pull;
static PullList plist[] =
	{
		{"home", &sys_pull,},
		{"optics", &presets_pull,},
		{"palette", &pal_pull,},
		{"tween", &twe_pull,},
		{"cel", &cel_pull,},
	};

static Errcode sall_pulls(FILE *f)
{
PullList *pl = plist;
int i = Array_els(plist);
Errcode err = Success;

while (--i >= 0)
	{
	if ((err = spull(f, pl->p, pl->name)) < Success)
		break;
	pl += 1;
	}
return(err);
}

Errcode save_all_pulls(char *fn)
{
FILE *f;
Errcode err;

if ((f = fopen(fn, "w")) == NULL)
	return(errno);
err = sall_pulls(f);
fclose(f);
return(err);
}


