/* Cluster.c - most of the routines dealing with color cluster one
   way or another.  Some connections to palet2.c and cpack.c */

#include "jimk.h"
#include "auto.h"
#include "errcodes.h"
#include "flx.h"
#include "marqi.h"
#include "memory.h"
#include "menus.h"
#include "palmenu.h"
#include "pentools.h"

static SHORT c1, c2;
static Rgb3 *ictab;

static void pp_unhi_bundle(Button *b);
static void pp_hi_bundle(Button *b, int ocolor);
static void pp_hi_ccolor(Button *b);
static int f_cbun(Button *m);

int in_bundle(Pixel color, Bundle *bun)
/* returns 0 if not found otherwise ix+1 of bundle color */
{
	return(in_cnums(color, bun->bundle, bun->bun_count));
}

static Button *bsel[] = {&pal_bun_sel, &pal_spe_sel};

/* macros for # of colors in current bundle, and current bundle
   colors list */
#define bctt (vs.buns[vs.use_bun].bun_count)
#define bndl (vs.buns[vs.use_bun].bundle)

void ctable_to_cluster(Rgb3 *ctab, int ccount)
{
int i;
Rgb3 *s;

	s = vb.pencel->cmap->ctab;
	for (i=0; i<ccount && i <bctt; i++)
	{
		s[bndl[i]] = *ctab;
		++ctab;
	}
}

static void clus_ctab(Rgb3 *ctab)
{
Rgb3 *s;
int i;

	s = vb.pencel->cmap->ctab;
	for (i=0; i<bctt; i++)
	{
		*ctab = s[bndl[i]];
		++ctab;
	}
}

Rgb3 *cluster_to_ctable(void)
{
Rgb3 *ctab;

	if ((ctab = begmem(bctt * sizeof(Rgb3)) ) != NULL)
		clus_ctab(ctab);
	return(ctab);
}

static Errcode ccycle1(void *data, int ix, int intween, int scale, Autoarg *aa)
{
Rgb3 *cm, *cm2;
Errcode err;
int si, di;
int ccount;
(void)data;
(void)scale;
(void)aa;

	if(intween == 1) /* make it so single frame cycles in segment or all work */
		ix = 1;
	err = Err_no_memory;
	ccount = cluster_count();
	cm = cluster_to_ctable();
	if(cm != NULL)
	{
		if((cm2 = begmem(ccount*sizeof(Rgb3))) != NULL)
		{
			for (si=0; si<ccount; si++)
			{
				di = (si+ix);
				while (di >= ccount)
					di -= ccount;
				cm2[di] = cm[si]; 
			}
			ctable_to_cluster(cm2, ccount);
			pj_free(cm2);
			err = 0;
		}
		pj_free(cm);
		refit_vf();
	}
	return(err);
}

void ccycle(void)
{
	pmhmpauto(ccycle1, NULL);
}

void shortcut_ccycle(Button *b)
/* Go switch all the nasty mode flags to be what you _really_ want when
   you're trying to color cycle over time. */
{
BYTE opal_fit;       /* fit option on palette menu */
BYTE omulti;			/* do it to many frames? */
SHORT opal_to;		/* 0 for to cluster, 1 for to all */
(void)b;

	if(flix.overlays != NULL) /* can't do with overlays present */
		return;

	opal_fit = vs.pal_fit;
	omulti = vs.multi;
	opal_to = vs.pal_to;

	vs.pal_fit =  FALSE;
	vs.multi = TRUE;
	vs.pal_to =  0;

	ccycle();

	vs.pal_to = opal_to;
	vs.multi = omulti;
	vs.pal_fit = opal_fit;
}

