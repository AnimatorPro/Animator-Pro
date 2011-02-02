#ifndef SOFTMENU_H
#define SOFTMENU_H

/* softmenu.h - stuff to read in pulldowns and numbered item menus from
 * ascii resource file.  
 *
 * A softmenu resource file has the following general format:
 *		MenuClass MenuName { data }
 * The only restriction on data is that it must not have a matching
 * number of { and }'s (except brackets embedded in quoted strings or
 * characters are ignored).
 *
 * Softmenu will then dispatch data to the parser associated with MenuClass.
 * 
 * On initialization Softmenu builds up an index of MenuName's in the 
 * resource file.   
 * 
 * Softmenu depends on */

#ifndef SOFTOK_H
	#include "softok.h"
#endif 

#ifndef MENUS_H
	#include "menus.h"
#endif

/* The major structure for Softmenu is */
	typedef struct softmenu
		{
		void *sf;	/* a FILE but don't want to force <stdio.h> */
		struct smu_class *classes;
		int class_count;
		long err_line;	/* where error has occured if any */
		} Softmenu;
#define smu_is_open(sm) ((sm)->sf != NULL)

/* Substructures of a Softmenu are */
	typedef struct smu_class
	/* one of these for each class */
		{
		char *name;
		struct smu_symbol *symbols;	/* singly linked list */
		} Smu_class;

	typedef struct smu_symbol
	/* one of these for each item in each class */
		{
		struct smu_symbol *next;
		char *name;
		long fline;
		long foff;			/* resource file offset */
		long fsize;			/* size of item */
		} Smu_symbol;

	typedef struct 
	/* holds a list of strings - result of smu_get_strings() */
		{
		int count;
		char **strings;
		} Smu_strings;

/* softmenu methods */
	Errcode smu_init(struct softmenu *sm,	/* constructor */
		char *resource_file);			/* resource file name */
	void smu_cleanup(struct softmenu *sm);	/* destructor */
	Errcode smu_lookup(struct softmenu *sm,	/* Find symbol */ 
		struct smu_symbol **psym,		/* return symbol here */
		unsigned class,					/* symbol class */
		char *name); 					/* symbol name */
	Errcode smu_get_strings(struct softmenu *sm,  /* read in Strings. */
		char *symname, 					/* name of string symbol */
		Smu_strings *s);				/* place to put strings */
			/* returns # of strings read or errcode */
	void smu_free_strings(Smu_strings *s);  /* free from smu_get_strings */
	/* Scan for err in NumString symname.  Fill in buf with matching text
	 * if found, and return length of text.  If a problem return errcode. */
	int smu_get_errtext(struct softmenu *sm,
		char *symname,		/* name of symbol in resource file */
		Errcode err,		/* errcode to find */
		char *buf);			/* place to put string */
	/* Allocates and reads in a pull-down from a file.  Does just about
	 * everything except calculate the pixel coordinates (see pullfmt.c)
	 * and install the dodata and domenu functions */
	Errcode smu_load_pull(struct softmenu *sm,		/* read in a pulldown */
		char *symname,					/* name of symbol */
		struct menuhdr *pullhdr);		/* place to put loaded pulldown */
	/* Free pullhdr gotten from smu_load_pull */
	void smu_free_pull(struct menuhdr *pullhdr);

	/* Read a string from a resource item composed of name/string pairs
	   into a fixed size buffer.  Returns size of string. */
	int smu_name_string(struct softmenu *sm,	/* read in a named string */
		char *symname,					/* name of symbol in resource file */
		char *strname,					/* name of string in symbol */
		char *strbuf,					/* place to put string */
		int bufsize);					/* length of place to put string */

	int soft_name_string(char *symname,	/* name of symbol in resource file */
						char *strname,	/* name of string in symbol */
						char *strbuf,	/* place to put string */
						int bufsize);	/* length of place to put string */

	Errcode smu_load_name_text(struct softmenu *sm,	/* read in a named string */
		char *symname,					/* name of symbol in resource file */
		char *strname,					/* name of string in symbol */
		char **ptext);					/* place to put allocated text */


	/* Allocates and reads in a qchoice text string from resource file 
	 * use smu_free_text() to free */
	Errcode smu_load_qchoice_text(Softmenu *sm, char *key, char **ptext);

	/* Allocates and reads in a string from resource file */
	Errcode smu_load_text(struct softmenu *sm, char *key, char **ptext);

	/* this can be freed with smu_free_text() */
	char *smu_clone_string(char *s);

	void smu_free_text(char **ptext);

	/* Reads in string from resource file into fixed sized buffer. */
	Errcode smu_string(struct softmenu *sm, char *key, 
		char *buf, int len);

