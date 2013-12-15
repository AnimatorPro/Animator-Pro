#include "serial.h"
#include "errcodes.h"
#include <stdio.h>

print_stat(int stat)
{
printf("Error code %d\n", stat);
}

test_time()
{
long itime = pj_clock_1000();
long time;
int i;

for (i=0; i<10000; ++i)
	time = pj_clock_1000();
printf("Elapsed time for 10000 clocks %ld\n", time - itime);
}

int ser_read(int port, char *buf, int size)
{
long end_time = pj_clock_1000() + 500 + size*10;	/* Calc timeout - 1/2 sec
											         * plus 1/100 for each char 													 * to read. */
int c;

while (size > 0)
	{
	if ((c = ser_get_char(port)) >= 0)
		{
		*buf++ = c;
		--size;
		}
	else
		{
		if ((c = ser_status(port)) < 0)
			return(c);	/* Line dropped or something */
		if (pj_clock_1000() > end_time)
			return(Err_timeout);
		}
	}
return(Success);
}

test(int argc, char *argv)
{
static char start[] = "Start serial communications\r\n";
int stat;
char buf[80];
int bsize;
int i = 0;
char user_key;
int port = 0;

pj_clock_init();
pj_set_gs();
test_time();
printf("[0x400] = %x %x %x %x\n", real_peek(0x400),
	real_peek(0x402), real_peek(0x404), real_peek(0x406));
ser_set_mode(port, BAUD_9600+PARITY_NONE+STOP_1+BITS_8);
if ((stat = ser_status(port)) < Success)
	goto DONE;
print_stat(stat);
stat = ser_write(port, start, sizeof(start));
if (stat < Success)
	goto DONE;
if ((stat = ser_status(port)) < Success)
	goto DONE;
print_stat(stat);
for (;;)
	{
	if (pj_key_is())
		{
		user_key = pj_key_in();
		if (user_key == 0x1b)	/* Escape?? */
			goto DONE;
		if ((stat = ser_write(port, &user_key, 1)) < Success)
			goto DONE;
		}
	if ((stat = ser_status(port)) == 1)
		{
		puts("reading");
		buf[2] = 0;
		stat = ser_read(port,buf,2);
		if (stat < Success)
			goto DONE;
		putchar(buf[0]);
		putchar(buf[1]);
		fflush(stdout);
		}
	}
DONE:
print_stat(stat);
pj_clock_cleanup();
}
