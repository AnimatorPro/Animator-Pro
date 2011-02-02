	include vram.i


_text	segment	para public use32 'code'
	assume cs:CGROUP,ds:DGROUP

	public _vram_put_dot
_vram_put_dot proc near
vpdp struc
	vpd_off dw ?
	vpd_bank dw ?
	vpd_ebp	dd ?
	vpd_ret dd ?
	vpd_v	dd ?
	vpd_color	dd ?
	vpd_x	dd ?
	vpd_y	dd ?
vpdp ends
	push ebp
	sub esp,4
vboth equ dword ptr 0[ebp]
voff equ word ptr 0[ebp]
vbank equ word ptr 2[ebp]
	mov ebp,esp
	push ds

	mov eax,[ebp].vpd_v
	mov eax,[eax].vs_bpr
	mul [ebp].vpd_y
	add eax,[ebp].vpd_x
	mov vboth,eax	;the low 16 bits of address are for real
					;but must use the top 3 for bank switching

	mov cx,vbank	
	setbank

	mov edx,[ebp].vpd_color
	mov ax,PHAR_REAL_SEG
	mov ds,ax
	movzx eax,voff
	add eax,0a0000h
	mov [eax],dl

	pop ds
	add esp,4
	pop ebp
	ret
_vram_put_dot endp
_text	ends
	end
