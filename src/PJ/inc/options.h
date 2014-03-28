#ifndef OPTIONS_H
#define OPTIONS_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef REXLIB_H
	#include "rexlib.h"
#endif

struct button;
struct menuhdr;
struct pentool;

typedef struct option_tool {
	void *next;
	char *name;  /* this may be a RL_KEYTEXT if it is a softmenu key */
	SHORT type;
	SHORT id;
	char *help;  /* this may be a RL_KEYTEXT if it is a softmenu key */
	Button *options;
	void (*closeit)(struct option_tool *ot);
} Option_tool;

extern Option_tool *ink_list;
extern Option_tool *ptool_list;

#define NOCLOSE NULL
#define NOINSTALL NULL
#define NOEXIT NULL
#define NO_SUBOPTS NULL

/* option types */

#define PTOOL_OPT 	1  /* pen tool option */
#define INK_OPT	 	2  /* ink option */

/* button functions that use a Optgroup_data in the b->group */

typedef struct optgroup_data {
	SHORT *optid;   /* pointer to SHORT containing current option id # */
	Option_tool *tlist; 	/* list options available in */
	Button *optb;        /* button options sub system initiated from */
	SHORT topname;
} Optgroup_data;

extern struct menuhdr options_menu;

extern void see_abovetext_qslider(struct button *b);
extern void close_option_tools(Option_tool **ppfirst);
extern Errcode set_curptool(struct pentool *ptool);
extern void *id_find_option(Option_tool *list, SHORT id);
extern void set_quickptool(struct pentool *ptool);
extern Errcode restore_pentool(struct pentool *pt);
extern void zero_sl(struct button *b);
extern void change_ink_mode(struct button *m);
extern void change_pen_mode(struct button *m);
extern void see_option_name(struct button *b);
extern void hang_toolopts(struct button *b);
extern void qload_titles(void);
extern void qsave_titles(void);
extern void go_inkopts(struct button *b);
extern void go_dtoolopts(struct button *b);
extern void qtools(void);
extern void qinks(void);

extern Errcode
load_option_names(Option_tool *list, char *symname, void **ptext, int fill_all);

#endif
