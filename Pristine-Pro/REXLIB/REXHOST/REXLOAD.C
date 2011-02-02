#include "memory.h"
#include "ptrmacro.h"
#include "errcodes.h"
#include "hw386.h"
#include "jfile.h"
#include "rexload.h"

Errcode pj_open_rex(char *path, Jfile *pfile, EXP_HDR *hdrsp)
{
Errcode err;
OEXP_HDR ohdrs;	 /* Old .EXP header */

	if(JNONE == (*pfile = pj_open(path,JREADONLY)))
		goto io_error;
	if(pj_read(*pfile, &ohdrs, sizeof(ohdrs)) < sizeof(ohdrs))
		goto io_error;

	if (ohdrs.exe_sign != REX_OLD)
	{
		err = Err_file_not_rex;
		goto error;
	}

	/* we convert rex header to new style exp header the way pharlaps sample
	 * code does it */

	clear_mem(hdrsp, sizeof(EXP_HDR)); /* fill new header with zeros */

	/* file offset to load image */
	hdrsp->exp_ldimg=(ohdrs.exe_hsize) << 4;

	/* size of load image in bytes */
	hdrsp->exp_ldisize= ((ohdrs.exe_size)-1) * REX_BLK_SIZE +
		    			(ohdrs.exe_szrem) - (hdrsp->exp_ldimg);

	/* File offset of relocation table */
	hdrsp->exp_rel = ohdrs.exe_reloff;

	/* Number of bytes in relocation table */
	hdrsp->exp_relsize = (ohdrs.exe_relcnt)*4;

	/* Minimum Data to allocate after load image */
	hdrsp->exp_minext=ohdrs.exe_minpg<<PAGE_SHIFT;

	/* Maximum Data to allocate after load image */
	hdrsp->exp_maxext=ohdrs.exe_maxpg<<PAGE_SHIFT;
	
	/* Initial EIP */
	hdrsp->exp_ieip= (ohdrs.exe_eip);

	/* Initial ESP */
	hdrsp->exp_iesp= (ohdrs.exe_esp);
	return(Success);

io_error:
	err = pj_ioerr();
error:
	pj_closez(pfile);
	return(err);
}
static Errcode read_fixup_code(Jfile file,EXP_HDR *hdrsp,
							   void **pprex, void **pplib)
{
ULONG load_size;	/* Size of memory to allocate to program code and data */
LONG err;			/* Function return code */
ULONG *load_addr;	/* Address in which to load program */
ULONG rel_buf[128]; /* work area for relocation tables */
ULONG *rel_entry; 	/* Relocation entry value (offset to be relocated) */
ULONG *rel_max;
#define RLIB ((Rexlib_entry *)rel_buf)
int relsize;		


	load_size= (hdrsp->exp_minext) + (hdrsp->exp_ldisize);

	if(pplib) /* we are expecting a pj style rex library so verify 
				* the header data */
	{
		if ((err = pj_readoset(file, RLIB, 
								   hdrsp->exp_ldimg, 
								   sizeof(Rexlib_entry))) < Success)
		{
			goto error;
		}
		if(RLIB->magic != PJREX_MAGIC
			 || RLIB->magic2 != PJREX_MAGIC2)
		{
			err = Err_not_rexlib;
			goto error;
		}
		if(RLIB->version != PJREX_BETAVERS 
			&& RLIB->data_end > hdrsp->exp_ldisize
			&& RLIB->data_end < load_size)
		{
			load_size = RLIB->data_end + 4; /* pad four bytes */
		}
	}

	load_addr = pj_malloc(load_size); /* allocate un-cleared memory */

	/* save pointer so we can free the memory later (with "unload"). */

	if(NULL == (*pprex = load_addr))
		return(Err_no_memory);

	/* read code and initialized data in from file */

	if ((err = pj_readoset(file, load_addr, 
						 hdrsp->exp_ldimg, hdrsp->exp_ldisize)) < Success)
	{
		goto error;
	}

	/* clear part of module not containing file (uninitialized data) */

	clear_mem(OPTR(load_addr,hdrsp->exp_ldisize),
			  load_size - hdrsp->exp_ldisize);

	if((err = pj_seek(file, hdrsp->exp_rel, JSEEK_START)) < 0)
		goto error;

	/* Now that the program is loaded in memory, we must relocate
	   all address that need relocation. The following loop does
	   this. Modified to read it in buffered chunks for speed. */

	relsize = sizeof(rel_buf);

	while(hdrsp->exp_relsize > 0)
	{
		if(hdrsp->exp_relsize < relsize)
			relsize = hdrsp->exp_relsize;	

		if(pj_read(file, rel_buf, relsize) < relsize) 
		{
			err = pj_ioerr();
			goto error;
		}

		hdrsp->exp_relsize -= relsize;
		rel_max = (ULONG *)OPTR(rel_buf,relsize);
		rel_entry = rel_buf;

		while(rel_entry < rel_max)
		{
			if(*rel_entry & REL32)
			{
				*((ULONG *)OPTR(load_addr,(*rel_entry&(~REL32)))) 
											+= *((ULONG*)&load_addr);
			}
			else
			{
				*((USHORT *)OPTR(load_addr,*rel_entry)) 
											+= *((ULONG*)&load_addr);
			}
			++rel_entry;
		}
	} /* end for loop */

	/* load pj rex library pointer if requested */

	if(pplib)
		*pplib = ((Rexlib_entry *)load_addr)->header_data;

	return(Success);
error:
	pj_freez(pprex);
	return(err);
}
void pj_rex_free(void **rexpp)
/* frees what is loaded by load_rex() */
{
	pj_freez(rexpp);
}
Errcode pj_rex_load(char *path,void **rexpp, void **prexlib)
{
Errcode	err;
EXP_HDR	hdrs;	/* New .EXP style header */
Jfile file;

	*rexpp = NULL; /* in case of error out */

	if((err = pj_open_rex(path,&file,&hdrs)) < Success)
		goto error;

	err = read_fixup_code(file,&hdrs,rexpp,prexlib);

error:
	pj_close(file);
	return(err);
}


