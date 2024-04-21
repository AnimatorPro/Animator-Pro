#define WORDWRAP_INTERNALS
#include "wordwrap.h"

void wwtext(Raster *screen,
			Vfont *f,
			char *s,
			int x,int y,int w,int h,
			int skiplines,
			int justify,
			Pixel color,Text_mode tmode,Pixel color2)
{
char buf[512];
int dy;

	dy = font_cel_height(f);
	for (;;)
	{
		if(s == NULL)
			break;
		wwnext_line(f, &s, w, buf, 0);
		if (--skiplines < 0)
		{
			justify_line(screen,f,buf,x,y,w,color,tmode,color2,justify,NULL,0);
			y += dy;
			h -= dy;
			if (h < tallest_char(f))
				break;
		}
	}
	return;
}
