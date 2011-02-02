
#include <stdio.h>
#include <ctype.h>

#define CH_WIDTH 6
#define CH_HEIGHT 8
#define XMAX 320

extern void *calloc(int, int);
typedef int (*Vector)();

#define OUT out

struct flicmenu
	{
	struct flicmenu *next;
	struct flicmenu *children;
	short x, y;
	unsigned short width, height;
	void *text;
	int (*seeme)();
	int (*feelme)();
	int *group;
	int identity;
	short key_equiv;
	int (*optme)();
	char *name;
	int level;
	};
typedef struct flicmenu Flicmenu;
extern Flicmenu *parent_menu();
extern Flicmenu quick_menu, edit_menu, title_menu;

#define NOTEXT NULL
#define NOSEE  0L
#define NOFEEL 0L
#define NOGROUP NULL
#define NOKEY	0
#define NOOPT 0L

struct pull
	{
	struct pull *next;
	short xoff, yoff, width, height;
	struct pull *children;
	char *data;  /* actually just some old pointer */
	Vector see;
	char disabled;
	char *name;	/* what do we call it??? */
	short level;
	};
typedef struct pull Pull;

char *inname, *outname;
FILE *in, *out;
int line;
Flicmenu *root, *last;
char *rootname = "fml";

char inbuf[256];
char strings[256*32];
char *freestring = strings;

char *
clone_string(s)
char *s;
{
int i;
char *ns;

i = strlen(s) + 1;
ns = freestring;
freestring += i;
if (freestring >= strings + sizeof(strings))
	{
	freestring -= i;
	return(0);
	}
strcpy(ns,s);
return(ns);
}

lcase(s)
char *s;
{
while (*s != 0)
	{
	if (isupper(*s))
		*s += 'a' - 'A';
	s++;
	}
}


n_line()
{
if ((fgets(inbuf, sizeof(inbuf), in)) == NULL)
	return(0);
else
	return(1);
}

next_line()
{
for (;;)
	{
	if (!n_line())
		return(0);
	if (inbuf[0] != '#')
		return(1);
	}
}

main(argc, argv)
int argc;
char *argv[];
{
if (argc != 4)
	{
	puts("Usage: ");
	puts("    makemenu infile.txt outfile.c rootname");
	puts("Example:");
	puts("    makemenu fontmenu.txt fontmenu.c fmu");
	puts("");
	puts("This creates a set of Flicmenu structures in outfile.c, one for ");
	puts("each line of infile.txt.");
	puts("");
	puts("If fontmenu.txt consisted of the single line:");
	puts("     points 5 9 88 10");
	puts("then fontmenu.c would have one structure named fmu_poi_sel and");
	puts("the x/y/width/height of fmu_poi_sel would be 5 9 (88 10).");
	exit(0);
	}
inname = argv[1];
outname = argv[2];
rootname = argv[3];
if ((in = fopen(inname, "r")) == NULL)
	{
	printf("Couldn't open %s to read, sorry.", inname);
	exit(0);
	}
if ((out = fopen(outname, "w")) == NULL)
	{
	printf("Couldn't open %s to write, sorry.", outname);
	exit(0);
	}
write_headers();
read_lines();
write_flicm();
cleanup();
}

cleanup()
{
fclose(in);
fclose(out);
exit(0);
}

write_headers()
{
fprintf(out, "/* generated with makemenu */\n");
fprintf(out, "#include \"jimk.h\"\n");
fprintf(out, "#include \"flicmenu.h\"\n");
fprintf(out, "\n\n");
fprintf(out, "/*** Display Functions ***/\n");
fprintf(out, "extern ccorner_text(), ncorner_text();\n");
fprintf(out, "\n\n");
fprintf(out, "/*** Select Functions ***/\n");
fprintf(out, "extern toggle_group(), change_mode();\n");
fprintf(out, "\n\n");
fprintf(out, "/*** Button Data ***/\n");
}

char *
skip_space(s)
char *s;
{
char c;

for (;;)
	{
	c = *s;
	if (c == 0 || !isspace(c))
		break;
	s++;
	}
return(s);
}

