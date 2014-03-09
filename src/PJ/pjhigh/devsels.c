/* devsels.c stuff to handle allocated arrays of "device" buttons for use on 
 * file menus */

#include <ctype.h>
#include <string.h>
#include "jimk.h"
#include "errcodes.h"
#include "jfile.h"
#include "libdummy.h"
#include "memory.h"
#include "menus.h"
#include "msfile.h"
#include "pjbasics.h"
#include "ptrmacro.h"

typedef struct dsel_group {
	SHORT devnum;
	char *drawer;
    Errcode (*on_newdrawer)(void *data);
    void *on_newd_data;
	char curdev[DEV_NAME_LEN];
} Dsel_group;

/* A list of disk drives that look like they're really on this machine */


static void go_updir(Button *b)
/* move up one directory */
{
Dsel_group *dg = b->group;
int len;
char *d;

	hilight(b);
	d = dg->drawer;
	len = strlen(d);
	if (len >= 2) /* move 'd' pointer past device if any */
	{
		if (d[1] == DEV_DELIM)
		{
			d += 2;
			len -= 2;
		}
	}
	if(len > 1 && d[len-1] == DIR_DELIM) /* truncate trailing '\\' */
		d[--len] = 0;

	while(--len >= 0)
	{
		if(d[len] == DIR_DELIM)
		{
			d[len+1] = 0;
			break;
		}
	}
	(*dg->on_newdrawer)(dg->on_newd_data);
	draw_buttontop(b);
}
static void go_rootdir(Button *b)
{
Dsel_group *dg = b->group;
char *drawer = dg->drawer;

	hilight(b);
	if (drawer[1] == DEV_DELIM)
		drawer += 2;
	strcpy(drawer, DIR_DELIM_STR);

	(*dg->on_newdrawer)(dg->on_newd_data);
	draw_buttontop(b);
}
static void check_devicewait(char *device)
{
	if(!pj_is_fixed(device))
		soft_put_wait_box("!%s", "wait_fdread", device );
}
static void set_device_group(Dsel_group *dg)
{
	current_device(dg->curdev);
	dg->devnum = toupper(dg->curdev[0])-'A'; /* this only good for ms-dos */
}
static void new_dev(Button *b)
{
Dsel_group *dg = b->group;
char devname[2];
Errcode err;

	switch(b->identity)
	{
		case -2:
			go_updir(b);
			return;
		case -1:
			go_rootdir(b);
			return;
		default:
			devname[0] = 'A' + b->identity;
			devname[1] = 0;

			check_devicewait(devname);
			if((err = pj_change_device(devname)) < Success)
				goto cd_error;
			set_device_group(b->group);
			mb_unhi_group(b);
			err = make_good_dir(dg->drawer);
			mb_hi_group(b);
		cd_error:
			errline(err,"%s:", devname);
			(*dg->on_newdrawer)(dg->on_newd_data);
			return;
	}
}
static void hang_dev_sels(Button *b)
{
	if(b->children)
		set_device_group(b->children->group);
	hang_children(b);
}
static void see_device(Button *b)
{
char buf[2];

	switch(b->identity)
	{
		case -2:
			b->datme = "..";
			b->key_equiv = '.';
			break;
		case -1:
			b->datme = DIR_DELIM_STR;
			b->key_equiv = DIR_DELIM;
			break;
		default:
			buf[0] = b->identity+'A';
			buf[1] = 0;
			b->datme = buf;
			break;
	}
	ccorner_text(b);
}
Errcode alloc_dev_sels(Button *hanger,  /* where to install device buttons */
					   Rectangle *size, /* width and height is size 
					   					 * x,y spacing UNSCALED referenced
										 * to 320 X 200 */
					   int numcols,int numrows,
					   char *drawer,  /* string to put directory into */
					   /* function and data to call after drawer is changed */
					   Errcode (*on_newdrawer)(void *), void *ond_data )
/*
 * Allocate and initialized one button on a file menu for each logical drive.
 * Store the button list on hanger->children.  Also allocates ".." and "\"
 * buttons at the start.
 *
 * The buttons will all have the same seeme & feelme.  The drive # will
 * be stored in the identity field, or -1 or -2 for the ".." and "\".
 * The seeme & feelme do a little switch on the identity to decide
 * how to process it if a drive or not.
 */
{
Errcode err;
UBYTE devices[MAX_DEVICES];
int dev_count;
Dsel_group *dg;
long bsize;
Button *sel;
int i,ix;

	if((dev_count = pj_get_devices(devices)) < 0)
		return(dev_count); 

	bsize = (dev_count+2)*sizeof(Button);

		/* Allocate enough space for all buttons and a Dsel_group. */
	if((err = ealloc(&(hanger->children),bsize + sizeof(Dsel_group)))<Success)
		return(err);

	sel = hanger->children;
	dg = (Dsel_group *)OPTR(sel,bsize);	/* Point dg to after buttons */
	dg->drawer = drawer;
	dg->on_newd_data = ond_data;

	if (on_newdrawer) {
		dg->on_newdrawer = on_newdrawer;
	}
	else { /* just in case it's not supplied */
		dg->on_newdrawer = (Errcode (*)(void *))pj_errdo_unimpl;
	}

	hanger->seeme = hang_dev_sels;

	ix = 0; 
	--sel;
	for(i = -2; i < dev_count;++i)
	{
		++sel;
		clear_struct(sel);
		sel->next = sel+1;
		sel->orig_rect.width = size->width;
		sel->orig_rect.height = size->height;
		sel->orig_rect.x = size->x * (ix%numcols);
		sel->orig_rect.y = size->y * (ix/numcols);
		sel->seeme = see_device;
		sel->feelme = new_dev;
		sel->group = dg;

		if(i >= 0)
		{
			sel->identity = devices[i];
			sel->key_equiv = devices[i]+'a';
			sel->flags = MB_GHILITE;
		}
		else
			sel->identity = i;

		if(++ix >= (numcols*numrows))
			break;
	}
	sel->next = NULL;
	return(Success);
}
static void no_see(Button *b)
{
	(void)b;
}
void cleanup_dev_sels(Button *hanger)
{
	pj_freez(&hanger->children);
	hanger->seeme = no_see;
}
