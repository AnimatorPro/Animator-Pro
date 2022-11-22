#ifndef CCACHE_H
#define CCACHE_H
#ifndef CMAP_H
	#include "cmap.h"
#endif

typedef struct ccache
	{
	char *name;
	Errcode (*make_ccache)(struct ccache *cc);
	void	(*free_ccache)(struct ccache *cc);
	int		(*ccache_closest)(struct ccache *cc, Rgb3 *rgb, Cmap *cmap);
	void 	*data;
	long	misses;
	long	total;
	} Ccache;
extern Ccache cc_bhash;
extern Ccache cc_bbhash64;
extern Ccache cc_ghash;

Errcode cc_make(Ccache **pcc, Boolean is_grey, UBYTE colors_256);
void cc_free(Ccache *cc);
int cc_closest(Ccache *cc, Rgb3 *rgb, Cmap *cmap, Boolean dither);
#endif /* CCACHE_H */
