/* Filemenu.c - code for the famous PJ file requestor with scrolling list */

#include <ctype.h>
#include "commonst.h"
#include "errcodes.h"
#include "filepath.h"
#include "memory.h"
#include "menus.h"
#include "ptrmacro.h"
#include "scroller.h"
#include "softmenu.h"
#include "util.h"
#include "wildlist.h"

static void fq_set_wild(Button *b);
void accept_file_name(Button *b);
void fq_ok_plus(), fq_wild_stringq(), fq_drawer_stringq(), undo_drawer(),
	undo_file();

extern Image ctriup, ctridown;

static Button fcancel_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	49,15,265,78,
	NODATA, /* "Cancel", */
	dcorner_text,
	mb_gclose_cancel,
	NOOPT,
	NOGROUP,0,
	' ',
	0
	);
static Button fok_sel = MB_INIT1(
	&fcancel_sel,
	NOCHILD,
	49,15,214,78,
	NULL,
	dcorner_text,
	mb_gclose_ok,
	NOOPT,
	NOGROUP,0,
	'\r',
	0
	);
static Button fplus_sel = MB_INIT1(
	&fok_sel,
	NOCHILD,
	18+1,15,188,78,
	NODATA, /* "+", */
	ccorner_text,
	fq_ok_plus,
	NOOPT,
	NOGROUP,0,
	'+',
	0
	);

/************** device buttons hanger ***********/


static Button fmu_dev_hanger = MB_INIT1(
	&fplus_sel,
	NOCHILD,
	17,13,106,28,
	NODATA,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
);
static Button fmu_topdev_hanger = MB_INIT1(
	&fplus_sel,
	NOCHILD,
	17,13,106,12,
	NODATA,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
);

static Rectangle fmu_devsel_320size = {17,13,20,16};

/********** end device buttons *********/

static Stringq wild_stringq = {
	2, 2, 11, WILD_SIZE-1, 0, NULL, NULL, 0, 0,
	};

static void see_opt_wild(Button *b)
{
	set_button_disable(b, b->datme == NULL);
	if(b->datme)
	{
		if(!txtcmp(*(char **)(b->group),b->datme))
			b->flags |= MB_HILIT;
		else
			b->flags &= ~MB_HILIT;

		ccorner_text(b);
	}
	else
		white_block(b);
}

	static Button fmu_wild3_sel = MB_INIT1(
		NONEXT,
		NOCHILD,
		31,13,283,60,
		NODATA,
		see_opt_wild,
		fq_set_wild,
		NOOPT,
		&wild_stringq.string,0,
		NOKEY,
		0
		);
	static Button fmu_wild2_sel = MB_INIT1(
		&fmu_wild3_sel,
		NOCHILD,
		31,13,249,60,
		NODATA,
		see_opt_wild,
		fq_set_wild,
		NOOPT,
		&wild_stringq.string,0,
		NOKEY,
		0
		);
	static Button fmu_wild1_sel = MB_INIT1(
		&fmu_wild2_sel,
		NOCHILD,
		31,13,215,60,
		NODATA,
		see_opt_wild,
		fq_set_wild,
		NOOPT,
		&wild_stringq.string,0,
		NOKEY,
		0
		);
static Button fmu_sdots_sel = MB_INIT1(
	NULL,		/* Filled in at run time. */
	&fmu_wild1_sel,
	24,13,188,60,
	"*.*",
	see_opt_wild,
	fq_set_wild,
	NOOPT,
	&wild_stringq.string,0,
	NOKEY,
	0
	);

