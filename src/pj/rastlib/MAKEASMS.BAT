rem single raster jumps with raster as first argument on stack 
rem  -------- pubname libname outname(.asm)

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
