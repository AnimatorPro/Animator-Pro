;*****************************************************************************
;* CBINT10.ASM - Issue int10/getinfo to vesa bios (CodeBuilder).
;*
;*  This is not used in the loadable device driver, only in the codebuilder
;*  version of the fliclib.
;*
;*  Two of the vesa functions we call via int 10, VBE_INFO and MODE_INFO,
;*  require a pointer to a DOS-area memory buffer to be passed in ES:DI.
;*  For CodeBuilder, we have to install an int 10 intercept routine to
;*  massage the flat pointer into a segmented pointer, so that the real mode
;*  int 10 sees the proper value in ES:DI.
;*
;*  The bulk of the vesa code uses an inline int 10 instruction to talk to
;*  the vesa bios.  This module is used only for the two INFO calls that
;*  require the buffer pointer; these occur only during mode switches and
;*  get_mode_info calls, and performance is not an issue in this routine.
;*
;*  Because the int 10 intercept slows down all calls to int 10, and because
;*  we can afford to be slow when processing the getinfo calls but not when
;*  doing video bankswitches, we install the intercept for the duration of
;*  the getinfo call, then remove it again as soon as int 10 returns.
;*****************************************************************************

	include stdmacro.i		; our standard macros

_BSS   segment

;-----------------------------------------------------------------------------
; some data structures from CodeBuilder's STK.AH header file, used in the
; intercept routine...
;-----------------------------------------------------------------------------


STK STRUC				;* ================================= *;
   STK_RLOC		DD  ?		;* Relocation factor		     *;
			DB  2 DUP (?)	;* Reserved			     *;
   STK_OPTS		DB  ?		;* Options			     *;
   STK_CC		DB  ?		;* Command code 		     *;
   STK_EDI		DD  ?		;* Registers of interrupted process  *;
   STK_ESI		DD  ?		;*     "     "       "         "     *;
   STK_EBP		DD  ?		;*     "     "       "         "     *;
   STK_TMP		DD  ?		;* (Points to global dat area-GDA)   *;
   STK_EBX		DD  ?		;*     "     "       "         "     *;
   STK_EDX		DD  ?		;*     "     "       "         "     *;
   STK_ECX		DD  ?		;*     "     "       "         "     *;
   STK_EAX		DD  ?		;*     "     "       "         "     *;
   STK_ERC		DW  ?		;* Error code or reserved	     *;
   STK_ID		DB  ?		;* Interrupt ID (software INTs)      *;
   STK_IDI		DB  ?		;* Intel interrupt ID (exceptions)   *;
   STK_EIP		DD  ?		;* Registers			 EIP *;
   STK_CS		DD  ?		;* of the			  CS *;
   STK_FLG		DD  ?		;* interrupted		      EFLAGS *;
   STK_ESP		DD  ?		;* process			 ESP *;
   STK_SS		DD  ?		;*				  SS *;
   STK_ES		DD  ?		;* V86-mode registers		     *;
   STK_DS		DD  ?		;*  "   "                            *;
   STK_FS		DD  ?		;*  "   "                            *;
   STK_GS		DD  ?		;*  "   "                            *;
STK ENDS				;* --------------------------------- *;

_STK_WRK	EQU	8		;* Length of stack work space	     *;
_STK_LEN	EQU	SIZE STK	;* Length of stack frame	     *;

					;* Stack options (STK_OPTS field) -- *;
_STK_NOINT	EQU	80H		;* Suppress interrupt		     *;
_STK_TERM	EQU	40H		;* Terminate application	     *;

					;* EFLAG Values -------------------- *;