/* Names and defines for the various classes of softmenus. */
#define SMU_STRINGS_CLASS 0
#define SMU_QCHOICE_CLASS 1
#define SMU_NUMSTRING_CLASS 2
#define SMU_PULL_CLASS 3
#define SMU_TEXT_CLASS 4
#define SMU_NAMESTRING_CLASS 5

extern char *smu_class_names[];
	/* you can use smu_class_names to find out name of class */

void swork_end(Swork *swork, Softmenu *sm);
	/* like swork_cleanup but transfers error line # to sm */


/* Shortcut routines that depend on the Softmenu - smu_sm - set up
 * during PJ initialization. */
extern Softmenu smu_sm;		/* global PJ softmenu handle */
Errcode load_soft_pull(
	struct menuhdr *mh, 	/* where to load pull */
	int subspace,			/* # of spaces to indent first leaf */
	char *name, 			/* symbolic name of pulldown */
	int muid,				/* Peter's menu id (???) */
	void (*selit)(struct menuhdr *mh, SHORT menuhit),/* selection function */
	int (*enableit)(struct menuhdr *mh)); /* Function to grey out items and do
								   * asterisk setting etc.  Called when 
								   * cursor goes into menu bar.
								   * May be NULL.  If present function should 
								   * finish by returning menu_dopull(mh) */

typedef struct smu_name_scats {
	char *name;
	union {
		char *s;
		char **ps;
	} toload;
} Smu_name_scats;

void smu_free_scatters(void **buf);

#define SCT_OPTIONAL 0x0001 /* if this is set items in the list
							 * may be left empty, default is that
							 there must be an item found for every 
							 item in the list */

#define SCT_INDIRECT  0     /* invert of indirect (default) if 0 */
#define SCT_DIRECT  0x0002 /* treats toload as direct pointers to
							 * string "s" default is indirect or "ps" */

#define SCT_STRINIT 0x8000	/* set on startup when loading common strings */

int smu_name_scatters(struct softmenu *sm,	/* read in a named string */
				char *symname,	/* name of symbol in resource file */
			  	Smu_name_scats *nscats,	/* names and where to put
			  						 	 * results */
			  	int num_scats, /* size of scts array */
			  	void **allocd,   /* pointer to what one needs to free.
			  					   * with smu_free_scatters() */
			 	USHORT flags );  /* see defines above */

Errcode soft_name_scatters(char *symname, Smu_name_scats *scts,
			  int num_scatters, void **allocd, USHORT flags);

typedef struct smu_button_list {
	char *name;
	union {
		Button *butn;
		char **ps;
	} toload;
} Smu_button_list;

int soft_buttons(char *listsym, Smu_button_list *blist, 
				 int bcount, void **allocd );

Errcode soft_qchoice_err(Errcode err, char *symname);
Errcode soft_strings(char *symname, Smu_strings *s);
char *soft_string(char *key, char *buf, int len);
#define stack_string(key,buf) \
	soft_string((key),(buf), sizeof(buf)/sizeof((buf)[0]))

extern char msg_name[];

/*** key equivalent stuff ****
 *
 *  The load function will only load the first time it is called.  It will load
 *  the 'key' values from the named items in a NameString list a key is 
 *  specified this way:  an ascii key is a single char, ie: k is "k".  
 *   in a NameString list A non ascii key
 *  is specified in reverse order to the hex value in input.h ie:
 *  0x2300 is specified as "\0\x23" in a namestring list.
 *
 * No freeing is needed for a loaded list.
 *****/

typedef struct keyequiv {
	char *name;
	VFUNC doit;
	USHORT flags;
	SHORT key;
} Keyequiv;

#define KE_HIDE 0x0001
#define KE_NOHIDE 0
#define KE_LOADED 0x0002

Errcode load_key_equivs(char *symname, Keyequiv *kfin, int count);
Boolean do_keyequiv(SHORT key, Keyequiv *kf, int count);

#endif /* SOFTMENU_H */
