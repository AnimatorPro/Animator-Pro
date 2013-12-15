
#include "errcodes.h"
#include "filepath.h"
#include "resource.h"
#include "pocorex.h"

Errcode pj_load_pocorex(Poco_lib **lib, char *name, char *id_string)
/*****************************************************************************
 *
 ****************************************************************************/
{
Errcode err;
char path_buf[PATH_SIZE];
char *path = name;
Pocorex *exe;
static Libhead *libs_for_pocorex[] = {
	&aa_syslib,
	&aa_stdiolib,
	&aa_gfxlib,
	&aa_pocolib,
	&aa_mathlib,
	NULL
	};


	while((err = pj_rexlib_load(path, REX_POCO, (Rexlib **)&exe,
								libs_for_pocorex, id_string)) < Success)
	{
		switch (err)
		{
			case Err_no_file:
				if(path == name)
				{
					path = make_resource_name(name,path_buf);
					continue;
				}
			default:
				goto error;
		}
	}
	if(exe->hdr.version != POCOREX_VERSION)
	{
		err = Err_library_version;
		goto error;
	}
	if(exe->lib.lib == NULL || exe->lib.count == 0)
	{
		err =  Err_bad_input;
		goto error;
	}
	if (Success > (err = pj_rexlib_init((Rexlib *)exe, NULL)))
		goto error;

	exe->lib.rexhead = exe;
	*lib = &exe->lib;
	return(Success);
error:
	pj_rexlib_free((Rexlib **)&exe);
	return(err);
}


void pj_free_pocorexes(Poco_lib **libs)
/*****************************************************************************
 * Free a singly linked list of loaded poco REX libraries.
 ****************************************************************************/
{
Poco_lib *lib, *next;

next = *libs;
while ((lib=next) != NULL)
	{
	next = lib->next;
	pj_rexlib_free(&lib->rexhead);
	}
*libs = NULL;
}


