/* pullfmt.c - calculate the xy positioning of a pulldown that has
   already been allocated, loaded, and has strings. */

#include <string.h>
#include "softmenu.h"

typedef struct pullf_cb
/* control block - variable space */
	{
	int xbor;			/* border spacing */
	int charw,charh;	/* character dimensions */
	Rectangle *srect;	/* screen rectangle */
	Pull *root;
	} Pullf_cb;

static int longest_pdata(Pull *p)
/* find longest pull data string */
{
int acc, len;

acc = 0;
len = 0;
while (p != NULL)
	{
	if (p->data != NULL)
		len = strlen(p->data);
	if (len > acc)
		acc = len;
	p = p->next;
	}
return(acc);
}


static void calc_leaf(Pull *p, Pullf_cb *sp)
{
#define MINWID 8
Pull *rp;
Pull *cp;
int w, h;
int y;
int charw = sp->charw, charh = sp->charh;
int xbor = sp->xbor;

rp = p->children;
cp = rp->children;
w = longest_pdata(cp);
if (w < MINWID)
	w = MINWID;
rp->width = w*charw+xbor+2;
h = slist_len((Slnode *)cp);
rp->height = h*charh+3;
rp->x = 0;
rp->y = charh-1;
rp->see = pull_oblock;
y = 1;
while (cp != NULL)
	{
	cp->x = 1;
	cp->y = y;
	cp->width = xbor+charw*w;
	cp->height = 1+charh;
	if (cp->data != NULL)
		{
		if (strncmp(cp->data,"----",4) == 0)
			{
			cp->see = pull_midline;
			cp->flags = PULL_DISABLED;
			cp->data = NULL;
			}
		else
			{
			cp->see = pull_leftext;
			cp->flags = PULL_DOESKEYS;
			}
		}
	y += charh;
	cp = cp->next;
	}
#undef MINWID
}

static void calc_leaf_positions(Pullf_cb *sp)
{
Pull *p = sp->root;

while (p!=NULL)
	{
	calc_leaf(p,sp);
	p = p->next;
	}
}

static void calc_root_positions(Pullf_cb *sp,int spaces)
{
#define MSPC 3 /* characters between top level menus */
int x, w;
Pull *p = sp->root;

x = sp->charw*spaces;
while (p != NULL)
	{
	w = (MSPC+strlen(p->data)) * sp->charw;
	p->x = x;
	p->y = 0;
	p->width = w;
	p->height = sp->charh;
	p->see = pull_toptext;
	p->flags = PULL_DOESKEYS;
	x += w;
	p = p->next;
	}
#undef MSPC
}

static void rclip_leafs(Pullf_cb *sp)
/* make sure leaf doesn't go off right edge of screen */
{
Pull *p = sp->root;
Pull *rp;

while (p != NULL)
	{
	rp = p->children;
	if (p->x + rp->x + rp->width > 
		sp->srect->width)
		{
		rp->x = sp->srect->width - p->x - rp->width;
		}
	p = p->next;
	}
}


void pullfmt(Menuhdr *mp, int subspace, 
	int charw, int charh, Rectangle *screen_rect)
{
Pullf_cb rpc;

rpc.xbor = charw>>1;
rpc.charw = charw;
rpc.charh = charh;
rpc.srect = screen_rect;
rpc.root = mp->mbs;
mp->x = screen_rect->x;
mp->y = screen_rect->y;
mp->width = screen_rect->width;
mp->height = charh;
calc_root_positions(&rpc,subspace);
calc_leaf_positions(&rpc);
rclip_leafs(&rpc);
}

