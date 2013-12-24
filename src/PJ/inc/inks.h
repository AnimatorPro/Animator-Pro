#ifndef INKS_H
#define INKS_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif


enum {
    opq_INKID,
    vsp_INKID, /* VGRAD */
    hsp_INKID, /* HGRAD */
    lsp_INKID, /* LGRAD */
    tlc_INKID, /* GLAZE */
    tsp_INKID, /* GLASS */
    rvl_INKID, /* SCRAPE */
    xor_INKID, /* XOR */
    jmb_INKID, /* JUMBLE */
    add_INKID, /* ADD */
    glr_INKID, /* GLOW */
    celt_INKID, /* TILE */
    cry_INKID, /* SPARK */
    shat_INKID, /* SPLIT */
    anti_INKID, /* UNZAG */
    out_INKID, /* HOLLOW */
	bri_INKID, /* BRIGHT */
    des_INKID, /* GREY */
    swe_INKID, /* SWEEP */
	clh_INKID, /* CLOSE */
    dar_INKID, /* DARK */
    emb_INKID, /* EMBOSS */
	pull_INKID, /* PULL */ 
	smea_INKID, /* SMEAR */
	rad_INKID, /* RGRAD */ 
    soft_INKID, /* soften */
	FIRST_LOADABLE_INKID,
};

void set_curink(Ink *newink);
Errcode id_set_curink(int id);


#endif /* INKS_H */
