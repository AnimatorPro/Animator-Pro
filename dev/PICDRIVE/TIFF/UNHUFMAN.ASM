; Set your editor tabsize to 8 for this file!
;****************************************************************************
; UNHUFMAN.ASM - Decompress TIFF type-2 data (modified one-d Huffman format).
;		 (386 protected mode version).
;****************************************************************************

_TEXT		segment word public use32 'CODE'
		assume	cs:_TEXT,DS:_TEXT

		public	decmprs2

;****************************************************************************
; decmprs2 -  Decompress the current line of data.
;	      This is the entry point from the outside world.
;
;  ENTRY:
;    [esp+4]   - Pointer to input buffer
;    [esp+8]   - Pointer to output line buffer
;    [esp+12]  - Size of output buffer (bytes to create during decompress)
;
;  EXIT:
;	eax    - Pointer to next byte in input buffer (data for next line)
;		 If this is NULL, a decompression error ocurred (either a
;		 bad runlength code, or a buffer overrun). all errors are
;		 considered fatal; since data alignment is lost, no recovery
;		 of the current or any following line is possible.
;	all other regs preserved
;****************************************************************************

decmprs2	proc	near

InputPointer	equ	dword ptr [ebp+8]
OutputPointer	equ	dword ptr [ebp+12]
OutputCount	equ	dword ptr [ebp+16]

		push	ebp
		mov	ebp,esp
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi
		cld

		mov	edi,OutputPointer
		mov	ecx,OutputCount
		mov	al,00h			; zero out buffer to avoid
		rep stosb			; having to write white bits.

		mov	edi,OutputPointer	; reload output pointer
		xor	edx,edx 		; clear output bit offset
		mov	bh,1			; prime input bit counter
mainloop:
		call	getwhiterun		; go get a white run.
		js	short goterror		; sign flag indicates error.
		sub	OutputCount,esi 	; Count the bits just done,
		jz	short done		; if zero, line is all done.
		call	getblackrun		; go get a black run.
		js	short goterror		; sign flag indicates error.
		sub	OutputCount,esi 	; count the bits just done,
		jz	short done		; if zero, line is all done.
		jmp	short mainloop		; loop until line is built.
goterror:
		mov	eax,0			; encountered a bad code,
		jmp	short returnit		; return NULL (error).
done:
		mov	eax,InputPointer	; return ptr to next input.
returnit:
		pop	edi
		pop	esi
		pop	edx
		pop	ecx
		pop	ebx
		leave
		ret

decmprs2	endp

;****************************************************************************
;
; getwhiterun - Process a run of white pixels
;		Since the output buffer is zero'd before we start, we don't
;		write white bits, we just add the runlength to the output
;		bit pointer.
;
;  ENTRY:
;	 bh - current input bit
;	 bl - current input byte
;	edx - offset (in bits) into output buffer
;  EXIT:
;	 bh - new current input bit
;	 bl - new current input byte
;	edx - new offset (in bits) into output buffer
;	esi - runlength value (bits 'added' to output)
;	eax - trashed by lower level routines
;	ecx - trashed by lower level routines
;****************************************************************************

getwhiterun	proc	near

		lea	esi,white_tree
		call	getrunlength
		js	short white_return	; punt on error.
		add	edx,esi
white_return:
		ret

getwhiterun	endp

;****************************************************************************
;
; getblackrun - Process a run of black pixels
;
;  ENTRY:
;	 bh - current input bit
;	 bl - current input byte
;	edx - offset (in bits) into output buffer
;	edi - pointer to start of output buffer
;  EXIT:
;	 bh - new current input bit
;	 bl - new current input byte
;	edx - new offset (in bits) into output buffer
;	edi - pointer to start of output buffer (not modified)
;	esi - runlength value (bits added to output)
;	eax - trashed
;	ecx - trashed
;****************************************************************************

