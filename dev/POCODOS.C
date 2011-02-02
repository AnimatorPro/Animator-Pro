
#include "errcodes.h"
#include "ptrmacro.h"
#include "memory.h"
#include "jfile.h"
#include "pocoface.h"
#include "pocolib.h"
#include "menus.h"
#include "inkaid.h"
#include "resource.h"

Popot po_malloc(int size);
void po_free(Popot ppt);

extern Poco_lib po_dos_lib;

static int po_dir_list(Popot ppdest, Popot ppat, int get_dirs)
/*****************************************************************************
 * int DirList(char ***list, char *wild, Boolean get dirs);
 ****************************************************************************/
{
Popot *pdest;
char *pat;
Names *wld, *w;
int name_count;
long mem_size;
Popot result;
int slen;
char *spt;
Popot *ppp;

if ((pdest = ppdest.pt) == NULL || (pat = ppat.pt) == NULL)
	return(builtin_err = Err_null_ref);
result.min = result.max = result.pt = NULL;
build_wild_list(&wld,pat,get_dirs);
mem_size = 0;
name_count = 0;
w = wld;
while (w != NULL)
	{
	mem_size += sizeof(Popot)+strlen(w->name)+1;
	name_count += 1;
	w = w->next;
	}
if (name_count == 0)
	goto OUT;
result = po_malloc(mem_size);
if (result.pt == NULL)
	{
	name_count = Err_no_memory;
	goto OUT;
	}
ppp = result.pt;
spt = OPTR(result.pt,name_count*sizeof(Popot));
/* set pointer bounds to just Popot array, not string space */
result.max = OPTR(spt,-1);
w = wld;
while (w != NULL)
	{
	slen = strlen(w->name);
	ppp->min = ppp->max = ppp->pt = spt;
	ppp->max = OPTR(ppp->max,slen);
	ppp += 1;
	slen += 1;
	pj_copy_bytes(w->name, spt, slen);
	spt += slen;
	w = w->next;
	}
OUT:
free_wild_list(&wld);
*pdest = result;
return(name_count);
}

