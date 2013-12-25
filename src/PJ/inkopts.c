/* Drawmode.c This file contains the data structures and routines for the
   ink types menu.  The ink types menu 'parasites' off of the draw tools menu
   in options.c.   That is I wrote options.c first and then hacked it
   about so most of the code in the draw tools and ink types menus could
   be shared.  */

#include "jimk.h"
#include "errcodes.h"
#include "jfile.h"
#include "linklist.h"
#include "menus.h"
#include "rastext.h"
#include "inks.h"
#include "options.h"
#include "inkdot.h"
#include "resource.h"
#include "softmenu.h"

/* #define BUILD_TEST_INKS */

extern Button dtintgroup_sel,tintgroup_sel, radgroup_sel, dithergroup_sel;

extern void set_rad_center();

void see_ink_strength(Button *b);
void see_dither_button(Button *b);


static Ink xor_ink_opt = INKINIT(
	NONEXT,
	RL_KEYTEXT("xor_n"),	/* Filled in from resource */
	INK_OPT,
	xor_INKID,
	RL_KEYTEXT("xor_help"),	/* Filled in from resource */
	NO_SUBOPTS,
	xor_dot,
	gink_hline,
	NOSTRENGTH,
	FALSE,
	NO_MC,
	NO_FC,
	INK_NEEDS_COLOR,
);
static Ink vsp_ink_opt = INKINIT(
	&xor_ink_opt,
	RL_KEYTEXT("vgrad_n"),  /* Filled in from resource */
	INK_OPT,
	vsp_INKID,
	RL_KEYTEXT("vgrad_help"),  /* Filled in from resource */
	&dithergroup_sel,
	vsp_dot,
	vsp_hline,
	NOSTRENGTH,
	TRUE,
	clear_random_cashe,
	NO_FC,
	0,
);
static Ink anti_ink_opt = INKINIT(
	&vsp_ink_opt,
	RL_KEYTEXT("unzag_n"),  /* Filled in from resource */
	INK_OPT,
	anti_INKID,
	RL_KEYTEXT("unzag_help"),  /* Filled in from resource */
	&dithergroup_sel,
	anti_dot,
	gink_hline,
	NOSTRENGTH,
	FALSE,
	make_ink_bhash,
	free_ink_bhash,
	INK_NEEDS_UNDO,
);

extern Errcode init_celt_ink();
extern void cleanup_celt_ink();

