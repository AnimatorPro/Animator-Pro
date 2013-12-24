/* cmapload.c - Stuff to read a color map file. */

#include "palchunk.h"
#include "errcodes.h"

#define OLD_SIZE (3*COLORS)

Errcode pj_col_load(char *name, Cmap *cmap)
/* This function checks the size of file to see if it's an old style cmap.
 * Otherwise tries to load it as new-style. */
{
Errcode err;
Jfile fd;
Fat_chunk id;
long size;

if((fd = pj_open(name, JREADONLY))==JNONE)
	return(pj_ioerr());
size = pj_seek(fd, 0L, JSEEK_END);		/* Find size with seek to end */
pj_seek(fd,0L, JSEEK_START);			/* and rewind. */

if (size == OLD_SIZE && cmap->num_colors == COLORS)
	{
	if ((err = pj_read_ecode(fd, cmap->ctab, sizeof(cmap->ctab))) < Success)
		goto ERROR;
	pj_shift_cmap(cmap->ctab,cmap->ctab,COLORS*3);
	}
else
	{
	if((err = pj_read_ecode(fd, &id, sizeof(id))) < Success)
		goto ERROR;

	if(id.type != CMAP_MAGIC)
		{
		err = Err_bad_magic;
		goto ERROR;
		}
	err = pj_read_palchunk(fd,&id,cmap);
	}

ERROR:
pj_close(fd);
return(err);
}