getblackrun	proc	near

		lea	esi,black_tree		; search tree for black run
		call	getrunlength		; go get run length
		push	esi			; save runlength for retval
		jle	black_return		; punt if error or 0 length

		mov	eax,edx 		; get output offset, isolate
		and	ax,7			; bits already done in byte
		jz	short dofullbytes	; if zero, no partial to fill

		mov	ecx,edx 		; calculate offset to current
		shr	ecx,3			; byte on output line, save
		push	ecx			; offset till we need it

		mov	cx,7			; convert bits-already-done
		sub	cx,ax			; to index of bit to start
		mov	ax,0			; filling with blackness
loop1:
		bts	ax,cx			; set the bit, count it as
		inc	edx			; done in the output offset
		dec	esi			; count as done in runlength
		jz	done1			; if runlength now zero, done
		sub	cx,1			; else change index to next
		jge	loop1			; bit and loop until done
done1:
		pop	ecx			; get saved out byte offset
		or	byte ptr [edi+ecx],al	; new bits into current byte
dofullbytes:
		mov	ecx,esi 		; get count of bits left to
		cmp	ecx,0			; to do in run, if count
		jz	short black_return	; is zero, we're all done.
		shr	edx,3			; convert to byte out offset
		shr	ecx,3			; convert bitcount to bytes
		jz	short doendbits 	; if 0, no full bytes to do

		and	esi,7			; reduce count to residual bits
		push	edi			; save ptr to start of output
		lea	edi,[edi+edx]		; make ptr to current output
		add	edx,ecx 		; count bytes we're doing
		mov	al,0ffh 		; init output value
		rep stosb			; lay in byte run of black
		pop	edi			; restore ptr to output
doendbits:
		mov	ecx,edx 		; save byte output offset
		shl	edx,3			; convert back to bit offset
		cmp	esi,0			; any bits left to do?
		je	short black_return	; nope, all done.

		push	ecx			; save byte output offset
		add	edx,esi 		; update bit output offset
		mov	cx,7			; prime index of bits to set
		xor	ax,ax			; in partial output byte
loop2:
		bts	ax,cx			; set the current bit
		dec	cx			; change bit index
		dec	esi			; count bit as done
		jnz	loop2			; if not zero, do more bits
		pop	ecx			; restore output byte offset
		or	byte ptr[edi+ecx],al	; lay in partial byte
black_return:
		pop	esi			; restore runlength and
		cmp	esi,0			; set sign flag to match,
		ret				; our caller needs both.

getblackrun	endp

;****************************************************************************
; getrunlength - Return runlength value for next code in input stream.
;		 This is the routine that walks the binary tree until a
;		 terminating code is found.  This routine also checks the
;		 runlength value it finds against the bytes-remaining count
;		 and sets the sign flag if an overrun is detected.
;
;		 Note that this routine copes with two type of leaf nodes:
;		 those representing terminating codes (value < 64), and
;		 those representing makeup codes.  The routine continues
;		 looping until it finds a terminating code, and it returns
;		 the sum of all makeup codes plus the terminating code.
;
;  ENTRY:
;	 bh - Current bit
;	 bl - Current byte
;	esi - ptr to tree to search
;  EXIT:
;	 bh - Current bit
;	 bl - Current byte
;	esi - run length, or -1 for code error or length overruns buffer
;	eax - trashed
;	ecx - trashed
;****************************************************************************

getrunlength	proc	near
		mov	eax,esi 		; save ptr for looping
		xor	esi,esi 		; zero out length accumulator
getnextlength:
		mov	ecx,eax 		; load/refresh tree pointer
searchsubtree:
		cmp	dword ptr[ecx],0	; leaf node?
		jnz	short found_leafnode	; yes, go check leaf type

		dec	bh			; still bits in current byte?
		jnz	short nextbit		; yep, go do next bit
		mov	ebx,InputPointer	; load pointer to input data
		inc	dword ptr InputPointer	; increment it for next time
		mov	bl,[ebx]		; get next byte of data
		mov	bh,8			; reset count to 8 bits
nextbit:
		shl	bl,1			; test bit. if a 1 bit is
		jc	short take_rightbranch	; found go take right branch