void cl_cut(void)
{
UBYTE *cbuf;
long ccut_size;

	if (vs.pal_to == 0)		/* bundle... */
	{
		ccut_size = bctt * sizeof(Rgb3) + 1;
		if ((cbuf = begmem(ccut_size) ) == NULL)
			return;
		cbuf[0] = bctt;
		clus_ctab((Rgb3 *)(cbuf+1));
	}
	else
	{
		ccut_size = COLORS*sizeof(Rgb3)+1;
		if ((cbuf = begmem(ccut_size) ) == NULL)
			return;
		cbuf[0] = 0;
		pj_copy_bytes(vb.pencel->cmap->ctab,cbuf+1,COLORS*3);
	}
	write_gulp(cclip_name, cbuf, ccut_size);
	pj_free(cbuf);
}


static void do_ramp_cluster(UBYTE *s1, UBYTE *s2, int count, 
						    struct bundle *b)
{
UBYTE rgb[3];
int i, j;

	for (i=0; i<count; i++)
	{
		for (j=0; j<3; j++)
			rgb[j] = interp_range( s1[j], s2[j], i, count);
		b->bundle[i] = closestc((Rgb3 *)rgb, vb.pencel->cmap->ctab, COLORS);
	}
	b->bun_count = count;
}


static LONG sum_ramp_error(UBYTE *s1, UBYTE *s2, UBYTE count)
/* figure how much difference there is from a 'perfect' ramp of colors
   and the closest count sized ramp we can find in current color map */
{
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
		d1 = (UBYTE *)
		(vb.pencel->cmap->ctab 
				+ closestc((Rgb3 *)rgb, vb.pencel->cmap->ctab, COLORS));
		for (j=0; j<3; j++)
			acc += (intabs(rgb[j] - d1[j])<<4);
	}

/* add in  normal sampling error for that big of sample */
	h = 0;
	for (j=0; j<3; j++)
		h += (intabs(s1[j] - s2[j])<<4);

	acc += h/(2*count);
	acc /= count;
	return(acc);
}

static int find_ramp_count(Rgb3 *r1, Rgb3 *r2) /* ramp truecolor endpoints */
/* Figure out best size for ramp during a 'find ramp' */
{
int best;
long dist, ldist;
int i;

	for (i=2; i<=32; i++)
	{
		ldist = sum_ramp_error((UBYTE *)r1, (UBYTE *)r2, i);
		if (i == 2 || ldist < dist)
		{
			best = i;
			dist = ldist;
		}
	}
	return(best);
}

static char *rgb_key;

static void show_rgb(Pixel c)
{
Rgb3 *rgb;

	rgb = vb.pencel->cmap->ctab + c;
	soft_top_textf("!%-3d%-3d%-3d%-3d", rgb_key,
					c, rgb->r, rgb->g, rgb->b );
}

static void show_startc(Pixel c)
{
	soft_top_textf("!%-3d", "top_startc", c );
}

static SHORT pwp_320xoff[33] = {
	0, 10, 20, 30, 40, 50, 60, 70, 80, 90,100,110,120,130,140,150,
	160,170,180,190,200,210,220,230,240,250,260,270,280,290,300,310,319
	};
static SHORT pwp_200yoff[9] = {
	0,7,14,21,29,36,43,50,57,
	};
static SHORT pwp_xoff[Array_els(pwp_320xoff)];
static SHORT pwp_yoff[Array_els(pwp_200yoff)];

void scale_powell_palette(Rscale *scale)
{
	scale_button(&pal_pal_sel,scale);
	scale_xlist(scale,pwp_320xoff,pwp_xoff,Array_els(pwp_xoff));
	pwp_xoff[Array_els(pwp_320xoff)-1] = pal_pal_sel.width-1;
	scale_ylist(scale,pwp_200yoff,pwp_yoff,Array_els(pwp_yoff));
	pwp_yoff[8] = pal_pal_sel.height-1;
}
static int pwp_width(int ix)
{
	return(pwp_xoff[ix+1] - pwp_xoff[ix]); 
}
static int pwp_height(int ix)
{
	return(pwp_yoff[ix+1] - pwp_yoff[ix]); 
}

