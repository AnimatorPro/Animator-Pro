#ifndef OPTIONS_H
#define OPTIONS_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
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

extern Option_tool *ink_list;

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

void hang_toolopts(Button *b);
void see_toolhelp(Button *b);
void see_option_name(Button *b);

#endif
