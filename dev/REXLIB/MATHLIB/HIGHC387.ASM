CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

code	ends

_BSS            SEGMENT PUBLIC DWORD USE32 'BSS'

	public _mw87_used
	public _mw387_used
                ORG     00000000H
_mw87_used      LABEL   BYTE
_mw387_used     LABEL   BYTE
                ORG     00000004H
_BSS            ENDS
	end
