                NAME    hc_fchop
                EXTRN   _mw87_used:WORD

DGROUP GROUP   data
CGROUP GROUP   code 

data SEGMENT COMMON DWORD USE32 'DATA'
     	DB      00H,00H,00H,00H,00H,00H,00H,00H
     	DB      00H
     	ORG     00000002H
L1   	LABEL   BYTE
     	ORG     00000004H
L2   	LABEL   BYTE
     	ORG     00000006H
L3   	LABEL   BYTE
     	ORG     0000000aH
L4   	LABEL   BYTE
     	ORG     0000000eH
L5   	LABEL   BYTE
     	ORG     00000012H


data ENDS

code    SEGMENT PUBLIC DWORD USE32 'CODE'
    ASSUME  CS:CGROUP,DS:DGROUP

    PUBLIC  _mwtrunc

_rounding_proc: 
	fstcw   word ptr L2
	wait    
	mov     ax,word ptr L2
	and     ax,0f3ffH
	or      eax,ecx
	mov     word ptr L1,ax
	fldcw   word ptr L1
	ret     

_mwtrunc:       
	mov     cx,0c00H
	call    near ptr _rounding_proc
	fistp   qword ptr L3
	fldcw   word ptr L2
	mov     eax,dword ptr L3
	ret     

code ENDS
	END