static Ink celt_ink_opt = INKINIT(
	&anti_ink_opt,
	RL_KEYTEXT("tile_n"),  /* Filled in from resource */
	INK_OPT,
	celt_INKID,
	RL_KEYTEXT("tile_help"),  /* Filled in from resource */
	&dithergroup_sel,
	celt_dot,
	celt_hline, 
	NOSTRENGTH,
	FALSE,
	init_celt_ink,
	cleanup_celt_ink,
	INK_NEEDS_CEL,
);
static Ink swe_ink_opt = INKINIT(
	&celt_ink_opt,
	RL_KEYTEXT("sweep_n"),  /* Filled in from resource */
	INK_OPT,
	swe_INKID,
	RL_KEYTEXT("sweep_help"),  /* Filled in from resource */
	NO_SUBOPTS,
	swe_dot,
	gink_hline,
	NOSTRENGTH,
	FALSE,
	NO_MC,
	NO_FC,
	INK_NEEDS_UNDO,
);
static Ink shat_ink_opt = INKINIT(
	&swe_ink_opt,
	RL_KEYTEXT("split_n"),  /* Filled in from resource */
	INK_OPT,
	shat_INKID,
	RL_KEYTEXT("split_help"),  /* Filled in from resource */
	&tintgroup_sel,
	shat_dot,
	gink_hline,
	1,
	FALSE,
	NO_MC,
	NO_FC,
	INK_NEEDS_UNDO,
);
extern void cry_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width);
static Ink cry_ink_opt = INKINIT(
	&shat_ink_opt,
	RL_KEYTEXT("spark_n"),  /* Filled in from resource */
	INK_OPT,
	cry_INKID,
	RL_KEYTEXT("spark_help"),  /* Filled in from resource */
	NO_SUBOPTS,
	cry_dot,
	cry_hline,
	NOSTRENGTH,
	FALSE,
	NO_MC,
	NO_FC,
	INK_NEEDS_UNDO,
);
extern void soft_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width);
static Ink soft_ink_opt = INKINIT(
	&cry_ink_opt,
	RL_KEYTEXT("soften_n"),  /* Filled in from resource */
	INK_OPT,
	soft_INKID,
	RL_KEYTEXT("soften_help"),  /* Filled in from resource */
	&dithergroup_sel,
	soft_dot,
	soft_hline,
	NOSTRENGTH,
	FALSE,
	make_ink_bhash,
	free_ink_bhash,
	INK_NEEDS_UNDO,
);
static Ink smea_ink_opt = INKINIT(
	&soft_ink_opt,
	RL_KEYTEXT("smear_n"),  /* Filled in from resource */
	INK_OPT,
	smea_INKID,
	RL_KEYTEXT("smear_help"),  /* Filled in from resource */
	NO_SUBOPTS,
	smea_dot,
	gink_hline,
	NOSTRENGTH,
	FALSE,
	NO_MC,
	NO_FC,
	INK_NEEDS_UNDO,
);
static Ink rvl_ink_opt = INKINIT(
	&smea_ink_opt,
	RL_KEYTEXT("scrape_n"),  /* Filled in from resource */
	INK_OPT,
	rvl_INKID,
	RL_KEYTEXT("scrape_help"),  /* Filled in from resource */
	&dithergroup_sel,
	rvl_dot,
	rvl_hline,
	NOSTRENGTH,
	FALSE,
	make_ink_thash,
	free_ink_thash,
	INK_NEEDS_ALT,
);
static Ink rad_ink_opt = INKINIT(
	&rvl_ink_opt,
	RL_KEYTEXT("rgrad_n"),  /* Filled in from resource */
	INK_OPT,
	rad_INKID,
	RL_KEYTEXT("rgrad_help"),  /* Filled in from resource */
	&radgroup_sel,
	rad_dot,
	gink_hline,
	NOSTRENGTH,
	TRUE,
	clear_random_cashe,
	NO_FC,
	0,
);
static Ink pull_ink_opt = INKINIT(
	&rad_ink_opt,
	RL_KEYTEXT("pull_n"),  /* Filled in from resource */
	INK_OPT,
	pull_INKID,
	RL_KEYTEXT("pull_help"),  /* Filled in from resource */
	NO_SUBOPTS,
	pull_dot,
	gink_hline,
	NOSTRENGTH,
	FALSE,
	NO_MC,
	NO_FC,
	0,
);
static Ink opq_ink_opt = INKINIT(
	&pull_ink_opt,
	RL_KEYTEXT("opaque_n"),  /* Filled in from resource */
	INK_OPT,
	opq_INKID,
	RL_KEYTEXT("opaque_help"),  /* Filled in from resource */
	NO_SUBOPTS,
	opq_dot,
	opq_hline,
	NOSTRENGTH,
	FALSE,
	NO_MC,
	NO_FC,
	INK_NEEDS_COLOR,
);
static Ink lsp_ink_opt = INKINIT(
	&opq_ink_opt,
	RL_KEYTEXT("lgrad_n"),  /* Filled in from resource */
	INK_OPT,
	lsp_INKID,
	RL_KEYTEXT("lgrad_help"),  /* Filled in from resource */
	&dithergroup_sel,
	hsp_dot,
	hsp_hline,
	NOSTRENGTH,
	TRUE,
	clear_random_cashe,
	NO_FC,
	0,
);
static Ink jmb_ink_opt = INKINIT(
	&lsp_ink_opt,
	RL_KEYTEXT("jumble_n"),  /* Filled in from resource */
	INK_OPT,
	jmb_INKID,
	RL_KEYTEXT("jumble_help"),  /* Filled in from resource */
	&tintgroup_sel,
	jmb_dot,
	gink_hline,
	3,
	FALSE,
	NO_MC,
	NO_FC,
	INK_NEEDS_UNDO,
);
static Ink out_ink_opt = INKINIT(
	&jmb_ink_opt,
	RL_KEYTEXT("hollow_n"),  /* Filled in from resource */
	INK_OPT,
	out_INKID,
	RL_KEYTEXT("hollow_help"),  /* Filled in from resource */
	NO_SUBOPTS,
	out_dot,
	gink_hline,
	NOSTRENGTH,
	FALSE,
	NO_MC,
	NO_FC,
	INK_NEEDS_UNDO,
);
static Ink hsp_ink_opt = INKINIT(
	&out_ink_opt,
	RL_KEYTEXT("hgrad_n"),  /* Filled in from resource */
	INK_OPT,
	hsp_INKID,
	RL_KEYTEXT("hgrad_help"),  /* Filled in from resource */
	&dithergroup_sel,
	hsp_dot,
	hsp_hline,
	NOSTRENGTH,
	TRUE,
	clear_random_cashe,
	NO_FC,
	0,
);
static Ink des_ink_opt = INKINIT(
	&hsp_ink_opt,
	RL_KEYTEXT("gray_n"),  /* Filled in from resource */
	INK_OPT,
	des_INKID,
	RL_KEYTEXT("gray_help"),  /* Filled in from resource */
	&dtintgroup_sel,
	des_dot,
	gink_hline,
	50,
	FALSE,
	make_ink_bhash,
	free_ink_bhash,
	INK_NEEDS_UNDO,
);
static Ink glr_ink_opt = INKINIT(
	&des_ink_opt,
	RL_KEYTEXT("glow_n"),  /* Filled in from resource */
	INK_OPT,
	glr_INKID,
	RL_KEYTEXT("glow_help"),  /* Filled in from resource */
	NO_SUBOPTS,
	glr_dot,
	gink_hline,
	NOSTRENGTH,
	FALSE,
	make_glow_cashe,
	free_glow_cashe,
	INK_NEEDS_UNDO,
);
static Ink tlc_ink_opt = INKINIT(
	&glr_ink_opt,
	RL_KEYTEXT("glaze_n"),  /* Filled in from resource */
	INK_OPT,
	tlc_INKID,
	RL_KEYTEXT("glaze_help"),  /* Filled in from resource */
	&dtintgroup_sel,
	tlc_dot,
	gink_hline,
	50,
	FALSE,
	make_tsp_cashe,
	free_ink_thash,
	INK_NEEDS_COLOR,
);
static Ink tsp_ink_opt = INKINIT(
	&tlc_ink_opt,
	RL_KEYTEXT("glass_n"),  /* Filled in from resource */
	INK_OPT,
	tsp_INKID,
	RL_KEYTEXT("glass_help"),  /* Filled in from resource */
	&dtintgroup_sel,
	tsp_dot,
	tsp_hline,
	50,
	FALSE,
	make_tsp_cashe,
	free_ink_thash,
	INK_NEEDS_UNDO|INK_NEEDS_COLOR,
);
static Ink emb_ink_opt = INKINIT(
	&tsp_ink_opt,
	RL_KEYTEXT("emboss_n"),  /* Filled in from resource */
	INK_OPT,
	emb_INKID,
	RL_KEYTEXT("emboss_help"),  /* Filled in from resource */
	&dtintgroup_sel,
	emb_dot,
	gink_hline,
	50,
	FALSE,
	make_ink_bhash,
	free_ink_bhash,
	INK_NEEDS_UNDO,
);
static Ink dar_ink_opt = INKINIT(
	&emb_ink_opt,
	RL_KEYTEXT("dark_n"),  /* Filled in from resource */
	INK_OPT,
	dar_INKID,
	RL_KEYTEXT("dark_help"),  /* Filled in from resource */
	&dtintgroup_sel,
	tsp_dot,
	tsp_hline,
	50,
	FALSE,
	make_dar_cashe,
	free_ink_thash,
	INK_NEEDS_UNDO,
);
static Ink clh_ink_opt = INKINIT(
	&dar_ink_opt,
	RL_KEYTEXT("close_n"),  /* Filled in from resource */
	INK_OPT,
	clh_INKID,
	RL_KEYTEXT("close_help"),  /* Filled in from resource */
	NO_SUBOPTS,
	clh_dot,
	gink_hline,
	NOSTRENGTH,
	FALSE,
	NO_MC,
	NO_FC,
	INK_NEEDS_UNDO,
);
static Ink bri_ink_opt = INKINIT(
	&clh_ink_opt,
	RL_KEYTEXT("bright_n"),  /* Filled in from resource */
	INK_OPT,
	bri_INKID,
	RL_KEYTEXT("bright_help"),  /* Filled in from resource */
	&dtintgroup_sel,
	bri_dot,
	gink_hline,
	50,
	FALSE,
	make_ink_bhash,
	free_ink_bhash,
	INK_NEEDS_UNDO,
);
Ink add_ink_opt = INKINIT(
	&bri_ink_opt,
	RL_KEYTEXT("add_n"),  /* Filled in from resource */
	INK_OPT,
	add_INKID,
	RL_KEYTEXT("add_help"),  /* Filled in from resource */
	NO_SUBOPTS,
	add_dot,
	gink_hline,
	NOSTRENGTH,
	FALSE,
	NO_MC,
	NO_FC,
	INK_NEEDS_UNDO|INK_NEEDS_COLOR,
);

