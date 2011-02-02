/* palet2.c - The main bits of code for the palette editor.  Also a lot
   of related code in cluster.c and palpull.c.  This file starts with
   the Flicmenu button declarations for the palette editor control panel. */

#include "jimk.h"
#include "flicmenu.h"
#include "palet2.str"

extern int see_colgrid(), see_menu_back(),tri_right(),tri_left(),
	wbtexty1(), text_boxp1(),see_qslider(),feel_qslider(),grey_block(),
	white_block(), ccolor_box(), gary_menu_back(), see_colors2(),
	see_powell_palette(),  see_cluster(), pal_feel_qslider(), feel_pp(),
	right_click_pp(), sliders_from_ccolor(), pal_menu_back(),
	ccorner_text(), text_boxp1(), hang_child(), move_tab_text();

extern int move_menu(), bottom_menu(),  change_mode(),
	change_hls_mode(), pget_color(), crestore(),
	go_multi(), mselect_bundle(), change_cluster_mode(),
	see_number_slider(), inc_slider(), dec_slider(), mrestore(),
	inc_rgb_slider(), dec_rgb_slider(), cundo_pic(),
	feel_cluster(),
	see_ink0(), see_ink(), fill_inkwell(), toggle_group(), set_crange(),
	crange(), cdefault(), cload(), csave(), ccut(), cpaste(),ccopy(),
	cpack(), feel_rgb_slider();

extern Pull pal_pull, rem_use_pull;

extern palette_selit();

int ccolor_from_sliders();

extern Flicmenu palette_menu;
extern Flicmenu ink_group_sel, ink0_sel, inks_sel, ccolor_sel, spec1_sel;

static WORD ccred,ccgreen,ccblue;
#ifdef SLUFFED
static UBYTE *cbuf;
#endif SLUFFED
static UBYTE *new_cmap;

static struct qslider red_sl = {0,63,&ccred,0,ccolor_from_sliders};
static struct qslider green_sl = {0,63,&ccgreen,0,ccolor_from_sliders};
static struct qslider blue_sl = {0,63,&ccblue,0,ccolor_from_sliders};


/*** Button Data ***/
Flicmenu pal_pal_sel = {
	NONEXT,
	NOCHILD,
	0, 143, 319, 57,
	NOTEXT,
	see_powell_palette,
	feel_pp,
	NOGROUP,0,
	NOKEY,
	right_click_pp,
	};
