
#include <stdio.h>

#define PIC_MAGIC 0x9119
struct pic_header
	{
	unsigned type;
	int w,h,x,y;
	char d;
	char compress;
	long csize;
	char reserved[16];
	};

FILE *in, *out;
char *inname, *outname;
struct pic_header pic;

bad_source()
{
printf("%s is wrong type of file, or is damaged\n", inname);
exit(-3);
}

bad_dest()
{
printf("%s truncated or damaged!\n", outname);
exit(-4);
}

main(argc, argv)
int argc;
char *argv[];
{
int i;

if (argc != 3)
	{
	puts("Makeicon - convert a .cel file to a single bit-plane C source");
	puts("file useful for menu icons.");
	puts("");
	puts("usage:");
	puts("\tmakeicon source.cel dest.c");
	}
inname = argv[1];
outname = argv[2];
if ((in = fopen(inname, "rb")) == NULL)
	{
	printf("Couldn't find %s\n", inname);
	exit(-1);
	}
if ((out = fopen(outname, "w")) == NULL)
	{
	printf("Couldn't create %s\n", outname);
	exit(-2);
	}
if (fread(&pic, sizeof(pic), 1, in) != 1)
	bad_source();
if (pic.type != PIC_MAGIC)
	bad_source();
/* skip color map... */
fseek(in, 3L*256L, SEEK_CUR);
fprintf(out, "/* %dx%d */\n", pic.w, pic.h);
for (i=0; i<pic.h; i++)
	do_line();
}

do_line()
{
int i;
int width_left;

width_left = pic.w;
for (i=0; i<pic.w; i+=8)
	{
	do_byte(width_left);
	width_left -= 8;
	if ((i/8)%10 == 9)
		fprintf(out, "\n\t");
	}
if ((i/8)%10 != 0)
	fprintf(out, "\n");
}

do_byte(bits)
int bits;
{
int byte;
int mask;
int c;

if (bits > 8)
	bits = 8;
byte = 0;
mask = 0x80;
while (--bits >= 0)
	{
	if ((c = getc(in)) == EOF)
		bad_source();
	if (c)
		byte |= mask;
	mask >>= 1;
	}
fprintf(out, "0x%x,", byte);
}

