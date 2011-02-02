
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
Flicmenu *root;
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
	puts("This creates a set of Button structures in outfile.c, one for ");
	puts("each line of infile.txt. (The first line makes a Menuhdr.)");
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
if (read_lines())
	{
	write_headers();
	write_flicm();
	write_button_list();
	fprintf(OUT, 
		"\n/*********   End of makemenu generated code *********/\n\n");
	}
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
	fprintf(out, "/* generated with makemenu 2.0 */\n");
	fprintf(out, "#include \"jimk.h\"\n");
	fprintf(out, "#include \"menus.h\"\n");
	fprintf(out, "\n\n");
	fprintf(out, "/*** Button Data ***/\n");
}

char *skip_space(s)
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

char *single_name(char *start, int is_root)
{
char buf[256];

if (is_root)
	sprintf(buf, "%s_menu", rootname);
else
	sprintf(buf, "%s_%.8s_sel", rootname, start);
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
read_m(int is_root)
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
if ((p->name = single_name(p->text,is_root)) == NULL)
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

pj_copy_bytes(s,d,count)
register char *s,*d;
int count;
{
while (--count >= 0)
	*d++ = *s++;
}


read_lines()
{
Flicmenu *p;
int i;
Flicmenu *last = NULL;

if ((root = read_m(1)) == NULL)
	{
	fprintf(stderr, "No data in %s!\n", inname);
	return(0);
	}
for (i = 0;;i++)
	{
	if ((p = read_m(0)) == NULL)
		break;
	if (last == NULL)
		{
		root->children = last = p;
		}
	else
		{
		last->next = p;
		last = p;
		}
	}
if (i == 0)
	{
	fprintf(stderr, "Only one line in %s, hope that's ok...\n", inname);
	}
return(1);
}

print_idents(ident)
int ident;
{
while (--ident >= 0)
	fprintf(OUT, "\t");
}


write_button(p, ident)
Flicmenu *p;
int ident;
{
print_idents(ident);
fprintf(OUT, "static Button %s = MB_INIT1(\n", p->name);
ident += 1;
print_idents(ident);
if (p->next == NULL)
	fprintf(OUT, "NONEXT, /* next */\n");
else
	fprintf(OUT, "&%s, /* next */\n", p->next->name);
print_idents(ident);
if (p->children == NULL)
	fprintf(OUT, "NOCHILD, /* children */\n");
else
	fprintf(OUT, "&%s,\n /* children */", p->children->name);
print_idents(ident);
fprintf(OUT, "%d, %d, %d, %d, /* w,h,x,y */\n", p->width,p->height,p->x,p->y);
print_idents(ident);
if (p->text == NULL)
	fprintf(OUT, "NOTEXT, /* datme */\n");
else
	fprintf(OUT, "\"%s\", /* datme */\n", p->text);
print_idents(ident);
fprintf(OUT, "NOSEE,\n");
print_idents(ident);
fprintf(OUT, "NOFEEL,\n");
print_idents(ident);
fprintf(OUT, "NOOPT,\n");
print_idents(ident);
fprintf(OUT, "NOGROUP,0,\n");
print_idents(ident);
fprintf(OUT, "NOKEY,\n");
print_idents(ident);
fprintf(OUT, "0,	/* flags */\n");
print_idents(ident);
fprintf(OUT, ");\n");
}


write_tree(p, ident)
Flicmenu *p;
int ident;
{
if (p == NULL)
	return;
write_tree(p->next, ident);
write_tree(p->children, ident+1);
write_button(p, ident);
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


write_menuhdr(Flicmenu *b)
{
fprintf(OUT, "\nMenuhdr %s = MENU_INIT0(\n", b->name);
fprintf(OUT, "\t%d,%d,%d,%d,\t/* w,h,x,y */ \n", 
					b->width, b->height, b->x, b->y);
fprintf(OUT, "\tMAKEMENU_MUID,\t\t/* id */\n");
fprintf(OUT, "\tPANELMENU,\t\t/* type */\n");
fprintf(OUT, "\t&%s,\t/* buttons */\n", 
	(b->children == NULL ? "NULL" : b->children->name));
fprintf(OUT, "\tSCREEN_FONT,\t/* font */\n");
fprintf(OUT, "\t&menu_cursor,\t/* cursor */\n");
fprintf(OUT, "\tseebg_white,\t/* seebg */\n");
fprintf(OUT, "\tNULL,\t\t\t/* data */\n");
fprintf(OUT, "\tNULL,\t\t\t/* domenu */\n");
fprintf(OUT, "\t(MBPEN|MBRIGHT|KEYHIT),\t/* ioflags */\n");
fprintf(OUT, "\t0,\t\t\t/* flags */\n");
fprintf(OUT, "\tNULL,\t\t\t/* procmouse */\n");
fprintf(OUT, "\tNULL,\t\t\t/* on_showhide */\n");
fprintf(OUT, "\tNULL,\t\t\t/* cleanup */\n");
fprintf(OUT, ");\n");
}

write_flicm()
{
write_tree(root->children,0);
write_menuhdr(root);
}

rwrite_blist(Flicmenu *p)
{
if (p == NULL)
	return;
rwrite_blist(p->next);
rwrite_blist(p->children);
fprintf(OUT, "&%s,\n",p->name);
}

write_button_list()
/* write out the scaling array and function to do the scale */
{
if (root != NULL && root->children != NULL)
	{
	fprintf(OUT, "\nstatic Button *%s_buttons[] = {\n", rootname);
	rwrite_blist(root->children);
	fprintf(OUT, "};\n\n");
	fprintf(OUT, "void scale_%s_menu(Rscale *scale)\n", rootname);
	fprintf(OUT, "{\n");
	fprintf(OUT, "scale_menu(&%s,%s_buttons,Array_els(%s_buttons),scale);\n",
		root->name, rootname, rootname);
	fprintf(OUT, "}\n");
	}
}


