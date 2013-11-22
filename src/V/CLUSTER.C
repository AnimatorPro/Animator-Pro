
/* Cluster.c - most of the routines dealing with color cluster one
   way or another.  Some connections to palet2.c and cpack.c */

#include "jimk.h"
#include "flicmenu.h"
#include "cluster.str"

extern Flicmenu pal_men_sel, pal_pal_sel,  rpal_pal_sel, pal_spe_sel,
	palette_menu, spec1_sel,
	pal_bun_sel, pal_cco_sel;

static WORD c1, c2;
static char *icmap;


static Flicmenu *bsel[] = {&pal_bun_sel, &pal_spe_sel};

/* macros for # of colors in current bundle, and current bundle
   colors list */
#define bctt (vs.buns[vs.use_bun].bun_count)
#define bndl (vs.buns[vs.use_bun].bundle)

cmap_to_cluster(cmap, ccount)
UBYTE *cmap;
int ccount;
{
int i;
UBYTE *s;
UBYTE *b;

s = render_form->cmap;
for (i=0; i<ccount && i <bctt; i++)
	{
	copy_bytes(cmap, s+3*bndl[i], 3);
	cmap += 3;
	}
}


static
clus_cmap(cmap)
UBYTE *cmap;
{
UBYTE *s;
int i;

s = render_form->cmap;
for (i=0; i<bctt; i++)
	{
	copy_bytes(s+3*bndl[i], cmap, 3);
	cmap += 3;
	}
}

UBYTE *
cluster_to_cmap()
{
UBYTE *cmap;

if ((cmap = begmem(bctt * 3) ) != NULL)
	clus_cmap(cmap);
return(cmap);
}

static
ccycle1(ix, intween)
int ix;
int intween;
{
UBYTE *cm, *cm2;
int ok;
int si, di;
int ccount;

if (intween == 1)	/* make it so single frame cycles in segment or all work */
	ix = 1;
ok = 0;
ccount = cluster_count();
cm = cluster_to_cmap();
if (cm != NULL)
	{
	if ((cm2 = begmem(ccount*3)) != NULL)
		{
		for (si=0; si<ccount; si++)
			{
			di = (si+ix);
			while (di >= ccount)
				di -= ccount;
			copy_bytes(cm + si*3, cm2 + di*3, 3);
			}
		cmap_to_cluster(cm2, ccount);
		freemem(cm2);
		ok = 1;
		}
	freemem(cm);
	refit_vf();
	}
return(ok);
}

ccycle()
{
pmhmpauto(ccycle1);
}



cl_cut()
{
UBYTE *cbuf;
long ccut_size;

if (vs.pal_to == 0)		/* bundle... */
	{
	ccut_size = bctt * 3 + 1;
	if ((cbuf = lbegmem(ccut_size) ) == NULL)
		return;
	cbuf[0] = bctt;
	clus_cmap(cbuf+1);
	}
else
	{
	ccut_size = 3*COLORS+1;
	if ((cbuf = lbegmem(ccut_size) ) == NULL)
		return;
	cbuf[0] = 0;
	copy_cmap(render_form->cmap, cbuf+1);
	}
write_gulp(cclip_name, cbuf, ccut_size);
freemem(cbuf);
}


static
do_ramp_cluster(s1, s2, count, b)
UBYTE *s1, *s2;
int count;
struct bundle *b;
{
UBYTE rgb[3];
int i, j;

for (i=0; i<count; i++)
	{
	for (j=0; j<3; j++)
		rgb[j] = interp_range(s1[j], s2[j], i, count);
	b->bundle[i] = closestc(rgb, render_form->cmap, COLORS);
	}
b->bun_count = count;
}


/* figure how much difference there is from a 'perfect' ramp of colors
   and the closest count sized ramp we can find in current color map */
static long
sum_ramp_error(s1, s2, count)
UBYTE *s1, *s2, count;
{
int d;
long acc;
UBYTE *d1;
UBYTE rgb[3];
int i, j;
long h;

/* sum deviation from ideal sample */
acc = 0;
for (i=0; i<count; i++)
	{
	for (j=0; j<3; j++)
		rgb[j] = interp_range(s1[j], s2[j], i, count);
	d1 = render_form->cmap + 3*closestc(rgb, render_form->cmap, COLORS);
	for (j=0; j<3; j++)
		{
		acc += (intabs(rgb[j] - d1[j])<<4);
		}
	}

/* add in  normal sampling error for that big of sample */
h = 0;
for (j=0; j<3; j++)
	h += (intabs(s1[j] - s2[j])<<4);

acc += h/(2*count);
acc /= count;
return(acc);
}

