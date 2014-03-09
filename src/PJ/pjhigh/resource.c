#include <stdio.h>
#include "commonst.h"
#include "errcodes.h"
#include "fontdev.h"
#include "jfile.h"
#include "pjbasics.h"
#include "reqlib.h"
#include "resource.h"
#include "softmenu.h"

char resource_dir[PATH_SIZE];	/* resource directory */
Softmenu smu_sm;		/* softmenu handle */


Errcode no_resource(Errcode err)
{
static char noresource[] =
"Can't find aa.mu in the resource subdirectory.\n"
"Resource must be a subdirectory of the directory\n"
"containing this program.\n"
"\n"
"Ne trouve pas aa.mu dans le sous-repertoire de resource.\n"
"La resource doit etre dans un sous-repertoire du repertoire\n"
"contenant cette program.\n"
"\n"
"Kann aa.mu im Resourcenunterverzeichnis nicht finden.\n"
"Die Resource muss sich in einem Unterverzeichnis desjenigen\n"
"Verzeichnisses befinden, welches ani.exe enthaelt.\n";

static char nomemory[] =
"Not enough free extended memory to run program.  Some ways to increase\n"
"the extended memory available are:\n"
"	Remove RAM disks (VDISK and RAMDRIVE) from config.sys.\n"
"	Reduce size of RAM disks (see DOS or Windows manuals).\n"
"	Remove EMM emulators from config.sys.\n"
"	Reduce size of disk caches such as SMARTDRV.SYS.\n"
"	Install more RAM.\n";

if (err == Err_no_memory)
	continu_box("%s", nomemory);
else if (smu_sm.err_line > 0)
	continu_box("%s\ncode %d line %d\n",noresource,-err,smu_sm.err_line);
else
	continu_box("%s\n(error code %d)\n",noresource,-err);
return(Err_reported);
}


Errcode init_resource_path(char *path)
{
Errcode err;

	if((err = get_full_path(path, resource_dir)) < Success)
	{
		sprintf(resource_dir,"%.*s", PATH_SIZE, path);
		goto error;
	}
	else
	{
		remove_path_name(resource_dir);
		if((err = add_subpath(resource_dir, "resource\\", resource_dir)) < 0)
			goto error;
	}
	return(Success);
error:
	return(no_resource(err));
}
char *make_resource_path(char *dir, char *name, char *path_buf)
{
	add_subpath(resource_dir, dir, path_buf);
	add_subpath(path_buf, name, path_buf);
	return(path_buf);
}
char *make_resource_name(char *name, char *path_buf)
{
	make_file_path(resource_dir, name, path_buf);
	return(path_buf);
}
void cleanup_menu_resource(void)
{
	cleanup_common_str();
	smu_cleanup(&smu_sm);
}
Errcode init_menu_resource(char *menu_file)

/* opens menu resource file and installs it in the global file handle 
 * the menu_file name opened will be in the resource directory */
{
Errcode err;
char msg_fname[PATH_SIZE];

	default_common_str();
	if ((err = smu_init(&smu_sm, make_resource_name(menu_file, msg_fname)))
		< Success)
	{
		return(no_resource(err));
	}
	if ((err = init_common_str(&smu_sm)) < Success)
		return(err);
	err = init_menufont_dev();
	return(err);
}
