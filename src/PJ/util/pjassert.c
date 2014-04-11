/* pjassert.c
 *
 * Assert functions that log errors but doesn't abort on failure.
 *
 * Note kludge with "int success" argument, because GCC is too dumb to
 * warn_unused_result for macros.
 */

#include <stdio.h>
#include "pjassert.h"

#if !defined(NDEBUG) && (defined(__GNUC__) || defined(__clang__))
/* Function: pj_assert_fail
 *
 *  Verbose error logger, used in debug mode where possible.
 */
int
pj_assert_fail(int success, const char *file, unsigned int line,
		const char *func, const char *str)
{
	if (!success) {
		fprintf(stderr, "%s:%u: %s: pj_assert( %s ) failed.\n",
				file, line, func, str);
	}

	return success;
}
#else
/* Function: pj_assert_fail
 *
 *  Simple error logger, used in release mode.
 */
int
pj_assert_fail(int success, const char *file, unsigned int line)
{
	if (!success) {
		fprintf(stderr, "%s:%u: pj_assert failed.\n",
				file, line);
	}

	return success;
}
#endif
