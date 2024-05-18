#ifndef SCALE_H
#define SCALE_H
#ifndef LSTDIO_H
	#include "lstdio.h"
#endif
#ifndef CCACHE_H
	#include "ccache.h"
#endif

extern FILE *rgb_files[3];
extern char *rgb_names[3];
Errcode rgb_temp_err(Errcode err);
void kill_rgb_files();
Errcode open_rgb_files(char *mode, int comp_count);
close_rgb_files();
Errcode abort_scale();
void pix_ave_scale(UBYTE *s, int sct, UBYTE *d, int dct);
Errcode yscale_file(char *name, int w, int oh, int nh);
Errcode rgb_files_to_cel(Rcel *cel, int comp_count, Boolean new_cmap,
						 Boolean flip);
void cc_fit_line(Ccache *cc, Cmap *cmap, UBYTE **rgb_bufs, UBYTE *dest, int count);
#endif /* SCALE_H */
