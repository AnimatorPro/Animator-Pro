	;The usual  80x86 segmentation nonsense.  Who really understands this?
	;But it must here, at least some of it.

_DATA           SEGMENT PUBLIC WORD USE32 'DATA'
_DATA           ENDS

CONST           SEGMENT PUBLIC WORD USE32 'DATA'
CONST           ENDS

_BSS            SEGMENT PUBLIC WORD USE32 'BSS'
_BSS            ENDS

DGROUP          GROUP   CONST,_DATA,_BSS

_text	segment	para public use32 'code'
_text	ends

CGROUP	group	_text

_text	segment	para public use32 'code'
	assume cs:CGROUP

	extrn  poco_entry:near

	jmp poco_entry

_text	ends
	end