/* Figure out best size for ramp during a 'find ramp' */
static
find_ramp_count(r1, r2)
UBYTE *r1, *r2;	/* ramp truecolor endpoints */
{
int best;
long dist, ldist;
int i;

for (i=2; i<=32; i++)
	{
	ldist = sum_ramp_error(r1, r2, i);
	if (i == 2 || ldist < dist)
		{
		best = i;
		dist = ldist;
		}
	}
return(best);
}


static char *rgb_word;

static
show_rgb(c)
int c;
{
UBYTE *r;
char buf[40];

r = render_form->cmap + 3*c;
sprintf(buf, " %s (%d %d %d)      ", rgb_word, r[0], r[1], r[2]);
top_text(buf);
}

static
show_startc(c)
int c;
{
char buf[40];

sprintf(buf, cluster_101 /* " start %d   " */, c);
top_text(buf);
}


#define PWP_HEIGHT 6
static unsigned pwp_xoff[32] = {
	  1, 11, 21, 31, 41, 51, 61, 71, 81, 91,101,111,121,131,141,151,
	161,170,180,190,200,210,220,230,240,250,260,270,280,290,300,310,
	};
static unsigned pwp_width[32] = {
	9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,8,
	8,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,
	};

static unsigned pwp_yoff[8] = {
	0,7,14,21,29,36,43,50,
	};

/* find out which color in color matrix cursor is over. */
static
which_pp(yoff)
int yoff;
{
int i,j;
int y, x;

for (j=0; j<8; j++)
	{
	y = yoff + pwp_yoff[j] + PWP_HEIGHT;
	if (uzy <= y)
		{
		for (i=0; i<32; i++)
			{
			x = pwp_xoff[i] + pwp_width[i];
			if (uzx <= x)
				{
				return(j*32+i);
				}
			}
		}
	}
return(255);	/* should never happen */
}

/* respond to left click over color matrix */
feel_pp(m)
Flicmenu *m;
{
int c;

vs.ccolor = which_pp(m->y);
predraw();
}



/* figure out which color cursor is over.  Just do a getdot in most cases,
   but if it's over a color control button of some sort be more
   sophisticated.  If over color matrix call color matrix which color
   routine so don't pick up grey dividing lines between colors.  Similarly
   if in a cluster call cluster color finder. */

static
get_pp_color()
{
int c;

c = getdot(uzx, uzy);
if (cur_menu == &palette_menu)
	{
	if (in_menu(&pal_pal_sel))
		{
		c = which_pp(pal_pal_sel.y);
		}
	else if (in_menu(&pal_bun_sel))
		{
		c = f_cbun(&pal_bun_sel);
		}
	else if (in_menu(&pal_spe_sel))
		{
		c = f_cbun(&pal_spe_sel);
		}
	}
else if (in_menu(&spec1_sel))
	c = f_cbun(&spec1_sel);
return(c);
}


static
draw_cco()
{
extern Flicmenu *ccolor_sel;

if (cur_menu == &palette_menu)
	draw_sel(&pal_cco_sel);
else
	draw_sel(&ccolor_sel);
}


get_a_end(dfunc)
Vector dfunc;
{
int c;
int occolor;

occolor = vs.ccolor;
for (;;)
	{
	c = get_pp_color();
	vs.ccolor = c;
	draw_cco();
	(*dfunc)(c);
	wait_input();
	if (PJSTDN)
		{
		break;
		}
	if (RJSTDN)
		{
		c = -1;
		break;
		}
	}
restore_top_bar();
vs.ccolor = occolor;
draw_cco();
return(c);
}

static UBYTE r1[3], r2[3];

static
define_ramp()
{
int c;

rgb_word = cluster_102 /* "start" */;
if ((c = get_a_end(show_rgb)) < 0)
	return(0);
copy_bytes(render_form->cmap + 3*c, r1, 3);
rgb_word = cluster_103 /* "stop" */;
if ((c = get_a_end(show_rgb)) < 0)
	return(0);
copy_bytes(render_form->cmap + 3*c, r2, 3);
return(1);
}

static
ramp_cluster(which)
int which;
{
if (!define_ramp())
	return;
pp_hi_bundle(sgrey);
do_ramp_cluster(r1, r2, find_ramp_count(r1, r2) , vs.buns+which);
pp_hi_bundle(sbright);
}

static
framp1()
{
cmap_to_cluster(icmap, bctt);
refit_vf();
return(1);
}

