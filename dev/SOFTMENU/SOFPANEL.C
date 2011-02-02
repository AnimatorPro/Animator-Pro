#include "softmenu.h"
#include "errcodes.h"
#include "ptrmacro.h"
#include "memory.h"
#include <string.h>


int soft_buttons(char *listsym, 
				 Smu_button_list *blist,
				 int bcount,
				 void **allocd )

/* this takes a list like smu_name_scatters but it is a list of pointers to 
 * buttons. 
 * It will set the text (data) pointer of the buttons to the text
 * string for the name.  It will set the key equivalent to what follows a 
 * '\n' in the text if there is a '\n' otherwise it will set it to 0 
 * if the first character of the key is a 'T' the key is assumed to follow
 * the 0 and the item is loaded as a text */
{
Smu_button_list *maxbl;
Smu_button_list *bl;
int ret;
char *keyequiv;
Button *b;

	/* first set all pointers not preceded by a 'T' to refer to the datme 
	 * area of the buttons */

	maxbl = blist + bcount;

	for(bl = blist;bl < maxbl;++bl)
	{
		if(bl->name[0] != 'T')
			bl->toload.butn = (Button *)(&(bl->toload.butn->datme));
	}

	ret = smu_name_scatters(&smu_sm,listsym,(Smu_name_scats *)blist,
					  		bcount,allocd,SCT_INDIRECT);

	/* set all pointers not 'T' keys to again refer to the actual buttons,
	 * if we have a successful return, set key equivalents and terminate
	 * compound text strings */

	for(bl = blist;bl < maxbl;++bl)
	{
		if(bl->name[0] == 'T')
			continue;

		b = bl->toload.butn = TOSTRUCT(Button,datme,bl->toload.butn);
		if(ret < Success)
			continue;

		if((keyequiv = strchr(b->datme,'\n')) != NULL)
		{
			*keyequiv++ = 0; /* null terminate text */
			b->key_equiv = *((SHORT *)keyequiv);
		}
		else
		{
			b->key_equiv = NOKEY;
		}
	}
	return(ret);
}