#define first_option ((Option_tool*)&add_ink_opt)
static Ink *static_inks = &add_ink_opt;
Option_tool *ink_list = NULL;

extern Button dtintgroup_sel,tintgroup_sel, radgroup_sel, dithergroup_sel;

static Ink_groups igs = { 
	&dithergroup_sel, 
	&tintgroup_sel, 
	&dtintgroup_sel,
};

static void close_static_ink(Ink *ink)
/* simply put ink back on static inks list and set close to null */
{
	ink->ot.next = static_inks;
	static_inks = ink;
	ink->ot.closeit = NULL;
}

typedef void (*ink_closer)(struct option_tool *ot);

static void *ink_ss = NULL;

void cleanup_inks()
{
	/* transfer to loaded or static list */
	close_option_tools((Option_tool **)&ink_list);
	smu_free_scatters(&ink_ss);
}

#ifdef BUILD_TEST_INKS
static void
add_root_ink(RootInk *ri)
{
	static SHORT loaded_id = FIRST_LOADABLE_INKID;
	Ink *loaded_ink = &(ri->ink);

	if (ri->init_inks != NULL)
		if ((*ri->init_inks)(&ink_aid, &igs) < Success)
			return;

	/* put loaded inks on end of ink list */
	ink_list = (Option_tool *)join_slists((Slnode *)ink_list,
			(Slnode *)loaded_ink);

	/* set closit function and set their id's */
	loaded_ink->ot.closeit = (ink_closer)close_static_ink;

	for (;;)
	{
		loaded_ink->ot.id = loaded_id++;
		loaded_ink->aid = &ink_aid;
		if (loaded_ink->hline == NULL)
			loaded_ink->hline = gink_hline;
		if ((loaded_ink = loaded_ink->ot.next) == NULL)
			break;
		loaded_ink->ot.closeit = NULL; /* only root counts */
	}
}
#endif

