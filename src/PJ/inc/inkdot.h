
#ifndef INKDOT_H
#define INKDOT_H


extern void opq_hline(const Ink *,SHORT ,const SHORT ,SHORT );
extern void vsp_hline(const Ink *,SHORT ,const SHORT ,SHORT );
extern void hsp_hline(const Ink *,SHORT ,const SHORT ,SHORT );
extern void tsp_hline(const Ink *,SHORT ,const SHORT ,SHORT );
extern void celt_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width);
extern void rvl_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width);
extern void gink_hline(const Ink *,SHORT ,const SHORT ,SHORT );
extern Pixel opq_dot(const Ink *,const SHORT,const SHORT);
extern Pixel vsp_dot(const Ink *,const SHORT,const SHORT);
extern Pixel hsp_dot(const Ink *,const SHORT,const SHORT);
extern Pixel rad_dot(const Ink *,const SHORT,const SHORT);
extern Pixel soft_dot(const Ink *,const SHORT,const SHORT);
extern Pixel anti_dot(const Ink *,const SHORT,const SHORT);
extern Pixel tsp_dot(const Ink *,const SHORT,const SHORT);
extern Pixel tlc_dot(const Ink *,const SHORT,const SHORT);
extern Pixel rvl_dot(const Ink *,const SHORT,const SHORT);
extern Pixel xor_dot(const Ink *,const SHORT,const SHORT);
extern Pixel jmb_dot(const Ink *,const SHORT,const SHORT);
extern Pixel add_dot(const Ink *,const SHORT,const SHORT);
extern Pixel glr_dot(const Ink *,const SHORT,const SHORT);
extern Pixel celt_dot(const Ink *,const SHORT,const SHORT);
extern Pixel cry_dot(const Ink *,const SHORT,const SHORT);
extern Pixel shat_dot(const Ink *,const SHORT,const SHORT);
extern Pixel out_dot(const Ink *,const SHORT,const SHORT);
extern Pixel bri_dot(const Ink *,const SHORT,const SHORT);
extern Pixel des_dot(const Ink *,const SHORT,const SHORT);
extern Pixel swe_dot(const Ink *,const SHORT,const SHORT);
extern Pixel clh_dot(const Ink *,const SHORT,const SHORT);
extern Pixel emb_dot(const Ink *,const SHORT,const SHORT);
extern Pixel pull_dot(const Ink *,const SHORT,const SHORT);
extern Pixel smea_dot(const Ink *,const SHORT,const SHORT);

extern Errcode make_glow_cashe(const Ink *inky);
void free_glow_cashe(const Ink *inky);
Errcode make_ink_bhash(Ink *inky);
void free_ink_bhash(Ink *inky);
Errcode make_ink_thash(Ink *inky);
Errcode make_tsp_cashe(const Ink *inky);
Errcode make_dar_cashe(const Ink *inky);
void free_ink_thash(Ink *inky);
Errcode clear_random_cashe(const Ink *inky);

#define SBSIZE  128		/* maximum width for an ink hline */

#endif /* INKDOT_H */
