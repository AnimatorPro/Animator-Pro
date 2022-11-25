/*****************************************************************************
 * DPMIUTIL.H - A couple util functions to get info about DPMI memory.
 ****************************************************************************/

#ifndef DPMIUTIL_H
#define DPMIUTIL_H

typedef struct dpmi_memory_info {
	unsigned long largest_available_bytes;
	unsigned long largest_available_unlocked_pages;
	unsigned long largest_available_locked_pages;
	unsigned long linear_address_space_pages;
	unsigned long total_unlocked_pages;
	unsigned long total_free_pages;
	unsigned long total_physical_pages;
	unsigned long linear_address_space_free_pages;
	unsigned long swap_file_size_pages;
	unsigned long dpmi_0500h_reserved[3];
	unsigned long bytes_per_page;
	unsigned char dpmi_machine;
	unsigned char dpmi_flags;
	unsigned char dpmi_minor_version;
	unsigned char dpmi_major_version;
} DPMIMemoryInfo;

enum {
	DPMIFLAG_386	 = 0x0001,
	DPMIFLAG_REALINT = 0x0002,
	DPMIFLAG_VMM	 = 0x0004
};

extern Boolean pj_dpmi_present(void);
extern unsigned long pj_dpmi_inquire_version(void);
extern int			 pj_dpmi_inquire_memory(DPMIMemoryInfo *pinfo);

#endif
