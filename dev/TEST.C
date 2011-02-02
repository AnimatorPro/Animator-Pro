#include "input.h"
#include "picdrive.h"
#include "lstdio.h"
#include "vertices.h"
#include "softmenu.h"
#include "aaconfig.h"
#include "rastcall.h"
#include "jimk.h"
#include "serial.h"
#include "flicel.h"
#include <math.h>


#ifndef TESTING
void test() {}
#else /* TESTING */

test()
{
/*test_font(vb.pencel, uvfont); */
}

#ifdef TEST_DMPI
#include "dpmiutil.h"

test()
{
	DPMIMemoryInfo info;
	unsigned long  bpp;
	int 		   have_dpmi;

	have_dpmi = pj_dpmi_present();
	if (have_dpmi)
		have_dpmi = pj_dpmi_inquire_memory(&info);
	if (!have_dpmi) {
		boxf("DPMI not present.\n");
	} else if (!(info.dpmi_flags & DPMIFLAG_VMM)) {
		boxf("DPMI present, but no virtual memory manager.\n");
	} else {
		bpp = info.bytes_per_page;
		boxf("largest_available_bytes          = %9ld\n"
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
#endif /* TEST_DPMI */

Errcode test_ptfunc()
{
Short_xy ends[2];

	if (!pti_input())
		return(Success);
	zoom_unundo();
	if((get_rub_line(ends)) < 0)
		return(Success);
	return(Success);
}

#ifdef PETERS
test()
{
extern UBYTE _end, _cstart_;
	printf("start - end %d", &_end - &_cstart_);
}
#endif

#ifdef TEST_SERIAL
int ser_init(int port, int ComParams);
ser_cleanup(int port);
ser_puts(int port, char *string);
ser_write(int port, char *buf, int size);
ser_read(int port, char *buf, int size);
int ser_write_left(int port);
int ser_read_left(int port);
int ser_status(int port);

test()
/* Work out serial port a little... */
{
Errcode err;
static SHORT qport = 1;
SHORT port;
SHORT wonderfuls = 1;
int i;
char buf[90];
char in,out;

if(!qreq_number(&qport,1,4,"COMM port to test?"))
	return;
port = qport-1;
if ((err = ser_init(port, BAUD_9600+PARITY_NONE+STOP_1+BITS_8)) < Success)
	goto REPORT;
if (qreq_number(&wonderfuls, 1, 100, "How many wonderfuls to send?"))
	{
	for (i=1; i<= wonderfuls; ++i)
		{
		sboxf(buf, "I'm having a wonderful day #%03d\r\n", i);
		ser_puts(port, buf);
		if ((err = ser_status(port)) < Success)
			{
			err = errline(err, "wonderful # %d\r\n", i);
			goto REPORT;
			}
		}
	}
i = 0;
boxf("About to start mini-term...");
for (;;)
	{
	if (pj_key_is())
		{
		in = pj_key_in();
		if (in == 0x1b)
			{
			if (yes_no_box("Quit serial test?"))
				goto REPORT;
			}
		ser_write(port, &in, 1);
		}
	if (ser_read_left(port) > 0)
		{
		if ((err = ser_read(port, &out, 1)) < Success)
			{
			err = errline(err, "Trouble reading");
			goto REPORT;
			}
		buf[i++] = out;
		if (out == '\n' || out == 'Z' || i >= 80)
			{
			buf[i] = 0;
			top_textf(buf);
			i = 0;
			}
		}
	}

REPORT:
	boxf("Before cleanup");
	ser_cleanup(port);
	boxf("After cleanup");
	errline(err, "Testing serial port %d", qport);
}
#endif


#endif /* TESTING */
