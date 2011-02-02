;----------------------------------------------------------------------------
;  serial.asm
;  interrupt driven serial communications stuff
;  This file only works with one port at a time (though you can specify which
;  one.
;
; :ts=8
;
; Ideally I'd like something like this interface:
;  int ser_init(int port, int ComParams);
;  ser_cleanup(int port);
;  ser_puts(int port, char *string)
;  ser_write(int port, char *buf, int size);
;  ser_read(int port, char *buf, int size);	
;					returns how many read (may be very few)
;  int ser_write_left(int port);		
;					how many characters in write buf
;  int ser_read_left(int port);
;					how many characters in read buf
;  int ser_status(int port);
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
IOPortNumber	equ	400h	; 400h = COM1, 402h = COM2

MDMSTATUS	equ	0	; identifying numbers for
TXREGEMPTY	equ	2	; cause of interrupt
RXDATAREADY	equ	4	;
RLINESTATUS	equ	6

; structure for buffers used to hold characters

BUFSIZE		equ	128
CharBuffer	struc
	InsertHere	dd	0	; insertion pointer
	RemoveHere	dd	0	; removal pointer
	BufferCount	dd	0	; count of charactres
	DataArea	db	BUFSIZE dup(0)
CharBuffer	ends
;----------------------------------------------------------------------------
; :ts=8
cgroup	group	code
dgroup	group	data

data 	segment dword 'DATA'

; declare variables, set for COM1
; note changes necessary for COM2

ComBase		DW	?	; base address of serial port
InterruptIx	DD	?	; 0 if 'standard' COM1, 1 if COM2, 0 if COM3...
IntEnableMask	DB	?	; Mask for the 8259 - either 0EFh or 0F7h
IntDisableMask	DB	?	; Inverse mask for 8259 -    010h or 008h
ComIntNumber	DB	?	; Hardware interrupt #  -    07Ch or 07Bh
OldRealHandler	DD	?	; old real handlers address
OldProtectedHandlerOff	DD	?
OldProtectedHandlerSeg	DW	?	; old protected handler's address
SerInitted	DB	0	; Set to 1 when system initialized.

InBuffer	CharBuffer	<>		;recieve buffer
OutBuffer	CharBuffer	<>		;transmit buffer
BufferSize	=	(($-OutBuffer))		;size of each buffer

;----debugging messages----
data ends
;----------------------------------------------------------------------------
code	segment dword 'CODE'

	assume cs:cgroup
	assume ds:dgroup
;----------------------------------------------------------------------------
	PUBLIC ser_init
; ser_init(int port, int mode)
;	set up interrupt driven serial communications at specified baud rate
;   parity, etc.  
ser_init	proc	near
si_params	struc
	si_ret	dd	?	; return address
	si_ebp	dd	?	; saved ebp
	si_port	dd	?	; port #
	si_mode	dd	?	; baud rate etc
si_params	ends
	push_for_c
; initialize the serial port
; get port address from BIOS data area at 400h
	mov	ax,REAL_SEG		; Get segment for real mode memory
	mov	gs,ax			; into es.
	mov	edx,[ebp].si_port	; Get port parameter.
	mov	ebx,IOPortNumber	; Look in low memory for port location
	mov	ax,gs:[edx*2][ebx]	; and save it somewhere.
	mov	ComBase,ax		; more convenient...
	test	ax,ax			; See if UART address is zero,
	jz	si_no_dev		; then no such port.
; Figure out what hardware interrupt it's tied to
	cmp	ax,3F8h
	jz	si_int_C
	cmp	ax,3E8h
	jz	si_int_C
	cmp	ax,2F8h
	jz	si_int_B
	cmp	ax,2E8h
	jz	si_int_B
si_int_C:		; Here it's tied to hardware #C, or Pharlap #7C
	mov	IntEnableMask,0EFh
	mov	IntDisableMask,010h
	mov	ComIntNumber,07Ch
	jmp	si_set_mode
si_int_B:		; Here it's tied to hardware #B, or Pharlap #7B
	mov	IntEnableMask,0F7h
	mov	IntDisableMask,008h
	mov	ComIntNumber,07Bh
; initialize using BIOS int 14h
si_set_mode:
	mov	eax,[ebp].si_mode	; Grab parameter from stack.
	mov	edx,[ebp].si_port	; Get port parameter.
	xor	ah,ah			; Zero out ah.
	int	14h			; and call bios.
; Ask PharLap for the protected mode interrupt vector
	mov	cl,ComIntNumber
	mov	ax,2502h
	int	21h
; and save it
	mov	OldProtectedHandlerSeg,es
	mov	OldProtectedHandlerOff,ebx
; Ask PharLap for real mode interrupt vector
	mov	cl,ComIntNumber
	mov	ax,2503h
	int	21h
; and save it
	mov	OldRealHandler,ebx
; Set our serial interrupt handler (so as to gain control always in protected 
; mode.
	mov	ax,2506h
	mov	edx,offset SerIntHandler 
	push	ds	; save data segment
	push	cs	; move code segment into
	pop	ds	; data segment as a parameter to int 21h 2506h
	mov	cl,ComIntNumber	; say which interrupt number to replace
	int 	21h	; call Phar Lap
	pop	ds	; restore data segment
; enable interrupts from serial port
	cli				; first disable all interrupts
	mov	dx,ComBase
	add	dx,ModemControlReg
	mov	al,ModemControlOn
	out	dx,al
; set the Interrupt Enable Register
	mov	dx,ComBase
	add	dx,IntEnableReg
	mov	al,IntEnableData
	out	dx,al
; program the 8259 to acknowledge interrupts
	in	al,21h			; read old interrupt masks
	and	al,IntEnableMask
	out	21h,al			; set new mask
	sti				; reenable interrupts
	mov	SerInitted,1		; Mark self as initialized
	call	ser_status
	jmp	si_exit
; Restore registers and return
si_no_dev:
	mov	eax,err_no_device	; No device hooked up error.
si_exit:
	pop_for_c
	ret
ser_init	endp
;----------------------------------------------------------------------------
	PUBLIC ser_cleanup
; ser_cleanup(int port)
;	remove interrupt handler and turn off interrupts for serial port
ser_cleanup	proc	near
sc_params	struc
	sc_ret	dd	?	; return address
	sc_ebp	dd	?	; saved ebp
	sc_port	dd	?	; port #
sc_params	ends
	push_for_c
; make sure initted before we try to clean up
	cmp	SerInitted,0	; test old real interrupt handler
	je	scl_exit		; bail out if not set
; clean up...
	cli				; first disable all interrupts
	mov	dx,ComBase
	add	dx,ModemControlReg
	mov	al,ModemControlOff
	out	dx,al
; next reset the Interrupt Enable Register
	mov	dx,ComBase
	add	dx,IntEnableReg
	mov	al,IntDisableAll
	out	dx,al
; turn off interrupts at the 8259
	in	al,21h
	or	al,IntDisableMask
	out	21h,al
	sti				; reenable interrupts
; restore old interrupt vectors
	mov	ebx,OldRealHandler	; get old real interrupt handler
	mov	edx,OldProtectedHandlerOff	; get old protected handler
	mov	es,OldProtectedHandlerSeg	; get old protected handler
	mov	cl,ComIntNumber		; say which interrupt number to replace
	mov	ax,2507h		; code to replace both at once
	int	21h			; call Phar Lap
	mov	SerInitted,0	; Show we've already cleaned up...
scl_exit:
	pop_for_c
	ret
ser_cleanup	endp
;----------------------------------------------------------------------------
start_send	proc	near
; Reenable TXREGEMPTY interrupts so processor will pump UART with data
	mov	dx,ComBase
	add	dx,IntEnableReg
	in	al,dx
	or	al,TXREGEMPTY
	out	dx,al
	ret
start_send	endp
;----------------------------------------------------------------------------
	PUBLIC	ser_puts
ser_puts	proc	near
; int ser_read(int port, char *buf, int size)
; Send out a buffer size bytes long
sp_parms	struc
; parameter structure
	sp_bp	dd 0	; original ebp
	sp_ret	dd 0	; return address
	sp_port	dd 0	; serial port #
	sp_string	dd 0	; NULL terminate string
sp_parms	ends
	push_for_c
	call	start_send
	mov	esi,cparameter		; point to string to send 
	mov	edi,offset OutBuffer	; point to output buffer
ss_one_char:
	mov	al,[esi]
	inc 	esi
	cmp	al,0		; is it zero?
	je	ss_exit		; then all done
	cli			; Don't interrupt InsertBuf.
	call	InsertBuf	; otherwise stuff it into buffer
	sti
	jmp	short ss_one_char ; and loop back
ss_exit:
	pop_for_c
	ret
ser_puts	endp
;----------------------------------------------------------------------------
	PUBLIC ser_write
ser_write	proc	near
; ser_write(int port, char *buf, int size)
; void ser_write(char *buf, int size)
; Send out a buffer size bytes long
sw_parms	struc
; parameter structure
	sw_bp	dd 0	; original ebp
	sw_ret	dd 0	; return address
	sw_port	dd 0	; serial port #
	sw_buf	dd 0	; buffer of characters
	sw_size dd 0	; count of characters
sw_parms	ends
	push_for_c
	call	start_send
	mov	esi,[ebp].sw_buf
	mov	ecx,[ebp].sw_size
	jecxz	sw_exit
	mov	edi,offset OutBuffer	; point to output buffer
sw_one_char:
	mov	al,[esi]
	inc	esi
	cli			; Don't interrupt InsertBuf.
	call	InsertBuf	; otherwise stuff it into buffer
	sti
	loop	sw_one_char
sw_exit:
	pop_for_c
	ret
ser_write	endp
;----------------------------------------------------------------------------
	PUBLIC	ser_read
ser_read	proc	near
; int ser_read(int port, char *buf, int size)
; Send out a buffer size bytes long
sr_parms	struc
; parameter structure
	sr_bp	dd 0	; original ebp
	sr_ret	dd 0	; return address
	sr_port	dd 0	; serial port #
	sr_buf	dd 0	; buffer of characters
	sr_size dd 0	; count of characters
sr_parms	ends
	push_for_c
	mov	esi,[ebp].sr_buf
	mov	ecx,[ebp].sr_size
	mov	edi,offset InBuffer
	jecxz	sr_end
sr_one:
	cli
	call	RemoveBuf	; get character from buffer
	sti
	jnc	sr_end		; if no character available finish up
	mov	[esi],al	; store character
	inc	esi
	loop	sr_one		; fetch another one to max
sr_end:
	mov	eax,[ebp].sr_size ; get # of characters asked for
	sub	eax,ecx		; return # of characters actually read
	pop_for_c
	ret
ser_read	endp
;----------------------------------------------------------------------------
	PUBLIC ser_write_left
ser_write_left	proc near
; int ser_write_left(int port);
; return # of characters in buffer left to send out port
	push	edi
	mov	edi,offset OutBuffer
	mov	eax,[edi].BufferCount
	pop	edi
	ret
ser_write_left	endp
;----------------------------------------------------------------------------
	PUBLIC ser_read_left
ser_read_left	proc near
; int ser_read_left(int port);
; return # of characters in buffer left to read
	push	edi
	mov	edi,offset InBuffer
	mov	eax,[edi].BufferCount
	pop	edi
	ret
ser_read_left	endp
;----------------------------------------------------------------------------
	PUBLIC ser_status
ser_status	proc	near
; Errcode ser_status(int port)
; Return Errcode or Success
; 
; Check modem status register
	mov	dx,ComBase
	add	dx,ModemStatusReg
	in	al,dx
; if top 4 bits zero then nothing is hooked up to serial port
	test	al,0F0h
	jz	sst_no_dev
; Check line status register
	mov	dx,ComBase
	add	dx,LineStatusReg
	in	al,dx
	test	al,08h			; test framing bit
	jnz	sst_framing
	test	al,04h			; test parity bit
	jnz	sst_parity
	test	al,02h			; test over run bit
	jnz	sst_overrun
	jmp 	sst_success
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
sst_success:
	xor	eax,eax			; zero is successful return
sst_out:
	ret
ser_status	endp
;----------------------------------------------------------------------------
; Serial Port Interrupt Handler
SerIntHandler proc near
	sti			; reenable interrupts
	pushad			; save the world
	push 	ds		; and the data segment
	mov 	eax,014h	; fetch our data segment
	mov 	ds,ax		; 
	ASSUME	DS:dgroup	
; check if any interrupts pending
AnyIntPending:
	mov	dx,ComBase
	add	dx,IntIdReg
	in	al,dx
	mov	cl,al
	test	al,1
	jz	ProcessAnother
; tell 8259 that processing of this interrupt 
; is complete
	mov	al,20h
	out	20h,al
	jmp	ExitSerInt
ProcessAnother:
; check which interrupt
IsItRXDATAREADY:
	cmp	cl,RXDATAREADY
	jne	IsItTXREGEMPTY
; read and place character in buffer
	mov	dx,ComBase
	in	al,dx
	mov	edi,offset InBuffer
; save character in buffer
	call	InsertBuf
	jmp	AnyIntPending
IsItTXREGEMPTY:
	cmp	cl,TXREGEMPTY
	jne	Unrecognized
; write a character to the transmit holding reg.
	mov	edi,offset OutBuffer
	call	RemoveBuf	; get character from buffer
	jnc	DoneForNow
	mov	dx,ComBase	; send out, if yes
	out	dx,al
	jmp	AnyIntPending
Unrecognized:
DoneForNow:
; nothing to send.  Disable TXREGEMPTY interrupts
	mov	dx,ComBase
	add	dx,IntEnableReg
	in	al,dx
	and	al,NOT TXREGEMPTY
	out	dx,al
	jmp	AnyIntPending
; Done!  Restore registers and go home.
ExitSerInt:
	pop	ds		; restore data segment
	popad			; restore other registers
	iretd			; go back to mainline processing
SerIntHandler	endp
;----------------------------------------------------------------------------
; buffer for queuing up characters to send
; or recieve
InsertBuf	proc 	near
; AL has character to insert
; EDI points to buffer.
	push	ebx
	cmp	[edi].BufferCount,BUFSIZE
	je	ExitInsertBuf			; data overflow - eeep!
	mov	ebx,[edi].InsertHere		; Grab insert pointer
	mov	byte ptr [edi+ebx+DataArea],al	; and store character.
	inc	[edi].BufferCount		; Now have one more char in buf
SkipIntEnable:
; adjust Insert Pointer
	inc	[edi].InsertHere
	cmp	[edi].InsertHere,BUFSIZE
	jnz	ExitInsertBuf
; wrap around
	mov	[edi].InsertHere,0
ExitInsertBuf:
	pop	ebx
	ret
InsertBuf	endp
;----------------------------------------------------------------------------
RemoveBuf	proc	near
; EDI points to buffer.
; character returned in AL.   Carry clear if no character
	push ebx
	clc				; set up for 'empty' return
	cmp	[edi].BufferCount,0	; is buffer empty?
	je	ExitRemoveBuf
	mov	ebx,[edi].RemoveHere
	mov	al,byte ptr [edi+ebx+DataArea]
	dec	[edi].BufferCount
; adjust Remove Pointer
	inc	[edi].RemoveHere
	cmp	[edi].RemoveHere,BUFSIZE
	jnz	GotSomething
; wrap around
	mov	[edi].RemoveHere,0
GotSomething:
	stc	; say we got something
ExitRemoveBuf:
	pop	ebx
	ret
RemoveBuf	endp
;----------------------------------------------------------------------------

code	ends



	end
