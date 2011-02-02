#ifndef ENCRYPT_H
#define ENCRYPT_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif


typedef struct cryptic_data {
	char *password;   /* null terminated password */
	char *pw;   	  /* current password char */
	void *in_data;    /* pointer to where to get data */
	void *out_data;   /* pointer to where to put data */

	/* internal state */
	ULONG crcsum;     
	ULONG pwcrc;
	ULONG op;
	UBYTE decrypt;
} Cryptic_data;

#ifdef BIG_COMMENT /************************/

	{
	Cryptic_data cd;
	UBYTE buf[bsize];

	  /* Init ONCE at start of a data stream. 
	  	 If FALSE encrypting, TRUE decrypting, password string must remain
		 valid while Cryptic_data is used */

		init_cryptic(&cd,password,buf,buf,FALSE);

		while(read_data(file,buf,bsize))
		{
			crypt_bytes(&cd,bsize);
			write_data(ofile,buf,bsize);

			/* buffer pointer are incremented by crypt_bytes so we must
			 * reset them. The in and out buffers may be separate */

			cd.in_data = buf;
			cd.out_data = buf;
		}
	}

#endif /**** BIG_COMMENT ************************/

void init_cryptic(Cryptic_data *cd,char *password,
				  void *in_data,void *out_data, int decrypt);

void crypt_bytes(Cryptic_data *cd, int count);


#endif /* ENCRYPT_H Leave at end of file */
