include errcodes.i

code segment dword 'CODE'

cgroup group code
dgroup group data

	assume cs:cgroup,ds:dgroup

; turn off abort/retry/fail...

	public de_handler
de_handler: 
	mov 	crit_errval,di
	mov     al,3            ; "Fail" status
	iretd

	public set_de
set_de proc near
	push ecx
	push edx
	push ds

	push cs	;move code segment to ds
	pop ds
	mov cl,024H
	mov edx,offset de_handler
	mov ax,02506H
	int 21H

	pop ds
	pop edx
	pop ecx
	ret
set_de endp

code	ends

data segment dword public 'DATA'

	public crit_errval
crit_errval dw ?

data ends


	end
