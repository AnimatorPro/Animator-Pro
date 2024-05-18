
	EXTRN   postack_to_cstack:WORD

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;(return type) poco_call_cfunc(void *stack_descriptor, void *poco_stack,
;							   int cstack_size, FUNC cvector );


; A raft of symbols for different data types. Maybe not needed if casts
; do the job at higher level
; the double call works with watcom since it returns it's result in the
; eax,edx register pair and not on the stack. This same code can also be 
; used to call functions that return an structure since the address of the 
; structure area in the caller is passed in in watcom in the esi register
; and the call copys it's result there.  It would be possible to make a version
; of this that would be passed in a pointer to the destination struct area
; and then stuff this into esi and then call this to return a structure value
; into *esi in the caller

	public poco_call_ifunc ; integer function
	public poco_call_pfunc ; pointer function
	public poco_call_sfunc ; short function
	public poco_call_bfunc ; byte function
	public poco_call_ffunc ; double float function
	public poco_call_ppfunc ; Popot function
poco_call_ifunc proc near
poco_call_pfunc proc near
poco_call_sfunc proc near
poco_call_bfunc proc near
poco_call_ffunc proc near
poco_call_ppfunc proc near

pococall_arg	struc	;poco_call_Xfunc parameter structure
	pc_ebp	dd ?
	pc_ret	dd ?
	pc_stack_desc dd ?
	pc_poco_stack dd ?
	pc_cstack_size dd ?
	pc_cvector dd ?
pococall_arg	ends

	push	ebp    	; save old stack frame
    mov		ebp,esp ; sp to ebp
	sub 	esp,[ebp].pc_cstack_size  ; make space requested on stack
	sub		esp,010H  ; allow space for *pocostack and rounding
	and 	esp,0fffffffcH   ; round up stack to be long word aligned

;	call postack_to_cstack(stack_desk,poco_stack,&c_stack);

	push	esp		 	; push pointer to cstack buffer on stack
	push	[ebp].pc_poco_stack
	push	[ebp].pc_stack_desc
	call    near ptr postack_to_cstack
    add     esp,0cH 	; pop args off stack

;   call (*cvector)(...[c_stack]...) call vector with stack frame for "C" stack

	call    dword ptr [ebp].pc_cvector

    leave  ; Restore ebp to esp and pop ebp to caller's stack frame  
    ret     

poco_call_ppfunc endp
poco_call_ffunc endp
poco_call_bfunc endp
poco_call_sfunc endp
poco_call_pfunc endp
poco_call_ifunc endp

code	ends
	end

