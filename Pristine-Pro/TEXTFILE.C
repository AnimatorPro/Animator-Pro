
#include "stdtypes.h"
#include "lstdio.h"
#include "errcodes.h"
#include "memory.h"
#include "jfile.h"
#include "textedit.h"
#include "commonst.h"
#include "ffile.h"
#include "jimk.h"

static long strip_zeros(register char *buf,long size)
/* Strip out 0 characters. */
{
char *out;
char *in;
char *max;

	in = out = buf;
	max = buf + size;
	while(in < max)
	{
		if((*out++ = *in++) == 0)
			--out;
	}
	return(out - buf);
}

Errcode load_text_file(Text_file *gf, char *name)
{
Errcode err;
FILE *f;
LONG oldsize;

	gf->text_name = name;
	if ((oldsize = pj_file_size(name)) < Success)
		return oldsize;
	if((err = ffopen(name, &f, r_str)) < Success)
		return(err);
	free_text_file(gf);
	gf->text_alloc = oldsize + DTSIZE;

	if ((gf->text_buf = pj_malloc((long)gf->text_alloc)) == NULL)
	{
		pj_close(f);
		return(Err_no_memory);
	}

	gf->text_size = fread(gf->text_buf,1,gf->text_alloc-1,f);
	gf->text_size = strip_zeros(gf->text_buf, gf->text_size);
	gf->text_buf[gf->text_size] = 0;
	ffclose(&f);
	return(Success);
}

static Errcode sv_text_file(Text_file *gf, char *name)
{
Errcode err;
FILE *f;

	if (!gf->text_buf)
		return(Success);
	if((err = ffopen(name, &f, w_str)) < Success)
		goto error;
	if(fwrite(gf->text_buf, 1, (long)gf->text_size, f) < gf->text_size)
	{
		err = ffile_error(f);
		goto error;
	}
error:
	ffclose(&f);
	return(err);
}
Errcode save_text_file(Text_file *gf)
/* uses name in text file struct and reports errors */
{
	return(softerr(sv_text_file(gf,gf->text_name),
			"!%s", "cant_save", gf->text_name));
}

Errcode load_titles(char *title)
/* a bit of a fudge, but it will do it certainly will check for all errors
 * like long text and out of memory in the process */
{
Errcode err;
Text_file tf;

	clear_struct(&tf);
	if((err = load_text_file(&tf,title)) < Success)
		goto error;
	if((err = sv_text_file(&tf,text_name)) < Success)
		goto error;
	vs.tcursor_p = vs.text_yoff = 0;
error:
	free_text_file(&tf);
	return(err);
}

