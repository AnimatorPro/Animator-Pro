
                NAME   	lomouse 
DGROUP          GROUP   CONST,_DATA,_BSS

_DATA           SEGMENT PUBLIC WORD USE32 'DATA'
_DATA           ENDS

_TEXT           SEGMENT PUBLIC BYTE USE32 'CODE'
                ASSUME  CS:_TEXT,DS:DGROUP

;jkeyis() - returns 1 if a key is ready to read from keyboard, 0 otherwise
	public jkeyis
jkeyis proc near
	mov ah,1
	int 16h
	jz #ret0
	and eax,0FFFFh
	jmp #retit
#ret0:
	xor eax,eax
#retit:
	ret
jkeyis endp

;jkeyin() - returns next keyboard input - will wait for it if necessary
	public jkeyin
jkeyin proc near
	mov ah,0
	int 16h
	and eax,0FFFFh
	ret
jkeyin endp

	public jkey_shift
;jkey_shift() 
;	returns keyboard shift/control/alt state
jkey_shift proc near
	mov ah,2
	int 16h
	and eax,0ffh
	ret
jkey_shift endp


_TEXT	ends

CONST           SEGMENT PUBLIC WORD USE32 'DATA'
CONST           ENDS

_BSS            SEGMENT PUBLIC WORD USE32 'BSS'
_BSS            ENDS

	end