char *
skip_to_space(s)
char *s;
{
char c;

for (;;)
	{
	c = *s;
	if (c == 0 || isspace(c))
		break;
	s++;
	}
return(s);
}

char *
remove_trailing(s)
char *s;
{
int i;

i = strlen(s);
while (--i >= 0)
	{
	if (isspace(s[i]))
		s[i] = 0;
	else
		break;
	}
return(s);
}

char *
single_name(start)
char *start;
{
char buf[256];
char sb[8];

copy_bytes(start, sb, 3);
sb[3] = 0;
sprintf(buf, "%s_%s_sel", rootname, sb);
lcase(buf);
return(clone_string(buf));
}

char *
next_word(line,buf)
char *line, *buf;
{
char c;
char *end;

line = skip_space(line);
if (line[0] == 0)
	return(NULL);
end = skip_to_space(line);
c = *end;
*end = 0;
strcpy(buf,line);
*end = c;
return(end);
}


Flicmenu *
read_m()
{
Flicmenu *p;
char buf[256];
char *line;

AGAIN:
if (!next_line())
	return(0);
line = inbuf;
if ((p = calloc(1, sizeof(*p))) == NULL)
	{
	puts("Out of memory");
	return(0);
	}
if ((line = next_word(line,buf)) == NULL)
	goto AGAIN;
if ((p->text = clone_string(buf))==NULL)
	{
	puts("Too many strings");
	return(0);
	}
if ((p->name = single_name(p->text)) == NULL)
	{
	puts("Too many strings");
	return(0);
	}
if ((line = next_word(line,buf)) == NULL)
	goto END;
p->x = atoi(buf);
if ((line = next_word(line,buf)) == NULL)
	goto END;
p->y = atoi(buf);
if ((line = next_word(line,buf)) == NULL)
	goto END;
p->width = atoi(buf);
if ((line = next_word(line,buf)) == NULL)
	goto END;
p->height = atoi(buf);
END:
return(p);
}

copy_bytes(s,d,count)
register char *s,*d;
int count;
{
while (--count >= 0)
	*d++ = *s++;
}


read_lines()
{
Flicmenu *p;

for (;;)
	{
	if ((p = read_m()) == NULL)
		break;
	if (root == NULL)
		{
		root = last = p;
		}
	else
		{
		last->next = p;
		last = p;
		}
	}
}

print_idents(ident)
int ident;
{
while (--ident >= 0)
	fprintf(OUT, "\t");
}


write_m(p, ident)
Flicmenu *p;
int ident;
{
print_idents(ident);
fprintf(OUT, "struct flicmenu %s = {\n", p->name);
ident += 1;
print_idents(ident);
if (p->next == NULL)
	fprintf(OUT, "NONEXT,\n");
else
	fprintf(OUT, "&%s,\n", p->next->name);
print_idents(ident);
if (p->children == NULL)
	fprintf(OUT, "NOCHILD,\n");
else
	fprintf(OUT, "&%s,\n", p->children->name);
print_idents(ident);
fprintf(OUT, "%d, %d, %d, %d,\n", p->x, p->y, p->width, p->height);
print_idents(ident);
if (p->text == NULL)
	fprintf(OUT, "NOTEXT,\n");
else
	fprintf(OUT, "\"%s\",\n", p->text);
print_idents(ident);
fprintf(OUT, "NOSEE,\n");
print_idents(ident);
fprintf(OUT, "NOFEEL,\n");
print_idents(ident);
fprintf(OUT, "NOGROUP,0,\n");
print_idents(ident);
fprintf(OUT, "NOKEY,\n");
print_idents(ident);
fprintf(OUT, "NOOPT,\n");
print_idents(ident);
fprintf(OUT, "};\n");
}


write_tree(p, ident)
Flicmenu *p;
int ident;
{
if (p == NULL)
	return;
write_tree(p->next, ident);
write_tree(p->children, ident+1);
write_m(p, ident);
}


list_els(p)
Pull *p;
{
int count = 0;

while (p != NULL)
	{
	count++;
	p = p->next;
	}
return(count);
}


write_flicm()
{
write_tree(root,0);
}

