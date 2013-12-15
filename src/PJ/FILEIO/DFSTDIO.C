#include "dfile.ih"

static struct jfl _jstdout = 
	{
	JFL_MAGIC,
	JWRITEONLY,
	NULL,
	0,
	};
static struct jfl _jstderr = 
	{
	JFL_MAGIC,
	JWRITEONLY,
	NULL,
	2,
	};

void *get_jstdout()
{
	return(&_jstdout);
}
void *get_jstderr()
{
	return(&_jstderr);
}