static int which_pp(int yoff)
/* find out which color in color matrix cursor is over. always called
 * in palette menu feelme or optme */
{
int i,j;
int y, x;

	for (j=0; j<8; j++)
	{
		y = yoff + pwp_yoff[j+1] - 1;
		if (icb.my <= y)
		{
			for (i=0; i<32; i++)
			{
				x = pwp_xoff[i+1] - 1;
				if (icb.mx <= x)
					return(j*32+i);
			}
		}
	}
	return(255);	/* should never happen */
}

void feel_pp(Button *m)
/* respond to left click over color matrix */
{
	update_ccolor(which_pp(m->y));
}

static int get_pp_color(void)
/* figure out which color cursor is over.  Just do a getdot in most cases,
   but if it's over a color control button of some sort be more
   sophisticated.  If over color matrix call color matrix which color
   routine so don't pick up grey dividing lines between colors.  Similarly
   if in a cluster call cluster color finder. */
{
int c;
Mouset mset;
Menuwndo *mw;

	c = pj_get_dot(vb.screen->viscel, icb.sx, icb.sy); 

	if(palette_menu.mw != NULL)
	{
		if(!(cursin_menu(&palette_menu)))
			goto got_color;

		get_mouset(&mset);
		load_wndo_mouset((Wndo *)palette_menu.mw);

		if (ptin_button(&pal_pal_sel,icb.mx,icb.my))
		{
			c = which_pp(pal_pal_sel.y);
		}
		else if (ptin_button(&pal_bun_sel,icb.mx,icb.my))
		{
			c = f_cbun(&pal_bun_sel);
		}
		else if (ptin_button(&pal_spe_sel,icb.mx,icb.my))
		{
			c = f_cbun(&pal_spe_sel);
		}
	}
	else if(NULL != (mw = get_button_wndo(&qmu_clus_sel)))
	{
		get_mouset(&mset);
		load_wndo_mouset((Wndo *)mw);
		if(ptin_button(&qmu_clus_sel,icb.mx,icb.my))
			c = f_cbun(&qmu_clus_sel);
	}
	else
		goto got_color;

	load_mouset(&mset);
got_color:
	return(c);
}

static void draw_cco(void)
{
	/* note that draw button top will only draw if menu is up */
	draw_buttontop(&pal_cco_sel);
}

static int check_getaend(void (*dfunc)(Pixel c))
{
	(*dfunc)(vs.ccolor = get_pp_color());
	draw_cco();
	return(0);
}

int get_a_end(void (*dfunc)(Pixel c))
{
int c;
Pixel occolor;

	occolor = vs.ccolor;
	anim_wndo_input(MBPEN|MBRIGHT,MMOVE|MBPEN|MBRIGHT,-1,check_getaend,dfunc);
	if(JSTHIT(MBRIGHT))
		c = Err_abort;
	else
		c = vs.ccolor;
	cleanup_toptext();
	vs.ccolor = occolor;
	draw_cco();
	return(c);
}

static Rgb3 r1, r2;

static Boolean define_ramp(void)
{
int c;

	rgb_key = "start_rgb";
	if ((c = get_a_end(show_rgb)) < 0)
		return(FALSE);
	get_color_rgb(c,vb.pencel->cmap,&r1);
	rgb_key = "stop_rgb";
	if ((c = get_a_end(show_rgb)) < 0)
		return(FALSE);
	get_color_rgb(c,vb.pencel->cmap,&r2);
	return(TRUE);
}

static void ramp_cluster(Button *b,int which)
{
Wscreen *s = b->root->w.W_screen;

	if (!define_ramp())
		return;
	pp_unhi_bundle(b);
	do_ramp_cluster((UBYTE *)&r1, (UBYTE *)&r2, 
		find_ramp_count(&r1, &r2) , vs.buns+which);
	pp_hi_bundle(b,s->SBRIGHT);
}

static Errcode framp1(void *data, int ix, int intween, int scale, Autoarg *aa)
{
	(void)data;
	(void)ix;
	(void)intween;
	(void)scale;
	(void)aa;

	ctable_to_cluster(ictab, bctt);
	refit_vf();

	return Success;
}

static int clp_count;

