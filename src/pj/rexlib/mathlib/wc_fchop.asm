CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

; Floating point support routine for watcom c compiler a clone of the one
; They have in their math386s.lib This is used to truncate floats to 32 bit
; ints the rounding control bits are passed in in EAX and the control word is
; loaded in eax on exit.

	PUBLIC  __CHP
__CHP:          push    eax
	push    ebp
	mov     ebp,esp
	push    eax
	fstcw   word ptr -2H[ebp]
	wait    
	mov     ax,word ptr -2H[ebp]
	or      word ptr -2H[ebp],0c00H
	fldcw   word ptr -2H[ebp]
	frndint 
	mov     word ptr -2H[ebp],ax
	fldcw   word ptr -2H[ebp]
	wait    
	mov     esp,ebp
	pop     ebp
	pop     eax
	ret     
code ENDS
	END

