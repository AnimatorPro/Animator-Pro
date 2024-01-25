#include "errcodes.h"
#include "memory.h"
#include "picfile.h"

Errcode
pj_read_pichead(XFILE *xf, Pic_header *pic)
{
	Errcode err;
	Opic_header *opic;
	Rectangle orect;

	err = xffread(xf, pic, sizeof(*pic));
	if (err < Success)
		return err;

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
