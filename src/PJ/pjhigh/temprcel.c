#include <stdio.h>
#include "errcodes.h"
#include "jimk.h"

Errcode temp_save_rcel(Rcel_save *sc, Rcel *cel)
/* Save a rcel to the card if possible or else to a temp file */
{
static int rnum = 0;
Errcode err;

	/* Try to put it on the video card */
	if ((err = alloc_vd_rcel(vb.vd, (Rasthdr *)cel, &sc->saved_cel,
					  cel->cmap->num_colors,FALSE)) >= Success)
	{
		pj_rcel_copy(cel, sc->saved_cel);
		sc->where = SSC_CEL;
		return(Success);
	}
	/* make up file name and swap it to a temp-file */
	sprintf(sc->saved_fname, "=:rcel%d.pic", rnum++);
	if ((err = save_pic(sc->saved_fname, cel, 0L, TRUE)) >= Success)
	{
		sc->where = SSC_FILE;
		return(Success);
	}
	sc->where = SSC_NONE;
	return(err);
}

Errcode report_temp_save_rcel(Rcel_save *sc, Rcel *cel)
{
	return(softerr(temp_save_rcel(sc,cel), "save_tempc"));
}

Errcode temp_restore_rcel(Rcel_save *sc, Rcel *cel)
/* restore cel saved with temp_restore_rcel().  */
{
Errcode err;

	switch(sc->where)
	{
		case SSC_CEL:
			pj_rcel_copy(sc->saved_cel, cel);
			pj_cmap_load(cel,sc->saved_cel->cmap);
			pj_rcel_free(sc->saved_cel);
		case SSC_NONE:
			err = Success;
			break;
		case SSC_FILE:
			err = load_pic(sc->saved_fname, cel, 0, TRUE);
			pj_delete(sc->saved_fname);
			break;
		default:
			err = Err_nogood;
			break;
	}
	sc->where = SSC_NONE;
	return(err);
}

Errcode report_temp_restore_rcel(Rcel_save *sc, Rcel *cel)
{
	return(softerr(temp_restore_rcel(sc,cel), "restore_tempc"));
}

