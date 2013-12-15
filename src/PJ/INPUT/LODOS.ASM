
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i
include abcdregs.i



MOUSEINT equ 51

	public jgot_mouse
;Boolean ismouse(void);	/* returns if mouse is installed */  
jgot_mouse proc near
	push ds
	push ebx
	mov ax,34h	;Phar Lap's ms-dos segment
	mov ds,ax

	; make sure there's an interrupt vector for Micky Microsoft.
 	mov eax,MOUSEINT*4
	mov eax,[eax]
	or eax,eax
	jz ism_bad

	; then call Micky driver with initialization code.
	xor ax,ax
	int MOUSEINT
	cmp ax,0ffffh	; good init?
	jnz ism_bad

	mov eax,1
	jmp ism_end

ism_bad:
	xor eax,eax
ism_end:

	pop ebx
	pop ds
	ret
jgot_mouse endp


	public jmousey
; jmousey(USHORT *abcd_regs); /* parameter pointer to 4 short array of regs */
jmousey proc near
jmop	struc
	jmo_edi dd ?
	jmo_esi dd ?
	jmo_edx dd ?
	jmo_ecx dd ?
	jmo_ebx dd ?
	jmo_ret dd ?
	jmo_regs dd ?
jmop ends
	push ebx
	push ecx
	push edx
	push esi
	push edi

	mov esi,[esp].jmo_regs
	mov ax,[esi].abcd_ax
	mov bx,[esi].abcd_bx
	mov cx,[esi].abcd_cx
	mov dx,[esi].abcd_dx
	int MOUSEINT
	mov [esi].abcd_ax,ax
	mov [esi].abcd_bx,bx
	mov [esi].abcd_cx,cx
	mov [esi].abcd_dx,dx

	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	ret
jmousey endp

code	ends
	end

