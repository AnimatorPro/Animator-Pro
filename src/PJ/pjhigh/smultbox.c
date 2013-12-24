#include "pjbasics.h"
#include "softmenu.h"
#include "ftextf.h"
#include "errcodes.h"

Boolean soft_multi_box(char **keys,char *symbol,...)

/* keys is an array of char pointers the first one is the formatted 
 * hailing text key.  The subsequent ones are the button text keys. The last one
 * is NULL to terminate the list. This will look for these keys in a 
 * Strings class item in the menu resource, You can have up to 5
 * buttons after the hailing text.
 *
 * returns (index of choice button) + 1  Err_abort if canceled < Success
 * if other error */
{
Errcode err;
va_list args;
char *formats;
char *choices[TBOX_MAXCHOICES+2];
Smu_name_scats scts[TBOX_MAXCHOICES+1];
void *ss;
int count;

	for(count = 0;count < (TBOX_MAXCHOICES+1);++count)
	{
		if((scts[count].name = *keys++) == NULL)
			break;
		scts[count].toload.ps = &choices[count];
	}
	if(count < 2)
		return(Err_bad_input);
	choices[count] = NULL;

	va_start(args,symbol);
	formats = ftext_format_type(&symbol,&args);

	if((err = soft_name_scatters(symbol, scts, count, &ss, 0)) < Success)
	{
		goto error;
	}

	err = tboxf_choice(icb.input_screen,formats,choices[0],args,
					   &choices[1],NULL);
error:

	va_end(args);
	smu_free_scatters(&ss);
	return(err);
}
