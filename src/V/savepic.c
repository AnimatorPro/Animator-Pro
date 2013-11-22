
/* savepic.c - Figure out whether it's GIF or uncompressed image.  
	Then load/save if uncompressed.  If GIF bail out to gif.c.  */

#include "jimk.h"
#include "fli.h"
#include "savepic.str"

extern unsigned *brun();


save_pic(name, screen, squawk)
char *name;
Vscreen *screen;
int squawk;
{
int f;
struct pic_header pic;
PLANEPTR buf, endbuf;
int cbuf;

if ((f = jcreate(name)) == 0)
	{
	if (squawk)
		cant_create(name);
	return(0);
	}
zero_structure(&pic, sizeof(pic) );
pic.csize = (long)screen->h*screen->bpr;
pic.type = PIC_MAGIC;
pic.w = screen->w;
pic.h = screen->h;
pic.x = screen->x;
pic.y = screen->y;
pic.d = DEPTH;
if (jwrite(f, &pic, (long)sizeof(pic)) < sizeof(pic) )
	goto TRUNC;
if (jwrite(f, screen->cmap, COLORS*3L) < COLORS*3L)
	goto TRUNC;
if (jwrite(f, screen->p, pic.csize) < pic.csize)
	goto TRUNC;
jclose(f);
return(1);
TRUNC:
if (squawk)
	truncated(name);
jclose(f);
jdelete(name);
return(0);
}


load_some_pic(name, screen)
char *name;
Vscreen *screen;
{
if (suffix_in(name, ".PIC"))
	return(load_pic(name,screen) );
else
	return(load_gif(name, screen));
}

load_pic(name, screen)
char *name;
Vscreen *screen;
{
int f;
struct pic_header pic;

if ((f = jopen(name, 0)) == 0)
	{
	cant_find(name);
	return(0);
	}
if (jread(f, &pic, (long)sizeof(pic)) < sizeof(pic) )
	goto TRUNC;
if (pic.type != PIC_MAGIC)
	{
	continu_line(savepic_101 /* "Not a PIC file" */);
	goto BADOUT;
	}
if (jread(f, screen->cmap, COLORS*3L) < COLORS*3L)
	goto TRUNC;
if (jread(f, screen->p, 64000L) < 64000L)
	goto TRUNC;
jclose(f);
return(1);
TRUNC:
truncated(name);
BADOUT:
jclose(f);
return(0);
}

#ifdef SLUFFED
unscale_cmap(cmap)
register PLANEPTR cmap;
{
register int i;
UBYTE r,g,b;

i = COLORS;
while (--i >= 0)
	{
	b = cmap[0]>>2;
	g = cmap[1]>>2;
	r = cmap[2]>>2;
	*cmap++ = r;
	*cmap++ = g;
	*cmap++ = b;
	}
}
#endif SLUFFED

#ifdef SLUFFED
load_vga(name, screen)
char *name;
Vscreen *screen;
{
int f;
struct vga_header pic;
char linebuf[XMAX];
int i;
PLANEPTR p;

if ((f = jopen(name, 0)) == 0)
	{
	cant_find(name);
	return(0);
	}
if (jread(f, &pic, (long)sizeof(pic)) < sizeof(pic) )
	goto TRUNC;
if (pic.type != VGA_MAGIC)
	{
	continu_line("Not a VGA file");
	goto BADOUT;
	}
if (jread(f, screen->cmap, COLORS*3L) < COLORS*3L)
	goto TRUNC;
unscale_cmap(screen->cmap);
p = screen->p + (screen->h-1)*XMAX;
for (i=0; i<screen->h; i++)
	{
	if (jread(f, linebuf, (long)XMAX) < XMAX)
		goto TRUNC;
	copy_structure(linebuf, p, XMAX);
	p -= XMAX;
	}
jclose(f);
return(1);
TRUNC:
truncated(name);
BADOUT:
jclose(f);
return(0);
}
#endif SLUFFED

