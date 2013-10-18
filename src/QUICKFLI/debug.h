#ifndef DEBUG_H
#define DEBUG_H

/* Compile time assertions. */
#define ASSERT_CONCAT_(a, b)    a##b
#define ASSERT_CONCAT(a, b)     ASSERT_CONCAT_(a, b)
#define STATIC_ASSERT(module, e) \
	struct ASSERT_CONCAT(static_assert_##module##_line_, __LINE__) \
		{ unsigned int bf : !!(e); }

#endif
