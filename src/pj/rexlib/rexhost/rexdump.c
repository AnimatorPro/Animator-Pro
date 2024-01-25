#include "errcodes.h"
#include "memory.h"
#include "plexp.h"
#include "hw386.h"
#include "jfile.h"
#include "rexload.h"
#include "encrypt.h"
#include <stdio.h>

void old_video() {} /* for debugging library */
static FILE *out_file;
static char *password;
Cryptic_data cd;

static void	conv_old(EXP_HDR *hdrsp, OEXP_HDR *ohdrsp)
{
	/* file offset to load image */
	hdrsp->exp_ldimg=(ohdrsp->exe_hsize) << 4;

	/* size of load image in bytes */
	hdrsp->exp_ldisize= ((ohdrsp->exe_size)-1) * REX_BLK_SIZE +
		    			(ohdrsp->exe_szrem) - (hdrsp->exp_ldimg);

	/* File offset of relocation table */
	hdrsp->exp_rel = ohdrsp->exe_reloff;

	/* Number of bytes in relocation table */
	hdrsp->exp_relsize = (ohdrsp->exe_relcnt)*4;

	/* Minimum Data to allocate after load image */
	hdrsp->exp_minext=ohdrsp->exe_minpg<<PAGE_SHIFT;

	/* Maximum Data to allocate after load image */
	hdrsp->exp_maxext=ohdrsp->exe_maxpg<<PAGE_SHIFT;
	
	/* Initial EIP */
	hdrsp->exp_ieip= (ohdrsp->exe_eip);

	/* Initial ESP */
	hdrsp->exp_iesp= (ohdrsp->exe_esp);
}
static void dump_code(void *code, int size)
{
LONG *lcode;
int done;
Cryptic_data cd;

	size = (size + 3)/4;

	fprintf(out_file,"\n#define REX_CODE_CRC 0x%08x\n", 
					  mem_crcsum(code,size*4));

	if(password)
	{
		fprintf(out_file,"\n#define REX_CODE_KEY \"%s\"\n", password); 
		init_cryptic(&cd,password,code,code,FALSE);
		crypt_bytes(&cd,size*4);
	}

	fprintf(out_file,"\nREX_CODE =\n{");
	lcode = (LONG *)code;

	for(done = 0; done < size; ++done)
	{
		if((done % 4) == 0)
			fprintf(out_file,"\n   ");
		fprintf(out_file," 0x%08x,", *lcode++);
	}
	fprintf(out_file,"\n};");
}
static Errcode dump_code_and_fixup(Jfile file,EXP_HDR *hdrsp,OEXP_HDR *ohdrsp)
{
ULONG mem_alloc;		/* Memory to allocate to program */
LONG retcode;		/* Function return code */
ULONG *load_addr;		/* Address in which to load program */
ULONG rel_addr;		/* Addr of value to be relocated */
int relsdone;

	/*  Convert .REX to new .EXP to make things easier to deal with. */

	conv_old(hdrsp,ohdrsp);

	/*
	Figure out how much memory we will need to load this program
	(we will always be loading MINDATA extra bytes, we will ignore
	the MAXDATA parameter) and call the C routine "malloc" to try 
	to allocate it. If there is an error, give up. If all works out, 
	read load image into memory.
	*/
	

	if((load_addr = pj_zalloc(hdrsp->exp_ldisize+4)) == NULL)
		return(Err_no_memory);

	if ((retcode = pj_readoset(file, load_addr, 
				   hdrsp->exp_ldimg, hdrsp->exp_ldisize)) < Success)
	{
		goto ERROUT;
	}

	mem_alloc= (hdrsp->exp_minext) + (hdrsp->exp_ldisize); 


	if (pj_seek(file, hdrsp->exp_rel, JSEEK_START) < 0)
	{
		retcode = Err_seek;
		goto ERROUT;
	}

	fprintf(out_file,"\n#define REX_ALLOCSIZE 0x%08x\n", mem_alloc );

	fprintf(out_file, "\n/* loadsize 0x%08x */\n", hdrsp->exp_ldisize );

	/* dump fixed up code and initialized data */

	dump_code(load_addr, hdrsp->exp_ldisize);

	relsdone = 0;

	fprintf(out_file,"\n\n/* NOTE: in fixups the REL32 bit 0x80000000"
				     "is inverted! */\n\nREX_FIXUP =\n{");

	for (;(hdrsp->exp_relsize != 0);(hdrsp->exp_relsize -= 4))
	{
		if((relsdone % 4) == 0)
			fprintf(out_file,"\n   ");
		++relsdone;

		if (pj_read(file, &rel_addr, 4) < 4)	/* read a relocation entry */
		{
			retcode = pj_ioerr();
			goto ERROUT;
		}
		fprintf(out_file, " 0x%08lx,", (ULONG)rel_addr^REL32 );
	} 
	fprintf(out_file,"\n};\n");

	retcode = Success;
ERROUT:
	pj_freez(&load_addr);
	return(retcode);
}
int main (int argc, char **argv)
{
Errcode	retcode;
OEXP_HDR ohdrs;			/* Old .EXP header */
EXP_HDR	hdrs;			/* New .EXP header */
char *fnamp;
char *ofname;
Jfile file;

	out_file = stdout;
	switch(argc)
	{
		case 4:
			password = argv[3];
		case 3:
			ofname = argv[2];
			if((out_file = fopen(ofname,"w")) == NULL)
			{
				printf("Can't open \"%s\"\n", ofname);
				retcode = -1;
				goto bad_open;
			}
		case 2:
			break;
		default:
			printf("Usage: rexdump rex_file_path [outfile [password]]\n"
				   "       If a password it will encrypt the code buffer\n");
			exit(-1);
	}

	fnamp = argv[1];
	if(JNONE == (file = pj_open(fnamp,JREADONLY)))
	{
		printf("Can't open %s\n", fnamp );
		retcode = pj_ioerr();
		goto bad_open;
	}
	if(pj_read(file, &ohdrs, sizeof(ohdrs)) < sizeof(ohdrs))
	{
		retcode = pj_ioerr();
		goto ERROUT;
	}
	clear_mem(&hdrs, sizeof(EXP_HDR)); /* fill new header with zeros */

	if(ohdrs.exe_sign != REX_OLD)
	{
		printf("%s is not a rex file!\n", fnamp);
		retcode = Err_file_not_rex;
		goto ERROUT;
	}
	retcode = dump_code_and_fixup(file,&hdrs,&ohdrs);

ERROUT:
	pj_close(file);
bad_open:
	if(retcode != Success)
	{
		printf("Failure! code %d\n", retcode);
		exit(-1);
	}
	if(out_file != NULL && out_file != stdout)
		fclose(out_file);
	return(0);
}

