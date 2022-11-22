	include stdmacro.i

_TEXT	segment

strcpy	proc near

	mov	ecx,edi
xloop:
	lodsw
	test	al,al
	jz	short found_end
	stosw
	test	ah,ah
	jnz	short xloop
	db	0A8h
found_end:
	stosb
all_done:
strcpy	endp
_TEXT	ends
	end
