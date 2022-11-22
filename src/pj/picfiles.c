/* file to take care of loadable still frame file types */

#include <stdio.h>
#include <string.h>
#include "jimk.h"
#include "aaconfig.h"
#include "animinfo.h"
#include "errcodes.h"
#include "fli.h"
#include "jfile.h"
#include "memory.h"
#include "menus.h"
#include "picdrive.h"
#include "picfile.h"
#include "resource.h"
#include "rexlib.h"
#include "softmenu.h"
#include "util.h"

/* there are two of these one for "picture" files and one for "fli" files
 * so we can have 2 yes 2 codes in one */

typedef struct config_pdr_info {
	void *next;
	char *local_name;
	char *save_pdr;
	char *default_pdr;
	char save_suffi[PDR_SUFFI_SIZE];
	UBYTE suffi_loaded;
	Errcode (*local_get_ainfo)(char *ifname,Anim_info *ainfo);
	char last_read[FILE_NAME_SIZE]; /* type of last successful read */
	UBYTE type;
} Config_pdr;

static Errcode select_save_pdr(int type);

#define PICTYPE 0
#define FLICTYPE 1

static Config_pdr pdrconf[2] = {
	{
		NULL,
		&pic_pdr_name[0],
		&vconfg.picsave_pdr[0],
		&gif_pdr_name[0],
		"",
		FALSE,
		pic_anim_info,
		"",
		PICTYPE,
	},
	{
		NULL,
		&fli_pdr_name[0],
		&vconfg.flisave_pdr[0],
		&fli_pdr_name[0],
		"",
		FALSE,
		pj_fli_info,
		"",
		FLICTYPE,
	},
};


static Boolean is_local_pdr(char *path,int type)
{
	return(!txtcmp(pj_get_path_name(path),pdrconf[type].local_name));
}
static Boolean is_pic_pdr_name(char *path)
{
	return(is_local_pdr(path,PICTYPE));
}
Boolean is_fli_pdr_name(char *path)
{
	return(is_local_pdr(path,FLICTYPE));
}
static char *get_save_pdr(char *pathbuf,int type)
{
Config_pdr *cpdr = &pdrconf[type];

	if(is_local_pdr(cpdr->save_pdr,type))
	{
		strcpy(pathbuf,cpdr->local_name);
	}
	else
	{
		if(cpdr->save_pdr[0] == 0)
			strcpy(cpdr->save_pdr,cpdr->default_pdr);
		make_resource_name(cpdr->save_pdr,pathbuf);
	}
	return(pathbuf);
}
char *get_flisave_pdr(char *pdr_path)
{
	return(get_save_pdr(pdr_path,FLICTYPE));
}
static char *get_picsave_pdr(char *pdr_path)
{
	return(get_save_pdr(pdr_path,PICTYPE));
}
static Errcode cur_pdrtype_info(char *sufbuf, char *titlebuf, int titlesize, 
							    Boolean flitype, 
								int rwmode) /* 0 = dontcare 
											   1 = must_write 
											   2 = must_read */

/* gets info for current user selection of picture io module type */
{
Errcode err;
char pdr_name[PATH_SIZE];
Config_pdr *cpdr;
Pdr *pd;

	if(flitype)
		flitype = 1;

	cpdr = &pdrconf[flitype];

	get_save_pdr(pdr_name,flitype);

	if((err = load_pdr(pdr_name, &pd)) < Success)
	{
		err = cant_query_driver(err,pdr_name);
		sufbuf[0] = 0;
		if(titlebuf)
			titlebuf[0] = 0;
	}
	else
	{
		if(rwmode == 1)
		{
			if(pd->max_write_frames < 1)
			{
				err = softerr(Err_no_message,"!%s", "readonly_pdr", pdr_name );
				goto error;
			}
		}
		pdr_get_suffi(pd, sufbuf);
		if(titlebuf)
			pdr_get_title(pd,titlebuf,titlesize);
	}

	strcpy(cpdr->save_suffi,sufbuf);
	cpdr->suffi_loaded = TRUE;
error:
	free_pdr(&pd);
	return(err);
}
static Errcode get_pdrsave_info(char *sufbuf, char *titlebuf, int titlesize,
								int type)
/* to be called before user request to save images to verify pdr is writable
 * and to get header info for prompt menu */
{
Errcode err;

	/* if get info fails or module is read only then query user to pick 
	 * another module, keep doing until abort, module success, or error 
	 * cur_pdrtype_info reports errors */

	while((err = cur_pdrtype_info(sufbuf, titlebuf, 
								  titlesize, type, 1)) < Success)
	{
		if((err = select_save_pdr(type)) < Success)
			break;
	}
	return(err);
}
Errcode get_picsave_info(char *sufbuf, char *titlebuf, int titlesize)
{
	return(get_pdrsave_info(sufbuf, titlebuf, titlesize, PICTYPE));
}
Errcode get_flisave_info(char *sufbuf, char *titlebuf, int titlesize)
{
	return(get_pdrsave_info(sufbuf, titlebuf, titlesize, FLICTYPE));
}
char *get_pictype_suffi(void)

