/*****************************************************************************
 * PACKBITS.C  - Pack and Unpack Mac PackBits compression format.
 ****************************************************************************/

#include "tiff.h"

#define DUMP	0
#define RUN 	1

#define MINRUN	3
#define MAXRUN	128
#define MAXDAT	128

int packbits(char *source, char *dest, int size)
/*****************************************************************************
 * do packbits encoding on one line of data.
 ****************************************************************************/
{
	char	c;
	char	lastc	= '\0';
	int 	mode	= DUMP;
	int 	nbuf	= 0;		/* number of chars in buffer */
	int 	rstart	= 0;		/* buffer index current run starts */
	int 	putsize = 0;
	char	buf[MAXDAT*2];

	buf[0] = lastc = *source++; /* so have valid lastc */
	nbuf = 1;
	--size; 					/* since one byte eaten. */

	for(; size; --size)
		{
		buf[nbuf++] = c = *source++;
		switch (mode)
			{
			case DUMP:
			/* If the buffer is full, write the length byte, then the data */
				if (nbuf > MAXDAT)
					{
					*dest++ = nbuf-2;
					memcpy(dest, buf, nbuf-1);
					dest += nbuf-1;
					putsize += nbuf;
					buf[0] = c;
					nbuf = 1;
					rstart = 0;
					break;
					}

				if (c == lastc)
					{
					if (nbuf - rstart >= MINRUN)
						{
						if (rstart > 0)
							{
							*dest++ = rstart-1;
							memcpy(dest, buf, rstart);
							dest += rstart;
							putsize += rstart+1;
							}
						mode = RUN;
						}
					else if (rstart == 0)
						mode = RUN;
						/* no dump in progress, so can't lose by making
						   these 2 a run. */
					}
				else
					rstart = nbuf - 1;/* first of run */
				break;

			case RUN:
				if ((c != lastc) ||(nbuf - rstart > MAXRUN))
					{
					*dest++ = -(nbuf - rstart - 2);
					*dest++ = lastc;
					putsize += 2;
					buf[0] = c;
					nbuf = 1;
					rstart = 0;
					mode = DUMP;
					}
				break;
			}
		lastc = c;
		}

	switch (mode)
		{
		case DUMP:
			*dest++ = nbuf-1;
			memcpy(dest, buf, nbuf);
			putsize += nbuf+1;
			break;
		case RUN:
			*dest++ = -(nbuf - rstart - 1);
			*dest = lastc;
			putsize += 2;
			break;
		}

	return putsize;
}

char *unpackbits(char *sourcep, char *destp, int destcount)
/*****************************************************************************
 * Unpack data from source buffer to dest buffer until destcount bytes done.
 ****************************************************************************/
{

	int runlength;

	while (destcount > 0)
		{
		runlength = *sourcep++;
		if (runlength == -128)
			continue;
		else if (runlength < 0)
			{
			runlength = (-runlength) + 1;
			if (runlength > destcount)
				return NULL;
			memset(destp, *sourcep, runlength);
			sourcep += 1;
			destp += runlength;
			}
		else
			{
			++runlength;
			if (runlength > destcount)
				return NULL;
			memcpy(destp, sourcep, runlength);
			sourcep += runlength;
			destp += runlength;
			}
		destcount -= runlength;
		}

ERROR_EXIT:

	return (destcount == 0) ? sourcep : NULL;
}
