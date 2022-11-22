#include "rastgfx.ih"

void do_leftbehind(Coor sx,Coor sy,
				   Coor dx,Coor dy,Coor width,Coor height,
				   do_leftbehind_func func, void *dat)
/* processes area left behind if rect moved from sx,sy to dx,dy */
{
Coor xdif, ydif;

	xdif = sx - dx;
	ydif = sy - dy;

	if((Absval(ydif) >= height) || (Absval(xdif) >= width))
	{
		(*func)(sx,sy,width,height,dat);
		return;
	}
	if(ydif > 0)
	{
		(*func)(sx,dy + height,width,ydif,dat);
		height -= ydif;
	}
	else if(ydif < 0)
	{
		(*func)(sx,sy,width,-ydif,dat);
		sy -= ydif;
		height += ydif;
	}
	if(xdif > 0)
	{
		(*func)(dx + width,sy,xdif,height,dat);
	}
	else if(xdif < 0)
	{
		(*func)(sx,sy,-xdif,height,dat);
	}
}
