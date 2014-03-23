/* qmacro.c - macro numbered choice menu. */

#define INPUT_INTERNALS
#include "jimk.h"
#include "filemenu.h"
#include "input.h"
#include "menus.h"

void qmacro(void)
{
USHORT mdisables[7];
int choice;

	for(;;)
	{
		clear_mem(mdisables, sizeof(mdisables));
		if (icb.macro_mode == MAKE_MACRO) /* if making disable all but close */
		{
			mdisables[0] = mdisables[2] = mdisables[3] = mdisables[4]
				= mdisables[5] = QCF_DISABLED;
		}
		else
		{
			/* let them close while using */
				/* otherwise only close while making */

			if (icb.macro_mode != USE_MACRO) 
				mdisables[1] = QCF_DISABLED;
			if (!pj_exists(macro_name)) /* if no macro defined can't use */
			{
				mdisables[2] = mdisables[3] = QCF_DISABLED;
			}
		}

		switch (choice = soft_qchoice(mdisables,"record"))
		{
			case 0:
				qstart_macro();
				break;
			case 1:
				qclose_macro();
				if((icb.macro_mode|MACRO_OK) == USE_MACRO)
					break;
				continue; /* leave menu up if closed and not executing a 
						   * macro */
			case 2:
				quse_macro();
				break;
			case 3:
				qrepeat_macro();
				break;
			case 4:
				qrealtime_macro();
				break;
			case 5:
				go_files(FTP_MACRO);
				continue; /* leave menu up */
			default:
				break;
		}
		return;
	}
}

