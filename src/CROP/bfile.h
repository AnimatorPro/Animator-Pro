#ifndef BFILE_H
#define BFILE_H

#include <stdio.h>
#include "jimk.h"

/* stuff for my buffered io */
#define BSIZE 2048

struct bfile
{
	FILE *fd;
	int left;
	UBYTE *buf;
	UBYTE *filept;
	long fpos;
	int writable;
};
typedef struct bfile Bfile;

#endif