static int clp_count;

static
cl_paste1()
{
if (vs.pal_to == 0)	/* to cluster */
	cmap_to_cluster(icmap, clp_count);
else
	copy_bytes(icmap, render_form->cmap, clp_count*3);
refit_vf();
return(1);
}

static
cl_pblend(autov)
Vector autov;
{
int colors;
long ccut_size;
unsigned char c;
int f;

icmap = NULL;
if ((f = jopen(cclip_name, 0)) == 0)
	{
	cant_find(cclip_name);
	goto OUT;
	}
jread(f, &c, 1L);
if ((clp_count = c) == 0)
	clp_count = 256;
ccut_size = clp_count * 3;
if ((icmap = lbegmem(ccut_size)) == NULL)
	{
	jclose(f);
	goto OUT;
	}
jread(f, icmap, ccut_size);
jclose(f);
uzauto(autov);
OUT:
gentle_freemem(icmap);
}

cl_paste()
{
hide_mp();
cl_pblend(cl_paste1);
draw_mp();
}

static
cl_blend_1c(scale, dcol, cix, ix, count)
int scale,cix,ix,count;
UBYTE *dcol;
{
if (ix >= clp_count)
	return;
true_blend(dcol, icmap + 3*ix, itmult(scale, vs.cblend), dcol);
}

static
cl_blend1(ix, intween, scale)
int ix, intween, scale;
{
some_cmod(cl_blend_1c,scale);
return(1);
}

cl_blend()
{
hide_mp();
if (qreq_number(cluster_104 /* "Max blend percent?" */, &vs.cblend, 0, 100) )
	cl_pblend(cl_blend1);
draw_mp();
}


some_cmod(f, scale)
Vector f;
int scale;
{
UBYTE *bun;
UBYTE *p, *d;
int i;
int cix, rix;
int bct;

p = render_form->cmap;
if (vs.pal_to == 0)		/* bundle... */
	{
	bun = bndl;
	bct = bctt;
	for (i=0; i<bct; i++)
		{
		cix = *bun++;
		d = p + 3 * cix;
		(*f)(scale, d, cix, i, bct);
		}
	}
else
	{
	for (cix = 0; cix < COLORS; cix++)
		{
		(*f)(scale, p, cix, cix, COLORS);
		p += 3;
		}
	}
refit_vf();
}


static
unique_cluster(s, d)
struct bundle *s;
struct bundle *d;
{
int i;
UBYTE c;

d->bun_count = 0;
for (i=0; i<s->bun_count; i++)
	{
	c = s->bundle[i];
	if (!in_a_cluster(c, d) )
		{
		d->bundle[d->bun_count++] = c;
		}
	}
}


static
tint_1c(scale, p)
int scale;
UBYTE *p;
{
true_blend(p, r1, itmult(scale, vs.ctint), p);
return(1);
}


static
ctint1(ix, intween, scale)
int ix, intween, scale;
{
some_cmod(tint_1c, scale);
return(1);
}

ctint()
{
int c;
struct bundle uniq;
struct bundle *cb;

rgb_word = cluster_105 /* "tinting source" */;
if ((c = get_a_end(show_rgb)) < 0)
	return;
copy_bytes(render_form->cmap + 3*c, r1, 3);
hide_mp();
if (!qreq_number(cluster_106 /* "Max tint percent?" */, &vs.ctint, 0, 100) )
	{
	draw_mp();
	return;
	}
/* force each cluster color to be used only once or will look wierd 
   if ping-ponged */
cb = vs.buns+vs.use_bun;
unique_cluster(cb,&uniq);
exchange_bytes(cb,&uniq,sizeof(uniq));
uzauto(ctint1);
exchange_bytes(cb,&uniq,sizeof(uniq));
draw_mp();
}

static
neg_1c(scale, p)
int scale;
UBYTE *p;	/* rgb value */
{
UBYTE nrgb[3];

nrgb[0] = 63-p[0];
nrgb[1] = 63-p[1];
nrgb[2] = 63-p[2];
true_blend(p, nrgb, itmult(scale, 100), p);
}

static
cneg1(ix, intween, scale)
int ix, intween, scale;
{
some_cmod(neg_1c, scale);
return(1);
}

cneg()
{
struct bundle uniq;
struct bundle *cb;

/* force each cluster color to be used only once or will look wierd 
   if ping-ponged */
cb = vs.buns+vs.use_bun;
unique_cluster(cb,&uniq);
exchange_bytes(cb,&uniq,sizeof(uniq));
hmpauto(cneg1);
exchange_bytes(cb,&uniq,sizeof(uniq));
}


