#include <stdio.h>
#include "jimk.h"
#include "rastext.h"
#include "errcodes.h"
#include "vsetfile.h"

static Vfont _uvfont;
Vfont *uvfont = &_uvfont;

extern char sixhi_font_name[];
static char *system_font_name = sixhi_font_name;

Vfont *get_poco_font()
/* Default to Aegis Animator style font */
{
static Vfont sfont;
	init_sail_vfont(&sfont);
	return(&sfont);
}
void release_uvfont(void)
/* Release memory associated with user font.  */
{
	fget_spacing(uvfont, &vs.font_spacing, &vs.font_leading);
	close_vfont(uvfont);
}

void get_uvfont_name(char *buf)
{
	vset_get_path(FONT_PATH,buf);
}

Errcode load_the_font(char *path)
/* returns error code and will reset name in settings file to match
 * current font. If the path is NULL it will load the one in the settings
 * file path */
{
Errcode err;
Vset_path pinfo;

	vset_get_pathinfo(FONT_PATH,&pinfo);
	if(path == NULL)
		path = pinfo.path;

	release_uvfont();
	if(pj_name_in_path(path,system_font_name))
	{
		err = Success;
	}
	else if((err = load_font(path, uvfont
	, vs.font_height, vs.font_unzag)) >= Success)
	{
		if(txtcmp(path,pinfo.path)==0)
			goto done;
		strcpy(pinfo.path,path);
		goto newpath;
	}
		/* this is always successful */
	load_font(system_font_name, uvfont, vs.font_height, vs.font_unzag); 
	if(pj_name_in_path(pinfo.path,system_font_name))
		goto done;
	pj_set_path_name(pinfo.path,system_font_name);
newpath:
	vset_set_pathinfo(FONT_PATH,&pinfo);
done:
	return(err);
}
void grab_uvfont(void)
/* Load up font our state variable says we're using */
{
	load_the_font(NULL);
	fset_spacing(uvfont, vs.font_spacing, vs.font_leading);
}

#ifdef SLUFFED
void systext(void *screen,char *s,int x,int y,
					int color,Text_mode tmode,int bcolor)
{
	gftext(screen,get_sys_font(),s,x,y,color,tmode,bcolor);
}
#endif /* SLUFFED */