take_leftbranch:
		add	ecx,8			; 0 bit takes the left branch
		jmp	short searchsubtree	; loop to search subtree
take_rightbranch:
		mov	ecx,dword ptr[ecx+4]	; point to right branch,
		jmp	short searchsubtree	; loop to search subtree
found_leafnode:
		mov	ecx,dword ptr[ecx+4]	; get value of leaf, if neg
		js	found_errorcode 	; we have bad code, report it
		add	esi,ecx 		; add runlength value to accum
		cmp	ecx,64			; if value >= 64, it was a
		jge	getnextlength		; makeup code, go get next
		cmp	OutputCount,esi 	; else check total runlength
		jge	goodrun 		; against remaining linelength
found_errorcode:
		mov	esi,-1			; indicate bad code or overrun
goodrun:
		cmp	esi,0			; set flags for caller
		ret				; return length in esi

getrunlength	endp

;****************************************************************************
; Decompression Tables
;****************************************************************************

white_tree:
w		dd	0,w1
w0		dd	0,w01
w00		dd	0,w001
w000		dd	0,w0001
w0000		dd	0,w00001
w00000		dd	0,w000001
w000000 	dd	0,w0000001
w0000000	dd	0,w00000001
w00000000	dd	0,w000000001
w000000000	dd	0,w0000000001
w0000000000	dd	0,w00000000001
w00000000000	dd	0,w000000000001
w000000000000	dd	1,-2
w000000000001	dd	1,-1
w00000000001	dd	1,-2
w0000000001	dd	1,-2
w000000001	dd	1,-2
w00000001	dd	0,w000000011
w000000010	dd	0,w0000000101
w0000000100	dd	0,w00000001001
w00000001000	dd	1,1792
w00000001001	dd	0,w000000010011
w000000010010	dd	1,1984
w000000010011	dd	1,2048
w0000000101	dd	0,w00000001011
w00000001010	dd	0,w000000010101
w000000010100	dd	1,2112
w000000010101	dd	1,2176
w00000001011	dd	0,w000000010111
w000000010110	dd	1,2240
w000000010111	dd	1,2304
w000000011	dd	0,w0000000111
w0000000110	dd	0,w00000001101
w00000001100	dd	1,1856
w00000001101	dd	1,1920
w0000000111	dd	0,w00000001111
w00000001110	dd	0,w000000011101
w000000011100	dd	1,2368
w000000011101	dd	1,2432
w00000001111	dd	0,w000000011111
w000000011110	dd	1,2496
w000000011111	dd	1,2560
w0000001	dd	0,w00000011
w00000010	dd	1,29
w00000011	dd	1,30
w000001 	dd	0,w0000011
w0000010	dd	0,w00000101
w00000100	dd	1,45
w00000101	dd	1,46
w0000011	dd	1,22
w00001		dd	0,w000011
w000010 	dd	0,w0000101
w0000100	dd	1,23
w0000101	dd	0,w00001011
w00001010	dd	1,47
w00001011	dd	1,48
w000011 	dd	1,13
w0001		dd	0,w00011
w00010		dd	0,w000101
w000100 	dd	0,w0001001
w0001000	dd	1,20
w0001001	dd	0,w00010011
w00010010	dd	1,33
w00010011	dd	1,34
w000101 	dd	0,w0001011
w0001010	dd	0,w00010101
w00010100	dd	1,35
w00010101	dd	1,36
w0001011	dd	0,w00010111
w00010110	dd	1,37
w00010111	dd	1,38
w00011		dd	0,w000111
w000110 	dd	0,w0001101
w0001100	dd	1,19
w0001101	dd	0,w00011011
w00011010	dd	1,31
w00011011	dd	1,32
w000111 	dd	1,1
w001		dd	0,w0011
w0010		dd	0,w00101
w00100		dd	0,w001001
w001000 	dd	1,12
w001001 	dd	0,w0010011
w0010010	dd	0,w00100101
w00100100	dd	1,53
w00100101	dd	1,54
w0010011	dd	1,26
w00101		dd	0,w001011
w001010 	dd	0,w0010101
w0010100	dd	0,w00101001
w00101000	dd	1,39
w00101001	dd	1,40
w0010101	dd	0,w00101011
w00101010	dd	1,41
w00101011	dd	1,42
w001011 	dd	0,w0010111
w0010110	dd	0,w00101101
w00101100	dd	1,43
w00101101	dd	1,44
w0010111	dd	1,21
w0011		dd	0,w00111
w00110		dd	0,w001101
w001100 	dd	0,w0011001
w0011000	dd	1,28
w0011001	dd	0,w00110011
w00110010	dd	1,61
w00110011	dd	1,62
w001101 	dd	0,w0011011
w0011010	dd	0,w00110101
w00110100	dd	1,63
w00110101	dd	1,0
w0011011	dd	0,w00110111
w00110110	dd	1,320
w00110111	dd	1,384
w00111		dd	1,10
w01		dd	0,w011
w010		dd	0,w0101
w0100		dd	0,w01001
w01000		dd	1,11
w01001		dd	0,w010011
w010010 	dd	0,w0100101
w0100100	dd	1,27
w0100101	dd	0,w01001011
w01001010	dd	1,59
w01001011	dd	1,60
w010011 	dd	0,w0100111
w0100110	dd	0,w01001101
w01001100	dd	0,w010011001
w010011000	dd	1,1472
w010011001	dd	1,1536
w01001101	dd	0,w010011011
w010011010	dd	1,1600
w010011011	dd	1,1728
w0100111	dd	1,18
w0101		dd	0,w01011
w01010		dd	0,w010101
w010100 	dd	0,w0101001
w0101000	dd	1,24
w0101001	dd	0,w01010011
w01010010	dd	1,49
w01010011	dd	1,50
w010101 	dd	0,w0101011
w0101010	dd	0,w01010101
w01010100	dd	1,51
w01010101	dd	1,52
w0101011	dd	1,25
w01011		dd	0,w010111
w010110 	dd	0,w0101101
w0101100	dd	0,w01011001
w01011000	dd	1,55
w01011001	dd	1,56
w0101101	dd	0,w01011011
w01011010	dd	1,57
w01011011	dd	1,58
w010111 	dd	1,192
w011		dd	0,w0111
w0110		dd	0,w01101
w01100		dd	0,w011001
w011000 	dd	1,1664
w011001 	dd	0,w0110011
w0110010	dd	0,w01100101
w01100100	dd	1,448
w01100101	dd	1,512
w0110011	dd	0,w01100111
w01100110	dd	0,w011001101
w011001100	dd	1,704
w011001101	dd	1,768
w01100111	dd	1,640
w01101		dd	0,w011011
w011010 	dd	0,w0110101
w0110100	dd	0,w01101001
w01101000	dd	1,576
w01101001	dd	0,w011010011
w011010010	dd	1,832
w011010011	dd	1,896
w0110101	dd	0,w01101011
w01101010	dd	0,w011010101
w011010100	dd	1,960
w011010101	dd	1,1024
w01101011	dd	0,w011010111
w011010110	dd	1,1088
w011010111	dd	1,1152
w011011 	dd	0,w0110111
w0110110	dd	0,w01101101
w01101100	dd	0,w011011001
w011011000	dd	1,1216
w011011001	dd	1,1280
w01101101	dd	0,w011011011
w011011010	dd	1,1344
w011011011	dd	1,1408
w0110111	dd	1,256
w0111		dd	1,2
w1		dd	0,w11
w10		dd	0,w101
w100		dd	0,w1001
w1000		dd	1,3
w1001		dd	0,w10011
w10010		dd	1,128
w10011		dd	1,8
w101		dd	0,w1011
w1010		dd	0,w10101
w10100		dd	1,9
w10101		dd	0,w101011
w101010 	dd	1,16
w101011 	dd	1,17
w1011		dd	1,4
w11		dd	0,w111
w110		dd	0,w1101
w1100		dd	1,5
w1101		dd	0,w11011
w11010		dd	0,w110101
w110100 	dd	1,14
w110101 	dd	1,15
w11011		dd	1,64
w111		dd	0,w1111
w1110		dd	1,6
w1111		dd	1,7