force_ramp()
{
if (!define_ramp())
	return;
if ((icmap = cluster_to_cmap()) == NULL)
	return;
rampit(r1, r2, icmap, bctt, 1);
pmhmpauto(framp1);
freemem(icmap);
}



/* 109 is 64*sqr_root(3) change if have more levels of grey */
#define CSCALE 109

static
make_close_bundle(rgb, threshold)
UBYTE *rgb;
int threshold;
{
int i;
UBYTE *c;

threshold = sscale_by(threshold, CSCALE, 100);
threshold *= threshold;

c = render_form->cmap;
bctt = 0;
for (i=0; i<COLORS; i++)
	{
	if (color_dif(rgb, c) <= threshold)
		bndl[bctt++] = i;
	c += 3;
	}
}

cclose()
{
int ok;

rgb_word = cluster_107 /* "Near Color" */;
if ((c1 = get_a_end(show_rgb)) < 0)
	return;
hide_mp();
ok = qreq_number(cluster_108 /* "Cluster near threshold?" */, 
	&vs.cclose, 0, 100);
if (ok)
	make_close_bundle(render_form->cmap + 3*c1, vs.cclose);
draw_mp();
}

static
cluster_unused(ctab, ccount)
char *ctab;
int ccount;
{
int i;

bctt = 0;
for (i=0; i<COLORS; i++)
	{
	if (!*ctab++)
		{
		if (--ccount < 0)
			break;
		bndl[bctt++] = i;
		}
	}
}

static
in_a_cluster(c,bun)
UBYTE c;
struct bundle *bun;
{
int i;
UBYTE *b;


i = bun->bun_count;
b = bun->bundle;
while (--i >= 0)
	{
	if (c == *b++)
		return(1);
	}
return(0);
}

cluster_invert()
{
struct bundle new;
int i;

new.bun_count = 0;
for (i=0; i<COLORS; i++)
	{
	if (!in_a_cluster(i, vs.buns+vs.use_bun))
		{
		new.bundle[new.bun_count++] = i;
		}
	}
pp_hi_bundle(sgrey);
copy_structure(&new, vs.buns+vs.use_bun, sizeof(struct bundle) );
pp_hi_bundle(sbright);
draw_sel(bsel[vs.use_bun]);
}

cluster_reverse()
{
reverse_bytes(bndl,bctt);
draw_sel(bsel[vs.use_bun]);
}


/* places colors inside line into cluster.  Ignores key color usually.  */
cluster_line()
{
hide_mp();
wait_click();
if (PJSTDN)
	{
	save_undo();
	if (rub_line())
		{
		dto_table(render_form->p,render_form->bpr, bndl,
			64, x_0,y_0,x_1,y_1);
		bctt = 64;
		}
	}
draw_mp();
}

cunused()
{
char cflags[256];
int unusedc, ccount;

hide_mp();
make_cused(render_form->p, cflags);
unusedc = ccount = COLORS - count_cused(cflags, (long)COLORS);
if (unusedc < 1)
	{
	continu_line(cluster_109 /* "No unused colors." */);
	}
else
	{
	if (qreq_number(cluster_110 /* "Number of unused colors for cluster?" */, 
		&ccount, 1, ccount) )
		{
		if (ccount > unusedc)
			ccount = unusedc;
		cluster_unused(cflags, ccount);
		}
	}
draw_mp();
}



static
show_secondc(c2)
int c2;
{
char buf[40];

sprintf(buf, cluster_111 /* " Start %3d Colors %3d Stop %3d" */, 
	c1, intabs(c1-c2)+1, c2);
top_text(buf);
}

static
scrange(b)
struct bundle *b;
{
int ccount, dc;
UBYTE *pb;

if ((c1 = get_a_end(show_startc)) < 0)
	return;
if ((c2 = get_a_end(show_secondc)) < 0)
	return;
pp_hi_bundle(sgrey);
if (c2 >= c1)
	{
	dc = 1;
	ccount = (c2 - c1 + 1);
	}
else
	{
	dc = -1;
	ccount = (c1 - c2 + 1);
	}
b->bun_count = ccount;
pb = b->bundle;
while (--ccount >= 0)
	{
	*pb++ = c1;
	c1 += dc;
	}
pp_hi_bundle(sbright);
}

static
select_bundle(which)
int which;
{
Flicmenu *m;

m = bsel[which];
hilight(m);
scrange(vs.buns+which);
draw_sel(m);
}

