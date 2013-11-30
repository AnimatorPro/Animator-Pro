/* truecol_.c */

#include "truecol_.h"

unsigned int
closestc(const UBYTE *rgb, const UBYTE *cmap, int count)
{
	const int r = rgb[0];
	const int g = rgb[1];
	const int b = rgb[2];
	unsigned int closest_diff2 = 32000;
	unsigned int closest_index = 0;
	int i = 0;

	for (i = 0; i < count; i++) {
		int dr = (cmap[3 * i + 0] - r);
		int dg = (cmap[3 * i + 1] - g);
		int db = (cmap[3 * i + 2] - b);
		unsigned int diff2 = dr*dr + dg*dg + db*db;

		if (diff2 < closest_diff2) {
			closest_index = i;
			closest_diff2 = diff2;
		}
	}

	return closest_index;
}