static int cl_paste1(void)
{
	if (vs.pal_to == 0)	/* to cluster */
		ctable_to_cluster(ictab, clp_count);
	else
		pj_copy_bytes(ictab, vb.pencel->cmap->ctab, clp_count*3);
	refit_vf();
	return(0);
}

static void cl_pblend(EFUNC autov)
{
	Errcode err;
	long ccut_size;
	unsigned char c;
	XFILE *xf;

	ictab = NULL;

	err = xffopen(cclip_name, &xf, XREADONLY);
	if (err < Success) {
		cant_find(cclip_name);
		return;
	}

	err = xffread(xf, &c, 1);
	if (err < Success)
		goto cleanup;

	clp_count = c;
	if (clp_count == 0)
		clp_count = 256;

	ccut_size = clp_count * 3;
	ictab = begmem(ccut_size);
	if (ictab == NULL)
		goto cleanup;

	xffread(xf, ictab, ccut_size);
	uzauto(autov, NULL);

cleanup:
	pj_gentle_free(ictab);
	xffclose(&xf);
}

void cl_paste(void)
{
	hide_mp();
	cl_pblend(cl_paste1);
	show_mp();
}

static void cl_blend_1c(int scale, Rgb3 *dcol, int cix, int ix)
{
	(void)cix;

	if (ix >= clp_count)
		return;
	true_blend(dcol, ictab + ix, itmult(scale, vs.cblend), dcol);
}

static int cl_blend1(void *dat,int ix, int intween, int scale)
{
	(void)dat;
	(void)ix;
	(void)intween;

	some_cmod(cl_blend_1c,scale);
	return(0);
}

void cl_blend(void)
{
	hide_mp();
	if (soft_qreq_number(&vs.cblend,0,100,"max_blend"))
		cl_pblend(cl_blend1);
	show_mp();
}

void
some_cmod(void (*f)(int scale, struct rgb3 *p, int cix, int ix), int scale)
{
UBYTE *bun;
Rgb3 *p;
int i;
int cix;
int bct;

	p = vb.pencel->cmap->ctab;
	if(vs.pal_to == 0)		/* bundle... */
	{
		bun = bndl;
		bct = bctt;
		for (i=0; i<bct; i++)
		{
			cix = *bun++;
			(*f)(scale, p + cix, cix, i);
		}
	}
	else
	{
		for (cix = 0; cix < COLORS; cix++)
		{
			(*f)(scale, p, cix, cix);
			++p;
		}
	}
	refit_vf();
}


static void unique_cluster(struct bundle *s, struct bundle *d)
{
int i;
UBYTE c;

	d->bun_count = 0;
	for (i=0; i<s->bun_count; i++)
	{
		c = s->bundle[i];
		if (!in_bundle(c, d) )
		{
			d->bundle[d->bun_count++] = c;
		}
	}
}

static void tint_1c(int scale, Rgb3 *p, int cix, int ix)
{
	(void)cix;
	(void)ix;

	true_blend(p, &r1, itmult(scale, vs.ctint), p);
}

static Errcode ctint1(void *data, int ix, int intween, int scale, Autoarg *aa)
{
	(void)data;
	(void)ix;
	(void)intween;
	(void)aa;

	some_cmod(tint_1c, scale);
	return Success;
}

void ctint(void)
{
int c;
struct bundle uniq;
struct bundle *cb;

	rgb_key = "tinting_rgb";
	if ((c = get_a_end(show_rgb)) < 0)
		return;
	get_color_rgb(c,vb.pencel->cmap,&r1);
	hide_mp();
	if(!soft_qreq_number(&vs.ctint,0,100,"max_tint") )
	{
		show_mp();
		return;
	}
	/* force each cluster color to be used only once or will look wierd 
	   if ping-ponged */
	cb = vs.buns+vs.use_bun;
	unique_cluster(cb,&uniq);
	swap_mem(cb,&uniq,sizeof(uniq));
	uzauto(ctint1, NULL);
	swap_mem(cb,&uniq,sizeof(uniq));
	show_mp();
}

