/* unbrun_.c */

#include <string.h>
#include "unbrun_.h"

void
unbrun(const UBYTE *src, UBYTE *dst, unsigned int linect)
{
	UBYTE *lineptr = dst;

	for (; linect > 0; linect--) {
		int lineopcount = *src++;
		dst = lineptr;

		for (; lineopcount > 0; lineopcount--) {
			int length = (BYTE) *src++;

			if (length >= 0) {
				memset(dst, *src, length);
				src++;
				dst += length;
			}
			else {
				length = -length;
				memcpy(dst, src, length);
				src += length;
				dst += length;
			}
		}

		lineptr += WIDTH;
	}
}
