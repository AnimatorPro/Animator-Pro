#include "dfile.ih"

long pj_file_size(char *title)
{
long size;
int doshand;
Doserr derr;

	if((derr = pj_dopen(&doshand,title,JREADONLY)) != Dsuccess)
		return((long)(jerr = pj_mserror(derr)));

	size = pj_dseek(doshand, 0, JSEEK_END);
	pj_dclose(doshand);
	if(size < 0)
		jerr = size;
	return(size);
}
