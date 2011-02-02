/***************************************************************
pictutil.c - Commonly used utilities and file io functions used 
			 in the pict.pdr modules.

Pict file pdr modules:

	Created by Peter Kennard.  Sept 29, 1991
		Implements non-pattern fill mode bitmap operations
		and transfer mode blit. Parses and ignores all other
		opcodes.
****************************************************************/
#include "errcodes.h"
#include "pict.h"

/*** A bunch of basic read and write byte file calls that use a pfile's 
 *** file handle.  These handle the pict file basic data types ***/

/**** write *****/
Errcode pf_write(Pfile *pf, void *buf, int size)
/* Write bytes to a pfile's handle */
{
	if(fwrite(buf,1,size,pf->file) != size)
		return(pj_errno_errcode());
	return(Success);
}
Errcode pf_write_oset(Pfile *pf, void *buf, int size, int offset)
/* Write after seeking to an offset in a pfile's handle */
{
	if(fseek(pf->file,offset,SEEK_SET) < 0)
	 	goto error; 
	if(fwrite(buf,1,size,pf->file) != size)
		goto error;
	return(Success);
error:
	return(pj_errno_errcode());
}
Errcode pf_write_short(Pfile *pf, SHORT val)
/* Intel swap and write a short integer to a pict file. */
{
	val = intel_swap_word(val);
	if(fwrite(&val,1,sizeof(val),pf->file) != 2)
		return(pj_errno_errcode());
	return(Success);
}
Errcode write_chunk8(Pfile *pf,void *buf, int size)
/* Write a chunk with a leading byte containing the size of the chunk. */
{
	if(fputc(size,pf->file) == EOF)
		return(pj_errno_errcode());
	return(pf_write(pf,buf,size));
}
Errcode write_chunk16(Pfile *pf,void *buf, int size)
/* Write a chunk with a leading 16 bit word containing the size of the chunk. */
{
Errcode err;

	if((err = pf_write_short(pf,size)) >= Success)
		err = pf_write(pf,buf,size);
	return(err);
}

