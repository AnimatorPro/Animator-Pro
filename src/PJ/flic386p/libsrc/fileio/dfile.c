#include "dfile.ih"
#include "memory.h"

Errcode jerr = Success;

Errcode pj_ioerr()
{
	return(jerr);
}
Errcode pj_close(Jfl *f)
{
	/* do some error checking first... */
	if(f == NULL)
		return(jerr = Err_null_ref);
	if(f->jfl_magic != JFL_MAGIC)
		return(jerr = Err_corrupted);
	if(f->handle.j)
		pj_dclose(f->handle.j);
	pj_free(f);
	return(Success);
}
static Jfl *jopen_it(char *name, int mode, 
					 Doserr (*openit)(int *phandle, char *name,int mode))
{
Jfl *tf;

	if((tf = pj_zalloc(sizeof(*tf))) == NULL)
	{
		jerr = Err_no_memory;
		return(NULL);
	}
	tf->jfl_magic = JFL_MAGIC;
	if((jerr = pj_mserror((*openit)(&(tf->handle.j),name,mode))) < Success)
		goto error;
	tf->rwmode = mode;
	return(tf);
error:
	pj_close(tf);
	return(NULL);
}
Jfl *pj_open(char *name,int mode)
{
	return(jopen_it(name,mode,pj_dopen));
}
Jfl *pj_create(char *name, int mode)
{
	return(jopen_it(name,mode,pj_dcreate));
}