black_tree:
b		dd	0,b1
b0		dd	0,b01
b00		dd	0,b001
b000		dd	0,b0001
b0000		dd	0,b00001
b00000		dd	0,b000001
b000000 	dd	0,b0000001
b0000000	dd	0,b00000001
b00000000	dd	1,-2
b00000001	dd	0,b000000011
b000000010	dd	0,b0000000101
b0000000100	dd	0,b00000001001
b00000001000	dd	1,1792
b00000001001	dd	0,b000000010011
b000000010010	dd	1,1984
b000000010011	dd	1,2048
b0000000101	dd	0,b00000001011
b00000001010	dd	0,b000000010101
b000000010100	dd	1,2112
b000000010101	dd	1,2176
b00000001011	dd	0,b000000010111
b000000010110	dd	1,2240
b000000010111	dd	1,2304
b000000011	dd	0,b0000000111
b0000000110	dd	0,b00000001101
b00000001100	dd	1,1856
b00000001101	dd	1,1920
b0000000111	dd	0,b00000001111
b00000001110	dd	0,b000000011101
b000000011100	dd	1,2368
b000000011101	dd	1,2432
b00000001111	dd	0,b000000011111
b000000011110	dd	1,2496
b000000011111	dd	1,2560
b0000001	dd	0,b00000011
b00000010	dd	0,b000000101
b000000100	dd	0,b0000001001
b0000001000	dd	1,18
b0000001001	dd	0,b00000010011
b00000010010	dd	0,b000000100101
b000000100100	dd	1,52
b000000100101	dd	0,b0000001001011
b0000001001010	dd	1,640
b0000001001011	dd	1,704
b00000010011	dd	0,b000000100111
b000000100110	dd	0,b0000001001101
b0000001001100	dd	1,768
b0000001001101	dd	1,832
b000000100111	dd	1,55
b000000101	dd	0,b0000001011
b0000001010	dd	0,b00000010101
b00000010100	dd	0,b000000101001
b000000101000	dd	1,56
b000000101001	dd	0,b0000001010011
b0000001010010	dd	1,1280
b0000001010011	dd	1,1344
b00000010101	dd	0,b000000101011
b000000101010	dd	0,b0000001010101
b0000001010100	dd	1,1408
b0000001010101	dd	1,1472
b000000101011	dd	1,59
b0000001011	dd	0,b00000010111
b00000010110	dd	0,b000000101101
b000000101100	dd	1,60
b000000101101	dd	0,b0000001011011
b0000001011010	dd	1,1536
b0000001011011	dd	1,1600
b00000010111	dd	1,24
b00000011	dd	0,b000000111
b000000110	dd	0,b0000001101
b0000001100	dd	0,b00000011001
b00000011000	dd	1,25
b00000011001	dd	0,b000000110011
b000000110010	dd	0,b0000001100101
b0000001100100	dd	1,1664
b0000001100101	dd	1,1728
b000000110011	dd	1,320
b0000001101	dd	0,b00000011011
b00000011010	dd	0,b000000110101
b000000110100	dd	1,384
b000000110101	dd	1,448
b00000011011	dd	0,b000000110111
b000000110110	dd	0,b0000001101101
b0000001101100	dd	1,512
b0000001101101	dd	1,576
b000000110111	dd	1,53
b000000111	dd	0,b0000001111
b0000001110	dd	0,b00000011101
b00000011100	dd	0,b000000111001
b000000111000	dd	1,54
b000000111001	dd	0,b0000001110011
b0000001110010	dd	1,896
b0000001110011	dd	1,960
b00000011101	dd	0,b000000111011
b000000111010	dd	0,b0000001110101
b0000001110100	dd	1,1024
b0000001110101	dd	1,1088
b000000111011	dd	0,b0000001110111
b0000001110110	dd	1,1152
b0000001110111	dd	1,1216
b0000001111	dd	1,64
b000001 	dd	0,b0000011
b0000010	dd	0,b00000101
b00000100	dd	1,13
b00000101	dd	0,b000001011
b000001010	dd	0,b0000010101
b0000010100	dd	0,b00000101001
b00000101000	dd	1,23
b00000101001	dd	0,b000001010011
b000001010010	dd	1,50
b000001010011	dd	1,51
b0000010101	dd	0,b00000101011
b00000101010	dd	0,b000001010101
b000001010100	dd	1,44
b000001010101	dd	1,45
b00000101011	dd	0,b000001010111
b000001010110	dd	1,46
b000001010111	dd	1,47
b000001011	dd	0,b0000010111
b0000010110	dd	0,b00000101101
b00000101100	dd	0,b000001011001
b000001011000	dd	1,57
b000001011001	dd	1,58
b00000101101	dd	0,b000001011011
b000001011010	dd	1,61
b000001011011	dd	1,256
b0000010111	dd	1,16
b0000011	dd	0,b00000111
b00000110	dd	0,b000001101
b000001100	dd	0,b0000011001
b0000011000	dd	1,17
b0000011001	dd	0,b00000110011
b00000110010	dd	0,b000001100101
b000001100100	dd	1,48
b000001100101	dd	1,49
b00000110011	dd	0,b000001100111
b000001100110	dd	1,62
b000001100111	dd	1,63
b000001101	dd	0,b0000011011
b0000011010	dd	0,b00000110101
b00000110100	dd	0,b000001101001
b000001101000	dd	1,30
b000001101001	dd	1,31
b00000110101	dd	0,b000001101011
b000001101010	dd	1,32
b000001101011	dd	1,33
b0000011011	dd	0,b00000110111
b00000110110	dd	0,b000001101101
b000001101100	dd	1,40
b000001101101	dd	1,41
b00000110111	dd	1,22
b00000111	dd	1,14
b00001		dd	0,b000011
b000010 	dd	0,b0000101
b0000100	dd	1,10
b0000101	dd	1,11
b000011 	dd	0,b0000111
b0000110	dd	0,b00001101
b00001100	dd	0,b000011001
b000011000	dd	1,15
b000011001	dd	0,b0000110011
b0000110010	dd	0,b00001100101
b00001100100	dd	0,b000011001001
b000011001000	dd	1,128
b000011001001	dd	1,192
b00001100101	dd	0,b000011001011
b000011001010	dd	1,26
b000011001011	dd	1,27
b0000110011	dd	0,b00001100111
b00001100110	dd	0,b000011001101
b000011001100	dd	1,28
b000011001101	dd	1,29
b00001100111	dd	1,19
b00001101	dd	0,b000011011
b000011010	dd	0,b0000110101
b0000110100	dd	0,b00001101001
b00001101000	dd	1,20
b00001101001	dd	0,b000011010011
b000011010010	dd	1,34
b000011010011	dd	1,35
b0000110101	dd	0,b00001101011
b00001101010	dd	0,b000011010101
b000011010100	dd	1,36
b000011010101	dd	1,37
b00001101011	dd	0,b000011010111
b000011010110	dd	1,38
b000011010111	dd	1,39
b000011011	dd	0,b0000110111
b0000110110	dd	0,b00001101101
b00001101100	dd	1,21
b00001101101	dd	0,b000011011011
b000011011010	dd	1,42
b000011011011	dd	1,43
b0000110111	dd	1,0
b0000111	dd	1,12
b0001		dd	0,b00011
b00010		dd	0,b000101
b000100 	dd	1,9
b000101 	dd	1,8
b00011		dd	1,7
b001		dd	0,b0011
b0010		dd	1,6
b0011		dd	1,5
b01		dd	0,b011
b010		dd	1,1
b011		dd	1,4
b1		dd	0,b11
b10		dd	1,3
b11		dd	1,2

_TEXT		ends

		end
