#include "errcodes.h"
#include "memory.h"
#include "picfile.h"

Errcode pj_read_pichead(Jfile f,Pic_header *pic)
{
Opic_header *opic;
Rectangle orect;

	if (pj_read(f, pic, sizeof(*pic)) < (long)sizeof(*pic) )
		return(pj_ioerr());
	if (pic->id.type != PIC_MAGIC)
	{
		opic = (Opic_header *)pic;
		if(opic->type != OPIC_MAGIC
			|| opic->csize != opic->width * opic->height
			|| opic->d != 8)
		{
			return(Err_bad_magic);
		}
		copy_rectfields(opic,&orect);
		clear_struct(pic);
		pic->id.type = OPIC_MAGIC;
		pic->depth = 8;
		copy_rectfields(&orect,pic);

		/* note size field remains unset (0) for old pics */
	}
	return(Success);
}