static Button fwild_sel = MB_INIT1(
	&fmu_sdots_sel,
	NOCHILD,
	71,13,243,44,
	&wild_stringq,
	see_string_req,
	fq_wild_stringq,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button fwildp_sel = MB_INIT1(
	&fwild_sel,
	NOCHILD,
	9*6+1,13,184,44,
	NODATA, /* "Wildcard:", */
	black_leftlabel,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	'*',
	0
	);
static char und_drawer[PATH_SIZE] ="c:";
static Stringq drawer_stringq = {
	2, 2, 16, PATH_SIZE-1, 0, NULL, und_drawer, 0, 0,
	};

static Button fdrawer_sel = MB_INIT1(
	&fwildp_sel,
	NOCHILD,
	101,13,213,28,
	&drawer_stringq,
	see_string_req,
	fq_drawer_stringq,
	NOOPT,
	&fwild_sel,0,
	NOKEY,
	0
	);

static Button fdrawerp_sel = MB_INIT1(
	&fdrawer_sel,
	NOCHILD,
	4*6+1,13,184,28,
	NODATA, /* "Dir:", */
	black_leftlabel,
	undo_drawer,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static char und_file[PATH_SIZE] = "zippy";
static Stringq file_stringq = {
		STQ_LJUST, STQ_CENTER, 15, PATH_SIZE-1, 0, NULL, und_file,
	};
static Button ffile_sel = MB_INIT1(
	&fdrawerp_sel,
	NOCHILD,
	95,13,219,12,
	&file_stringq,
	see_string_req,
	accept_file_name,
	NOOPT,
	&fdrawer_sel,0,
	'\t',
	0
	);
static Button ffilep_sel = MB_INIT1(
	&ffile_sel,
	NOCHILD,
	6*5+1,13,184,12,
	NODATA, /* "File:", */
	black_leftlabel,
	undo_file,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Name_scroller fscroller;
static Button flist_sel = MB_INIT1(
	&ffilep_sel,
	NOCHILD,
	81,83,20,12,
	NODATA,
	see_scroll_names,
	feel_scroll_cels,
	NOOPT,
	&fscroller,0,
	NOKEY,
	0
	);

static Button fdown_sel = MB_INIT1(
	&flist_sel,
	NOCHILD,
	15,13,3,82,
	&ctridown,
	ccorner_image,
	scroll_incdown,
	NOOPT,
	&fscroller,0,
	DARROW,
	0
	);

static Button fscroll_sel = MB_INIT1(
	&fdown_sel,
	NOCHILD,
	15,55,3,26,
	&fscroller,
	see_scrollbar,
	feel_scrollbar,
	NOOPT,
	&fscroller,0,
	NOKEY,
	0
	);

static Button fup_sel = MB_INIT1(
	&fscroll_sel,
	NOCHILD,
	15,13,3,12,
	&ctriup,
	ccorner_image,
	scroll_incup,
	NOOPT,
	&fscroller,0,
	UARROW,
	0
	);

static Button ftitle_sel = MB_INIT1(
	&fup_sel,
	NOCHILD,
	320,10,0,0,
	NULL, /* gets set to prompt by pj_get_filename(prompt) */
	see_titlebar,
	mb_clipmove_menu,
	mb_menu_to_bottom,
	NOGROUP,0,
	NOKEY,
	0
	);

static Menuhdr fileq_menu = {
	{320,97,0,103,},   /* width, height, x, y */
	0,				/* id */
	PANELMENU,		/* type */
	&ftitle_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	SCREEN_CURSOR,	/* use screen cursor */
	seebg_white, 	/* seebg */
	NULL,			/* dodata */
	NULL,			/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
};

static Smu_button_list fmu_smblist[] = {
	{ "ok", &fok_sel },
	{ "cancel", &fcancel_sel },
	{ "plus", &fplus_sel },
	{ "wild", &fwildp_sel },
	{ "dir", &fdrawerp_sel },
	{ "file", &ffilep_sel },
};

static SHORT *fscroller_top_name;

extern Stringq drawer_stringq, file_stringq, wild_stringq;
extern char und_drawer[];


static FUNC redisplay_drawer;	/* helps share code with browse.c */
static char *fq_drawer;

static Names *wild_lst;

static Errcode new_drawer(void)
{
Errcode err;

	if((err = change_dir(fq_drawer)) < Success)
		return(errline(err,fq_drawer));
	get_dir(fq_drawer);
	init_fscroller();
	init_stq_string(&drawer_stringq);
	draw_buttontop(&fdrawer_sel);
	draw_button(&fmu_dev_hanger);
	draw_button(&fmu_topdev_hanger);
	redraw_fscroller();
	return(Success);
}

static void feel_1_fname(Button *m,void *rast,int x,int y,Names *entry,
					  int why)
{
char *name;

	name = entry->name;
	if (name[0] == '\\')	/* a directory */
	{
		if(why == SCR_ARROW)
			return;
		strcpy(und_drawer, fq_drawer);
		make_file_path(fq_drawer,name,fq_drawer);
		strcat(fq_drawer, "\\");
		init_stq_string(&drawer_stringq);
		if(new_drawer() < Success)
		{
			strcpy(fq_drawer,und_drawer);
			init_stq_string(&drawer_stringq);
		}
	}
	else
	{
		if(why & (SCR_MDHIT|SCR_ENTER))
			mb_close_ok(m);
		setf_stringq(&ffile_sel,1,"%s", name);
	}
}

static void init_fscroller()
{
SHORT tname;

	tname = fscroller.top_name; /* save top name */
	free_wild_list(&wild_lst);
	clear_struct(&fscroller);
	fscroller.top_name = tname; /* replace top name */

	build_wild_list(&wild_lst, wild_stringq.string, TRUE);
	fscroller.names = wild_lst;
	fscroller.scroll_sel = &fscroll_sel;
	fscroller.list_sel = &flist_sel;
	fscroller.feel_1_cel = feel_1_fname;
	fscroller.font = icb.input_screen->mufont;
	init_name_scroller(&fscroller,icb.input_screen);
}

static void redraw_fscroller(void)
{
	redraw_scroller(&fscroller);
}

static void undo_file(Button *m)
{
	undo_stringq(m, &ffile_sel);
}

static void undo_drawer(Button *m)
{
	undo_stringq(m, &fdrawer_sel);
	if ((*redisplay_drawer)() < 0)
		undo_stringq(m, &fdrawer_sel);
}

static void fq_new_drawer()
{
int len;

tr_string(fq_drawer, '/', '\\');	/* convert slash to backslash */
len = strlen(fq_drawer);
if(len > 0)		/* Insure trailing \ */
	{			/* unless it's something like a:  or just empty. */
	if(fq_drawer[len-1] != '\\' && fq_drawer[1] != ':')
		{
		fq_drawer[len] = '\\';
		fq_drawer[len+1] = 0;
		}
	}
if((*redisplay_drawer)() < Success)
	{
	undo_stringq(&fdrawer_sel, &fdrawer_sel);
	(*redisplay_drawer)();
	}
}

static void fq_drawer_stringq(Button *m)
{
int ret;

	ret = feel_string_req(m);
	if(ret & STQ_ALTERED)
		{
		fq_new_drawer();
		}
}

static void fq_redraw_new_wild()
/* Redraw parts of menu that need it whenever the wildcard changes. */
{
	draw_buttontop(&fwild_sel);
	init_fscroller();
	redraw_fscroller();
	draw_button(&fmu_sdots_sel); /* re hilite wild buttons */
}

static void fq_set_wild(Button *b)
{
	setf_stringq(&fwild_sel,FALSE,"%s",b->datme);
	fq_redraw_new_wild();
}

static void fq_wild_stringq(Button *b)
{
	if(feel_string_req(b) & STQ_ALTERED)
	{
		fq_redraw_new_wild();
	}
}

static void fq_ok_plus(Button *b)
{
	inc_file();
	mb_gclose_ok(b);
}

static void inc_file(void)
{
	pj_inc_filename(file_stringq.string);
	*fscroller_top_name = fscroller.top_name;
}

void accept_file_name(Button *b)
{
int ret;
Stringq *sq = b->datme;
char *string;
char file_side[PATH_SIZE];
char path_side[PATH_SIZE];

for (;;)
	{
	ret = feel_string_req(b);
	if (ret & STQ_ESCAPE)
		{
		goto OUT;
		}
	string = sq->string;
	tr_string(string, '/', '\\');	/* convert slash to backslash */
	split_copy_path(string, path_side, file_side);
	if (name_is_wild(file_side))
		{
		setf_stringq(&fwild_sel, FALSE, "%s", file_side);
		if (path_side[0] != 0)
			{
			setf_stringq(&fdrawer_sel,TRUE,"%s",path_side);
			fq_new_drawer();
			draw_buttontop(&fwild_sel);
			draw_button(&fmu_sdots_sel); /* re hilite wild buttons */
			}
		else
			{
			fq_redraw_new_wild();
			}
		}
	else
		{
		if (file_side[0] == 0)	/* Directory only. */
			{
			setf_stringq(&fdrawer_sel,TRUE,"%s",path_side);
			fq_new_drawer();
			}
		else
			{
			if (is_directory(string))
				{
				setf_stringq(&fdrawer_sel,TRUE,"%s",string);
				fq_new_drawer();
				}
			else
				{
				fix_suffix(string);
				if(ret & STQ_ENTER)
					mb_gclose_ok(b);
				goto OUT;
				}
			}
		}
	stringq_revert_to_undo(sq);
	draw_button(b);
	if (ret & STQ_TAB)
		goto OUT;
	}
OUT:
return;
}

static Errcode qfile(
	char *path_out,	/* Initial contents ignored.  Returns full path. */
	char *dir_in, 	/* Directory.  Updated by routine if user changes it */
	char *file_in,  /* File name.  Updated by routine if user changes it */
	char *prompt,	/* Line of text to display at top of requestor */
	char *ok_text,	/* string for the 'ok' button */
	int num_wilds,     /* number of wildcards */
	char **wildcards,  /* list of wildcards (<3) First is default read only */
	char *wildbuf) /* buffer for wildcard must be at least "WILD_SIZE" long */
{
char odir[PATH_SIZE];
Button *dhanger, *wildb;
Errcode err;
void *ss = NULL;

	redisplay_drawer = new_drawer;
	get_dir(odir);
	if ((err = change_dir(dir_in)) < Success)
		softerr(err,"!%s", "cant_find", dir_in);
	make_good_dir(dir_in);

	fq_drawer = dir_in;

	set_stq_string(&drawer_stringq,dir_in);
	set_stq_string(&file_stringq,file_in);

	if(num_wilds <= 0)
	{
		if(!wildbuf[0])
			wildbuf[0] = 0;
	}
	else
	{
		if(!wildbuf[0])
			strncpy(wildbuf,wildcards[0],WILD_SIZE-1);
		--wildcards;
		for(wildb = fmu_sdots_sel.children;wildb != NULL;wildb = wildb->next)
		{
			for(;;)
			{
				if(num_wilds-- <= 0)
				{
					wildb->datme = NULL;
					break;
				}
				++wildcards;
				if(!txtcmp(*wildcards,fmu_sdots_sel.datme))
					continue;
				wildb->datme = *wildcards;
				break;
			}
		}
	}

	set_stq_string(&wild_stringq,wildbuf);


	/* if more than 12 buttons hang up higher -2 for '\' and '..' */

	if(pj_get_devices(NULL) > (12 - 2)) 
		dhanger = &fmu_topdev_hanger;
	else
		dhanger = &fmu_dev_hanger;
	fmu_sdots_sel.next = dhanger;

	if((err = alloc_dev_sels(dhanger,&fmu_devsel_320size,4,5,dir_in,
							 new_drawer )) < Success)
	{
		goto error;
	}
	if((err = soft_buttons("fscroll_panel", fmu_smblist,
						Array_els(fmu_smblist), &ss)) < Success)
	{
		goto error;
	}

	/* note text put in after buttons are loaded */
	fok_sel.datme = ok_text;
	ftitle_sel.datme = prompt;	/* display prompt in move area... */
	fscroller.top_name = *fscroller_top_name;
	init_fscroller();

	menu_to_cursor(icb.input_screen,&fileq_menu);
	err = do_reqloop(icb.input_screen, &fileq_menu, 
						  &ffile_sel,NULL,NULL);
	full_path_name(drawer_stringq.string,file_stringq.string,path_out);
	*fscroller_top_name = fscroller.top_name;

error:
	smu_free_scatters(&ss);
	cleanup_dev_sels(dhanger);
	free_wild_list(&wild_lst);
	change_dir(odir);
	return(softerr(err, NULL));
}


char *pj_get_filename(char *prompt, char *suffi, char *button, 
		   			  char *inpath, char *outpath, 
					  Boolean force_suffix, SHORT *scroll_top_name,
					  char *wildcard)

/* put up a file requestor.  If inpath or outpath are NULL then
 * use static buffer.  Else start with inpath and update outpath
 * to directory user selects on exit.  Returns NULL if user
 * cancels, otherwise a pointer to outpath with the full path name selected.
 * Wildcard is the initial contents of the wildcard field and is altered to 
 * reflect it's new contents, If wildcard is NULL it will use the stack buffer
 * and if wildcard is empty or NULL it will default to the first suffix as in
 * *.XXX */
{
Errcode err;
static char	path_buf[PATH_SIZE];
char drawer_buf[PATH_SIZE];
char fname_buf[PATH_SIZE];
char wild_buf[WILD_SIZE];
char wildbufs[6*4];
char *wbuf;
char *wilds[4];
int num_wilds;
int len;
SHORT top_name = 0;

	if(NULL == (fscroller_top_name = scroll_top_name))
		fscroller_top_name = &top_name;

	if(!inpath)
		inpath = path_buf;
	if(!outpath)
		outpath = path_buf;
	if(!wildcard)
	{
		wildcard = wild_buf;
		wild_buf[0] = 0;
	}

	num_wilds = 0;
	if(suffi)
	{
		wbuf = wildbufs;
		for(;num_wilds < Array_els(wilds);++num_wilds)
		{
			wilds[num_wilds] = wbuf;
			*wbuf++ = '*';
			if((len = parse_to_semi(&suffi,wbuf,5)) == 0)
				break;
			wbuf += len;
			*wbuf++ = 0;
		}
	}

	if(!num_wilds || !pj_valid_suffix(&wildbufs[1]))
		force_suffix = FALSE;

	split_copy_path(inpath,drawer_buf,fname_buf);
	if(fname_buf[0] == 0)
		strcpy(fname_buf,unnamed_str);
	else if(force_suffix)
		sprintf(pj_get_path_suffix(fname_buf),"%.4s", &wildbufs[1]);

	if((err = qfile(outpath, drawer_buf, fname_buf, 
					prompt, button, num_wilds, wilds, wildcard)) < Success)
	{
		outpath = NULL;
		goto error;
	}

	if( *fix_suffix(outpath) == '\0' && force_suffix)
		strcat(outpath, &wildbufs[1]);

error:
	return(outpath);
}
