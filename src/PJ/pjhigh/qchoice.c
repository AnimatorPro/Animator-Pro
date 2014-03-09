/* qmenu.c - build up simple list of selections type menus out of
   strings etc.  */

#include <string.h>
#include "jimk.h"
#include "commonst.h"
#include "errcodes.h"
#include "ftextf.h"
#include "rastext.h"
#include "softmenu.h"

/***** qchoice menus directed to icb.input_screen *****************************/


static int makedo_qchoice(char *header, char **choices, 
						  int ccount, VFUNC *feelers, Boolean hide_on_hit,
						  USHORT *qc_flags)

/* Pass this one a string for the title bar of menu, and an array of strings
   ccount long of choices  (make sure ccount less than 10).  This will
   put up a menu and return the item from that menu chosen.  Returns after
   one choice 0 for the first choice, 8 for the maximum choice.  
   The last item in the 'choices' list is always numbered zero, 
   Err_abort is returned if this is selected or the space bar or right click
   outside menu area. do reqmenu may return an error and fail (no ram) */
{
Menuhdr *qc;
int ret;

	if((ret = build_qchoice(icb.input_screen, &qc, header,choices,ccount,
							feelers,hide_on_hit,qc_flags)) < 0)
	{
		return(ret);
	}
	qc->cursor = SCREEN_CURSOR;
	ret = do_reqloop(icb.input_screen,qc,NULL,NULL,NULL);
	cleanup_qchoice(qc);
	return(ret);
}

#ifdef WITH_POCO
int qchoice(USHORT *qc_flags, char *header, char **choices, int ccount)
/* Does not format and takes an array of strings returns 0 to 8 for choices
 * Err_abort if canceled other error code if error */
{
	return(makedo_qchoice(header,choices,ccount,NULL,TRUE,qc_flags));
}
#endif /* WITH_POCO */

static Errcode va_qchoicef(USHORT *qc_flags,char *formats, 
						   char *text,va_list args)

/** takes input as one string with formatting info first line is header, each
 * subsequent line is a choice, returns Err_abort if Aborted or errcode on 
 * failure 0 to 8 for a selected choice */
{
Errcode err;
int count;
char *choices[11];
char *tbuf;

	tbuf = NULL;
	if((err = get_formatted_ftext(&tbuf,0,formats,text,args,TRUE)) <= 0)
	{
		if(err == 0)
			err = Err_bad_input; /* no text! */
		goto done;
	}
	if(tbuf)
		text = tbuf;

	for(count = 0;count < Array_els(choices);++count)
	{
		choices[count] = text;
		if(NULL == (text = strchr(text,'\n')))
			break;
		if(!(*(++text))) /* if this was a terminal new line that's it */
			break;
	}
	if(count)
		err = makedo_qchoice(choices[0],&choices[1],count,NULL,TRUE,qc_flags);

done:
	pj_freez(&tbuf);
	va_end(args);
	return(softerr(err,"smu_choice")); 
}
int qchoicef(USHORT *qc_flags, char *fmt,...)
{
va_list args;
char *formats;
int ret;

	va_start(args,fmt);
	formats = ftext_format_type(&fmt,&args);
	ret = va_qchoicef(qc_flags, formats, fmt, args);
	va_end(args);
	return(ret);
}

int soft_qchoice(USHORT *qc_flags, char *key,...)
/* bring up formatted qchoice loaded from resource if key is the formats
 * string, ie: "!%..." the next item is the key followed by the args */
{
Errcode err;
char *text;
va_list args;
char *formats;

	va_start(args,key);
	if(NULL == (formats = ftext_format_type(&key,&args)))
		formats = "";
	if ((err = smu_load_qchoice_text(&smu_sm, key, &text)) < Success)
		goto error;
	err = va_qchoicef(qc_flags, formats, text, args);
	smu_free_text(&text);
error:
	va_end(args);
	return(soft_menu_err(err,SMU_QCHOICE_CLASS,key));
}
