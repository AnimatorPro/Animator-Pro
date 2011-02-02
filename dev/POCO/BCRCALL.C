/*
 * this is a dummy module used when compiling with TC/BC.
 * it takes the place of runccall.asm, which is used only in
 * protected mode code.  a rule in makefile.tc compiles this
 * module into a dummy runccall.obj, just to keep the make
 * and link rules happy.
 */
