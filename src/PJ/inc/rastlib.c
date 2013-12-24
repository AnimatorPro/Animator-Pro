#include "rastlib.h"
#include "makehdr.c"

main(int argc, char **argv)
{
#define rloset(name,field) soset(Rastlib,name,field)

	openit(argc,argv);
	outf("RL_TO_SAME equ %d\n", 4*RL_TO_SAME );
	outf("RL_TO_BMAP equ %d\n", 4*RL_TO_BYTEMAP );
	outf("RL_FROM_BMAP equ %d\n", 4*RL_FROM_BYTEMAP );
	outf("RL_TO_OTHER equ %d\n", 4*RL_TO_OTHER );
	outf("\n");
	outf("RL_NUM_LIB_CALLS equ %d\n", NUM_LIB_CALLS);
	outf("\n");
	rloset(RL_CLOSE, close_raster);
	rloset(RL_CPUT_DOT, cput_dot);
	rloset(RL_PUT_DOT, put_dot);
	rloset(RL_CGET_DOT, cget_dot);
	rloset(RL_GET_DOT, get_dot);
	rloset(RL_PUT_HSEG, put_hseg);
	rloset(RL_GET_HSEG, get_hseg);
	rloset(RL_PUT_VSEG, put_vseg);
	rloset(RL_GET_VSEG, get_vseg);
	outf("\n");
	rloset(RL_PUT_RPIX, put_rectpix);
	rloset(RL_GET_RPIX, get_rectpix);
	outf("\n");
	rloset(RL_SET_HLINE, set_hline);
	rloset(RL_SET_VLINE, set_vline);
	rloset(RL_SET_RECT, set_rect);
	rloset(RL_SET_RAST, set_rast);
	rloset(RL_XOR_RECT, xor_rect);
	rloset(RL_MASK1BLIT, mask1blit);
	rloset(RL_MASK2BLIT, mask2blit);
	outf("\n");
	rloset(RL_DTO_PTAB, diag_to_ptable);
	outf("\n");
	rloset(RL_UNBRUN, unbrun_rect);
	rloset(RL_UNLCCOMP, unlccomp_rect);
	rloset(RL_UNSS2RECT, unss2_rect);
	outf("\n\n");
	rloset(RL_BLITRECT, blitrect);
	rloset(RL_SWAPRECT, swaprect);
	rloset(RL_TBLITRECT, tblitrect);
	rloset(RL_XOR_RAST, xor_rast);
	rloset(RL_ZOOMBLIT, zoomblit);
	outf("\n\n");
	rloset(RL_SET_COLORS, set_colors);
	rloset(RL_UNCC64, uncc64);
	rloset(RL_UNCC256, uncc256);
	outf("\n\n");
	rloset(RL_WAIT_VSYNC, wait_vsync);
	closeit();
}