static void neg_1c(int scale, Rgb3 *p, int cix, int ix)
{
	Rgb3 nrgb;
	(void)cix;
	(void)ix;

	nrgb = *p;
	nrgb.r = (RGB_MAX-1)-nrgb.r;
	nrgb.g = (RGB_MAX-1)-nrgb.g;
	nrgb.b = (RGB_MAX-1)-nrgb.b;
	true_blend(p, &nrgb, itmult(scale, 100), p);
}

static Errcode cneg1(void *data, int ix, int intween, int scale, Autoarg *aa)
{
	(void)data;
	(void)ix;
	(void)intween;
	(void)aa;

	some_cmod(neg_1c, scale);
	return Success;
}

void cneg(void)
{
struct bundle uniq;
struct bundle *cb;

	/* force each cluster color to be used only once or will look wierd 
	   if ping-ponged */
	cb = vs.buns+vs.use_bun;
	unique_cluster(cb,&uniq);
	swap_mem(cb,&uniq,sizeof(uniq));
	hmpauto(cneg1, NULL);
	swap_mem(cb,&uniq,sizeof(uniq));
}


void force_ramp(void)
{
	if (!define_ramp())
		return;
	if ((ictab = cluster_to_ctable()) == NULL)
		return;
	if (vs.hls)
		hls_rampit(&r1, &r2, ictab, bctt);
	else
		rampit(&r1, &r2, ictab, bctt);
	pmhmpauto(framp1, NULL);
	pj_free(ictab);
}

static void make_close_bundle(Rgb3 *rgb, int threshold)
{
int i;
Rgb3 *c;
int cscale;

	cscale = sqr_root((long)RGB_MAX*RGB_MAX*3);
	threshold = sscale_by(threshold, cscale, 100);
	threshold *= threshold;

	c = vb.pencel->cmap->ctab;
	bctt = 0;
	for (i=0; i<COLORS; i++)
	{
		if(color_dif(rgb, c) <= threshold)
			bndl[bctt++] = i;
		++c;
	}
}

void cclose(void)
{
int ok;

	rgb_key = "nearc_rgb";
	if ((c1 = get_a_end(show_rgb)) < 0)
		return;
	hide_mp();
	ok = soft_qreq_number(&vs.cclose,0,100,"near_sep");
	if (ok)
		make_close_bundle(vb.pencel->cmap->ctab + c1, vs.cclose);
	show_mp();
}

static void cluster_unused(char *ctab, int ccount)
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


void cluster_invert(void)
{
struct bundle new;
int i;
Button *b;

	new.bun_count = 0;
	for (i=0; i<COLORS; i++)
	{
		if (!in_bundle(i, &vs.buns[vs.use_bun]))
			new.bundle[new.bun_count++] = i;
	}
	b = bsel[vs.use_bun];
	pp_unhi_bundle(b);
	pj_copy_structure(&new, vs.buns+vs.use_bun, sizeof(struct bundle) );
	pp_hi_bundle(b,mc_bright(b));
	draw_buttontop(b);
}

static void reverse_bytes(char *b, int count)
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

void cluster_reverse(void)
{
	reverse_bytes(bndl,bctt);
	draw_buttontop(bsel[vs.use_bun]);
}

void cluster_line(void)
/* places colors inside line into cluster.  Ignores key color usually.  */
{
Short_xy xys[2];

	hide_mp();
	wait_click();
	if (JSTHIT(MBPEN))
	{
		save_undo();
		if((get_rub_line(xys)) >= 0)
		{
			pj_diag_to_ptable(undof, bndl,
						   64, xys[0].x,xys[0].y,xys[1].x,xys[1].y);
			bctt = 64;
		}
	}
	show_mp();
}

void cunused(void)
{
char cflags[COLORS];
short unusedc, ccount;

hide_mp();
make_cused(vb.pencel, cflags, sizeof(cflags));
unusedc = ccount = COLORS - nonzero_bytes(cflags, COLORS);
if (unusedc < 1)
	{
	soft_continu_box("all_colused");
	}
else
	{
	if (soft_qreq_number(&ccount,1,ccount,"clus_unused"))
		{
		if (ccount > unusedc)
			ccount = unusedc;
		cluster_unused(cflags, ccount);
		}
	}
show_mp();
}

