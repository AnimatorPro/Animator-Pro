#include <ctype.h>
#include <string.h>
#include "memory.h"
#include "menus.h"

static void see_pullist(int x, int y, Pull *p, Pullwork *pw);

void draw_menupull(Menuhdr *mh)
/* used only for drawing top level in pull menu window called by draw_menu()
 * draws first list of pulls into menu window */
{
Pullwork pw;

	if(mh->mw == NULL)
		return;
	init_pullwork(&pw,mh);
	pw.port = (Rcel *)(&(mh->mw->w));
	see_pullist(0,0,mh->mbs,&pw);
}
static void see_pullist(int x, int y, Pull *p, Pullwork *pw)
{
	while(p != NULL)
	{
		if(p->see != NULL)
			(*(p->see))(x + p->x,y + p->y,p,pw);

		p = p->next;
	}
}
static void get_pull_key(Pull *p,char *buf)
{
UBYTE key;
char *str;

	key = p->key_equiv;

	if(isgraph(key))
	{
		*buf++ = key;
		*buf = 0;
		return;
	}
	switch(key)
	{
		case ESCKEY:
			str = "[esc]";
			break;
		case '\n':
			str = "[cr]";
			break;
		case '\t':
			str = "[tab]";
			break;
		case '\b':
			str = "[back]";
			break;
		default:
			str = "";
	}
	strcpy(buf,str);
}

/****************** pull see functions ********************/

void pull_leftext( int x, int y, Pull *p, Pullwork *pw)

/* Will display left justified text.  If the first character is a space ' ' 
 * it will make that space the size of an asterisk for alignment */
{
char buf[32];
char *ptxt;
int txtx;
int txty;
Pixel color;
char k, k2;
struct vfont *font = pw->font;


	ptxt = (char *)(p->data);
	txty = y+font_ycent_oset(font,p->height);
	if (p->flags & PULL_HILIT) 
		color = pw->screen->SRED;
	else if (p->flags & PULL_DISABLED)
		color = pw->screen->SGREY;
	else
		color = pw->screen->SBLACK;


	txtx = x+(pw->spwidth/3);
	if(ptxt[0] == ' ')
	{
		++ptxt;
		txtx += fchar_spacing(font,"*");
	}

	gftext(pw->port,font,ptxt,txtx,txty,color,TM_MASK1);

	if (ptxt[0] == '*')	/* skip over initial asterisk */
	{
		txtx += fchar_spacing(font,ptxt++);
	}
	/* now underline 2-key equiv character... */
	k2 = toupper(p->key2);
	if (toupper(ptxt[0]) != k2)		/* Only underline if key equiv is
									 * not the first letter */
	{
	for (;;)
		{
		txtx += fchar_spacing(font, ptxt++);
		k = toupper(ptxt[0]);
		if (k == 0)	/* oh well, it's not in the string at all */
			break;		
		if (k == k2) /* got a match */
			{
			pj_set_hline(pw->port, color, txtx,y+p->height-1,
						 fchar_spacing(font, ptxt));
			break;
			}
		}
	}


	if(p->key_equiv && p->flags & PULL_DOESKEYS)
	{
		get_pull_key(p,buf);
		x += p->width;
		x -= (fstring_width(font,buf) + (pw->spwidth/3));
		gftext(pw->port,font,buf,x,txty,color,TM_MASK1);
	}
}
void pull_toptext( int x, int y, Pull *p, Pullwork *pw)
/* Adjust text by one line depending on resolution of screen arrr */
{
void *oport;

	if (p->height == 8)		/* it's lo res unscaled  */
		y -= 1;
	/* draw text in window, not on screen because text is not erased */
	oport = pw->port;
	pw->port = (Rcel *)(pw->root->mw); 
	pull_leftext(x,y,p,pw);
	pw->port = oport;
}

void pull_oblock(int x, int y, Pull *p, Pullwork *pw)
{
	pj_set_rect(pw->port,pw->screen->SWHITE,x+1,y+1,p->width-2,p->height-2);
	draw_quad((Raster *)pw->port, pw->screen->SGREY, x, y, p->width, p->height);
}
void pull_midline(int x, int y, Pull *p, Pullwork *pw)
{
	pj_set_hline(pw->port,pw->screen->SGREY,x,y+(p->height>>1),p->width);
}
void see_pull(int x, int y, Pull *p, Pullwork *pw)
{
	(*p->see)(x, y, p, pw);
	p = p->children;
	while (p)
	{
		(*p->see)(x + p->x, y+p->y, p, pw);
		p = p->next;
	}
}

