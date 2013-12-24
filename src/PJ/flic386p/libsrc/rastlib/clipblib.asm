include rastlib.i
include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

; ******************* put dot ************************

	public pj_cbox_put_dot
	public pj__cbox_put_dot

pj_cbox_put_dot proc near

; clip here 

	mov		eax, dword ptr [esp].bufxyarg_r ; eax = stack raster (clip box)
    movzx   ecx,[eax].rast_width	 ; (ULONG)ecx = w->width    
    cmp     ecx,dword ptr [esp].bufxyarg_x  ; if(ecx <= bufxyarg_x 
	jbe		cbox_pdot_ret				 ; goto ret
    movzx   ecx,[eax].rast_height
    cmp     ecx,dword ptr [esp].bufxyarg_y
	ja		cbox_pdot
cbox_pdot_ret:
	ret

pj__cbox_put_dot proc near
	mov		eax, dword ptr [esp].bufxyarg_r ; eax = stack clip box
cbox_pdot:

	movsx   ecx, [eax].rast_x 		 ; cx = r->x
	add		[esp].bufxyarg_x, ecx   ; arg_x += cx
	movsx   ecx, [eax].rast_y 		 ; cx = r->y
	add		[esp].bufxyarg_y, ecx   ; arg_y += cx
    mov     eax,dword ptr CBOX_ROOT[eax] ; eax = cbox->root
	mov 	dword ptr [esp].bufxyarg_r, eax ; r = eax
   	mov     eax,[eax].rast_lib     ; eax = r->lib
    jmp    	dword ptr RL_PUT_DOT[eax]      ; jump eax[_PUT_DOT]

pj__cbox_put_dot endp
pj_cbox_put_dot endp

; ******************* get dot ************************

	public pj_cbox_get_dot
	public pj__cbox_get_dot

pj_cbox_get_dot proc near

; clip here 

	mov		eax, dword ptr [esp].blitarg_src ; eax = stack raster 
    movzx   ecx,[eax].rast_width	 		; (ULONG)ecx = r->width    
    cmp     ecx,dword ptr [esp].blitarg_sx  ; if(ecx <= blitarg_sx 
	jbe		cbox_gdot_ret					; goto ret
    movzx   ecx,[eax].rast_height
    cmp     ecx,dword ptr [esp].blitarg_sy
	ja		cbox_gdot
cbox_gdot_ret:
	ret

pj__cbox_get_dot proc near
	mov		eax, dword ptr [esp].blitarg_src ; eax = stack cbox
cbox_gdot:

	movsx   ecx, [eax].rast_x 		; ecx = r->x
	add		[esp].blitarg_sx, ecx 	; arg_x += ecx
	movsx   ecx, [eax].rast_y 	 	; ecx = r->y
	add		[esp].blitarg_sy, ecx   	; arg_y += ecx

    mov     eax,dword ptr CBOX_ROOT[eax]  ; r = cbox->root
	mov 	dword ptr [esp].blitarg_src, eax  ; r = eax
   	mov     eax,dword ptr [eax].rast_lib      ; eax = r->lib
    jmp    	dword ptr RL_GET_DOT[eax]       ; jump eax[GET_DOT]

pj__cbox_get_dot endp
pj_cbox_get_dot endp

BXY_JUMP MACRO PUBNAME,LIBCALL
	public PUBNAME 
PUBNAME proc near
	mov		edx, LIBCALL
	jmp short _cbox_bxy_jump_edx
PUBNAME endp
	ENDM

BXY_JUMP pj__cbox_put_hseg,RL_PUT_HSEG
BXY_JUMP pj__cbox_get_hseg,RL_GET_HSEG
BXY_JUMP pj__cbox_put_vseg,RL_PUT_VSEG
BXY_JUMP pj__cbox_get_vseg,RL_GET_VSEG
BXY_JUMP pj__cbox_set_hline,RL_SET_HLINE
BXY_JUMP pj__cbox_set_vline,RL_SET_VLINE
BXY_JUMP pj__cbox_get_rectpix,RL_GET_RPIX
BXY_JUMP pj__cbox_put_rectpix,RL_PUT_RPIX
BXY_JUMP pj__cbox_set_rect,RL_SET_RECT
BXY_JUMP pj__cbox_xor_rect,RL_XOR_RECT

_cbox_bxy_jump_edx:
	mov		eax, dword ptr [esp].bufxyarg_r ; eax = stack cbox
	movsx   ecx, [eax].rast_x 		 ; cx = r->x
	add		[esp].bufxyarg_x, ecx   ; arg_x += cx
	movsx   ecx, [eax].rast_y 		 ; cx = r->y
	add		[esp].bufxyarg_y, ecx   ; arg_y += cx

    mov     eax,dword ptr CBOX_ROOT[eax]  ; r = cbox->root
	mov 	dword ptr [esp].bufxyarg_r, eax  ; r = eax
   	mov     eax,dword ptr [eax].rast_lib    ; eax = r->lib
	add		eax,edx
    jmp    	dword ptr [eax]    		; jump eax[LIBCALL]


MASK_JUMP MACRO PUBNAME,LIBCALL
	public PUBNAME 
PUBNAME proc near
	mov		edx, LIBCALL
	jmp short _cbox_mask_jump_edx
PUBNAME endp
	ENDM

MASK_JUMP pj__cbox_mask1blit,RL_MASK1BLIT
MASK_JUMP pj__cbox_mask2blit,RL_MASK2BLIT

_cbox_mask_jump_edx:
	mov		eax, dword ptr [esp].maskarg_r ; eax = stack cbox
	movsx   ecx, [eax].rast_x 		 ; cx = r->x
	add		[esp].maskarg_x, ecx   ; arg_x += cx
	movsx   ecx, [eax].rast_y 		 ; cx = r->y
	add		[esp].maskarg_y, ecx   ; arg_y += cx

    mov     eax,dword ptr CBOX_ROOT[eax]  ; r = cbox->root
	mov 	dword ptr [esp].maskarg_r, eax  ; r = eax
   	mov     eax,dword ptr [eax].rast_lib    ; eax = r->lib
	add		eax,edx
    jmp    	dword ptr [eax]    		; jump eax[LIBCALL]


code	ends
	end
