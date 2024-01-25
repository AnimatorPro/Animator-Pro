CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public __init_387_emulator
__init_387_emulator proc near
	ret
__init_387_emulator endp

code	ends

_DATA           SEGMENT DWORD PUBLIC USE32 'DATA'
                PUBLIC  __8087
                PUBLIC  __real87
__8087          LABEL   BYTE
                DB      00H
__real87        LABEL   BYTE
                DB      00H
_DATA           ENDS


_BSS            SEGMENT PUBLIC DWORD USE32 'BSS'

	public _fltused_
                ORG     00000000H
_fltused_       LABEL   BYTE
                ORG     00000004H
_BSS            ENDS
	end
