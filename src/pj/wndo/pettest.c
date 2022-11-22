/* Peter's test suite. */
#include "jimk.h" /* to get globals */
#include "wndo.h"
#include "memory.h"
#include "errcodes.h"

static char *rex_name = "\\mcga\\";
static char menu_font_name[] = "nothere";

static char goodconf;

static Wndo *wndo0;
static Wndo *wndo1;
static Wndo *wndo2;
static Wndo *wndo3;
#define SWS 4
#define XX (SWS+3)
static Wndo *smallw[XX];

static WndoInit newin = {
	100,40,80,150,0,0,NULL,NULL,WNDO_NOCLEAR,
};

static WndoInit newin1 = {
	100,75,75,30,400,350,NULL,NULL,WNDO_NOCLEAR,
};

static WndoInit newin2 = {
	100,75,150,40,0,0,NULL,NULL,WNDO_NOCLEAR,
};

static WndoInit newin3 = {
	40,110,20,40,0,0,NULL,NULL,0
};

static WscrInit newscr;


static void spray_dot(Wndo *w)
{
short xy[2];
char rgb[3];
short i,roff=0;

i = 50;
while (--i >= 0)
	{
	polar( random()+roff++, random()%vs.air_spread, xy);
	xy[0]+=icb.sx;
	xy[1]+= icb.sx;
	cycle_ccolor();
	pj_put_dot(w, vs.ccolor, xy[0] + w->width/2, xy[1] + w->height/2);
	}
}

#define CIMIN 0
#define CIMAX 31
static int cicolor = CIMIN;
static int cirad=51;
static int cird=3;

static ciline(LONG y, LONG left, LONG right,Rcel *ciwin)
{
	pj_set_hline(ciwin, cicolor, left, y, right-left+1);
#ifdef NOTNOW
	pj_set_vline(ciwin, cicolor, y, left, right-left+1);
#endif
}

static void circlein(Rcel *w)
{
int temp;
static int dcol = 1;

if (cicolor >= CIMAX)
	dcol = -1;
else if(cicolor <= 0)
	dcol = 1;
cicolor += dcol;

#ifdef NOTNOW
	sdisk(w->height/2, w->width/2,  cirad, ciline, w);
#endif
	sdisk(w->width/2, w->height/2,  cirad, ciline, w);

temp = cird + cirad;

if (temp < 0 || temp > w->width)
	{
	cird = -cird;
	cirad = cird + cirad;
	}
else
	cirad = temp;
}



static fill_wndo(Wndo *w, int clear)
{
int y;
int x;

	for(y = 1; y < w->height - 1;++y)
	{
		for(x = 1;x < w->width - 1;++x)
			if(clear)
				pj_put_dot(w,0,x,y);
			else
				pj_put_dot(w,(x + (y * w->W_rastid)) & 0xff,x,y);
	}
}

static void do_text(Raster *rast, register char *s,int x,int y,
					int color,VFUNC tblit,int bcolor)
{
extern UBYTE sixhi_data[];
register char c;
static Pixel scol = 0;

	to_upper(s);
	y+=1;
	while ((c = *s++) != 0)
	{
		/*
		pj_set_rect(rast,color,x,y,6,6);
		*/
		(*tblit)(sixhi_data, 192, 6*c, 0, rast, x, y, 6,6, (++scol & 0xff), bcolor);
		x+=6;
	}
}

static test_rects(Wndo *w,Raster *r)
{
UBYTE lbuf[320];
int y;
int x;

#ifdef FORTESTING

	gen_swaprect(w,0,0,r,0,0,w->width,w->width);

	for(y = 0;y < w->height; ++y)
	{
 		pj_set_hline(w,y,0,y,w->width + 5);
	}

	for(x = 0;x < w->width; ++x)
	{
 		pj_set_vline(w,x,x,0,w->height + 5);
	}
	for(x = 0;x < w->width;++x)

	for(y = 0;y < w->height; ++y)
	{
		pj_stuff_words(y,lbuf,sizeof(lbuf)/2);
		/* get_vseg(r,lbuf,x,0,w->height + 5); */
		pj_put_hseg(w,lbuf,0,y,w->width + 5);
	}
	pj_xor_rect(w, -1 /* w->W_rastid + (32 * 3) */,-5,-5,w->width,w->height);

#endif

	for(y = 0;y < w->height; y += 7)
	{
		do_text((Raster *)w,
			"lots and lots.  Megabytes.  Gigabytes.  Terabytes.  RAM!!!!",
			0,y,0,pj_mask1blit,20);
	}

}
void return_to_main() {};

