#include "pjbasics.h"

ULONG jiffies_to_millisec(USHORT jiffies)
{
	return(pj_uscale_by((int)jiffies,1000,70));
}
USHORT millisec_to_jiffies(ULONG millis)
{
	return(((((long)millis) * 70L) + 500L) / 1000L);
}
