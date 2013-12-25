#ifndef OPTIONS_H
#define OPTIONS_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif

#ifndef INKAID_H
	#include "inkaid.h"
#endif

#ifndef REXLIB_H
	#include "rexlib.h"
#endif

typedef struct option_tool {
	void *next;
	char *name;  /* this may be a RL_KEYTEXT if it is a softmenu key */
	SHORT type;
	SHORT id;
	char *help;  /* this may be a RL_KEYTEXT if it is a softmenu key */
	Button *options;
	void (*closeit)(struct option_tool *ot);
} Option_tool;

#undef OPTHDR_FIELDS

void close_option_tools(Option_tool **ppfirst);
void *id_find_option(Option_tool *list, SHORT id);

#define NOCLOSE NULL
#define NOINSTALL NULL
#define NOEXIT NULL
#define NO_SUBOPTS NULL


typedef struct pentool {
	Option_tool ot;
	Errcode (*doptool)(struct pentool *pt, struct wndo *w);
	Cursorhdr *cursor; 	/* cursor for tool */
	Errcode (*on_install)(struct pentool *pt);
	void (*on_remove)(struct pentool *pt);
} Pentool;

extern Pentool null_pentool;

#define PTOOLINIT0(nx,na,t,id,hlp,opt,cur,tl,cl) \
 {{nx,na,t,id,hlp,opt,cl},tl,cur,NULL,NULL}

#define PTOOLINIT1(nx,na,t,id,hlp,opt,cl,tl,cur,oi,or) \
 {{nx,na,t,id,hlp,opt,cl},tl,cur,oi,or}


typedef struct ink {
	Option_tool ot;
	Pixel (*dot)(const struct ink *i,const SHORT x, const SHORT y);
	void (*hline)(const struct ink *i,SHORT x, const SHORT y, SHORT width);
	SHORT default_strength;
	SHORT strength;
	SHORT default_dither;
	SHORT dither;
	void *inkdata;
	Errcode (*make_cashe)(struct ink *i);
	void (*free_cashe)(struct ink *i);
	Aa_ink_data *aid;
	USHORT needs;
} Ink;

#define INK_NEEDS_UNDO	0x0001
#define INK_NEEDS_ALT	0x0002
#define INK_NEEDS_CEL	0x0004
#define INK_NEEDS_COLOR 0x0008 /* uses a source color other than undo 
								* (ccolor or raster) as input */


#define INK_CASHE_MADE	0x8000

/* macro to help with static initialization */

#define INKINIT(nx,na,t,id,hlp,opt,dot,hline,dstren,ddither,mcashe,fcashe,nd) \
 {{nx,na,t,id,hlp,opt,NOCLOSE}, \
 dot, hline,dstren,0,ddither,0,NULL,mcashe,fcashe,NULL,nd}

#define NOSTRENGTH 0
#define NO_MC NULL
#define NO_FC NULL

/* option types */

#define PTOOL_OPT 	1  /* pen tool option */
#define INK_OPT	 	2  /* ink option */

/* option ids for types */

extern Pentool null_pentool;

enum pentool_ids {
	NULL_PTOOL = -1,  /* we don't want any other tool to have this id */
	TEXT_PTOOL = 0,
	STREAK_PTOOL,
	STARF_PTOOL,
	SPRAY_PTOOL,
	CURVE_PTOOL,
	SPIRAL_PTOOL,
	SHAPEF_PTOOL,
	SEP_PTOOL,
	RPOLYF_PTOOL,
	POLYF_PTOOL,
	PETLF_PTOOL,
	OVALF_PTOOL,
	MOVE_PTOOL,
	LINE_PTOOL,
	GEL_PTOOL,
	FLOOD_PTOOL,
	FILL_PTOOL,
	EDGE_PTOOL,
	DRIZ_PTOOL,
	DRAW_PTOOL,
	CIRCLE_PTOOL,
	BOX_PTOOL,
	TWEEN_PTOOL,		/* unique to tween subsystem */
	COPY_PTOOL,
	TEST_PTOOL = 100,
};

/* button functions that use a Optgroup_data in the b->group */

typedef struct optgroup_data {
	SHORT *optid;   /* pointer to SHORT containing current option id # */
	Option_tool *tlist; 	/* list options available in */
	Button *optb;        /* button options sub system initiated from */
	SHORT topname;
} Optgroup_data;


/* first ink in chain of rex library inks has rexlib header */
typedef struct rootink {
	Ink ink; 	 /* first ink in singly linked chain (at least one) */
	Errcode (*init_inks)(Aa_ink_data *aid, Ink_groups *g);
} RootInk;


void hang_toolopts(Button *b);
void see_toolhelp(Button *b);
void see_option_name(Button *b);


#endif /* OPTIONS_H */
