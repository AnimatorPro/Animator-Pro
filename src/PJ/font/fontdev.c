
/* fontdev.c - 'Device' level functions for fonts.  One device for each
   type of font supported. */

#include "errcodes.h"
#include "fontdev.h"
#include "jfile.h"
#include "linklist.h"
#include "rastext.h" /* sixhi_font_name */
#include "stfont.h"

Font_dev *font_dev_list;

static void add_font_dev(Font_dev *f)
/* makes sure it is not in the list then makes sure it is the first element */
{
	font_dev_list = (Font_dev *)remove_el(font_dev_list, f);
	f->next = font_dev_list;
	font_dev_list = f;
}

void init_font_dev(void)
{
add_font_dev(&ami_font_dev);
add_font_dev(&hpjet_font_dev);
add_font_dev(&st_font_dev);
add_font_dev(&type1_font_dev);
}

Errcode init_menufont_dev(void)
{
	init_font_dev();
	return(Success);
}

Errcode load_font(char *title, Vfont *font, SHORT height, SHORT unzag_flag)
{
Font_dev *fd;

	if(pj_name_in_path(title,sixhi_font_name))
	{
		init_sixhi_vfont(font);
		return(Success);
	}

	if(!pj_exists(title))
		return(pj_ioerr());

	fd = font_dev_list;
	while (fd != NULL)
	{
		if (fd->check_font(title) == Success)
			return(fd->load_font(title, font, height, unzag_flag));
		fd = fd->next;
	}
	return(Err_unknown_font_type);
}

