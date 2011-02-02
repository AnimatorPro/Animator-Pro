
#ifndef FONTDEV_H
#define FONTDEV_H

#ifndef JFILE_H
	#include "jfile.h"
#endif /* JFILE_H */

#ifndef RASTEXT_H
	#include "rastext.h"
#endif /* RASTEXT_H */

typedef struct font_dev
	{
	struct font_dev *next;
	char *type_name;
	char *wild_pat;
	EFUNC check_font;
	EFUNC load_font;
	SHORT type;
	/* after here just the concern of the device loader */
	SHORT flags;
	} Font_dev;
extern Font_dev *font_dev_list;

Font_dev *find_fdev_for_type(SHORT type);

#endif /* FONTDEV_H */
