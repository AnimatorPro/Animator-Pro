#include "encrypt.h"

void init_cryptic(Cryptic_data *cd,char *password,
				  void *in_data, void *out_data, int decrypt)
{
ULONG op;
ULONG crcsum;
UBYTE pwc;

	if(!*password)
		password = "!";
	cd->in_data = in_data;
	cd->out_data = out_data;
	cd->pw = cd->password = password;
	op = (*cd->pw) % 25; 
	crcsum = 0;
	cd->decrypt = (decrypt?0xFF:0);
	while((pwc = *password++) != 0)
	{
       	crcsum += (ULONG)(pwc) << op;
       	if(++op == 25) 
			op = 0;
	}
	cd->pwcrc = cd->crcsum = crcsum;
	cd->op = op;
}
void crypt_bytes(Cryptic_data *cd, int count)
{
UBYTE pwc;
UBYTE inc;
UBYTE decrypt = cd->decrypt;
ULONG op = cd->op;
ULONG crcsum = cd->crcsum;
UBYTE *in = (char *)(cd->in_data);
UBYTE *out = (char *)(cd->out_data);
ULONG pwcrc = cd->pwcrc;
ULONG temp;

	while(count-- > 0)
	{
		while((pwc = *cd->pw++) == 0)
			cd->pw = cd->password;

      	pwcrc += ((ULONG)pwc) << op;

		/* Make every bit in both crcs count. Xor the whole mess of them 
		 * into the out xor byte */

		temp = pwcrc^crcsum;
		temp ^= (temp >> 16);
		pwc = temp ^ (temp >> 8);

		/* if encrypting use input char to make crc 
		 * if decrypting use output char */

		inc = *in++;
		*out++ = pwc^inc;

		inc ^= (decrypt&pwc); /* if decrypt is all ones this will xor all 0s
							   * it will not */

      	crcsum += ((ULONG)inc) << op;
       	if(++op >= 25) 
			op = 0;
	}
	cd->op = op;
	cd->crcsum = crcsum;
	cd->in_data = in;
	cd->out_data = out;
	cd->pwcrc = pwcrc;
}

