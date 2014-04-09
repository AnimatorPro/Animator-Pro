
#include <stdio.h>
#include <ctype.h>

#define CH_WIDTH 6
#define CH_HEIGHT 8
#define XMAX 320
/* characters between top level menus */
#define MSPC 3

extern void *calloc(int, int);
typedef int (*Vector)();

#define OUT out

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
Pull *root;
Pull *last_hi, *last_lo;

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
if (argc != 3)
	{
	puts("Usage: makepull infile.txt outfile.c");
	puts("   creates a c file with menu data structures suggested by");
	puts("   what's in infile.txt");
	exit(0);
	}
inname = argv[1];
outname = argv[2];
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
read_menus();
write_menus();
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
fprintf(out, "/* generated with makepull */\n");
fprintf(out, "#include \"jimk.h\"\n");
fprintf(out, "#include \"flicmenu.h\"\n");
fprintf(out, "\n\n");
}

char *
skip_space(s)
char *s;
{
while (isspace(*s))
	s++;
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

Pull *
read_m()
{
Pull *p;

if (!next_line())
	return(0);
if ((p = calloc(1, sizeof(*p))) == NULL)
	{
	puts("Out of memory");
	return(0);
	}
p->level = isspace(inbuf[0]);	/* 1'st or second level */
if ((p->data = clone_string(remove_trailing(skip_space(inbuf))))==NULL)
	{
	puts("Too many strings");
	return(0);
	}
return(p);
}

copy_bytes(s,d,count)
register char *s,*d;
int count;
{
while (--count >= 0)
	*d++ = *s++;
}


char *
single_name()
{
char buf[256];
char sb[8];

copy_bytes(last_hi->data, sb, 3);
sb[3] = 0;
sprintf(buf, "%s_pull", sb);
return(clone_string(buf));
}

char *
double_name()
{
char buf[256];
char sb[8],db[8];

copy_bytes(last_hi->data, sb, 3);
sb[3] = 0;
copy_bytes(last_lo->data, db, 3);
db[3] = 0;
sprintf(buf, "%s_%s_pull", sb, db);
return(clone_string(buf));
}



read_menus()
{
Pull *p;

for (;;)
	{
	if ((p = read_m()) == NULL)
		break;
	if (root == NULL)
		{
		root = last_hi = p;
		}
	else
		{
		if (p->level == 0)	/* root level */
			{
			last_hi->next = p;
			last_lo = NULL;
			last_hi = p;
			}
		else
			{
			if (last_lo == NULL)
				{
				last_hi->children = last_lo = p;
				}
			else
				{
				last_lo->next = p;
				last_lo = p;
				}
			}
		}
	if (p->level == 0)
		p->name = single_name();
	else
		p->name = double_name();
	if (p->name == NULL)
		{
		puts("Out of memory");
		break;
		}
	lcase(p->name);
	}
}

print_idents(ident)
int ident;
{
while (--ident >= 0)
	fprintf(OUT, "\t");
}

print_by_cw(w)
int w;
{
fprintf(OUT,"%d+%d*CH_WIDTH, ",
	w%CH_WIDTH, w/CH_WIDTH);
}

print_by_ch(h)
int h;
{
fprintf(OUT,"%d+%d*CH_HEIGHT, ",
	h%CH_HEIGHT, h/CH_HEIGHT);
}

write_m(p, ident)
Pull *p;
int ident;
{
print_idents(ident);
fprintf(OUT, "struct pull %s = {\n", p->name);
ident += 1;
print_idents(ident);
if (p->next == NULL)
	fprintf(OUT, "NONEXT,\n");
else
	fprintf(OUT, "&%s,\n", p->next->name);
print_idents(ident);
print_by_cw(p->xoff);
print_by_ch(p->yoff);
print_by_cw(p->width);
print_by_ch(p->height);
fprintf(OUT,"\n");
print_idents(ident);
if (p->children == NULL)
	fprintf(OUT, "NOCHILD,\n");
else
	fprintf(OUT, "&%s,\n", p->children->name);
print_idents(ident);
if (p->data == NULL)
	fprintf(OUT, "NODATA,\n");
else
	fprintf(OUT, "\"%s\",\n", p->data);
print_idents(ident);
if (p->data == NULL)
	fprintf(OUT, "pull_oblock,\n");
else
	fprintf(OUT, "pull_text,\n");
print_idents(ident);
fprintf(OUT, "0,\n");
print_idents(ident);
fprintf(OUT, "};\n");
}

write_tree(p, ident)
Pull *p;
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

longest_string(p)
Pull *p;
{
int longest = 0;
int i;

while (p != NULL)
	{
	i = strlen(p->data);
	if (i > longest)
		longest = i;
	p = p->next;
	}
return(longest);
}


calc_p(p)
Pull *p;
{
Pull *rp;
Pull *cp;
int w, h;
int y;
char buf[256];

if ((rp = calloc(1, sizeof(*rp))) == NULL)
	{
	puts("Out of memory");
	return(0);
	}
w = longest_string(p->children);
rp->width = w*CH_WIDTH+5;
h = list_els(p->children);
rp->height = h*CH_HEIGHT+3;
rp->xoff = -1;
rp->yoff = CH_HEIGHT;
sprintf(buf, "r%s", p->name);
if ((rp->name = clone_string(buf)) == NULL)
	{
	puts("Out of memory");
	return(0);
	}
cp = rp->children = p->children;
p->children = rp;
y = 1;
while (cp != NULL)
	{
	cp->xoff = 1;
	cp->yoff = y;
	cp->width = 3+CH_WIDTH*w;
	cp->height = 1+CH_HEIGHT;
	y += CH_HEIGHT;
	cp = cp->next;
	}
return(1);
}

calc_sub_positions()
{
Pull *p;

p = root;
while (p!=NULL)
	{
	if (!calc_p(p))
		break;
	p = p->next;
	}
}

calc_root_positions()
{
Pull *p;
Pull *rp;
int x, w;

p = root;
x = CH_WIDTH;
while (p != NULL)
	{
	w = (MSPC+strlen(p->data))*CH_WIDTH;
	p->xoff = x;
	p->yoff = -1;
	p->width = w;
	p->height = CH_HEIGHT;
	x += w;
	p = p->next;
	}
if ((rp = calloc(1, sizeof(*rp))) == NULL)
	{
	puts("Out of memory");
	return(0);
	}
rp->name = "root_pull";
rp->xoff = rp->yoff = 0;
rp->width = XMAX;
rp->height = 8;
rp->children = root;
root = rp;
return(1);
}

write_menus()
{
calc_sub_positions();
calc_root_positions();
write_tree(root,0);
}