Errcode init_inks()
{
Errcode err;
Ink *ink;
Names *ink_devs = NULL;
Names *inkd;
Ink *loaded_ink;

	/* move static inks into ink_list */
	while(static_inks != NULL)
	{
		ink = static_inks;
		static_inks = ink->ot.next;
		ink->ot.next = ink_list;
		ink_list = (Option_tool *)ink;
		ink->ot.closeit = (ink_closer)close_static_ink;
	}

	while (ink != NULL)
	{
		ink->aid = &ink_aid;
		if(ink->ot.help == NULL)  /* make sure we have a help text */
			ink->ot.help = RL_KEYTEXT("no_help");
		ink = ink->ot.next;
	}
	if((err = load_option_names((Option_tool *)ink_list, "ink_texts",
						         &ink_ss, FALSE)) < Success)
	{
		goto error;
	}
	ink_list = (Option_tool *)sort_names((Names *)ink_list);
	goto done;
error:
	free_wild_list(&ink_devs);
done:
	change_dir(vb.init_drawer);
	return(err);
}

get_default_ink_strengths(UBYTE *inktab)
{
Ink *l;

l = (Ink *)ink_list;
while (l != NULL)
	{
	l->strength = l->default_strength;
	l->dither = l->default_dither;
	l = l->ot.next;
	}
save_ink_strengths(inktab);
}

load_ink_strengths(UBYTE *inktab)
{
Ink *l;
UBYTE code;

l = (Ink *)ink_list;
while (l != NULL)
	{
	code = inktab[l->ot.id];
	l->dither = FALSE;
	if (code&0x80)
		l->dither = TRUE;
	l->strength = (code & 0x7f);
	l = l->ot.next;
	}
}

save_ink_strengths(UBYTE *inktab)
{
Ink *l;
UBYTE code;

l = (Ink *)ink_list;
while (l != NULL)
	{
	code = (l->strength&0x7f);
	if (l->dither)
		code |= 0x80;
	inktab[l->ot.id] = code;
	l = l->ot.next;
	}
}

static Qslider tint_sl = 
	QSL_INIT1(0,100,NULL, 0, NULL, leftright_arrs);

