/* unbrun_.c */

#include <string.h>
#include "unbrun_.h"

static const UBYTE *
unbrun_copy(const UBYTE *src, UBYTE *dst)
{
	unsigned int lineopcount = *src++;

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

	return src;
}

static const UBYTE *
unbrun_skip(const UBYTE *src)
{
	unsigned int lineopcount = *src++;

	for (; lineopcount > 0; lineopcount--) {
		int length = (BYTE) *src++;

		if (length >= 0) {
			src++;
		}
		else {
			length = -length;
			src += length;
		}
	}

	return src;
}

void
unbrun(const UBYTE *src, UBYTE *dst, unsigned int linect)
{
	for (; linect > 0; linect--) {
		src = unbrun_copy(src, dst);
		dst += WIDTH;
	}
}

void
un5brun(const UBYTE *src, UBYTE *dst, unsigned int linect)
{
	if (linect <= 0)
		return;

	for (;;) {
		src = unbrun_copy(src, dst);
		dst += WIDTH;

		linect--;
		if (linect <= 0)
			break;

		src = unbrun_skip(src);
		src = unbrun_skip(src);
		src = unbrun_skip(src);
		src = unbrun_skip(src);
	}
}
