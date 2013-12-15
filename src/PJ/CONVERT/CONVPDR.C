/* convpdr.c - Let user select a PDR for which there is no pulldown entry.
 * Builds up and displays a list of PDR's.
 */

#include "errcodes.h"
#include "convert.h"
#include "linklist.h"
#include "resource.h"
#include "softmenu.h"
#include "rexlib.h"


static Errcode select_pdr(char *pdr_name_buf, char *title_header_key)
/*****************************************************************************
 * Bring up PDR selector and get user choice.
 ****************************************************************************/
{
char sbuf[80];

	return(go_pdr_menu( stack_string(title_header_key,sbuf),
						pdr_name_buf,  /* put name in here */
						NULL,  /* dont retrieve suffi */
						NULL,  /* no local pdrs here */
						0,	   /* list all of them */
						FALSE ));  /* list single frame ones too */
}

Errcode load_other()
{
Errcode err;
static char pdr[FILE_NAME_SIZE];

	if ((err = select_pdr(pdr, "conv_set_load_type")) >= Success)
	{
		err = get_a_flic(pdr,NULL,NULL);
	}
	return(err);
}

Errcode save_other()
{
Errcode err;
static char pdr[FILE_NAME_SIZE];

	if ((err = select_pdr(pdr, "conv_set_save_type")) >= Success)
	{
		err = save_a_flic(pdr, NULL, cs.ifi.ai.num_frames,
					conv_seek);
	}
	return(err);
}

Errcode conv_pdropt_qchoice(Pdroptions *popt)
/*****************************************************************************
 * the PDR has given us a non-NULL Pdroptions pointer, conduct Qchoice dialogs
 * so that the PDR's option values are set before pd->create_ifile() is called.
 ****************************************************************************/
{
	int    counter;
	int    err;
	char   **lst;
	char   *soft_text;
	char   *text;
	UBYTE  *option;

	if (popt == NULL)
		return Success;

	lst = &popt->choicelst1;
	option = &popt->option1;

	for (counter = 0; counter < 4; ++counter, ++lst, ++option)
	{
		if (*lst != NULL)
		{
			soft_text = rex_key_or_text(*lst, &text);

			if( ((soft_text = rex_key_or_text(*lst,&text)) != NULL)
				&& (smu_load_name_text(&smu_sm,"picdrive_texts", 
							   soft_text, &soft_text) >= Success))
			{
				text = soft_text;
			}
			err = qchoicef(NULL, text);
			smu_free_text(&soft_text);
			if(err < Success)
				return(err);
			else
				*option = err;
		}
	}

	popt->options_valid = TRUE;

	return Success;
}
