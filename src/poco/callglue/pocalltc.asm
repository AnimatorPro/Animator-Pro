                EXTRN   _postack_to_cstack:WORD
DGROUP          GROUP   _DATA,_BSS
_TEXT         SEGMENT PUBLIC BYTE 'CODE'
                ASSUME  CS:_TEXT,DS:DGROUP

;(return type) poco_call_cfunc(void *stack_descriptor, void *poco_stack,
;							   int cstack_size, FUNC cvector );

	public _poco_call_ifunc ; integer function
	public _poco_call_pfunc ; pointer function
	public _poco_call_sfunc ; short function
	public _poco_call_bfunc ; byte function
	public _poco_call_ffunc ; double float function
	public _poco_call_ppfunc ; Popot function
_poco_call_ifunc proc near
_poco_call_pfunc proc near
_poco_call_sfunc proc near
_poco_call_bfunc proc near
_poco_call_ffunc proc near
_poco_call_ppfunc proc near

pococall_arg	struc	;poco_call_Xfunc parameter structure
	pc_bp	dw ?
	pc_ret	dd ?
	pc_stack_desc dw ?
	pc_sdesc_seg  dw ?
	pc_poco_stack dw ?
	pc_pstack_seg  dw ?
	pc_cstack_size dw ?
	pc_cvector dd ?
pococall_arg	ends

	push    bp
    mov     bp,sp
	sub 	sp,[bp].pc_cstack_size  ; make space requested on stack
	sub		sp,0aH  				; allow space bp and for pocostack pointer
    and     sp,0fffeH  				; round up for word alignment

;	call postack_to_cstack(stack_desk,poco_stack,&c_stack);

	mov		ax,sp
	push	ss	; push segment and pointer to cstack buffer on stack
	push 	ax	
	push	[bp].pc_pstack_seg
	push	[bp].pc_poco_stack
	push	[bp].pc_sdesc_seg
	push	[bp].pc_stack_desc

    call    far ptr _postack_to_cstack
	add     sp,000cH 	; pop args off stack

;   call (*cvector)(...[c_stack]...) call vector with stack frame for "C" stack

	call    dword ptr [bp].pc_cvector

    mov     sp,bp
    pop     bp
    retf    

_poco_call_ppfunc endp
_poco_call_ffunc endp
_poco_call_bfunc endp
_poco_call_sfunc endp
_poco_call_pfunc endp
_poco_call_ifunc endp


_TEXT         ENDS

_DATA           SEGMENT PUBLIC WORD 'DATA'
_DATA           ENDS

_BSS            SEGMENT PUBLIC WORD 'BSS'
_BSS            ENDS

                END
