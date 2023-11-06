#include "flipath.h"
#include "commonst.h"
#include "errcodes.h"
#include "jfile.h"
#include "memory.h"
#include "ptrmacro.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

/* note this sets size to sizeof(Flipath) not length of string! it
 * returns adjusted size of length for actual path string length if
 * successful */
Errcode set_flipath(char* fliname, Fli_id* flid, Flipath* fp)
{
	Errcode err;
	char pbuf[PATH_SIZE];

	clear_struct(&fp->fid);
	if (fliname && flid) {
		err = get_full_path(fliname, pbuf);
		if (err < 0)
			return err;
		fp->fid = *flid;
	}
	else {
		sprintf(pbuf, "%s.flc", unnamed_str);
	}

	fp->id.type	   = FP_FLIPATH;
	fp->id.size	   = sizeof(Flipath);
	fp->id.version = FLIPATH_VERSION;

	return ((OFFSET(Flipath, path) + 1) + sprintf(fp->path, "%s", pbuf));
}

void clear_flipath(Flipath* fp)
{
	set_flipath(NULL, NULL, fp);
}

void copy_flipath(Flipath* a, Flipath* b)
{
	copy_mem(a, b, OFFSET(Flipath, path));
	strcpy(b->path, a->path);
}

/* allocate and make up a flipath chunk to represent a fli will free before
 * allocating so the input must be initialized if FLIF is null the id will
 * be 0s */
Errcode alloc_flipath(char* fliname, Flifile* flif, Flipath** pfpath)
{
	Errcode err;
	Flipath fp;
	Fli_id zid;

	clear_struct(&zid); /* a fudge but if it's not a fli we zero out id */

	err = set_flipath(fliname, flif ? &flif->hdr.id : &zid, &fp);
	if (err < Success) {
		return err;
	}

	fp.id.size = err;
	free_flipath(pfpath);
	*pfpath = pj_malloc(fp.id.size);
	if (*pfpath == NULL) {
		return Err_no_memory;
	}
	copy_flipath(&fp, *pfpath);
	return Success;
}

void free_flipath(Flipath** fp)
{
	pj_freez(fp);
}

bool flipaths_same(Flipath* pa, Flipath* pb)
{
	return (0 == memcmp(&pa->fid, &pb->fid, sizeof(pa->fid)) && 0 == txtcmp(pa->path, pb->path));
}
