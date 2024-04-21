;****************************************************************************
;*
;* RUNCCALL.ASM - Call glue routines for calling C functions from Poco code.
;*
;*  This routine is the interface between the Poco runops interpreter and
;*  functions written in compiled C (such as builtin library or POE routines.)
;*  The routine exists under five names, each corresponding to the return
;*  value type of the C function being called.	(This eliminates the need
;*  for mega-recasting in the runops code.)
;*
;*  This routine works by the rather simple expedient of changing the 386
;*  stack pointer to point to the Poco run stack, then calling the target C
;*  function.  Upon return from the C function, the 386 stack pointer is
;*  restored to its entry value, and we return to runops.  Since we do not
;*  disturb any registers other than ESP/EBP in this glue routine, parameter
;*  passing and return values needs no special handling.  However, this
;*  only with C functions that adhere to the Watcom -3s passing conventions;
;*  High C and other compilers cannot be supported this easily.
;*
;*  Usage from our caller is:
;*     po_Xccall(poco_stack_pointer, addr_of_function_to_call);
;*
;* MAINTENANCE:
;*  05/01/91	(Ian)
;*		We now try to molify watcom library routines which have calls
;*		to __STK, the watcom stack overflow checker.  We can't
;*		disable those calls, so instead, we keep watcom's _STACKLOW
;*		variable in sync with the changes we do to the hardware
;*		stack.	When we load ESP with the address of the parms on
;*		the poco stack, we load _STACKLOW with an address 4kbytes
;*		below ESP.  (4k because that's what we guarantee will be
;*		available when calling out to a C function.  There may be
;*		more stack available, but if they use more than 4k, they're
;*		breaking the rules anyway.)  This, of course, is horribly
;*		watcom-specific (although High C likely uses something
;*		similar), but we were already watcom-specific in here anyway.
;*  09/06/91	(Jim)
;*		Added po_string_ccall label.
;****************************************************************************

_DATA	segment public dword use32 'DATA'
	extrn	_STACKLOW:dword 	; in watcom's cstart.obj
_DATA	ends

_CODE	segment public dword use32 'CODE'

CGROUP	group	_CODE
DGROUP	group	_DATA
	assume	cs:CGROUP,ds:DGROUP

	public	po_vccall		; return void
	public	po_iccall		; return int
	public	po_lccall		; return long
	public	po_dccall		; return double
	public	po_pccall		; return Popot
	public	po_string_ccall		; return PoString

po_vccall proc near
po_iccall proc near
po_lccall proc near
po_dccall proc near
po_pccall proc near
po_string_ccall proc near

	push	_STACKLOW		; save var watcom uses for stack check.
	push	ebp			; save base pointer
	mov	ebp,esp 		; save stack pointer

	mov	esp,dword ptr [ebp+12]	; load pointer to poco stack.
	lea	eax,[esp-4096]		; point to 'bottom' of poco stack.
	mov	_STACKLOW,eax		; set watcom stack check var.

	call	dword ptr [ebp+16]	; call C function

	mov	esp,ebp 		; restore stack pointer
	pop	ebp			; restore base pointer
	pop	_STACKLOW		; restore watcom stack check var.
	ret				; return

po_string_ccall endp
po_pccall endp
po_dccall endp
po_lccall endp
po_iccall endp
po_vccall endp

_CODE	ends
	end
