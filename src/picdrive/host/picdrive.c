/* file to take care of loadable image file types */

#include <stdio.h>
#include "animinfo.h"
#include "errcodes.h"
#include "picdrive.h"
#include "linklist.h"
#include "memory.h"
#include "filepath.h"


Local_pdr *local_pdrs;


/*****************************************************************************
 *
 ****************************************************************************/
static void remove_local_pdr(Local_pdr *lpd)
{
	local_pdrs = remove_el((Slnode *)local_pdrs, (Slnode *)lpd);
}


/*****************************************************************************
 *
 ****************************************************************************/
void add_local_pdr(Local_pdr *lpd)
{
	lpd->header->hdr.version = ~(PDR_VERSION);
	remove_local_pdr(lpd);
	lpd->next = local_pdrs;
	local_pdrs = lpd;
}


/*****************************************************************************
 *
 ****************************************************************************/
void free_pdr(Pdr **ppdr)
{
	Pdr *pd = *ppdr;
	if(pd != NULL)
	{
		if (pd->hdr.version == (USHORT)~(PDR_VERSION)) {
			*ppdr = NULL;
		}
	}
}


/*****************************************************************************
 *
 ****************************************************************************/
Errcode load_pdr(char *path, Pdr **ppdr)
{
	Local_pdr *lpd;
	Errcode err;
	char *name;

	name = pj_get_path_name(path);

	if((lpd = (Local_pdr *)(name_in_list(name,
								(Names *)local_pdrs))) != NULL)
	{
		*ppdr = lpd->header;
		return(Success);
	}
	return(Err_not_found);
}


/*****************************************************************************
 *
 ****************************************************************************/
int pdr_get_title(Pdr *pd, char *buf, int maxlen)
{
	return sprintf(buf,"%.*s",maxlen - 1, pd->title_info);
}


/*****************************************************************************
 *
 ****************************************************************************/
char *pdr_alloc_info(Pdr *pd)
{
	return pd->long_info;
}


void pdr_free_info(char *info)
/*****************************************************************************
 *
 ****************************************************************************/
{
	(void)info;
}


/*****************************************************************************
 *
 ****************************************************************************/
int pdr_get_suffi(Pdr *pd, char *buf)
{
	return sprintf(buf, "%.*s",
			(int)sizeof(pd->default_suffi)-1, pd->default_suffi);
}


/*****************************************************************************
 *
 ****************************************************************************/
bool pdr_best_fit(Pdr *pd, Anim_info *spec)
{
	if(pd->spec_best_fit == NULL)
	{
		clear_struct(spec);
		return(0);
	}
	return (*pd->spec_best_fit)(spec);
}


/*****************************************************************************
 *
 ****************************************************************************/
Errcode pdr_create_ifile(struct pdr *pd, char *path, Image_file **pifile,
								 Anim_info *spec )
{
	Errcode err;

	*pifile = NULL;
	if(pd->create_image_file == NULL)
		return(Err_unimpl);

	err = (*(pd->create_image_file))(pd, path, pifile, spec);
	if(err >= Success)
	{
		(*pifile)->pd = pd;
		(*pifile)->write_mode = true;
	}
	return err;
}


/*****************************************************************************
 *
 ****************************************************************************/
Errcode pdr_open_ifile(struct pdr *pd, char *path, Image_file **pifile,
							   Anim_info *ainfo )
{
	Errcode err;

	*pifile = NULL;
	if(pd->open_image_file == NULL) {
		return Err_unimpl;
	}

	err = (*(pd->open_image_file))(pd, path, pifile, ainfo);
	if(err >= Success)
	{
		(*pifile)->pd = pd;
		(*pifile)->write_mode = false;

		if(ainfo != NULL && (ainfo->width == 0 || ainfo->height == 0))
		{
			err  = Err_corrupted;
			pdr_close_ifile(pifile);
		}
	}
	return err;
}


/*****************************************************************************
 *
 ****************************************************************************/
void pdr_close_ifile(Image_file **pifile)
{
	Image_file *ifile;

	if((ifile = *pifile) != NULL) {
		(*ifile->pd->close_image_file)(pifile);
	}
}


/*****************************************************************************
 *
 ****************************************************************************/
Errcode pdr_read_first(Image_file *ifile, Rcel *screen)
{
	if (ifile->pd->read_first_frame == NULL) {
		return Err_unimpl;
	}

	return (*(ifile->pd->read_first_frame))(ifile, screen);
}


/*****************************************************************************
 *
 ****************************************************************************/
Errcode pdr_read_next(Image_file *ifile,Rcel *screen)
{
	if (ifile->pd->read_delta_next == NULL) {
		return Err_unimpl;
	}

	return (*(ifile->pd->read_delta_next))(ifile, screen);
}


/*****************************************************************************
 *
 ****************************************************************************/
Errcode pdr_save_frames(Image_file *ifile,
						Rcel *screen,
						ULONG num_frames,
						Errcode (*seek_frame)(int ix,void *seek_data),
						void *seek_data,
						Rcel *work_screen )
{
	Pdr *pd = ifile->pd;

	if(pd->max_write_frames < num_frames)
		return(Err_too_many_frames);
	if (pd->save_frames == NULL)
		return Err_unimpl;

	return (*(pd->save_frames))(ifile,screen,num_frames,seek_frame,
								seek_data, work_screen );
}