qselect_bundle()
{
select_bundle(vs.use_bun);
}


find_ramp()
{
Flicmenu *me;

me = bsel[vs.use_bun];
hilight(me);
ramp_cluster(vs.use_bun);
draw_sel(me);
}

mselect_bundle(m)
Flicmenu *m;
{
select_bundle(m->identity);
}


static
bp_color(ix, ocolor)
int ix;
int ocolor;
{
int x,y,w,h;

y = pal_pal_sel.y + pwp_yoff[ix>>5];
ix &= 31;
x = pal_pal_sel.x + pwp_xoff[ix];
w = pwp_width[ix]+1;
draw_frame(ocolor, x-1, y-1, x+w, y+PWP_HEIGHT+1);
}

static
pp_hi_bundle(ocolor)
int ocolor;
{
int color, i, count;

count = bctt;
while (--count >= 0)
	bp_color(bndl[count], ocolor);
}


change_cluster_mode(m)
Flicmenu *m;
{
pp_hi_bundle(sgrey);
change_mode(m);
pp_hi_bundle(sbright);
}

static
pp_inner_colors(yoff)
int yoff;
{
int i,j;
int y, h, color;

color = 0;
for (j=0; j<8; j++)
	{
	y = yoff + pwp_yoff[j];
	for (i=0; i<32; i++)
		{
		cblock(vf.p,pwp_xoff[i],y,pwp_width[i], PWP_HEIGHT, color);
		color++;
		}
	}
}

static
pp_hi_ccolor(yoff)
int yoff;
{
int tx, ty;

tx = vs.ccolor&31;
ty = (vs.ccolor>>5);
tx = pwp_xoff[tx] + ((pwp_width[tx])>>1);
ty = yoff + pwp_yoff[ty] + 2;
putdot(tx,ty,sbright);
putdot(tx,ty+1,sblack);
bp_color(vs.ccolor, sred);
}

/* erode frames around system coopted colors */

static
pp_eat_sys()	
{
extern UBYTE sys5;
int i;

if (sys5)
	{
	for (i=251; i<COLORS; i++)
		bp_color(i, sblack);
	}
}

see_powell_palette(m)
Flicmenu *m;
{
int yoff;
static int lastcc;


bp_color(lastcc, sgrey);	/* stomp down old ccolor hilight */
lastcc = vs.ccolor;
yoff = m->y;
pp_inner_colors(yoff);
pp_hi_bundle(sbright);
pp_hi_ccolor(yoff);
pp_eat_sys();
}


static
show_ccp1(c1)
int c1;
{
char buf[40];

sprintf(buf, cluster_112 /* " Source %3d" */, c1);
top_text(buf);
}

static unsigned char ccopy_rgb[3];

static
ccopy1()
{
copy_bytes(ccopy_rgb,render_form->cmap+3*c2,3);
return(1);
}



ping_cluster()
{
int count;
struct bundle *bun;
UBYTE *b, *endb, *bb, *newb;

bun = vs.buns+vs.use_bun;
count = bun->bun_count;
b = bun->bundle;
endb = b + COLORS;
newb = b + count;
bb = newb-1;
count -= 1;
while (--count >= 0)
	{
	if (newb == endb)
		break;
	*newb++ = *(--bb);
	bun->bun_count++;
	}
draw_sel(bsel[vs.use_bun]);
}


static
cl_swap1()
{
static int bcount;
int i;
UBYTE *b1, *b2;

/* set count to smaller of 2 bundles */
bcount = vs.buns[0].bun_count;
i = vs.buns[1].bun_count;
if (i < bcount)
	bcount = i;

b1 = vs.buns[0].bundle;
b2 = vs.buns[1].bundle;
while (--bcount >= 0)
	{
	exchange_bytes(render_form->cmap + 3 * *b1++,
		render_form->cmap + 3 * *b2++, 3);
	}
refit_vf();
return(1);
}

cl_swap()
{
pmhmpauto(cl_swap1);
}

right_click_pp(m)
Flicmenu *m;
{
int s;

c2 = which_pp(m->y);	/* get destination color from matrix */
bp_color(c2, sred);		/* red outline abuser feedback */
s = get_a_end(show_ccp1);
if (s >= 0)
	{
	copy_bytes(render_form->cmap+3*s,ccopy_rgb,3);
	pmhmpauto(ccopy1);
	}
else
	{
	bp_color(c2, sgrey);		/* stomp on feedback... */
	draw_sel(&pal_pal_sel);
	}
}


