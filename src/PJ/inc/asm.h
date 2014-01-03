
#ifndef ASM_H
#define ASM_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif	/* STDTYPES_H */

#ifndef REGS_H
#include "regs.h"
#endif /* REGS_H */

#ifndef JFILE_H
#include "jfile.h"
#endif /* JFILE_H */

#ifndef GFX_H
#include "gfx.h"
#endif /* GFX_H */


#ifdef __FOOLISH__

#pragma aux i86_outb "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux i86_outw "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux i86_inb "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux i86_inw "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux i86_swout "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];

#pragma aux pj_bsame "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_fsame "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_bcompare "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_fcompare "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_bcontrast "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux fcontrast "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_stuff_bytes "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_stuff_words "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_stuff_dwords "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_copy_bytes "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_copy_words "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_copy_structure "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux xor_words "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux xor_group "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux xlate "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_enorm_pointer "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux long_to_pt "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pt_to_long "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];

#pragma aux pj_clock_init "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux _getclock "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];

#pragma aux jgot_mouse "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux jmousey "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux jcomm "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_key_is "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_key_in "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux dos_key_shift "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux msjcreate "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dopen "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dclose "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dread "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dwrite "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_ddelete "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_drename "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dseek "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dset_dta "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dfirst "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dnext "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dset_drive "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dget_drive "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dcount_floppies "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dis_drive "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_ddfree "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dmake_dir "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux _lodos_set_dir "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_dget_dir "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux dos_mem_free "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];

#pragma aux get_vmode "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux set_vmode "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux wait_vblank "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux wait_novblank "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux closestc "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];

#pragma aux dto_table "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];

#pragma aux pj_unbrun "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_unlccomp "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_fcuncomp "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];

#pragma aux pj_tnskip "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];
#pragma aux pj_tnsame "*" parm caller [] \
   value struct float struct routine [eax]	modify [eax];

#endif /*  __WATCOMC__ */

int i86_inb(int port);
int i86_inw(int port);
void i86_outb(int port, int value);
void i86_outw(int port, int value);
void i86_swout(int port, void *data, int count);

int pj_bsame(void *buf, unsigned count);
int pj_fsame(void *buf, unsigned count);
int pj_bcompare(void *buf1, void *buf2, unsigned count);
int pj_fcompare(void *buf1, void *buf2, unsigned count);
int pj_bcontrast(void *buf1, void *buf2, unsigned count);
int fcontrast(void *buf1, void *buf2, unsigned count);
void pj_stuff_bytes(UBYTE data, void *buf, unsigned count);
void pj_stuff_words(USHORT data, void *buf, unsigned count);
void pj_stuff_dwords(ULONG data, void *buf, unsigned count);
void pj_copy_bytes(void *buf1, void *buf2, unsigned count);
void pj_copy_words(void *buf1, void *buf2, unsigned count);
void pj_copy_structure(void *buf1, void *buf2, unsigned count);
void xor_words(USHORT data, void *buf, unsigned words_8);
void xor_group(USHORT data, void *buf, unsigned words);
void xlate(UBYTE *table, UBYTE *buf, unsigned count);
void *pj_enorm_pointer(void *p);
void *long_to_pt(long l);
long pt_to_long(void *p);

Boolean pj_clock_init(void);
short	pj_set_gs(void);	/* sets GS reg to (and returns) PHAR_REAL_SEG */
short	pj_get_gs(void);	/* returns contents of GS segreg */
short	pj_get_ds(void);	/* returns contents of DS segreg */

Boolean jgot_mouse(void);
void jmousey(struct wabcd_regs *mouse_regs);
void jcomm(union abcd_regs *comm_regs);

int pj_key_is(void);
int pj_key_in(void);
int dos_key_shift(void);
Jfile pj_dcreate(char *name, int mode); /* sets jioerr on failure */
Jfile pj_dopen(char *name, int mode);	/* sets jioerr on failure */
void pj_dclose(Jfile f);
LONG pj_dread(Jfile f, void *buf, LONG size);	/* sets jioerr on failure */
LONG pj_dwrite(Jfile f, void *buf, LONG size);	/* sets jioerr on failure */
Errcode pj_ddelete(char *name);
Errcode pj_drename(char *oldname, char *newname);
LONG pj_dseek(int file, LONG offset, int mode); /* if negative is Errcode */
void pj_dset_dta(Fndata *dta);
Boolean pj_dfirst(char *pattern, int attributes);
Boolean pj_dnext(void);
int pj_dset_drive(int drive); /* 0 = A: 1 = B: ... returns # of drives installed*/
int pj_dget_drive(void);	/* returns current drive.  0 = A: 1 = B: ... */
int pj_dcount_floppies(void);
Boolean pj_dis_drive(int drive);	/* drive 0 = A:  1 = B: True if drive present*/
long pj_ddfree(int drive);	/* 0 = current.  1 = A:  2 = B: etc. */
Boolean pj_dmake_dir(char *path);
Boolean _lodos_set_dir(char *path);
Boolean pj_dget_dir(int drive, char *dir);	/* 0 = current, 1 = A: for drive */
long dos_mem_free();

int get_vmode(void);
void set_vmode(int mode);
void wait_vblank(void);
void wait_novblank(void);

void dto_table(Raster *sv,Pixel *dtable,int dsize,
	int x0,int y0,int x1,int y1);
void pj_fcuncomp(void *csource, Rgb3 *cmap);

int pj_tnskip(Pixel *s1,Pixel *s2,int bcount,int mustmatch);
int pj_tnsame(Pixel *s2x,int bcount,int mustmatch);

#endif /* ASM_H */
