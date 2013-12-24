#include "pjbasics.h"

void toggle_menu(void)
{
	if(curr_group(vb.screen)->non_hidden)
		hide_mp();
	else
		show_mp();
}