static void po_free_dir_list(Popot plist)
/*****************************************************************************
 * void FreeDirList(char ***list);
 ****************************************************************************/
{
Popot *list;

if ((list = plist.pt) == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
if (list->pt != NULL)
	{
	po_free(*list);
	list->pt = list->min = list->max = NULL;
	}
}

static Errcode po_get_dir(Popot dir)
/*****************************************************************************
 * ErrCode GetDir(char *dir);
 ****************************************************************************/
{
Errcode err;

if ((err = Popot_bufcheck(&dir, PATH_SIZE)) >= Success)
	err = get_dir(dir.pt);
return(err);
}

static void po_get_resource_dir(Popot dir)
/*****************************************************************************
 * void GetResourceDir(char *dir);
 ****************************************************************************/
{
if ((Popot_bufcheck(&dir,PATH_SIZE)) >= Success)
	strcpy(dir.pt, resource_dir);
}

static Errcode po_set_dir(Popot dir)
/*****************************************************************************
 * ErrCode SetDir(char *dir);
 ****************************************************************************/
{
if (dir.pt == NULL)
	return( builtin_err = Err_null_ref );
return(change_dir(dir.pt));
}

static Errcode po_delete(Popot name)
/*****************************************************************************
 * ErrCode DosDelete(char *filename);
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(pj_delete(name.pt));
}

static Errcode po_rename(Popot old, Popot new)
/*****************************************************************************
 * ErrCode DosRename(char *old, char *new);
 ****************************************************************************/
{
if (old.pt == NULL || new.pt == NULL)
	return(builtin_err = Err_null_ref);
return(pj_rename(old.pt,new.pt));
}

static Errcode po_dos_copy(Popot source, Popot dest)
/*****************************************************************************
 * ErrCode DosCopy(char *source, char *dest);
 ****************************************************************************/
{
char *errfile;

if (source.pt == NULL || dest.pt == NULL)
	return(builtin_err = Err_null_ref);
return(pj_cpfile(source.pt, dest.pt, &errfile));
}

static Boolean po_exists(Popot name)
/*****************************************************************************
 * Boolean DosExists(char *filename);
 ****************************************************************************/
{
if (name.pt == NULL)
	return(builtin_err = Err_null_ref);
return(pj_exists(name.pt));
}


static Errcode po_fnsplit(
		Popot path, Popot device, Popot dir, Popot file, Popot suffix)
/*****************************************************************************
 * ErrCode fnsplit(char *path, char *device, char *dir, char *file, char *suf);
 ****************************************************************************/
{
Errcode err;

if (path.pt == NULL)
	return(builtin_err = Err_null_ref);
if ((err = Popot_bufcheck(&device, 3)) < Success)
	return(err);
if ((err = Popot_bufcheck(&dir, 66)) < Success)
	return(err);
if ((err = Popot_bufcheck(&file, 9)) < Success)
	return(err);
if ((err = Popot_bufcheck(&suffix, 5)) < Success)
	return(err);
return(fnsplit(path.pt,device.pt,dir.pt,file.pt,suffix.pt));
}

static Errcode po_fnmerge(
	Popot path, Popot device, Popot dir, Popot file, Popot suffix)
/*****************************************************************************
 * ErrCode fnmerge(char *path, char *device, char *dir, char *file, char *suf);
 ****************************************************************************/
{
Errcode err;

if ((err = Popot_bufcheck(&path, PATH_SIZE)) < Success)
	return(err);
if (device.pt == NULL || dir.pt == NULL || file.pt == NULL ||
	suffix.pt == NULL)
	return(builtin_err = Err_null_ref);
return(fnmerge(path.pt,device.pt,dir.pt,file.pt,suffix.pt));
}

static po_get_program_dir(Popot dir)
/*****************************************************************************
 * void GetProgramDir(char *dir);
 ****************************************************************************/
{
extern char po_current_program_path[];	/* defined in qpoco.c */

if ((Popot_bufcheck(&dir,PATH_SIZE)) >= Success)
	strcpy(dir.pt, po_current_program_path);
}

/*----------------------------------------------------------------------------
 * library protos...
 *
 * Maintenance notes:
 *	To add a function to this library, add the pointer and prototype string
 *	TO THE END of the following list.  Then go to POCOLIB.H and add a
 *	corresponding prototype to the library structure typedef therein which
 *	matches the name of the structure below.  When creating the prototype
 *	in POCOLIB.H, remember that all arguments prototyped below as pointers
 *	must be defined as Popot types in the prototype in pocolib.h; any number
 *	of stars in the Poco proto still equate to a Popot with no stars in the
 *	pocolib.h prototype.
 *
 *	DO NOT ADD NEW PROTOTYPES OTHER THAN AT THE END OF AN EXISTING STRUCTURE!
 *	DO NOT DELETE A PROTOTYPE EVER! (The best you could do would be to point
 *	the function to a no-op service routine).  Breaking these rules will
 *	require the recompilation of every POE module in the world.  These
 *	rules apply only to library functions which are visible to POE modules
 *	(ie, most of them).  If a specific typedef name exists in pocolib.h, the
 *	rules apply.  If the protos are coded as a generic array of Lib_proto
 *	structs without an explicit typedef in pocolib.h, the rules do not apply.
 *--------------------------------------------------------------------------*/

PolibDos po_libdos = {
po_fnsplit,
	"ErrCode fnsplit(char *path, char *device, char *dir, char *file, char *suf);",
po_fnmerge,
	"ErrCode fnmerge(char *path, char *device, char *dir, char *file, char *suf);",
po_exists,
	"Boolean DosExists(char *filename);",
po_dos_copy,
	"ErrCode DosCopy(char *source, char *dest);",
po_delete,
	"ErrCode DosDelete(char *filename);",
po_rename,
	"ErrCode DosRename(char *old, char *new);",
po_set_dir,
	"ErrCode SetDir(char *dir);",
po_get_dir,
	"ErrCode GetDir(char *dir);",
po_dir_list,
	"int     DirList(char ***list, char *wild, Boolean get_dirs);",
po_free_dir_list,
	"void    FreeDirList(char ***list);",
po_get_resource_dir,
	"void    GetResourceDir(char *dir);",
po_get_program_dir,
	"void    GetProgramDir(char *dir);",
};


Poco_lib po_dos_lib = {
	NULL, "DOS",
	(Lib_proto *)&po_libdos, POLIB_DOS_SIZE,
	};


