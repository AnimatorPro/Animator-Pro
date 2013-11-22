
/* textwind.c - A file with utility text-window oriented routines used
  by textedit.c. */

#include "jimk.h"
#include "flicmenu.h"
#include "fli.h"
#include "gemfont.h"
#include "text.h"

extern render_bitmap_blit();


extern char *text_buf;
extern int text_size;
extern int text_alloc;

static char twin_up;

dtextcel()
{
if (text_buf != NULL)
	{
	wwtext(render_form, usr_font, text_buf, 
		vs.twin_x, vs.twin_y, vs.twin_w, vs.twin_h, 
		vs.ccolor,a1blit, vs.text_yoff, 0);
	zoom_it();
	}
rub_twin();
}



rub_twin()
{
some_frame(vs.twin_x-1, vs.twin_y-1, 
	vs.twin_x+vs.twin_w, vs.twin_y+vs.twin_h,sdot);
twin_up = 1;
}


free_text()
{
gentle_freemem(text_buf);
text_buf = NULL;
}

