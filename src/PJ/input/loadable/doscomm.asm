;; DOSCOMM.ASM - this countains a routine to call the DOS communications
;; software interrupt.

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i
include abcdregs.i

	public jcomm
; jcomm(ULONG *abcd_regs); /* parameter pointer to 4 array of regs */
jcomm proc near
jcop	struc
	jco_esi dd ?
	jco_edx dd ?
	jco_ecx dd ?
	jco_ebx dd ?
	jco_ret dd ?
	jco_regs dd ?
jcop ends
	push ebx
	push ecx
	push edx
	push esi

	mov esi,[esp].jco_regs
	mov ax,[esi].abcd_ax
	mov bx,[esi].abcd_bx
	mov cx,[esi].abcd_cx
	mov dx,[esi].abcd_dx
	int 14h
	mov [esi].abcd_ax,ax
	mov [esi].abcd_bx,bx
	mov [esi].abcd_cx,cx
	mov [esi].abcd_dx,dx

	pop esi
	pop edx
	pop ecx
	pop ebx
	ret
jcomm endp

code	ends
	end

