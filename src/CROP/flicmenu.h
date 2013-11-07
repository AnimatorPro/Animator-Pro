
struct rectangle
	{
	WORD MinX, MinY;
	WORD MaxX, MaxY;
	};

struct slidepot
	{
	WORD min, max, value;
	};

struct qslider
	{
	WORD min, max, *value;
	WORD voff;
	Vector update;
	};

struct range
	{
	WORD min, max, v1, v2;
	};

extern struct range trange;

struct grid
	{
	WORD divx, divy;
	};

struct flicmenu
	{
	struct flicmenu *next;
	struct flicmenu *children;
	WORD x, y;
	UWORD width, height;
	void *text;
	int (*seeme)();
	int (*feelme)();
	int *group;
	int identity;
	WORD key_equiv;
	int (*optme)();
	UBYTE disabled, flags;
	};
typedef struct flicmenu Flicmenu;
extern Flicmenu *parent_menu();
extern Flicmenu quick_menu, edit_menu, title_menu;

#define NOTEXT NULL
#define NOSEE  0L
#define NOFEEL 0L
#define NOGROUP NULL
#define NOKEY	0
#define NOOPT 0L

struct option_list
	{
	struct option_list *next;
	char *name;
	char *help;
	Flicmenu *options;
	};
typedef struct option_list Option_list;
#define NOOOM NULL


extern Flicmenu *cur_menu;
extern char break_menu;

/* This is the tree structure that is the core of the pull-down system */
struct pull
	{
	struct pull *next;
	WORD xoff, yoff, width, height;
	struct pull *children;
	char *data;  /* actually just some old pointer */
	Vector see;
	char disabled;
	};
typedef struct pull Pull;
extern struct pull *cur_pull;

extern Pull root_pull;

/* some defines to make it easier to edit skeletal pull data files... */
#define NONEXT NULL
#define NOCHILD NULL
#define NODATA NULL

/*some functions to put into pull->see */
extern pull_block(), pull_color(), pull_oblock(), 
	spull_text(), pull_text(), pull_brush();

extern int ccolor;
extern char qstring[];

struct cursor 
    {
    WORD type;
    UBYTE *image;
    WORD width,height;
    WORD xhot, yhot;
    };

struct stringq
	{
	WORD pxoff, pyoff;	/* pixel starting position */
	WORD dcount;	/* # of characters displayed */
	WORD bcount;	/* # of characters that can fit in string */
	WORD ccount;	/* # of characters currently in string */
	char *string;
	char *undo;
	WORD cpos;		/* cursor position on screen */
	WORD dpos;		/* pol of leftmost visible character */
	};
typedef struct stringq Stringq;
#define STRINGQSZ 128

struct name_scroller
	{
	WORD yoff;
	WORD ycount;
	WORD name_count;
	WORD knob_height;
	WORD top_name;
	struct name_list *name_list;
	Flicmenu *scroll_sel, *list_sel;
	};
typedef struct name_scroller Name_scroller;
extern char *sel_name();
extern void *which_sel();

extern WORD *sunder_menu();
extern WORD *mbehind;

extern Flicmenu minitime_sel, ink_group_sel;
