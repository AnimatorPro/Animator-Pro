#ifndef UTIL_H
#define UTIL_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

extern void intel_swap(void *v, int count);
extern void intel_dswap(void *v, int count);

extern void upc(char *s);
extern ULONG str_crcsum(char *buf);
extern int txtcmp(const char *as, const char *bs);
extern int txtncmp(const char *as, const char *bs, int len);
extern char *clone_string(char *s);
extern void tr_string(char *string, char in, char out);

#endif