static void show_secondc(Pixel c2)
{
	soft_top_textf("!%3d%3d%3d", "top_secondc", c1, intabs(c1-c2)+1, c2);
}

static void scrange(Button *bt,struct bundle *b)
{
int ccount, dc;
UBYTE *pb;
Wscreen *s = bt->root->w.W_screen;

	if ((c1 = get_a_end(show_startc)) < 0)
		return;
	if ((c2 = get_a_end(show_secondc)) < 0)
		return;
	pp_unhi_bundle(bt);
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
	pp_hi_bundle(bt,s->SBRIGHT);
	if(vs.cycle_draw) /* turn off cycle draw */
		toggle_ccycle();
}

static void load_bundle(int which)
{
Button *m;

	m = bsel[which];
	hilight(m);
	scrange(m,&vs.buns[which]);
	draw_buttontop(m);
}
static void show_lastc(Pixel c2)
{
Bundle *bun;

	bun = &vs.buns[vs.use_bun];
	soft_top_textf("!%3d%3d%3d", "top_lastc",
					bun->bundle[0], bun->bun_count, c2 );
}
void qpick_bundle(void)
{
int color;
Bundle *bun;
Button *b;
Pixel occolor;

	b = bsel[vs.use_bun];
	occolor = vs.ccolor;

	if((color = get_a_end(show_startc)) < 0)
		goto done;

	pp_unhi_bundle(b);
	bun = &vs.buns[vs.use_bun];
	bun->bundle[0] = color;
	bun->bun_count = 1;
	draw_button(b);
	pp_hi_bundle(b,mc_bright(b));
	if(vs.cycle_draw) /* turn off cycle draw */
		toggle_ccycle();

	while(bun->bun_count < COLORS-1)
	{
		if((color = get_a_end(show_lastc)) < 0)
			break;
		bun->bundle[bun->bun_count] = color;
		++bun->bun_count;
		pp_hi_bundle(b,mc_bright(b));
		draw_button(b);
	}
done:
	update_ccolor(occolor);
}
void qselect_bundle(void)
{
	load_bundle(vs.use_bun);
}
void mselect_bundle(Button *m)
{
	load_bundle(m->identity);
}

void find_ramp(void)
{
Button *me;

	me = bsel[vs.use_bun];
	hilight(me);
	ramp_cluster(me,vs.use_bun);
	draw_buttontop(me);
}

static void pp_unframe_color(Button *b, int ix)
{
Boolean bottom, left, right;
int x,y,w,h,yix, xix;

	yix = ix >> 5;
	xix = ix & 31;
	x = pwp_xoff[xix]+b->x;
	y = pwp_yoff[yix]+b->y;
	w = pwp_width(xix);
	h = pwp_height(yix);

	bottom = (yix >= 7);
	right = (xix >= 31);
	left = (xix == 0);

	pj_set_hline(b->root,ix,x,y,w);
	pj_set_vline(b->root,left?sgrey:ix,x,y,h);
	pj_set_hline(b->root,bottom?sgrey:ix+32,x,y+h,w);
	pj_set_vline(b->root,right?sgrey:ix+1,x+w,y,h);
	pj_put_dot(b->root,bottom||right?sgrey:ix+33,x+w,y+h);
	if(left)
		pj_put_dot(b->root,sgrey,x,y+h);
}
static void pp_frame_color(Button *b, int ix, int ocolor)
{
int yix, xix;

	yix = ix >> 5;
	xix = ix & 31;
	draw_quad((Raster *)b->root, ocolor, pwp_xoff[xix]+b->x, pwp_yoff[yix]+b->y,
			 pwp_width(xix)+1,pwp_height(yix)+1);
}
static void pp_color_box(Button *b,int ix)
/* will color the box and draw an X on the box if it is a taken over menu
 * color */
{
int yix, xix;
Vfont *f;

	yix = ix >> 5;
	xix = ix & 31;
	pj_set_rect(b->root,ix,pwp_xoff[xix]+b->x,pwp_yoff[yix]+b->y,
			 pwp_width(xix),pwp_height(yix));

	if( ix >= FIRST_MUCOLOR 
		&& ix < (FIRST_MUCOLOR+NUM_MUCOLORS)
	    && vb.screen->mc_alt )
	{
		f = b->root->font;
		gftext(b->root,f,"X",
			   pwp_xoff[xix]+b->x + (fchar_spacing(f, "X")>>1),
			   pwp_yoff[yix]+b->y + font_ycent_oset(f,pwp_height(yix)),
			   ix == (FIRST_MUCOLOR+MC_RED)?(FIRST_MUCOLOR+MC_BLACK):
			   					(FIRST_MUCOLOR+MC_RED), TM_MASK1 );
	}
}
static void pp_inner_colors(Button *b)
{
int color;

	for(color = 0;color <= 255;++color)
		pp_color_box(b,color);
}