/**** read *****/
Errcode pf_read(Pfile *pf,void *buf,LONG size)
/* Read bytes from a pfile */
{
	if(fread(buf,1,size,pf->file) != size)
		pf->lasterr = pj_errno_errcode();
	pf->bytes_since += size;
	return(pf->lasterr);
}
Errcode pf_seek_bytes(Pfile *pf, LONG count)
/* Skips forward a number of bytes using seek */
{
	if(count != 0)	
	{
		pf->bytes_since += count;
		if(fseek(pf->file,count, SEEK_CUR) < 0)
			goto error;
	}
	return(Success);
error:
	return(pf->lasterr = Err_seek); /* pj_errno_errcode()); */
}
Errcode pf_read_oset(Pfile *pf,void *buf,LONG size,LONG offset)
/* Read bytes from a pict file at a specific offset. */
{
	if(fseek(pf->file,offset,SEEK_SET) < 0)
		return(pf->lasterr = Err_seek); /* pj_errno_errcode()); */

	if(fread(buf,1,size,pf->file) != size)
	{
		if ((pf->lasterr = pj_errno_errcode()) >= Success)
			pf->lasterr = Err_truncated;
	}
	return(pf->lasterr);
}
/************** read and seek by for each of 3 basic chunk types ********/
Errcode read_chunk8(Pfile *pf,void *packline, unsigned maxsize)
/* Read a byte and then read the number of bytes indicated by that byte's
 * value. */
{
int size;

	if((size = fgetc(pf->file)) == EOF)
		return(pf->lasterr = pj_errno_errcode());
	++pf->bytes_since;
	if((unsigned)size > maxsize)
		return(pf->lasterr = Err_format);
	return(pf_read(pf,packline,size));
}
Errcode skip_chunk8(Pfile *pf)
/* Read a byte and then seek the number of bytes indicated by that byte's
 * value. */
{
int size; 

	if((size = fgetc(pf->file)) == EOF)
		return(pf->lasterr = pj_errno_errcode());
	++pf->bytes_since;
	return(pf_seek_bytes(pf,size));
}
Errcode read_chunk16(Pfile *pf,void *buf,unsigned maxsize)
/* Read a word and then read the number of bytes indicated by that word's
 * value. */
{
USHORT size;

	if(pf_read(pf,&size,sizeof(size)) >= Success)
		return(pf_read(pf,buf,intel_swap_word(size)));
	if(size > maxsize)
		return(pf->lasterr = Err_format);
	return(pf->lasterr);
}
Errcode skip_chunk16(Pfile *pf)
/* Read a word and then seek the number of bytes indicated by that word's
 * value. */
{
USHORT size;

	if(pf_read(pf,&size,sizeof(size)) >= Success)
		return(pf_seek_bytes(pf,intel_swap_word(size)));
	return(pf->lasterr);
}
#ifdef SLUFFED
Errcode read_chunk32(Pfile *pf,void *buf)
{
ULONG size;

	if(pf_read(pf,&size,sizeof(size)) >= Success)
		return(pf_read(pf,buf,intel_swap_long(size)));
	return(pf->lasterr);
}
#endif /* SLUFFED */
Errcode skip_chunk32(Pfile *pf)
/* Read a long word and then seek the number of bytes indicated by that word's
 * value. */
{
ULONG size;

	if(pf_read(pf,&size,sizeof(size)) >= Success)
		return(pf_seek_bytes(pf,intel_swap_long(size)));
	return(pf->lasterr);
}
/************** read and seek for basic data types ********/
Errcode read_short(Pfile *pf, SHORT *ps)
/* Read and intel swap a short integer. */
{
SHORT s;

	if(fread(&s,1,2,pf->file) != 2)
		pf->lasterr = pj_errno_errcode();
	*ps = intel_swap_word(s);
	pf->bytes_since += 2;
	return(pf->lasterr);
}
Errcode read_pRect(Pfile *pf, pRect *rect)
/* Read and intel swap a pRect record. */
{
	pf_read(pf,rect,sizeof(*rect));
	intel_swap_words(rect,4);
	return(pf->lasterr);
}

/******* read opcodes one for each version **********/
Errcode read_v1_opcode(Pfile *pf)
/* Read a one byte opcode at current file position. */
{
	if((pf->op = fgetc(pf->file)) == EOF)
		pf->lasterr = pj_errno_errcode();
	return(pf->lasterr);
}
Errcode read_v2_opcode(Pfile *pf)
/* Version 2 items are word aligned in the file and will always be separated
 * by an even number of bytes.  For this reason all read and seek calls should 
 * keep track of the number of bytes currently past the last opcode.  The
 * bytes since if odd will require skipping an extra pad byte. */
{
struct {
	UBYTE align;
	SHORT op;
} buf;

	if(pf->bytes_since & 1)
	{
		if(fread(&buf.align,1,3,pf->file) != 3)
			goto error;
	}
	else 
	{	
		if(fread(&buf.op,1,2,pf->file) != 2)
			goto error;
	}
	pf->op = intel_swap_word(buf.op);
	pf->bytes_since = 0;
	return(pf->lasterr);
error:
	return(pf->lasterr = pj_errno_errcode());
}
void freez(void *pmem)
/* Free an item and set the pointer to it to NULL really gets passed
 * in a void ** */
{
void *mem;

	if((mem = *((void **)pmem)) != NULL)
		free(mem);
	*((void **)pmem) = NULL;
}
void cleanup_buffers(Pfile *pf)
/* Free the dynamic work buffers */
{
	freez(&pf->cTable);
	freez(&pf->pixbuf);
}

/* geometry operations */

void offset_pRect(pRect *r, SHORT dx, SHORT dy)
/* Offset a pict type rectangle dx,dy */
{
	r->top += dy;
	r->bot += dy;
	r->left += dx;
	r->right += dx;
}
void set_rect_relto(pRect *r, pRect *relto)
/* Set one rectangle that is relative to another to have an absolute 
 * position in the relto's coordinate system. */
{
	offset_pRect(r,-relto->left,-relto->top);
}
