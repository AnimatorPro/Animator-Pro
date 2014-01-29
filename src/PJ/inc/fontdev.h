
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
	Errcode (*check_font)(char *name);

	Errcode (*load_font)(char *title, Vfont *vfont,
			SHORT height, SHORT unzag_flag);

	SHORT type;
	/* after here just the concern of the device loader */
	SHORT flags;
	} Font_dev;

extern Font_dev ami_font_dev;
extern Font_dev hpjet_font_dev;
extern Font_dev st_font_dev;
extern Font_dev type1_font_dev;

extern Font_dev *font_dev_list;

extern void init_font_dev(void);
extern Errcode init_menufont_dev(void);

#endif /* FONTDEV_H */
