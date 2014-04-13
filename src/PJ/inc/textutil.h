#ifndef TEXTUTIL_H
#define TEXTUTIL_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

extern Errcode text_ncopy(char *dst, const char *src, size_t n);
extern Errcode text_ncopy_dir_delim(char *dst, const char *src, size_t n);

#endif
