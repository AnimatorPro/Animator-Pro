_DATA           SEGMENT PUBLIC WORD USE32 'DATA'
				EXTRN tridbank:BYTE
_DATA           ENDS

CONST           SEGMENT PUBLIC WORD USE32 'DATA'
CONST           ENDS

_BSS            SEGMENT PUBLIC WORD USE32 'BSS'
_BSS            ENDS

DGROUP          GROUP   CONST,_DATA,_BSS


_text	segment	para public use32 'code'
	extrn	_setbank:near
_text	ends

CGROUP	group	_text

evga_screen struc
	vs_type dw ?
	vs_pdepth dw ?
	vs_lib dd ?
	vs_aspect_dx dw ?
	vs_aspect_dy dw ?
	vs_reserved dd ?
	vs_w dw ?
	vs_h dw ?
	vs_x dw ?
	vs_y dw ?
	vs_wreserved0 dw ?
	vs_wreserved1 dw ?
	vs_dreserved2 dd ?
	vs_bpr dd ?
	vs_ytable dd ?
evga_screen ends

ytab struc
	yta_bank dw ?
	yta_split dw ?
	yta_address dd ?
ytab ends


SC_INDEX equ 3c4h
ER_PAGE_SEL equ 0f9h
ER_BANK_SEL equ 0f6h
MISC_INPUT equ 3cch
MISC_OUTPUT equ 3c2h

XMAX equ 640
YMAX equ 400

PHAR_REAL_SEG equ 34h

setbank macro
	local inbank
;input: cl has bank #
;trashes: ax,dx
	cmp cl,tridbank
	jz inbank
	call _setbank
inbank:
	endm

nextbank macro
        push    DX
        push    CX
        push    AX
        inc     tridbank
        mov     CL, tridbank
	call    _setbank
        pop     AX
        pop     CX
        pop     DX
	endm
