#ifndef INKS_H
#define INKS_H

#ifndef OPTIONS_H
#include "options.h"
#endif

struct aa_ink_data;
struct button;
struct ink;

#define INK_NEEDS_UNDO	0x0001
#define INK_NEEDS_ALT	0x0002
#define INK_NEEDS_CEL	0x0004
/* uses a source color other than undo (ccolor or raster) as input */
#define INK_NEEDS_COLOR 0x0008
#define INK_CASHE_MADE	0x8000

#define NOSTRENGTH 0
#define NO_MC NULL
#define NO_FC NULL

enum {
	opq_INKID,
	vsp_INKID,  /* VGRAD */
	hsp_INKID,  /* HGRAD */
	lsp_INKID,  /* LGRAD */
	tlc_INKID,  /* GLAZE */
	tsp_INKID,  /* GLASS */
	rvl_INKID,  /* SCRAPE */
	xor_INKID,  /* XOR */
	jmb_INKID,  /* JUMBLE */
	add_INKID,  /* ADD */
	glr_INKID,  /* GLOW */
	celt_INKID, /* TILE */
	cry_INKID,  /* SPARK */
	shat_INKID, /* SPLIT */
	anti_INKID, /* UNZAG */
	out_INKID,  /* HOLLOW */
	bri_INKID,  /* BRIGHT */
	des_INKID,  /* GREY */
	swe_INKID,  /* SWEEP */
	clh_INKID,  /* CLOSE */
	dar_INKID,  /* DARK */
	emb_INKID,  /* EMBOSS */
	pull_INKID, /* PULL */
	smea_INKID, /* SMEAR */
	rad_INKID,  /* RGRAD */
	soft_INKID, /* soften */
	FIRST_LOADABLE_INKID,
};

typedef struct ink {
	Option_tool ot;
	Pixel (*dot)(const struct ink *i, SHORT x, SHORT y);
	void (*hline)(const struct ink *i, SHORT x, SHORT y, SHORT width);
	SHORT default_strength;
	SHORT strength;
	SHORT default_dither;
	SHORT dither;
	void *inkdata;
	Errcode (*make_cashe)(struct ink *i);
	void (*free_cashe)(struct ink *i);
	struct aa_ink_data *aid;
	USHORT needs;
} Ink;

/* macro to help with static initialization */
#define INKINIT(nx,na,t,id,hlp,opt,dot,hline,dstren,ddither,mcashe,fcashe,nd) \
	{{nx,na,t,id,hlp,opt,NOCLOSE}, \
	dot, hline,dstren,0,ddither,0,NULL,mcashe,fcashe,NULL,nd}

extern struct button ink_opts_sel;

/* inkopts.c */
extern void cleanup_inks(void);
extern void see_cur_ink(struct button *m);
extern void attatch_inks(void);
extern Errcode init_inks(void);
extern void get_default_ink_strengths(UBYTE *inktab);
extern void load_ink_strengths(UBYTE *inktab);
extern void save_ink_strengths(UBYTE *inktab);
extern Errcode load_inkopt_texts(void);
extern void free_inkopt_texts(void);

/* options.c */
extern void set_curink(Ink *newink);
extern Errcode id_set_curink(int id);

/* tileink.c */
extern Pixel celt_color(SHORT x, SHORT y);

#endif
