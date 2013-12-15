	include evga.i


_text	segment	para public use32 'code'
	assume cs:CGROUP,ds:DGROUP

	public _evga_get_dot
_evga_get_dot proc near
vgdp struc
	vgd_off dw ?
	vgd_bank dw ?
	vgd_ebp	dd ?
	vgd_ret dd ?
	vgd_v	dd ?
	vgd_x	dd ?
	vgd_y	dd ?
vgdp ends
	push ebp
	sub esp,4
vboth equ dword ptr 0[ebp]
voff equ word ptr 0[ebp]
vbank equ word ptr 2[ebp]
	mov ebp,esp
	push ds

	mov eax,[ebp].vgd_v
	mov eax,[eax].vs_bpr
	mul [ebp].vgd_y
	add eax,[ebp].vgd_x
	mov vboth,eax	;the low 16 bits of address are for real
					;but must use the top 3 for bank switching

	mov cx,vbank	
	setbank

	mov ax,PHAR_REAL_SEG
	mov ds,ax
	movzx eax,voff
	add eax,0a0000h
	movzx eax,BYTE PTR[eax]

	pop ds
	add esp,4
	pop ebp
	ret
_evga_get_dot endp

_text	ends
	end
