/*****************************************************************************
 * FASTCLK.C - Bogus fastclk module for POE fliclib usage.
 *
 *	POE modules get pj_clock_1000() already through the SYSLIB host library,
 *	but we need do-nothing clock init/cleanup routines.
 ****************************************************************************/


int pj_clock_init(void)
{
	return 1; /* return TRUE */
}

void pj_clock_cleanup(void)
{
	return;
}

