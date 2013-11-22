#ifdef UNUSED  /* But interesting */
#include "jimk.h"

typedef struct vip_head
	{
	UWORD vip_magic;
	Vector init_vip_device;
	Vector vip_input;
	Vector close_vip_device;
	char reserved[32];
	} Vip_head;

typedef struct exe_head
	{
	char M,Z;
	UWORD mod512;
	UWORD blocks;
	UWORD reloc_count;
	UWORD head_size;
	UWORD min_data;
	UWORD max_data;
	void *istack;
	UWORD checksum;
	Vector entry_point;
	UWORD reloc_list;
	UWORD overlay;
	UWORD one;
	} Exe_head;

typedef struct vlib
	{
	struct exe_head eh;
	struct vip_head vh;
	void *alloced;
	} Vlib;

#define VIP_MAGIC 0xA033
#define DVIP_MAGIC 0x2231


test_exe_head(int file, Exe_head *exe)
{
if (jread(file, exe, sizeof(*exe)) != sizeof(*exe) )
	return(0);
if (exe->M != 'M' || exe->Z != 'Z' || exe->one != 1)
	return(0);
return(1);
}

verify_exe_head(int file, Exe_head *exe)
{
if (!test_exe_head(file, exe))
	{
	continu_line("Not a good tablet driver.");
	return(0);
	}
return(1);
}

check_exe(char *name)
{
int file;
Vlib ev;
int ok;

if ((file = jopen(name, 0)) == 0)
	{
	cant_find(name);
	return(0);
	}
ok = verify_exe_head(file, &ev.eh);
jclose(file);
return(ok);
}

void *load_exe(char *filename, Exe_head *eh)
{
long retval;
char *alligned_buf;
char *alloc_buf = NULL;
long (*rfunc)();
unsigned long code_offset;
unsigned long init_size;
unsigned long bss_size;
unsigned long total_size;
void *v;
unsigned long fixup_offset;
UWORD fixup[2];
UWORD *segpt;
UWORD code_seg;
int f;
unsigned i;
int ok = 0;

if ((f = jopen(filename, JREADONLY)) == 0)
	{
	cant_find(filename);
	return(NULL);
	}
if (!verify_exe_head(f, eh))
	goto OUT;
code_offset = eh->head_size;
code_offset *= 16;	/* make it a paragraph */
init_size = eh->blocks;
init_size *= 512;
init_size += eh->mod512;
if (eh->mod512 != 0)
	init_size -= 512;
init_size -= code_offset;
bss_size = eh->min_data;
bss_size *= 16;
total_size = init_size + bss_size;
if ((alloc_buf = begmem((unsigned)total_size+16)) == NULL)
	goto OUT;
code_seg = ptr_seg(alloc_buf) + 1;
alligned_buf = make_ptr(0, code_seg);
zero_structure(alligned_buf, (unsigned)total_size);
jseek(f, code_offset, JSEEK_START);
if (jread(f, alligned_buf, init_size) < init_size)
	{
	truncated(filename);
	goto OUT;
	}
v = alligned_buf;
eh->entry_point = v;
if (eh->reloc_count > 0)
	{
	fixup_offset = eh->reloc_list;
	jseek(f, fixup_offset, JSEEK_START);
	for (i=0; i<eh->reloc_count; i++)
		{
		if (jread(f, fixup, sizeof(fixup)) != sizeof(fixup))
			{
			truncated(filename);
			goto OUT;
			}
		segpt = make_ptr(fixup[0], code_seg + fixup[1]);
		segpt[0] += code_seg;
		}
	}
ok = 1;
OUT:
if (!ok)
	{
	gentle_freemem(alloc_buf);
	alloc_buf = NULL;
	}
jclose(f);
return(alloc_buf);
}

static Vlib vdh;

extern continu_box(char **lines);

init_vip(char *name)
{
if (name == NULL || name[0] == 0)
	return(0);
if ((vdh.alloced = load_exe(name, &vdh.eh)) != NULL)
	{
	zero_structure(&vdh.vh, sizeof(vdh.vh) );
	vdh.vh.vip_magic = DVIP_MAGIC;
	(*vdh.eh.entry_point)(&vdh.vh, continu_box);
	if (vdh.vh.vip_magic != VIP_MAGIC)
		{
		continu_line("Bad tablet driver protocol.");
		freemem(vdh.alloced);
		return(0);
		}
	if (!(*vdh.vh.init_vip_device)(&vdh.vh, continu_box))
		{
		char *bufs[3];

		bufs[0] = name;
		bufs[1] = "Tablet not attatched";
		bufs[2] = NULL;
		continu_box(bufs);
		freemem(vdh.alloced);
		return(0);
		}
	return(1);
	}
}


vip_get_input()
{
if (vdh.vh.vip_input != NULL)
	(*vdh.vh.vip_input)(&vdh.vh, &aa_vip);
}

cleanup_vip()
{
if (vdh.vh.close_vip_device != NULL)
	(*vdh.vh.close_vip_device)(&vdh.vh);
gentle_freemem(vdh.alloced);
vdh.alloced = NULL;
}

#endif /* UNUSED */
