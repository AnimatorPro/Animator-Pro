/*****************************************************************************
 *
 ****************************************************************************/

#ifndef CCACHE_H
#define CCACHE_H 1

#ifndef STDTYPES_H
  #include "stdtypes.h"
#endif

#ifndef CMAP_H
  #include "cmap.h"
#endif

extern Errcode cc_init(Cmap *cmap, Boolean colors_256, Boolean do_dither);
extern void    cc_cleanup(void);
extern void    cc_cfitinit(void);
extern void    cc_histinit(void);
extern void    cc_cfitline(Pixel *pixbuf, Rgb3 *linebuf, int width);
extern void    cc_histline(Rgb3 *linebuf, int width);
extern int	   cc_hist_color_count(void);
extern void    cc_hist_to_ctab(Rgb3 *big_ctab);

#endif
