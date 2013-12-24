
include rastlib.i
include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


DO_BLITJUMP MACRO PUBNAME,CLIPCALL
 	EXTRN CLIPCALL:WORD
	public PUBNAME 
PUBNAME proc near
	mov edx, offset CLIPCALL
	jmp short _cbox_blit_jump_edx
PUBNAME endp
	ENDM

DO_BLITJUMP pj_sclip_get_dot pj_get_dot
DO_BLITJUMP pj_sclip_blitrect pj_blitrect 
;DO_BLITJUMP sclip_swaprect pj_swaprect 
;DO_BLITJUMP sclip_tblitrect pj_tblitrect 
;DO_BLITJUMP sclip_zoomblit pj_zoomblit 

_cbox_blit_jump_edx:
	mov		eax, dword ptr [esp].blitarg_src ; eax = stack raster (clip box)
 	movsx   ecx, [eax].rast_x 	 ; ecx = r->x
 	add		[esp].blitarg_sx, ecx   ; arg_x += ecx
 	movsx   ecx, [eax].rast_y 	 ; ecx = r->y
 	add		[esp].blitarg_sy, ecx   ; arg_y += ecx
    mov     eax,dword ptr CBOX_ROOT[eax] ; eax = cbox->root
	mov 	dword ptr [esp].blitarg_src, eax ; r = eax
    jmp    	edx    		; jump edx


DO_BXYJUMP MACRO PUBNAME,CLIPCALL
 	EXTRN CLIPCALL:WORD
	public PUBNAME 
PUBNAME proc near
	mov edx, offset CLIPCALL
	jmp short _cbox_bxy_jump_edx
PUBNAME endp
	ENDM

DO_BXYJUMP pj_sclip_put_dot pj_put_dot
DO_BXYJUMP pj_sclip_put_hseg pj_put_hseg 
DO_BXYJUMP pj_sclip_put_vseg pj_put_vseg 
DO_BXYJUMP pj_sclip_set_hline pj_set_hline 
DO_BXYJUMP pj_sclip_put_rectpix _pj_put_rectpix 
DO_BXYJUMP pj_sclip_set_vline pj_set_vline 
DO_BXYJUMP pj_sclip_set_rect pj_set_rect 
DO_BXYJUMP pj_sclip_xor_rect pj_xor_rect 

_cbox_bxy_jump_edx:
	mov		eax, dword ptr [esp].bufxyarg_r ; eax = stack raster (clip box)
 	movsx   ecx, [eax].rast_x 	   ; ecx = r->x
 	add		[esp].bufxyarg_x, ecx   ; arg_x += ecx
 	movsx   ecx, [eax].rast_y 	   ; ecx = r->y
 	add		[esp].bufxyarg_y, ecx   ; arg_y += ecx
    mov     eax,dword ptr CBOX_ROOT[eax] ; eax = cbox->root
	mov 	dword ptr [esp].bufxyarg_r, eax ; r = eax
	jmp 	edx



DO_MASKJUMP MACRO PUBNAME,CLIPCALL
 	EXTRN CLIPCALL:WORD
	public PUBNAME 
PUBNAME proc near
	mov edx, offset CLIPCALL
	jmp short _cbox_mask_jump_edx
PUBNAME endp
	ENDM

DO_MASKJUMP pj_sclip_mask1blit pj_mask1blit
DO_MASKJUMP pj_sclip_mask2blit pj_mask2blit

_cbox_mask_jump_edx:
	mov		eax, dword ptr [esp].maskarg_r ; eax = stack raster (clip box)
 	movsx   ecx, [eax].rast_x 	   ; ecx = r->x
 	add		[esp].maskarg_x, ecx   ; arg_x += ecx
 	movsx   ecx, [eax].rast_y 	   ; ecx = r->y
 	add		[esp].maskarg_y, ecx   ; arg_y += ecx
    mov     eax,dword ptr CBOX_ROOT[eax] ; eax = cbox->root
	mov 	dword ptr [esp].maskarg_r, eax ; r = eax
	jmp 	edx


code	ends
	end

