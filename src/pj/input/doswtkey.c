#include "stdtypes.h"

extern SHORT pj_key_in();
Boolean pj_key_is();

SHORT dos_wait_key()

/* waits for key exclusive of all else going on makes sure it waits 
 * by clearing buffer first */
{
int cleared = 0;
SHORT keyin;

	for(;;)
	{
		while(pj_key_is())
		{
			keyin = pj_key_in();
			if(cleared)
				return(keyin);
		}
		cleared = 1;
	}
}
