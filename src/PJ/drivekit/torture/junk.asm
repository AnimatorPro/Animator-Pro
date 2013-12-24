	include stdmacro.i

_TEXT	segment

x	proc near

	mov	ecx,edx
	and	dl,3
	shr	ecx,2
	xor	eax,eax
	repe cmpsd
	jne	short done
	mov	cl,dl
	repe cmpsb
	sete	al
done:

x	endp

_TEXT	ends