static void pp_unhi_bundle(Button *b)
{
int count;

	b = &pal_pal_sel;
	count = bctt;
	while (--count >= 0)
		pp_unframe_color(b,bndl[count]);
	pp_hi_ccolor(b);
}
static void pp_hi_bundle(Button *b,int ocolor)
{
int count;

	b = &pal_pal_sel;
	count = bctt;
	while (--count >= 0)
		pp_frame_color(b,bndl[count], ocolor);
	pp_hi_ccolor(b);
}

void change_cluster_mode(Button *b)
{
	pp_unhi_bundle(b);
	set_use_bun(b->identity);
	mb_draw_ghi_group(b);
	pp_hi_bundle(b,mc_bright(b));
}
static void pp_hi_ccolor(Button *b)
{
int tx, ty;
Wscreen *s = b->root->w.W_screen;

	b = &pal_pal_sel;
	tx = vs.ccolor&31;
	ty = (vs.ccolor>>5);
	tx = b->x + pwp_xoff[tx] + ((pwp_width(tx))>>1);
	ty = b->y + pwp_yoff[ty] + ((pwp_height(ty))>>1);
	pj_put_dot(b->root,s->SBRIGHT,tx,ty);
	pj_put_dot(b->root,s->SBLACK,tx,ty+1);
	pp_frame_color(b,vs.ccolor,s->SRED);
}


void see_powell_palette(Button *b)
{
int grey;

	/* draw border lines on bottom and sides */
	grey = mc_grey(b);
	pp_inner_colors(b);
	pj_set_hline(b->root, grey, b->x, b->y + b->height - 1, b->width);
	pj_set_vline(b->root, grey, b->x + b->width - 1, b->y, b->height);
	pj_set_vline(b->root, grey, b->x, b->y, b->height);
	pp_hi_bundle(b,mc_bright(b));
}

static void show_ccp1(Pixel c1)
{
	soft_top_textf("!%3d", "top_source", c1);
}

#ifdef SLUFFED
int cswap1(void)
{
	swap_mem(vb.pencel->cmap->ctab + *c1, 
				   vb.pencel->cmap->ctab + *c2, sizeof(Rgb3));
	refit_vf();
	return(0);
}
#endif /* SLUFFED */

#ifdef SLUFFED
void cswap(void)
{
if (!ccpswap())
	return;
pmhmpauto(cswap1);
}
#endif /* SLUFFED */

void ping_cluster(void)
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
draw_buttontop(bsel[vs.use_bun]);
}

static Errcode cl_swap1(void *data, int ix, int intween, int scale, Autoarg *aa)
{
static int bcount;
int i;
UBYTE *b1, *b2;
Rgb3 *ctab;
(void)data;
(void)ix;
(void)intween;
(void)scale;
(void)aa;

	/* set count to smaller of 2 bundles */
	bcount = vs.buns[0].bun_count;
	i = vs.buns[1].bun_count;
	if (i < bcount)
		bcount = i;

	b1 = vs.buns[0].bundle;
	b2 = vs.buns[1].bundle;
	ctab = vb.pencel->cmap->ctab;
	while (--bcount >= 0)
	{
		swap_mem(ctab + *b1++, ctab + *b2++, sizeof(Rgb3));
	}
	refit_vf();

	return Success;
}