/* we have a little static suffi area to avoid re-loading picture module
 * every time */
{
	if(!pdrconf[PICTYPE].suffi_loaded)
		cur_pdrtype_info(pdrconf[PICTYPE].save_suffi,NULL,0,PICTYPE,0);
	return(pdrconf[PICTYPE].save_suffi);
}
static char *get_flitype_suffi(void)

/* we have a little static suffi area to avoid re-loading picture module
 * every time */
{
	if(!pdrconf[FLICTYPE].suffi_loaded)
		cur_pdrtype_info(pdrconf[FLICTYPE].save_suffi,NULL,0,FLICTYPE,0);
	return(pdrconf[FLICTYPE].save_suffi);
}
static void reset_pdr_stuff(int type)
{
Config_pdr *cpdr = &pdrconf[type];
Vset_path vsp;
int ptype;
char *suffi;
char suffix[5];

	ptype = (type == PICTYPE)?PIC_PATH:FLI_PATH;

	if(vset_get_pathinfo(ptype,&vsp) >= Success)
	{
		suffi = cpdr->save_suffi;
		parse_to_semi(&suffi,suffix,sizeof(suffix));
		sprintf(vsp.wildcard,"*%.4s",suffix);
		vset_set_pathinfo(ptype,&vsp);
	}
	rewrite_config();
}
static Errcode select_save_pdr(int type)
{
Errcode err;
Config_pdr *cpdr;
Config_pdr *opdr;
char hdr[80];
char *hdr_key;

	cpdr = &pdrconf[type];

	if(type == PICTYPE)
	{
		opdr = &pdrconf[FLICTYPE];
		hdr_key =  "pdrpic_hdr";
	}
	else
	{
		opdr = &pdrconf[PICTYPE];
		hdr_key =  "pdrfli_hdr";
	}
	cpdr->next = opdr;
	opdr->next = NULL;

	err = go_pdr_menu(stack_string(hdr_key,hdr),	
					  cpdr->save_pdr,cpdr->save_suffi,
					  (Names *)cpdr,0,type);

	if(err >= Success)
		reset_pdr_stuff(type);
	return(err);
}
void go_pic_pdr_menu(void)
/* called when you right click over 'picture' on files menu */
{
	select_save_pdr(PICTYPE);
}
void go_flic_pdr_menu(void)
/* called when you right click over 'flic' on files menu */
{
	select_save_pdr(FLICTYPE);
}

static Errcode check_try_pdr(char *pdr_name,char *ifname, Anim_info *ainfo )
/* will open the pdr and attempt open of image file, and retrieve info,
 * close image file and free pdr if this is checked out you can read the
 * image */
{
Errcode err;
Pdr *pd;
Image_file *ifile;

	if((err = load_pdr(pdr_name, &pd)) >= Success)
	{
		if(pd->max_read_frames < 1)
			err = Err_unimpl;
		else
		{
			err = pdr_open_ifile(pd, ifname, &ifile, ainfo);
			pdr_close_ifile(&ifile);
		}
	}
	else
		err = cant_use_module(err,pdr_name);

	free_pdr(&pd);
	return(err);
}
Errcode find_pdr_loader(char *ifname, Boolean multi_frame, 
					  	Anim_info *ainfo, char *pdr_name, Rcel *screen) 

/* attempts to get info and open image file for any module currently in the
 * available resources.  If it finds a valid module that can read the image file
 * it closes the Image_file and the pdr and puts the name of the pdr in 
 * pdr_name. The screen is the screen tho match if loader is a resolution
 * independent loader */
{
Errcode err;
int type, check_type;
Anim_info screen_info;
Config_pdr *cpd;

	if(!pj_exists(ifname))
		return(Err_no_file);

	if(multi_frame)
		type = FLICTYPE;
	else
		type = PICTYPE;

	/* pre load anim info with resolutions to match in res independent 
	 * loader case */

	get_screen_ainfo(screen,&screen_info);
	cpd = &pdrconf[type];

	/* start with current type if not local type,
	 * then last successfully read type if not above or local type,
	 * then local types,
	 * then search the directory of modules */

	if(!is_local_pdr(get_save_pdr(pdr_name,type),type))
	{
		*ainfo = screen_info;
		if((err = check_try_pdr(pdr_name,ifname,ainfo)) >= Success)
			goto done; 
	}

	if( cpd->last_read[0] != 0 
		&& cpd->last_read[0] != LOCAL_PDR_CHAR
		&& txtcmp(cpd->last_read,pj_get_path_name(pdr_name)))
	{
		make_resource_name(cpd->last_read,pdr_name);
		*ainfo = screen_info;
		if((err = check_try_pdr(pdr_name,ifname,ainfo)) >= Success)
			goto out; 
	}

		/* Here check local PDR's.  Start with FLC if it's a multi-frame
		 * load, PIC if it's a single frame.  (But check the others too.)
		 */
	check_type = type;
	for(;;)
	{
		*ainfo = screen_info;
		if((err = (*(cpd->local_get_ainfo))(ifname,ainfo)) >= Success)
		{
			strcpy(pdr_name,(cpd->local_name));
			goto done;
		}
		check_type = !check_type;
		cpd = &pdrconf[check_type];
		if(check_type == type)
			break;
	}

	err = softerr(err, "!%s", "unknown_image", ifname);
	goto out;
done:
	strcpy(cpd->last_read,pj_get_path_name(pdr_name));
out:
	return(err);
}
Errcode load_any_picture(char *name,Rcel *screen)

