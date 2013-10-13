;  kbd.asm  */
;  kbd_poll()
;  scr_getc()
;
KBD     equ 16H
KBDGETC equ 0
KBDPEND equ 1
NOCH    equ -1
SPEC    equ 100H

	TITLE   kbd
_TEXT	SEGMENT  BYTE PUBLIC 'CODE'
_TEXT	ENDS
_DATA	SEGMENT  WORD PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS
DGROUP	GROUP	CONST,	_BSS,	_DATA
	ASSUME  CS: _TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP
_TEXT      SEGMENT


	PUBLIC	_kbdgetc
;int kbdgetc()

_kbdgetc  PROC far
	pushf
	push si
	push di
	cli  ; disable interrupts
	mov  ah,KBDPEND
	int  KBD
	jnz  getc1
	mov  ax,NOCH
	jmp short getc9
getc1:  mov  ah,KBDGETC
	int KBD
	or  al,al  ;check the lower byte
	je  getc2  ; is it a non ascii special char
	xor ah,ah  ; if not just send the lower byte
	jmp short getc9
getc2:  mov al,ah
        xor ah,ah
	add ax,SPEC
getc9:  pop di
	pop si
	popf
	ret
_kbdgetc  ENDP

_TEXT  ENDS
END