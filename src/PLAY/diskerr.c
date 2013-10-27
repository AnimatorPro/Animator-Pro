
/* Diskerr.c - Tell DOS not to put up retry/abort/fail message if
   can't read floppy. */


de_handler(errval, ax, bp, si)
int errval, ax, bp, si;
{
return(0);
}

init_de()
{
harderr(de_handler);
}

