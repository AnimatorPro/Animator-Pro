#include "rastgfx.ih"

struct setitdat {
	Raster *r;
	Pixel c;
};
static void setit(Coor x,Coor y, Coor w, Coor h, void *data)
{
	struct setitdat *sd = data;
	pj_set_rect(sd->r,sd->c,x,y,w,h);
}
void set_leftbehind(Raster *s,Pixel color,Coor sx,Coor sy,
				    Coor dx,Coor dy,Coor width,Coor height)

/* sets area left behind if args were given to a blitmove_rect() */
{
struct setitdat sd;

	sd.r = s;
	sd.c = color;
	do_leftbehind(sx,sy,dx,dy,width,height,setit,&sd);
}
