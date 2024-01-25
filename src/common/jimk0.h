#ifndef JIMK0_H
#define JIMK0_H

/* Compile time assertions. */
#define ASSERT_CONCAT_(a, b)    a##b
#define ASSERT_CONCAT(a, b)     ASSERT_CONCAT_(a, b)
#define STATIC_ASSERT(module, e) \
	struct ASSERT_CONCAT(static_assert_##module##_line_, __LINE__) \
		{ unsigned int bf : !!(e); }


#define GCC_PACKED

#if defined(__GNUC__)
#undef GCC_PACKED
#define GCC_PACKED  __attribute__((packed))
#endif


typedef char BYTE;
typedef unsigned char UBYTE;
typedef short WORD;
typedef unsigned short UWORD;

#if defined(__TURBOC__)
typedef long LONG;
typedef unsigned long ULONG;
#else
typedef int LONG;
typedef unsigned int ULONG;
#endif

STATIC_ASSERT(jimk, sizeof( BYTE) == 1);
STATIC_ASSERT(jimk, sizeof(UBYTE) == 1);
STATIC_ASSERT(jimk, sizeof( WORD) == 2);
STATIC_ASSERT(jimk, sizeof(UWORD) == 2);
STATIC_ASSERT(jimk, sizeof( LONG) == 4);
STATIC_ASSERT(jimk, sizeof(ULONG) == 4);


#if defined(__TURBOC__)
struct byte_regs
	{
	UBYTE al, ah, bl, bh, cl, ch, dl, dh;
	UWORD si, di, ds, es;
	};
struct word_regs
	{
	UWORD ax, bx, cx, dx;
	UWORD si, di, ds, es;
	};
union regs
	{
	struct byte_regs b;
	struct word_regs w;
	};
#endif /* __TURBOC__ */

#endif
