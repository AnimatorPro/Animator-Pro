#include <stdio.h>
#include "rastcall.h"
#include "ptrmacro.h"
#include "makehdr.c"

char fmt1[] = "%s equ +%02xH\n";

#define poset(struc,name,f) \
	outf(fmt1, name, OFFSET(struc,f))
#define roset(name,f) poset(Raster,name,f)

main(int argc,char **argv)
{
	openit(argc,argv);

	roset("R_TYPE", type);
	roset("R_PDEPTH", pdepth);
	roset("R_LIB", lib);
	roset("R_ASPCTDX", aspect_dx);
	roset("R_ASPCTDY", aspect_dy);
	roset("R_WIDTH", width);
	roset("R_HEIGHT", height);
	roset("R_X", x);
	roset("R_Y", y);
	roset("R_HDW", hw);
	fprintf(ofile,"\n");
	poset(Clipbox, "CBOX_ROOT", root);
	fprintf(ofile,"\n");

	fprintf(ofile,"RT_UNDEF equ %d\n", RT_UNDEF);
	fprintf(ofile,"RT_BITMAP equ %d\n", RT_BITMAP);
	fprintf(ofile,"RT_BYTEMAP equ %d\n", RT_BYTEMAP);
	fprintf(ofile,"RT_MCGA equ %d\n", RT_MCGA);
	fprintf(ofile,"RT_WINDOW equ %d\n", RT_WINDOW);
	fprintf(ofile,"RT_ROOTWNDO equ %d\n", RT_ROOTWNDO); 
	fprintf(ofile,"RT_NULL equ %d\n", RT_NULL);
	fprintf(ofile,"RT_CLIPBOX equ %d\n", RT_CLIPBOX);
	fprintf(ofile,"\n\n");


	fprintf(ofile,"\n\n");
	fprintf(ofile,"PHAR_SEG equ %03xh\n", PHAR_SEG);
	fprintf(ofile,"VGA_SEG equ %03xh\n", VGA_SEG);
	fprintf(ofile,"\n\n");

	fclose(ofile);
}
