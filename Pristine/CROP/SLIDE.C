
/* slide.c - stuff to implement the crop/slide selection.  Lets you
   make a FLI out of a scrolling background among other things. */

#include "jimk.h"
#include "crop.h"
#include "slide.str"

static int dx, dy, dframes = 50;
int complete;


qslide()
{
char *bufs[8];
char b1[30],b2[30],b3[30],b4[30];
int ouzx,ouzy;	/* bad kludge to keep menus from moving... */
int choice;
int ocx, ocy;

ouzx = uzx;
ouzy = uzy;
for (;;)
	{
	uzx = ouzx;
	uzy = ouzy;

	bufs[0] =   slide_100 /* "Slide with Mouse" */;
	sprintf(b1, slide_101 /* "Set X        %3d" */, dx);
	bufs[1] = b1;
	sprintf(b2, slide_102 /* "Set Y        %3d" */, dy);
	bufs[2] = b2;
	sprintf(b3, slide_103 /* "Set Frames   %3d" */, dframes);
	bufs[3] = b3;
	sprintf(b4, slide_104 /* "%s Complete" */, complete ? "*" : " ");
	bufs[4] = b4;
	bufs[5] = slide_107 /* "Preview" */;
	bufs[6] = slide_108 /* "Render and Save" */;
	bufs[7] = slide_109 /* "Exit Menu" */;
	if ((choice = qchoice(slide_110 /* "Slide..." */, 
		bufs, Array_els(bufs))) == 0)
		break;
	switch (choice)
		{
		case 1:
			ocx = pic_cel->x;
			ocy = pic_cel->y;
			qmove();
			dx = pic_cel->x - ocx;
			dy = pic_cel->y - ocy;
			pic_cel->x = ocx;
			pic_cel->y = ocy;
			tile_s_cel(pic_cel);
			break;
		case 2:
			qreq_number(slide_111 /* "Set x slide" */, 
				&dx, -pic_cel->w, pic_cel->w);
			break;
		case 3:
			qreq_number(slide_112 /* "Set y slide" */, 
				&dy, -pic_cel->h, pic_cel->h);
			break;
		case 4:
			qreq_number(slide_113 /* "Set frame count" */, 
				&dframes, 1, 100);
			if (dframes < 1)
				dframes = 1;
			break;
		case 5:
			complete = !complete;
			break;
		case 6:
			preview_slide();
			break;
		case 7:
			render_slide();
			break;
		}
	}
}


calc_time_scale(ix, intween)
int ix, intween;
{
int time_scale;
int time_frames;
int time_ix;

if (intween <= 1)
	return(SCALE_ONE);
time_ix = ix;
time_frames = intween - complete;
time_scale = rscale_by(SCALE_ONE, time_ix, time_frames);
return(time_scale);
}


preview_slide()
{
int ox, oy;
int scale;
int i;

ox = pic_cel->x;
oy = pic_cel->y;
hide_mouse();
for (i=0; i<dframes; i++)
	{
	scale = calc_time_scale(i, dframes);
	pic_cel->x = ox + itmult(dx, scale);
	pic_cel->y = oy + itmult(dy, scale);
	tile_s_cel(pic_cel);
	c_input();
	if (key_hit || RJSTDN)
		break;
	wait_sync();
	}
pic_cel->x = ox;
pic_cel->y = oy;
tile_s_cel(pic_cel);
show_mouse();
}

static int cframe;
static int ox, oy;

start_slide()
{
cframe = 0;
return(next_slide());
}

next_slide()
{
int scale;

scale = calc_time_scale(cframe, dframes);
pic_cel->x = ox + itmult(dx, scale);
pic_cel->y = oy + itmult(dy, scale);
tile_s_cel(pic_cel);
cframe++;
return(1);
}


render_slide()
{
char *title;

ox = pic_cel->x;
oy = pic_cel->y;
if ((title = get_filename(slide_114 /* "Save sliding background FLIC?" */, 
	".FLI", 1)) == NULL)
	return;
if (!s_fli(title, start_slide, next_slide, dframes, 4))
	jdelete(title);
pic_cel->x = ox;
pic_cel->y = oy;
tile_s_cel(pic_cel);
}


