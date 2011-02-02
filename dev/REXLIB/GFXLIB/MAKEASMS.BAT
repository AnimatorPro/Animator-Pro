rem routines in _a_a_gfxlib
rem          symbol, name(.asm) If no name symbol(.asm) 

call gfxcall pj_clipbox_make makecbox pj_clipbox_make
call gfxcall close_raster closeras pj_close_raster
call gfxcall pj_rast_free freerast pj_rast_free
call gfxcall put_hseg put_hseg pj_put_hseg
call gfxcall get_hseg get_hseg pj_get_hseg
call gfxcall put_vseg put_vseg pj_put_vseg
call gfxcall get_vseg get_vseg pj_get_vseg
call gfxcall put_rectpix putrpix pj_put_rectpix
call gfxcall get_rectpix getrpix pj_get_rectpix
call gfxcall set_hline sethline pj_set_hline
call gfxcall set_vline setvline pj_set_vline
call gfxcall pj_clear_rast clearas pj_clear_rast
call gfxcall set_rect set_rect pj_set_rect
call gfxcall xor_rect xor_rect pj_xor_rect
call gfxcall mask1blit mask1bli pj_mask1blit
call gfxcall mask2blit mask2bli pj_mask2blit
call gfxcall xor_rast xor_rast pj_xor_rast
call gfxcall _blitrect _blitrec pj__blitrect
call gfxcall blitrect blitrect pj_blitrect
call gfxcall _tblitrect _tblirec pj__tblitrect
call gfxcall tblitrect tblirec pj_tblitrect
call gfxcall _swaprect _swaprec pj__swaprect
call gfxcall swaprect swaprect pj_swaprect
call gfxcall zoomblit zoomblit pj_zoomblit

call gfxcall pj_cmap_alloc cmallocc
call gfxcall pj_cmap_free  cmapfree
call gfxcall pj_cmap_load  cmapload
call gfxcall pj_cmap_copy  cmapcopy

call gfxcall pj_rcel_bytemap_open opnbymrc
call gfxcall pj_rcel_bytemap_alloc getbymrc
call gfxcall pj_rcel_make_virtual virtuarc

call gfxcall pj_rcel_close closrcel
call gfxcall pj_rcel_free  freercel

rem /* these routines jump to the raster librarys and don't need _a_a_gfxlib
rem
rem single raster jumps with raster as first argument on stack 
rem  -------- pubname libname outname(.asm) */

call rast1jmp pj__get_dot		RL_GET_DOT		rc_getdo
call rast1jmp pj__get_hseg		RL_GET_HSEG 	rc_ghseg
call rast1jmp pj__get_rectpix	RL_GET_RPIX		rc_grpix	
call rast1jmp pj__get_vseg		RL_GET_VSEG     rc_gvseg
call rast1jmp pj__put_dot		RL_PUT_DOT		rc_pdot
call rast1jmp pj__put_hseg		RL_PUT_HSEG		rc_phseg
call rast1jmp pj__put_rectpix	RL_PUT_RPIX		rc_prpix
call rast1jmp pj__put_vseg		RL_PUT_VSEG		rc_pvseg
call rast1jmp pj__set_hline		RL_SET_HLINE 	rc_shlin
call rast1jmp pj__set_rect		RL_SET_RECT 	rc_setre
call rast1jmp pj__set_vline		RL_SET_VLINE  	rc_svlin
call rast1jmp pj__xor_rect		RL_XOR_RECT		rc_xorec
call rast1jmp pj_get_dot		RL_CGET_DOT		rcgetdot
call rast1jmp pj_put_dot		RL_CPUT_DOT		rcputdot
call rast1jmp pj_set_colors		RL_SET_COLORS	rcsetcol
call rast1jmp pj_set_rast		RL_SET_RAST		rcsetras
call rast1jmp pj_unbrun_rect	RL_UNBRUN 		rcunbrun
call rast1jmp pj_uncc256		RL_UNCC256		rcset256
call rast1jmp pj_uncc64			RL_UNCC64		rcset64
call rast1jmp pj_unlccomp_rect	RL_UNLCCOMP 	rculccmp
call rast1jmp pj_unss2_rect		RL_UNSS2RECT 	rcunss2
call rast1jmp pj_wait_rast_vsync	RL_WAIT_VSYNC 	rcwaitvs
call rmaskjmp pj__mask1blit		RL_MASK1BLIT	rc_msk1b
call rmaskjmp pj__mask2blit		RL_MASK2BLIT	rc_msk2b

