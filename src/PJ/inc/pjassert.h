#ifndef PJASSERT_H
#define PJASSERT_H

#include <assert.h>

#if defined(__GNUC__) || defined(__clang__)
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define WARN_UNUSED_RESULT
#endif

/* Verbose logger. */
#ifndef NDEBUG
#if defined(__GNUC__) || defined(__clang__)

#define pj_assert(expr)             \
	pj_assert_fail((expr) ? 1 : 0,  \
			__FILE__, __LINE__, __ASSERT_FUNCTION, __STRING(expr))

extern int
pj_assert_fail(int success, const char *file, unsigned int line,
		const char *func, const char *str)
	WARN_UNUSED_RESULT;

#endif
#endif /* NDEBUG */

/* Simple logger. */
#ifndef pj_assert

#define pj_assert(expr)             \
	pj_assert_fail((expr) ? 1 : 0,  \
			__FILE__, __LINE__)

extern int
pj_assert_fail(int success, const char *file, unsigned int line)
	WARN_UNUSED_RESULT;

#endif

#endif