/* attempts to load any of the current valid picture file types if pic is 
 * smaller will clear screen with vs.inks[0] */
{
Errcode err;
Anim_info ainfo;
char pdr_name[PATH_SIZE];

	if((err = find_pdr_loader(name,FALSE,&ainfo,pdr_name,screen)) < Success)
		goto error;

	if(ainfo.num_frames > 1
		&& !soft_yes_no_box("!%s%d", "load_first", name, ainfo.num_frames))
	{
		err = Err_abort;
		goto error;
	}

	if(ainfo.width < screen->width
		|| ainfo.height < screen->height)
	{
		pj_set_rast(screen,vs.inks[0]);
	}

	err = pdr_load_picture(pdr_name,name,screen);
	
	/* if big failure cancel last sucessful read type it will check local
	 * type first next time */

	if(err < Success && err != Err_abort)
		pdrconf[PICTYPE].last_read[0] = 0;

error:
	return(cant_load(err,name));
}
static char *get_fload_suffi(char *sufbuf, int for_cel )
/* what a kludge!! but this will do it right */
{
char *buf,*pdrsuf;
char suffi[PDR_SUFFI_SIZE];
int num_todo;
Boolean picdone = FALSE;
static char *suffs[] = { ".FL?;", ".CEL;" };

	if(!for_cel)
		pdrsuf = get_flitype_suffi();
	else
		pdrsuf = "";

	buf = sufbuf;
	*buf = 0;
	num_todo = 3;

	for(;;)
	{
		for(;;)
		{
			if(!num_todo)
				goto done;
			if(!parse_to_semi(&pdrsuf,suffi,10))
				break;
			if(!txtncmp(suffi,".FL", 3))
				continue;
			if(!txtcmp(suffi,".CEL"))
				continue;
			buf += sprintf(buf,"%s;",suffi);
			--num_todo;
		}
		if(picdone)
			break;
		buf += sprintf(buf,suffs[for_cel]);
		if(--num_todo <= 0)
			break;
		buf += sprintf(buf,suffs[!for_cel]);
		if(--num_todo <= 0)
			break;
		pdrsuf = get_pictype_suffi();
		picdone = TRUE;
	}
done:
	return(sufbuf);
}
char *get_fliload_suffi(char *sufbuf)
{
	return(get_fload_suffi(sufbuf,0));
}
char *get_celload_suffi(char *sufbuf)
{
	return(get_fload_suffi(sufbuf,1));
}
static Errcode pdr_check_save_abort(int ix, void *dat)
{
	(void)ix;

	if(poll_abort() < Success)
	{
		if(soft_yes_no_box("!%s", "save_abort",
					   pj_get_path_name((char *)dat)))
		{
			return(Err_abort);
		}
	}
	return(Success);
}
static Errcode save_picture_file(char *pdr_path,char *picname, Rcel *screen)
{
Errcode err;
Pdr *pd;
Image_file *ifile = NULL;
Anim_info spec;
Rcel virt;

	if(is_pic_pdr_name(pdr_path))
		return(save_pic(picname, screen,0,TRUE));
	if((err = load_pdr(pdr_path, &pd)) < Success)
		return(cant_use_module(err,pdr_path));

	get_screen_ainfo(screen,&spec);

	if(!pdr_best_fit(pd, &spec))
	{
		if(spec.depth < 8
			|| spec.width != screen->width
			|| spec.width != screen->height)
		{
			if(!soft_yes_no_box("!%d%d%d%d%d%d", "picsv_exact",
						   screen->width, screen->height,
						   0x0FF, 
						   spec.width, spec.height,
						   spec.depth < 8? 0x0FF>>(8 - spec.depth): 0x0FF))
			{
				err = Err_abort;
				goto done;
			}
		}
	}

	if((err = pdr_create_ifile(pd, picname, &ifile, &spec )) < Success)
		goto error;

	start_abort_atom();

	screen = center_virtual_rcel(screen, &virt, spec.width, spec.height);

	err = pdr_save_frames(ifile, screen, 1
	,	pdr_check_save_abort
	,	picname, NULL);

	if((err = errend_abort_atom(err)) < Success)
		goto error;
	goto done;

error:
	pdr_close_ifile(&ifile);
	pj_delete(picname);
	goto out;
done:
	pdr_close_ifile(&ifile);
out:
	free_pdr(&pd);
	return(err);
}
Errcode save_current_pictype(char *name, Rcel *screen)
/* save picture to current configured picture type */
{
char pdr_name[PATH_SIZE];

	get_picsave_pdr(pdr_name);
	return(save_picture_file(pdr_name, name, screen));
}
Errcode save_gif(char *name, Rcel *screen)
{
char pathbuf[PATH_SIZE];

	make_resource_name(gif_pdr_name,pathbuf);
	return(save_picture_file(pathbuf, name, screen));
}