static Button tinting_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	121,11,8,38,
	&tint_sl,
	see_ink_strength,
	feel_qslider,
	NOOPT,
	NOGROUP, /* "Ink Strength", */ 0,
	NOKEY,
	0
	);
Button tintgroup_sel = MB_INIT1(
	NONEXT,
	&tinting_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
/* -------------------------- */	/* for dithered ink strengths */
static Button dtinting_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	121,11,8,64,
	&tint_sl,
	see_ink_strength,
	feel_qslider,
	feel_qslider,
	NOGROUP, /* "Ink Strength", */ 0,
	NOKEY,
	0
	);
static Button dtint_sel = MB_INIT1(
	&dtinting_sel,
	NOCHILD,
	74,14,33,25,
	NODATA, /* "Dither", */
	see_dither_button,
	toggle_bgroup,
	NOOPT,
	NOGROUP,1,
	NOKEY,
	MB_GHILITE,	
	);
Button dtintgroup_sel = MB_INIT1(
	NONEXT,
	&dtint_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
/* ------------ */
static Button dither_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	74,14,33,25,
	NODATA, /* "Dither", */
	see_dither_button,
	toggle_bgroup,
	NOOPT,
	NOGROUP,1,
	NOKEY,
	MB_GHILITE,
	);
Button dithergroup_sel = MB_INIT1(
	NONEXT,
	&dither_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
/* -------- */
static Button rdither_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	74,14,33,20,
	NODATA, /* "Dither", */
	see_dither_button,
	toggle_bgroup,
	NOOPT,
	NOGROUP,1,
	NOKEY,
	MB_GHILITE,	
	);
static Button setrad_sel = MB_INIT1(
	&rdither_sel,
	NOCHILD,
	74,13,33,40,
	NODATA, /* "Center", */
	ccorner_text,
	set_rad_center,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button radgroup_sel = MB_INIT1(
	NONEXT,
	&setrad_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

static void set_rad_center(void)
{
	hide_mp();
	save_undo();
	if(marqi_cut_xy() >= 0)
	{
 		if(get_rub_circle(&vl.rgc,&vl.rgr,vs.ccolor) >= 0)
		{
			vl.rgr >>= 1;
			vs.rgcx = scale_vscoor(vl.rgc.x,vb.pencel->width);
			vs.rgcy = scale_vscoor(vl.rgc.y,vb.pencel->height);
			vs.rgr = scale_vscoor(vl.rgr,vl.flidiag_scale);
		}
	}
	show_mp();
}

void see_cur_ink(Button *m)
{
	if(vl.ink->ot.id == opq_INKID)
		m->identity = -1;
	else
		m->identity = vl.ink->ot.id;
	m->datme = vl.ink->ot.name;
	dcorner_text(m);
}

void attatch_inks(void)
{
extern Button ink_opts_sel;
int i;
int id;
Button *ob;
Option_tool *o;

static Ink *default_inks[] = {
	&opq_ink_opt,
	&tsp_ink_opt,
	&soft_ink_opt,
	&vsp_ink_opt,
	&rvl_ink_opt,
	&celt_ink_opt,
	&rvl_ink_opt,
	&celt_ink_opt,
};

	ob = &ink_opts_sel;
	for (i=0; i<8; i++)
	{
		id = vs.ink_slots[i];
		if(NULL == (o = id_find_option(ink_list, id)))
			o = (Option_tool *)(default_inks[i]);
		ob->identity = o->id;
		ob->datme = o;
		ob = ob->next;
	}
	if(NULL == (vl.ink = id_find_option(ink_list, vs.ink_id)))
		set_curink(default_inks[0]);
}

static void see_dither_button(Button *b)
{
b->group = &(vl.ink->dither);
dcorner_text(b);
}

void see_ink_strength(Button *b)
{
tint_sl.value = &(vl.ink->strength);
see_abovetext_qslider(b);
}

static Smu_button_list iopt_smblist[] = {
	{ "dither", &dither_sel },
	{ "rgcent", &setrad_sel },
// texts preceded by 'T'
	{ "Tstrength", &tinting_sel.group },
};

void *ss;
Errcode load_inkopt_texts()
/* called by options menu before using buttons */
{
Errcode err;

	err = soft_buttons("ink_panels",iopt_smblist,Array_els(iopt_smblist),&ss);
	dtint_sel.datme = rdither_sel.datme = dither_sel.datme;
	dtinting_sel.group = tinting_sel.group;
	return(err);
}
void free_inkopt_texts()
{
	smu_free_scatters(&ss);
}
