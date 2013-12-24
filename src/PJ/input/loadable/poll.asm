;----------------------------------------------------------------------------
;  poll.asm
;  Polling driven serial communications stuff
;
; :ts=8
;
;  void ser_set_mode(int port, int ComParams);
;  Errcode ser_write(int port, char *buf, int size);
;  Errcode ser_get_char(int port);	
;  Errcode ser_status();
;

include errcodes.i
include useful.i

;----------------------------------------------------------------------------
; constants
IntEnableReg	equ	1	; offset of register from
IntIdReg	equ	2	; base register
LineControlReg	equ	3
ModemControlReg	equ	4
LineStatusReg	equ	5
ModemStatusReg	equ	6

ModemControlOn	equ	0Fh
ModemControlOff	equ	0
IntEnableData	equ	03h
IntDisableAll	equ	0

MDMSTATUS	equ	0	; identifying numbers for
TXREGEMPTY	equ	2	; cause of interrupt
RXDATAREADY	equ	4	;
RLINESTATUS	equ	6

;----------------------------------------------------------------------------
; :ts=8
cgroup	group	code
dgroup	group	data

data 	segment dword 'DATA'

; declare variables, set for COM1
; note changes necessary for COM2

PortNumber	DW	0	; 0 = COM1, 1 = COM2
IOPortNumber	DD	400h	; 400h = COM1, 402h = COM2
ComBase		DW	?	; base address of serial port
ComIntNumber	DB	7Ch	; interrupt number, 7Bh for COM2
SerInitted	DB	0	; Set to 1 when system initialized.

cstring macro label,s
; add carriage return, line feed and money sign to string with a label
; so that DOS string output stuff is easier to type...
label 	db	s,0
endm

cstring timed	'ser_get_char timed out'

data ends
;----------------------------------------------------------------------------
code	segment dword 'CODE'

EXTRN	boxf:near

dbox	macro	s
; do a boxf with only the one string arg
	pushad
	mov		eax,offset s
	push	eax
	call	boxf
	pop		eax
	popad
endm



	assume cs:cgroup
	assume ds:dgroup
;----------------------------------------------------------------------------
	PUBLIC ser_set_mode
; void ser_set_mode(int port, int mode)
ser_set_mode	proc	near
ssm_parms	struc
; parameter structure
	ssm_ebp		dd 0	; original ebp
	ssm_ret		dd 0	; return address
	ssm_port	dd 0	; Serial port number (0 to 3)
	ssm_mode	dd 0	; Mode (see serial.h)
ssm_parms	ends
	push 	ebp
	mov	ebp,esp
	mov	eax,[ebp].ssm_mode	; Grab mode parameter from stack.
	mov	edx,[ebp].ssm_port	; Tell BIOS which port
	xor	ah,ah			; Zero out ah (BIOS comm function 0).
	int	14h			; and call bios.
	pop	ebp
	ret
ser_set_mode	endp
;----------------------------------------------------------------------------
GetComBase	macro
; input:	serial port # in edx
; output:	serial port base address in dx
	add	edx,edx
	add	edx,400h
	mov	dx,gs:[edx]
endm
;----------------------------------------------------------------------------
	PUBLIC ser_write
ser_write	proc	near
sw_parms	struc
; parameter structure
	sw_ebp		dd 0	; original ebp
	sw_ret		dd 0	; return address
	sw_port		dd 0	; Serial port number (0 to 3)
	sw_buf		dd 0	; character buffer
	sw_size		dd 0	; size of buffer
sw_parms	ends
	push_for_c
	mov	ecx,[ebp].sw_size	; Get # of characters to send
	jecxz	sw_exit			; and bail out now if none.
	mov	esi,[ebp].sw_buf	; Fetch buffer to send.
	mov	edx,[ebp].sw_port	; Get COM port #
	GetComBase			; and corresponding UART start address.
sw_wl:	
	add	dx,LineStatusReg  ; First look at line status.
sw_tl:
	in	al,dx		  ; Read lines status port.
	test	al,20h		  ; See if transmit holding reg empty is set.
	jz	sw_tl		  ; If not go back.
	add	dx,ModemStatusReg-LineStatusReg	; Look at modem status.
sw_cc:
	in	al,dx		
;;; These next two lines would be nice to have in, but Summa and Wacom tablets
;;; don't assert carrier detect!
;	test	al,80h		  ; Make sure have 'carrier detect'
;	jz	sw_dropped	  ; return error if not.
	lodsb			  ; Fetch next character to send.
	sub	dx,ModemStatusReg ; Go back to data register
	out	dx,al		  ; and pump out the character.
	loop	sw_wl		  ; Keep going while there's characters....
	xor	eax,eax		  ; Return Success.
	jmp	sw_exit		  ; And go back home.
sw_dropped:
	mov	eax,err_timeout	  ; Dropped line....
sw_exit:
	pop_for_c
	ret
ser_write	endp
;----------------------------------------------------------------------------
	PUBLIC ser_get_char
ser_get_char	proc	near
; Errcode ser_get_char(int port)
; Returns err_timeout, or the character read....
	push	ebp
	mov	ebp,esp
	mov	edx,cparameter		; Get port parameter
	GetComBase			; and corresponding UART start address.
	add	dx,LineStatusReg  	; First look at line status.
	in	al,dx
	test	al,1			; Is data ready?
	jz	sgc_not_ready
	sub	dx,LineStatusReg	; Go back to data register.
	xor	eax,eax			; Clear hi bits of result
	in	al,dx			; and read in lo bits.
	jmp	sgc_exit		; All done!
sgc_not_ready:
	mov	eax,err_timeout
sgc_exit:
	pop	ebp
	ret
ser_get_char	endp
;----------------------------------------------------------------------------
	PUBLIC ser_status
ser_status	proc	near
; Errcode ser_status(int port)
; Return Errcode if a problem.   Returns 1 if port ready to read, 0 otherwise.
; 
; Check modem status register
	push	ebp
	mov	ebp,esp
	mov	edx,cparameter	; get port parameter
	GetComBase
	add	dx,ModemStatusReg
	in	al,dx
; if top bit's zero then nothing is hooked up to serial port
;;; These next two lines would be nice to have in, but Summa and Wacom tablets
;;; don't assert carrier detect!
;	test	al,080h		; test data carrier detect
;	jz	sst_no_dev
; Check line status register
	add	dx,LineStatusReg-ModemStatusReg
	in	al,dx
	test	al,08h			; test framing bit
	jnz	sst_framing
	test	al,04h			; test parity bit
	jnz	sst_parity
	test	al,02h			; test over run bit
	jnz	sst_overrun
	and	eax,1			; mask out all but Data Ready bit
	jmp 	sst_out
sst_overrun:
	mov	eax,err_overflow
	jmp	sst_out
sst_parity:
	mov	eax,err_bad_input
	jmp	sst_out
sst_framing:
	mov	eax,err_format
	jmp	sst_out
sst_no_dev:
	mov	eax,err_no_device
	jmp	sst_out
sst_out:
	pop	ebp
	ret
ser_status	endp
;----------------------------------------------------------------------------
	PUBLIC real_peek
real_peek	proc	near
	push_for_c
	mov	ax,REAL_SEG
	mov	es,ax
	mov	eax,cparameter
	mov	ax,es:[eax]
	and	eax,0FFFFh
	pop_for_c
	ret
real_peek	endp

code	ends



	end
