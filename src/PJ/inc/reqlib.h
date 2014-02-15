#ifndef REQLIB_H
#define REQLIB_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef MENUS_H
	#include "menus.h"
#endif


/***** standard textboxes directed to icb.input_screen ********/
/*
 * For the non "soft_" requestors the "fmt" string is used the same way
 * a format string is used for printf() or,
 * If the format string begins with  a leading "!%" then it is interpreted
 * as a ftextf() formats string and the next argument is assumed to be the
 * text string followed by the items to be formatted into the text.
 *
 * For the "soft_" requestors the "key" argument may either be a ftexf()
 * formats string (in which case the following argument is the softmenu key
 * followed by the arguments to be formatted) or, if no arguments are to be
 * formatted the "key" argument is simply the key.  The formats string MUST
 * start with a '!%' conversely no key values may start with '!%' or they
 * will be interpreted as a formats to be followed by a key string.
 *
 *	Arguments specified in the formats string are referenced inside the text
 * using ![?] where '?' is an the index(+1) of the formatted argument.
 * ![1] woud be the first argument and ![14] would be the fourteenth, etc.
 * arguments may be inserted in the text more than once.
 */

Errcode boxf(char *fmt,...);

#define TTEXTF_MAXCHARS 80	 /* size of maximum displayed chars 
							  * in ttextf() call */

Errcode ttextf(char *fmt,va_list argptr, char *formats);
Errcode soft_ttextf(char *key,va_list *pargs);

extern Errcode continu_box(char *fmt, ...);
extern Errcode soft_continu_box(char *key, ...);

extern Errcode
varg_continu_box(char *formats, char *text, va_list args, char *etext);

Boolean yes_no_box(char *fmt,...);
Boolean soft_yes_no_box(char *key,...);
extern Boolean varg_yes_no_box(char *formats, char *text, va_list args);

Errcode multi_box(char **choices, char *fmt,...);

/* plase wait window that self removes next time input is waited on
 * unless cleanup is called explicitly */

extern void cleanup_wait_box(void);
extern Errcode soft_put_wait_box(char *key, ...);
extern Errcode varg_put_wait_box(char *formats, char *text, va_list args);

/*** canned special purpose requestor menus ******/

Boolean qreq_number(short *inum,short min,short max,char *hailing,...);
Boolean soft_qreq_number(short *inum,short min,short max,char *key,...);

/* if update returns < success the requestor is canceled */

Boolean ud_qreq_number(short *inum,short min,short max,
					   EFUNC update, void *vfuncdat,char *hailing,...);
Boolean soft_ud_qreq_number(short *inum,short min,short max,
							EFUNC update, void *vfuncdat,char *key,...);
Boolean clip_soft_qreq_number(short *inum,short min,short max,
					   	  	EFUNC update, void *vfuncdat,char *key,...);

Boolean qreq_string(char *strbuf,int bufsize,char *hailing,...);
Boolean soft_qreq_string(char *strbuf,int bufsize,char *key,...);

int qchoicef(USHORT *qc_flags, char *fmt,...);
int soft_qchoice(USHORT *qc_flags, char *key,...);

char *pj_get_filename(char *prompt, char *suffi, char *button,
					  char *inpath, char *outpath, Boolean force_suffix,
					  SHORT *scroll_top_name, char *wildcard);

/**** device button allocator *****/

Errcode alloc_dev_sels(Button *hanger, Rectangle *size320x200, int numcols,
		int numrows, char *drawer, Errcode (*on_newdrawer)(void *),...);

#endif /* REQLIB_H */
