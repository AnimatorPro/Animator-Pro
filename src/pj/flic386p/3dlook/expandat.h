

void gentle_free(void *pt);
Errcode read_alloc_buf(char *filename, char **pbuf, int *psize);
Errcode expand_ats(int *pargc, char *(*pargv[]));
Errcode report_error(Errcode err, char *format, ...);

