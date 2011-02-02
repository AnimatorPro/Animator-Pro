	include raster.i
	include rastlib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


	public _grc_cput_dot

pdot_args struc
	pdot_ret dd ? 
	pdot_rast dd ? 
	pdot_color dd ?
	pdot_x dd ?
	pdot_y dd ?
pdot_args ends

_grc_cput_dot proc near
    mov eax,dword ptr [esp].pdot_rast
    movzx   ecx,word ptr [eax].rast_width	; (ULONG)ecx = w->width    
    cmp     ecx,dword ptr [esp].pdot_x   	; if(ecx <= pdot_x 
	jbe		grc_pdot_ret				 	; goto ret
    movzx   ecx,word ptr [eax].rast_height
    cmp     ecx,dword ptr [esp].pdot_y
	jbe		grc_pdot_ret				 	; goto ret
	mov eax,dword ptr [eax].rast_lib 
	jmp dword ptr RL_PUT_DOT[eax]
grc_pdot_ret:
	ret
_grc_cput_dot endp


	public _grc_cget_dot

gdot_args struc
	gdot_ret dd ? 
	gdot_rast dd ? 
	gdot_x dd ?
	gdot_y dd ?
gdot_args ends

_grc_cget_dot proc near
    mov eax,dword ptr [esp].gdot_rast
    movzx   ecx,word ptr [eax].rast_width	; (ULONG)ecx = w->width    
    cmp     ecx,dword ptr [esp].gdot_x   	; if(ecx <= pdot_x) 
	jbe		grc_gdot_ret				 	; goto ret
    movzx   ecx,word ptr [eax].rast_height
    cmp     ecx,dword ptr [esp].gdot_y
	jbe		grc_gdot_ret				 	; goto ret
	mov eax,dword ptr [eax].rast_lib 
	jmp dword ptr RL_PUT_DOT[eax]
grc_gdot_ret:
	xor eax,eax ; return 0 if clipped out
	ret
_grc_cget_dot endp

code	ends
	end
