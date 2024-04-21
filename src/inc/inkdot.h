#ifndef INKDOT_H
#define INKDOT_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

struct ink;
struct rgb3;

#define SBSIZE  128		/* maximum width for an ink hline */

/* Hash for transparent color */
typedef struct thash {
	UBYTE valid, closest;
} Thash;

extern struct rgb3 tcolor_src;
extern Thash *gel_thash;
extern SHORT gel_factor;

/* inkcashe.c */
extern void free_ink_bhash(struct ink *inky);
extern Errcode make_ink_bhash(struct ink *inky);
extern void free_ink_thash(struct ink *inky);
extern Errcode make_ink_thash(struct ink *inky);
extern Errcode make_tsp_cashe(struct ink *inky);
extern Errcode make_dar_cashe(struct ink *inky);
extern Errcode make_glow_cashe(struct ink *inky);
extern void free_glow_cashe(struct ink *inky);
extern Errcode clear_random_cashe(struct ink *inky);

/* inkdot.c, inkdot2.c, and tileink.c */
extern void disable_lsp_ink(void);
extern void enable_lsp_ink(void);

extern Errcode init_celt_ink(struct ink *inky);
extern void cleanup_celt_ink(struct ink *inky);

extern Pixel add_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel anti_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel bri_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel celt_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel clh_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel cry_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel des_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel emb_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel glr_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel hsp_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel jmb_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel opq_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel out_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel pull_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel rad_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel rvl_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel shat_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel smea_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel soft_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel swe_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel tlc_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel tsp_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel vsp_dot(const struct ink *inky, SHORT x, SHORT y);
extern Pixel xor_dot(const struct ink *inky, SHORT x, SHORT y);

extern void celt_hline(const struct ink *inky, SHORT x0, SHORT y, SHORT width);
extern void cry_hline(const struct ink *inky, SHORT x0, SHORT y, SHORT width);
extern void gink_hline(const struct ink *inky, SHORT x0, SHORT y, SHORT width);
extern void hsp_hline(const struct ink *inky, SHORT x0, SHORT y, SHORT width);
extern void opq_hline(const struct ink *inky, SHORT x0, SHORT y, SHORT width);
extern void rvl_hline(const struct ink *inky, SHORT x0, SHORT y, SHORT width);
extern void soft_hline(const struct ink *inky, SHORT x0, SHORT y, SHORT width);
extern void tsp_hline(const struct ink *inky, SHORT x0, SHORT y, SHORT width);
extern void vsp_hline(const struct ink *inky, SHORT x0, SHORT y, SHORT width);

#endif
