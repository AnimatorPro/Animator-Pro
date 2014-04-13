/* textutil.c
 *
 * Common text manipulation routines.
 */

#include "pjassert.h"
#include "errcodes.h"
#include "textutil.h"

#define DIR_DELIM   '/'
#define DIR_DELIM2  '\\'

enum TextCopyDelim {
	TEXT_COPY_DELIM_NONE,
	TEXT_COPY_DELIM_DIRS
};

/* Function: text_ncopy_delim
 *
 *  Copies at most n bytes from dst to src, stopping at NUL or delim.
 *  The NUL byte is guaranteed.
 *
 *  Returns the number of bytes copied from src, or an error code.
 */
static Errcode
text_ncopy_delim(char *dst, const char *src, size_t n, enum TextCopyDelim delim)
{
	size_t rem = n;

	if (!pj_assert(dst != NULL)) return Err_bad_input;
	if (!pj_assert(src != NULL)) return Err_bad_input;
	if (!pj_assert(n > 0)) return Err_range;

	while (*src != '\0') {
		if (delim == TEXT_COPY_DELIM_DIRS
				&& (*src == DIR_DELIM || *src == DIR_DELIM2)) {
			break;
		}

		if (--rem <= 0)
			break;

		*dst++ = *src++;
	}

	*dst = '\0';
	return (rem > 0) ? (Errcode)(n - rem) : Err_overflow;
}

/* Function: text_ncopy
 *
 *  text_ncopy_delim with no delimiters.
 */
Errcode
text_ncopy(char *dst, const char *src, size_t n)
{
	return text_ncopy_delim(dst, src, n, TEXT_COPY_DELIM_NONE);
}

/* Function: text_ncopy_dir_delim
 *
 *  text_ncopy_delim with directory separator delimiters.
 */
Errcode
text_ncopy_dir_delim(char *dst, const char *src, size_t n)
{
	return text_ncopy_delim(dst, src, n, TEXT_COPY_DELIM_DIRS);
}
