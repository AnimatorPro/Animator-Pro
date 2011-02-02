
                NAME   	loclock 
DGROUP          GROUP   CONST,_DATA,_BSS

_DATA           SEGMENT PUBLIC WORD USE32 'DATA'
_DATA           ENDS

_TEXT           SEGMENT PUBLIC BYTE USE32 'CODE'
                ASSUME  CS:_TEXT,DS:DGROUP

;jclock18() - returns 18hz system clock
	public jclock18
jclock18 proc near
	mov ah,0
	int 1Ah
	mov eax,ecx
	shl eax,16
	mov ax,dx
	ret
jclock18 endp

_TEXT	ends

CONST           SEGMENT PUBLIC WORD USE32 'DATA'
CONST           ENDS

_BSS            SEGMENT PUBLIC WORD USE32 'BSS'
_BSS            ENDS

	end
