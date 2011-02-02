#ifndef BFILE_H
#define BFILE_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef JFILE_H
	#include "jfile.h"
#endif

#define BSIZE 2048
struct bfile
	{
	Jfile fd;
	SHORT left;
	UBYTE *buf;
	UBYTE *filept;
	SHORT writable;
	};
typedef struct bfile Bfile;

int bio_err(void);
Errcode bclose(Bfile*bf);
int bopen(char*name,Bfile*bf);
int bcreate(char*name,Bfile*bf);
int bflush(register Bfile*bf);
long bseek(Bfile*bf,long offset,int mode);
int bputbyte(register Bfile*bf,unsigned char c);
int bwrite(register Bfile*bf,void*in_buf,int count);
int bgetbyte(register Bfile*bf);
int bread(register Bfile*bf,void*in_buf,int count);

#endif /* BFILE_H */