static Flicmenu rpal_pal_sel = {
	NONEXT,
	&pal_pal_sel,
	0, 142, 319, 57,
	NOTEXT,
	grey_block,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu pal_bsl_sel = {
	NONEXT,
	NOCHILD,
	229, 129, 86, 10,
	&blue_sl,
	see_qslider,
	pal_feel_qslider,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu pal_gsl_sel = {
	&pal_bsl_sel,
	NOCHILD,
	141, 129, 85, 10,
	&green_sl,
	see_qslider,
	pal_feel_qslider,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu pal_rsl_sel = {
	&pal_gsl_sel,
	NOCHILD,
	53, 129, 85, 10,
	&red_sl,
	see_qslider,
	pal_feel_qslider,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu pal_hls_sel = {
	&pal_rsl_sel,
	NOCHILD,
	28, 129, 22, 10,
	palet2_100 /* "hls" */,
	ccorner_text,
	change_hls_mode,
	&vs.hls,1,
	NOKEY,
	NOOPT,
	};
static Flicmenu pal_rgb_sel = {
	&pal_hls_sel,
	NOCHILD,
	3, 129, 22, 10,
	palet2_101 /* "rgb" */,
	ccorner_text,
	change_hls_mode,
	&vs.hls,0,
	NOKEY,
	NOOPT,
	};
Flicmenu pal_cco_sel = {
	&pal_rgb_sel,
	NOCHILD,
	292, 106, 23, 20,
	NOTEXT,
	ccolor_box,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
Flicmenu pal_spe_sel = {
	&pal_cco_sel,
	NOCHILD,
	204, 117, 84, 9,
	NOTEXT,
	see_cluster,
	feel_cluster,
	NOGROUP, 1,
	NOKEY,
	mselect_bundle,
	};
static Flicmenu pal_tsp_sel = {
	&pal_spe_sel,
	NOCHILD,
	187, 117, 14, 8,
	palet2_102 /* "B" */,
	ccorner_text,
	change_cluster_mode,
	&vs.use_bun,1,
	NOKEY,
	NOOPT,
	};
Flicmenu pal_bun_sel = {
	&pal_tsp_sel,
	NOCHILD,
	98, 117, 84, 9,
	NOTEXT,
	see_cluster,
	feel_cluster,
	NOGROUP, 0,
	NOKEY,
	mselect_bundle,
	};
static Flicmenu pal_tbu_sel = {
	&pal_bun_sel,
	NOCHILD,
	81, 117, 14, 8,
	palet2_103 /* "A" */,
	ccorner_text,
	change_cluster_mode,
	&vs.use_bun, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu pal_clu_sel = {
	&pal_tbu_sel,
	NOCHILD,
	30, 117, 46, 8,
	palet2_104 /* "CLUSTER" */,
	ccorner_text,
	change_mode,
	&vs.pal_to, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu pal_all_sel = {
	&pal_clu_sel,
	NOCHILD,
	3, 117, 24, 8,
	palet2_105 /* "ALL" */,
	ccorner_text,
	change_mode,
	&vs.pal_to, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu pal_ink_sel = {
	&pal_all_sel,
	&ink_group_sel,
	204, 106, 85, 9,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu pal_m_sel = {
	&pal_ink_sel,
	NOCHILD,
	190, 106, 10, 8,
	palet2_106 /* "T" */,
	ccorner_text,
	toggle_group,
	&vs.multi,1,
	NOKEY,
	go_multi,
	};
static Flicmenu pal_min_sel = {
	&pal_m_sel,
	&minitime_sel,
	112, 106, 81, 8,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu pal_fit_sel = {
	&pal_min_sel,
	NOCHILD,
	83, 106, 22, 8,
	palet2_107 /* "FIT" */,
	ccorner_text,
	toggle_group,
	&vs.pal_fit,1,
	NOKEY,
	NOOPT,
	};
static Flicmenu pal_res_sel = {
	&pal_fit_sel,
	NOCHILD,
	52, 106, 28, 8,
	palet2_108 /* "UNDO" */,
	ccorner_text,
	cundo_pic,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu pal_tit_sel = {
	&pal_res_sel,
	NOCHILD,
	2, 106, 46, 8,
	palet2_109 /* "palette" */,
	move_tab_text,
	move_menu,
	NOGROUP, 0,
	NOKEY,
	bottom_menu,
	};
static Flicmenu pal_most_sel = 
	{
	&rpal_pal_sel,
	&pal_tit_sel,
	0, 103, 319, 39,
	NOTEXT,
	pal_menu_back,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
Flicmenu palette_menu = {
	NOCHILD,
	&pal_most_sel,
	0, 103, 319, 96,
	NOTEXT,
	sliders_from_ccolor,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};

static
pal_feel_qslider(m)
Flicmenu *m;
{
save_undo();
feel_qslider(m);
dirties();
if (vs.ccolor == sred || vs.ccolor == sbright || vs.ccolor == swhite ||
	vs.ccolor == sgrey || vs.ccolor == sblack)
	{
	hide_mp();
	draw_mp();
	}
}

static
cundo_pic()
{
hide_mp();
undo_pic();
draw_mp();
}

static
save_colors(name)
char *name;
{
write_gulp(name, render_form->cmap, COLORS*3L);
}


static
load_colors(name, buf)
char *name;
UBYTE *buf;
{
int f;
int i;

if ((f = jopen(name, 0))==0)
	{
	cant_find(name);
	return(0);
	}
if (jread(f, buf, COLORS*3L) < COLORS*3L)
	{
	truncated(name);
	jclose(f);
	return(0);
	}
i = COLORS*3;
while (--i >= 0)
	{
	if (*buf++ > 63)
		{
		continu_line(palet2_110 /* "Not a palette file" */);
		jclose(f);
		return(0);
		}
	}
jclose(f);
return(1);
}

static
cmapcopy1()
{
copy_cmap(new_cmap, render_form->cmap);
return(1);
}

static
refit1()
{
if (vs.pal_fit)
	refit_screen(render_form, new_cmap, vf.cmap, COLORS);
cmapcopy1();
return(1);
}

qload_palette()
{
char *title;
Vector how;

title =  get_filename(palet2_111 /* "Load a Palette?" */, ".COL");
if (title != NULL)
	{
	if ((new_cmap = begmem(COLORS*3)) != NULL)
		{
		if (load_colors(title, new_cmap))
			{
			exchange_words(render_form->cmap, new_cmap, COLORS/2*3);
			see_cmap();
			if (yes_no_line(palet2_113 /* "Color fit screen?" */) )
				how = refit1;
			else
				how = cmapcopy1;
			/* for undo to work change colors again...*/
			exchange_words(render_form->cmap, new_cmap, COLORS/2*3);
			see_cmap();
			uzauto(how);
			}
		freemem(new_cmap);
		}
	}
}


refit_screen(c, ncmap, ocmap,cpnum)
Vscreen *c;
PLANEPTR ncmap, ocmap;
int cpnum;
{
int i;
unsigned char cvmap[COLORS];

nz_fitting_cmap(ocmap, ncmap, cvmap);
xlat(cvmap, c->p, (unsigned)c->bpr*(unsigned)c->h);
}


in_cmap(rgb, cmap, count)
register PLANEPTR rgb, cmap;
int count;
{
while (--count >= 0)
	{
	if (rgb[0] == cmap[0] && rgb[1] == cmap[1] && rgb[2] == cmap[2])
		return(1);
	cmap += 3;
	}
return(0);
}




qsave_palette()
{
char *title;

unzoom();
if ((title =  get_filename(palet2_114/* "Save a Palette?" */, ".COL")) != NULL)
	if (overwrite_old(title) )
		save_colors(title);
rezoom();
}

refit_1c(scale, p, ccolor, cix, ccount)
WORD scale;
UBYTE *p;
WORD ccolor, cix, ccount;
{
copy_bytes(new_cmap + 3*ccolor, p, 3);
}



refit_vf()
{
if (vs.pal_fit)
	refit_screen(render_form, render_form->cmap, uf.cmap, COLORS);
}

static
cl_refit1()
{
some_cmod(refit_1c, SCALE_ONE);
return(1);
}

cdefault()
{
new_cmap = init_cmap;
pmhmpauto(cl_refit1);
}

cuse_cel()
{
if (cel != NULL)
	{
	if ((new_cmap = begmem(COLORS*3)) != NULL)
		{
		copy_cmap(cel->cmap, new_cmap);
		pmhmpauto(cl_refit1);
		freemem(new_cmap);
		}
	}
}



static
sliders_from_ccolor()
{
char *rgb;
WORD r, g, b;

rgb = render_form->cmap + 3*vs.ccolor;
r = *rgb++;
g = *rgb++;
b = *rgb++;
if (vs.hls)
	{
	rgb_to_hls(r, g, b, &ccred, &ccgreen, &ccblue);
	red_sl.max = green_sl.max = blue_sl.max = 255;
	}
else
	{
	ccred = r;
	ccgreen = g;
	ccblue = b;
	red_sl.max = green_sl.max = blue_sl.max = 63;
	}
}


static Flicmenu *slides[3] = {&pal_rsl_sel, &pal_gsl_sel, &pal_bsl_sel};

static
see_color_sliders()
{
Flicmenu **s, *f;
int i;

sliders_from_ccolor();
s = slides;
i = 3;
while (--i >= 0)
	{
	f = *s++;
	white_block(f);
	draw_sel(f);
	}
}


redraw_ccolor()
{
draw_sel(&pal_bun_sel);
draw_sel(&pal_cco_sel);
qdraw_a_menu(&ink_group_sel);
draw_sel(&pal_spe_sel);
draw_sel(&pal_pal_sel);
see_color_sliders();
}




rampit(r1, r2, dr, ccount, dc)
UBYTE *r1, *r2, *dr;
int ccount, dc;
{
int i,j;

dc *= 3;
for (i=0; i<ccount; i++)
	{
	for (j=0; j<3; j++)
		dr[j] = interp_range(r1[j], r2[j],  i, ccount);
	dr += dc;
	}
}


a_break_key()
{
switch (key_in&0xff)
	{
	case 'x':
	case 'X':
	case 'q':
	case 'Q':
	case ' ':
		return(1);
	}
return(0);
}

extern char break_menu;

get_menu_colors()
{
UBYTE omc[5*3];
UBYTE *nmc;
int vis;

/* save old interface colors */
copy_bytes(vs.mcideals, omc, sizeof(omc) );
/* use the last 5 slots in user color map for ideals and force menu redisplay */
nmc = render_form->cmap + (COLORS-5)*3;
copy_bytes(nmc, vs.mcideals, sizeof(omc) );
hide_mp();
draw_mp();
vis =  visible_cmap();

/* go back to current ideals and query user... */
copy_bytes(omc, vs.mcideals, sizeof(omc) );
if (vis)
	{
	if (yes_no_line(palet2_116 /* "Use this color set for menus?" */) )
		copy_bytes(nmc, vs.mcideals, sizeof(omc) );
	}
else
	continu_line(palet2_117 /* "Sorry, menus wouldn't be visible...." */);
hide_mp();
draw_mp();
}

#ifdef SLUFFED
change_menc(ix)
int ix;
{
UBYTE *d;
UBYTE c1;
char buf[40];


for (;;)
	{
	wait_input();
	c1 = getdot(uzx, uzy);
	d = vf.cmap + 3*c1;
	sprintf(buf, " r %d  g %d  b %d", d[0], d[1], d[2]);
	top_text(buf);
	if (RJSTDN || PJSTDN || key_hit)
		{
		break;
		}
	c1 = getdot(uzx, uzy);
	}
if (PJSTDN)
	{
	d = vs.mcideals[MC_BLACK];
	copy_bytes(sys_cmap+3*getdot(uzx, uzy), d+ix*3, 3);
	find_colors();
	draw_a_menu(cur_menu);
	}
draw_pull(cur_pull->xoff, cur_pull->yoff, cur_pull);
}
#endif SLUFFED


crestore()
{
hide_mp();
restore();
sliders_from_ccolor();
draw_mp();
}

static
ccolor_from_sliders()
{
char *rgb;
WORD r, g, b;

rgb = render_form->cmap + 3*vs.ccolor;
if (vs.hls)
	{
	hls_to_rgb(&r, &g, &b, ccred, ccgreen, ccblue);
	}
else
	{
	r = ccred;
	g = ccgreen;
	b = ccblue;
	}
rgb[0] = r;
rgb[1] = g;
rgb[2] = b;
wait_sync();
jset_colors(vs.ccolor,1,rgb);
}


static
change_hls_mode(m)
Flicmenu *m;
{
change_mode(m);
sliders_from_ccolor();
see_color_sliders();
}

static
pal_menu_back(m)
Flicmenu *m;
{
pal_disables();
gary_menu_back(m);
}


ppalette()
{
hide_mp();
palette();
draw_mp();
}

static
get_color()
{
vs.ccolor = getdot(uzx, uzy);
redraw_ccolor();
}


palette()
{
Flicmenu *omenu;
Pull *orpc;
Pull *ocp;
static in_palette = 0;

if (!in_palette)	/* make sure don't recurse back into self */
	{
	in_palette = 1;
	unzoom();
	clip_rmove_menu(&palette_menu, 
		cur_menu->x - palette_menu.x, cur_menu->y-palette_menu.y); 
	omenu = cur_menu;	/* save old 'panel' (quick) menu */
	cur_menu = &palette_menu;	/* and currently no panel menu */
	orpc = root_pull.children;
	ocp = cur_pull;
	cur_pull = &root_pull;
	root_pull.children = &pal_pull;	/* go to Palette pulldowns */
	pal_checks();
	draw_mp();
	sub_menu_loop(palette_selit,get_color);
	hide_mp();
	cur_menu = omenu;
	cur_pull = ocp;
	root_pull.children = orpc;	/* and back to regular ones */
	rezoom();
	in_palette = 0;
	}
else
	{
	go_in_circles_message(palet2_118 /* "palette" */);
	}
}

