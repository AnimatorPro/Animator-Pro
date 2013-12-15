	include widget.i
	include errcodes.i

	EXTRN widge_check:word
	EXTRN widgelib:word
	EXTRN copy_free_code:word

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public	widge_bracket

; This will save the stack in ebp, 
; call the widge check function in the rex module
; then jump to the library vector on the host side that will 
; both free the rex code and jump to the vector passed in from the
; host side to both free the code area and to jump to the "bvector"
; returned by the check widget call and passed on the stack to the
; free call
;
; It is intended to jump to the code in selffree.asm which is linked with
; the "host" side code

widge_bracket proc near	 ; This will be linked as a .REX file.

; set exit code to return err widget

; **** call the widget checker. It return the address to jump to from 
; **** init bracket

	call near ptr widge_check 	; bvector = widge_check()

; **** copy self_freeing exit code into the externally provided buffer

	push eax					  ; widge check returns address to "bracket"
	push offset bracket_exit_end
	push offset bracket_exit_code
	call near ptr copy_free_code  ; will move this code into lib.self_free
	add esp,08H 				  ; pop TWO args off stack leave eax there

	push dword ptr widgelib+whl_pwlib    ; push pointer to library pointer

	mov eax, dword ptr widgelib+whl_free_wlib ; get pointer to free 
											  ; function into eax

	jmp dword ptr widgelib+whl_self_free ; "self_free_code" 

widge_bracket endp

; *** this code is copyed into the "self_free" buffer and then executed since
; *** the area that contains this code will be freed by this code. The buffer
; *** will not.  This code can contain no relocatable addresses.


bracket_exit_code label byte
	call eax		; free code area passed on stack using vector in eax
	add esp, 04H    ; pop arg off stack
	pop eax         ; pop off "bracket" vector or error code

	public bracket_exit
bracket_exit label byte
	mov eax, err_widget   ; this is a 5 byte instruction, the first long word
	ret					  ; will be altered by the set_exit functions
bracket_exit_end label byte

; **** setting bracket_exit to ok_exit will set the exit code to call the 
; **** bracket function, If the exit code is left un-altered or set to 
; **** err_widget_exit it will return err_widget

	public ok_exit
ok_exit label byte
    jmp eax	 ; "call" bracket function at bvector in eax 2 byte instruction

	public exit_err_widget
exit_err_widget label byte
	mov eax, err_widget  ; this is a 5 byte instruction 

code	ends
	end