static void main(int argc, char **argv)
{
Errcode err;
int i, inc;
Rectangle pos[XX];
short dx[XX],dy[XX],dw[XX],dh[XX];
char buf[20];
Wndo *cw;
Wndo *backdrop;
Clipbox cb;

	if (argc > 1)
		rex_name = argv[1];
	if((err = startup_init()) < 0)
		exit(-1);
	if((err = init_screen()) < 0)
		exit(-1);

	find_mucolors(vl.screen);
	pj_set_rast(vl.screen,10);
	backdrop = &(vl.screen->wndo);

	{
		pj_clipbox_make(&cb,vl.screen,20,20,200,150);
		fill_wndo((Wndo *)&cb,0);
		errline(Err_no_memory,"circles");
	}

	for(i = 0;;)
	{
		if(++i > 150)
		{
			i = 0;
			if(yes_no_line("Quit circles ??"))
				break;
		}
		circlein((Rcel *)&cb);
	}


#ifdef NOPE
	if(yes_no_line("open backdrop ??"))
	{
	WndoInit wi;

		clear_struct(&wi);
		wi.x = wi.y = 0;
		wi.width = wi.height = 150;
		wi.flags = WNDO_BACKDROP;
		wi.screen = vl.screen;
		if((err = open_wndo(&backdrop,&wi)) < 0)
			goto error;
	}
#endif

	fill_wndo(backdrop,0);

	inc = 1;
	i = 0;
	for(;;)
	{
		if(i > 100)
			inc = -1;
		else if(i < -100)
			inc = 1;

		blitmove_rect(backdrop,i,i,backdrop,i+inc,i+inc,50,50); 
		set_leftbehind(backdrop,0,i,i,i+inc,i+inc,50,50);

		if(pj_key_is())
		{
			break;
			goto exitit;
		}
		i += inc;
	}

	{
	Rectangle rect;

		boxf("rubba rect");
		err = screen_cut_rect(vl.screen,&rect, NULL);
		boxf("e %d w %d h %d", err, rect.width, rect.height );
		goto exitit;
	}


	newin.screen = vl.screen;
	newin1.screen = vl.screen;
	newin2.screen = vl.screen;
	newin3.screen = vl.screen;
	
	if((err = open_wndo(&wndo0,&newin)) < 0)
		goto error;

	fill_wndo(backdrop,0);

	if((err = open_wndo(&wndo1,&newin1)) < 0)
		goto error;


	if((err = open_wndo(&wndo2,&newin2)) < 0)
		goto error;

	if((err = open_wndo(&wndo3,&newin3)) < 0)
		goto error;

	smallw[0] = wndo1;
	for (i=1; i<SWS+1; i++)
		{

		newin.x += 11;
		newin.y -= 20;
		if ((err = open_wndo(smallw+i,&newin)) < 0)
			goto error;
		pj_set_rast(smallw[i],3*i+32);
		sprintf(buf, "#%d Window", i);
		do_text((Raster *)smallw[i],
			buf,
			0,0,0,pj_mask1blit,20);
		}
	smallw[SWS+1] = wndo2;
	smallw[SWS+2] = wndo3;

	fill_wndo(wndo0,0);
	fill_wndo(wndo1,0);
	fill_wndo(wndo2,0);
	fill_wndo(wndo3,0);

	test_rects(backdrop,0);
	test_rects(wndo0,(Raster *)(vl.screen));
	test_rects(wndo1,(Raster *)(vl.screen));
	test_rects(wndo2,(Raster *)(vl.screen));
	fill_wndo(backdrop,0);

{
SHORT dscrollx = 1;
SHORT dscrolly = 1;
Short_xy oset;

	for(;;)
	{
		get_wndo_oset(wndo1,&oset);
		oset.x += dscrollx;
		oset.y += dscrolly;

		if(oset.x < 0)
		{
			oset.x = -oset.x;
			dscrollx = -dscrollx;
		}
		else if(oset.x > wndo1->width - (wndo1->W_xmax - wndo1->x))
			dscrollx = -dscrollx;

		if(oset.y < 0)
		{
			oset.y = -oset.y;
			dscrolly = -dscrolly;
		}
		else if(oset.y > wndo1->height - (wndo1->W_ymax - wndo1->y))
			dscrolly = -dscrolly;

		reposit_wndo(wndo1,NULL,&oset);

		if(pj_key_is())
		{
			pj_key_in();
			break;
		}
	}
}



		for (i=0; i<XX; i++)
		{
			dx[i] = random()%4|1;
			dy[i] = random()%4|1;
			dw[i] = random()%4|1;
			dh[i] = random()%4|1;
		}

		for(;;)
		{
			circlein((Rcel *)backdrop);
			spray_dot(smallw[2]);
			for (i=0; i<XX; i++)
			{
				cw = smallw[i];
				if (pj_key_is())
					{
					pj_key_in();
					goto exitit;
					}

				crect_torect((Cliprect *)(&cw->CRECTSTART),&pos[i]);
				pos[i].x += dx[i];
				pos[i].y += dy[i];
				if((SHORT)(pos[i].width += dw[i]) < 0)
					pos[i].width = 0;
				if((SHORT)(pos[i].height += dh[i]) < 0)
					pos[i].height = 0;

				reposit_wndo(cw,&pos[i],NULL);
				if((cw->width < pos[i].width && dw[i] > 0)
					|| (pos[i].width <= WNDO_MINWIDTH && dw[i] < 0))
					dw[i] = -dw[i];
				if((cw->height < pos[i].height && dh[i] > 0)
					|| (pos[i].height <= WNDO_MINHEIGHT && dh[i] < 0))
					dh[i] = -dh[i];

				if( ((cw->W_xmax <= vl.screen->wndo.x - 5) && dx[i] < 0)
					|| ((cw->x >= vl.screen->wndo.W_xmax + 5) && dx[i] > 0))
					dx[i] = -dx[i];
				if( ((cw->W_ymax <= vl.screen->wndo.y - 5) && dy[i] < 0)
					|| ((cw->y >= vl.screen->wndo.W_ymax + 5) && dy[i] > 0))
					dy[i] = -dy[i];
			}
		}
	goto exitit;

error:
	boxf("error %d", err);
exitit:
	close_wndo(wndo0);
	close_wndo(wndo1);
	close_wndo(wndo3);
	cleanup_all();
	exit(-1);
}

