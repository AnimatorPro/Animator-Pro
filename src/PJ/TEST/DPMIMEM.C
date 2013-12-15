#include <stdio.h>
#include "dpmiutil.h"

void main(void)
{
	DPMIMemoryInfo info;
	unsigned long  bpp;
	int 		   have_dpmi;

	have_dpmi = pj_dpmi_inquire_memory(&info);

	if (!have_dpmi) {
		printf("DPMI not present.\n");
	} else if (!(info.dpmi_flags & DPMIFLAG_VMM)) {
		printf("DPMI present, but no virtual memory manager.\n");
	} else {
		bpp = info.bytes_per_page;
		printf("largest_available_bytes          = %9ld\n"
			   "largest_available_unlocked_pages = %9ld (%9ld bytes)\n"
			   "largest_available_locked_pages   = %9ld (%9ld bytes)\n"
			   "linear_address_space_pages       = %9ld (%9ld bytes)\n"
			   "total_unlocked_pages             = %9ld (%9ld bytes)\n"
			   "total_free_pages                 = %9ld (%9ld bytes)\n"
			   "total_physical_pages             = %9ld (%9ld bytes)\n"
			   "linear_address_space_free_pages  = %9ld (%9ld bytes)\n"
			   "swap_file_size_pages             = %9ld (%9ld bytes)\n"
			   "bytes_per_page                   = %9ld\n"
			   "dpmi_machine                     = %02X\n"
			   "dpmi_flags                       = %02X\n"
			   "dpmi version                     = %d.%02d\n"
			   ,
			   info.largest_available_bytes,
			   info.largest_available_unlocked_pages, bpp * info.largest_available_unlocked_pages,
			   info.largest_available_locked_pages,   bpp * info.largest_available_locked_pages,
			   info.linear_address_space_pages, 	  bpp * info.linear_address_space_pages,
			   info.total_unlocked_pages,			  bpp * info.total_unlocked_pages,
			   info.total_free_pages,				  bpp * info.total_free_pages,
			   info.total_physical_pages,			  bpp * info.total_physical_pages,
			   info.linear_address_space_free_pages,  bpp * info.linear_address_space_free_pages,
			   info.swap_file_size_pages,			  bpp * info.swap_file_size_pages,
			   info.bytes_per_page,
			   info.dpmi_machine,
			   info.dpmi_flags,
			   info.dpmi_major_version,
			   info.dpmi_minor_version
			  );
	}

}
