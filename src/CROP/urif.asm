;urif.asm - Routine to convert from a 320x200x5 bitplane Amiga screen
;to a 320x200 byte-a-pixel screen.

;conv_screen(bits)
;PLANEPTR bits;
;{
;int i;
;PLANEPTR bytes;
;
;bytes = (PLANEPTR)0xa0000000;
;i = 8000;
;while (--i >= 0)
;	{
;	conv8(bits[0*PLANE_SIZE],
;		bits[1*PLANE_SIZE],
;		bits[2*PLANE_SIZE],
;		bits[3*PLANE_SIZE],
;		bits[4*PLANE_SIZE], bytes);
;	bits += 1;
;	bytes += 8;
;	}
;}

	TITLE   urif

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


	PUBLIC	_conv_screen
	;conv_screen(offset, seg)
_conv_screen	PROC FAR
	push	bp
	mov	bp,sp
	push ds
	push es
	push di
	push si
	cld

	;source bitplane pointer in ds:si
	mov	ax,[bp+6+2]	;seg
	mov ds,ax
	mov	si,[bp+4+2]	;offset
	;dest byte-screen pointer in es:di
	mov ax,0A000h
	mov es,ax
	mov di,0
	mov cx,8000
	;fetch 5 bytes from 5 planes of source
read_byte:
	mov bh,[si]
	mov bl,[si+8000]
	mov dh,[si+16000]
	mov dl,[si+24000]
	mov ah,[si+32000]
	push cx
	mov cx,8
	;shift bits in all 5 planes once to make a single byte of dest
make_byte:
	xor al,al ; al=0
	rcl ah,1
	rcl al,1
	rcl dl,1
	rcl al,1
	rcl dh,1
	rcl al,1
	rcl bl,1
	rcl al,1
	rcl bh,1
	rcl al,1
	stosb
	loop make_byte
	pop cx
	inc si
	loop read_byte

	pop si
	pop di
	pop es
	pop ds
	pop	bp
	ret	
_conv_screen	ENDP


_TEXT	ENDS
END