_FLAG_CARRY	EQU	00000001H	;* Carry flag			     *;
_FLAG_PARITY	EQU	00000004H	;* Parity flag			     *;
_FLAG_AUXCARRY	EQU	00000010H	;* Auxillary carry flag 	     *;
_FLAG_ZERO	EQU	00000040H	;* Zero flag			     *;
_FLAG_SIGN	EQU	00000080H	;* Sign flag			     *;
_FLAG_TRAP	EQU	00000100H	;* Trap flag			     *;
_FLAG_INTERRUPT EQU	00000200H	;* Interrupt enable flag	     *;
_FLAG_DIRECTION EQU	00000400H	;* Direction flag		     *;
_FLAG_OVERFLOW	EQU	00000800H	;* Overflow flag		     *;
_FLAG_IOPL	EQU	00003000H	;* I/O privilege level mask	     *;
_FLAG_NESTED	EQU	00004000H	;* Nested task flag		     *;
_FLAG_RESUME	EQU	00010000H	;* Resume flag			     *;
_FLAG_VM	EQU	00020000H	;* Virtual 8086 mode		     *;

oldint10 dd	?			; old int10 handler to chain to.

_BSS   ends

_TEXT	segment

	public	_pj_vesa_int10

	extrn	_dos_getvect:near
	extrn	_dos_setvect:near

;*****************************************************************************
;* int10_intercept
;*
;*   this function, when installed, gets control before the real mode int 10
;*   handler.  on the stack, addressable via [ebp], is a STK structure that
;*   contains an image of the registers that will be passed to the real mode
;*   handler.  we change the flat pointer in the edi reg to a segmented
;*   pointer in ES:DI, then chain to the prior int 10 handler.
;*
;*   any registers used in this routine must be preserved!
;*****************************************************************************

VESA_VBE_INFO	equ	4f00h		; get VESA hardware information
VESA_MODE_INFO	equ	4f01h		; get video mode information

int10_intercept proc near

	push	eax

	test	[ebp].STK_FLG,_FLAG_VM	; Is interrupt from V86 (real) mode?
	jnz	short #punt		; yep, nothing for us to do.

	mov	eax,[ebp].STK_EAX	; load int 10 function/subfunction.
	cmp	eax,VESA_VBE_INFO	; make sure the function is one of
	jb	short #punt		; the two get-info functions that
	cmp	eax,VESA_MODE_INFO	; need the address translation; if
	ja	short #punt		; not, just chain to prior handler.

	mov	eax,[ebp].STK_EDI	; get edi that was passed to int 10,
	shr	eax,4			; convert flat addr to segment addr,
	mov	[ebp].STK_ES,eax	; now realmode int10 will see it in ES.
	mov	eax,[ebx].STK_EDI	; get edi again,
	and	eax,0Fh 		; mask out seg, leaving offset,
	mov	[ebp].STK_EDI,eax	; now realmode int10 will see it in DI.
#punt:
	pop	eax
	jmp	dptr [oldint10] 	; chain to prior handler

int10_intercept endp

;*****************************************************************************
;* _pj_vesa_int10
;*
;*  Issue INT10 call to VESA BIOS, such that flat pointer in edi is seen by
;*  the real mode handler as a segmented pointer in ES:DI.  The value in edi
;*  will be a pointer to a buffer in real DOS memory.  For codebuilder, we
;*  do this by installing an intercept that handles the translation, then
;*  issuing a normal int 10, then removing the intercept before returning.
;*
;*  Input:
;*	eax    = VESA function number.
;*	es:edi = pointer (far32) to buffer in DOS memory (ES is ignored).
;*  Output:
;*	values returned by VESA BIOS.
;*****************************************************************************

_pj_vesa_int10 proc near

	Entry

	pushad				; save everything.
	push	10h			; get address of old int10
	call	_dos_getvect		; handler via codebuilder library
	add	esp,4			; function.
	mov	oldint10,eax		; save old handler address.
	lea	eax,int10_intercept	; install our intercept routine
	push	eax			; to handle int10 calls,
	push	10h
	call	_dos_setvect
	add	esp,8
	popad				; restore everything

	int 10h 			; call the vesa bios.
	cld				; Let's be paranoid.

	pushf				; save flags set by vesa bios.
	pushad				; save all regs set by vesa bios.
	mov	eax,oldint10		; remove our intercept routine...
	push	eax
	push	10h
	call	_dos_setvect
	add	esp,8
	popad				; restore regs
	popf				; restore flags

	ret

_pj_vesa_int10 endp

_TEXT	ends
	end