void cl_swap(void)
{
	pmhmpauto(cl_swap1, NULL);
}

typedef struct copy1dat {
	Rgb3 rgb;
	Pixel where;
} Copy1dat;

static Errcode ccopy1(Copy1dat *cd)
{
	set_color_rgb(&cd->rgb,cd->where,vb.pencel->cmap);
	pj_set_colors(vb.pencel,cd->where,1,(UBYTE *)&(cd->rgb));
	return(Success);
}

static Errcode
auto_ccopy1(void *cd, int ix, int intween, int scale, Autoarg *aa)
{
	(void)ix;
	(void)intween;
	(void)scale;
	(void)aa;

	return ccopy1(cd);
}

void right_click_pp(Button *m)
{
int s;
Wscreen *ws = m->root->w.W_screen;
Copy1dat cd;

	cd.where = which_pp(m->y);	/* get destination color from matrix */
	pp_frame_color(m,cd.where,ws->SRED); /* red frame abuser feedback */
	s = get_a_end(show_ccp1);
	if (s >= 0)
	{
		get_color_rgb(s, vb.pencel->cmap,&cd.rgb);
		if(vs.multi)
			pmhmpauto(auto_ccopy1,&cd);
		else
		{
			save_undo();	
			ccopy1(&cd);
			dirties();
		}
	}
	draw_buttontop(&pal_pal_sel);
}
/********************** cluster seeme functions ********************/

static void s_colors(Button *m, UBYTE *lookup, int divx)
{
SHORT j, count;
SHORT nextx, lastx;
SHORT x, y;
SHORT width, height;
SHORT tx,ty;
Pixel col;
Pixel outcolor;
Wscreen *s;

	s = m->root->w.W_screen;

	outcolor = wbg_textcolor(m);
	if(outcolor == s->SBLACK)
		outcolor = s->SGREY;
	a_frame(m,outcolor);

	width = m->width-2;
	height = m->height-2;
	count = 0;
	x = m->x+1;
	y = m->y+1;
	lastx = m->x;

	for (j=1; j <= divx; ++j)
	{
		if(j == divx)
			nextx = x + width - 2;
		else
			nextx = x + j*width/divx;

		col = lookup[count];
		pj_set_rect(m->root, col, lastx+1, y, 
				 (nextx-lastx)+1, height);

		if (col == vs.ccolor)
		{
			tx = ((nextx+lastx)>>1)+1;
			ty = y + (height/3);
			pj_put_dot(m->root,s->SBRIGHT,tx,ty);
			pj_put_dot(m->root,s->SBLACK,tx,ty+1);
		}
		lastx = nextx;
		++count;
	}
}

static int f_colors(Button *m, UBYTE *lookup, int divx)
{
SHORT j, count;
SHORT nextx;
SHORT x;
SHORT width;


	x = m->x+1;
	width = m->width-2;
	count = 0;
	for (j=0; j<divx-1; ++j)
	{
		nextx = (j+1)*width/divx + x;
		if (icb.mx <= nextx)
			return(lookup[count]);
		++count;
	}
	return(lookup[count]);
}
static int f_cbun(Button *m)
{
struct bundle *bun;

	bun = vs.buns + m->identity;
	return(f_colors(m, bun->bundle, bun->bun_count));
}

void see_cluster(Button *m)
{
struct bundle *bun;

	bun = vs.buns + m->identity;
	s_colors(m, bun->bundle, bun->bun_count);
}

void feel_cluster(Button *m)
{
	update_ccolor(f_cbun(m));
}

void ccolor_box(Button *b)
{
	mb_isquare(b,vs.ccolor);
	a_frame(b,mc_grey(b));
}


