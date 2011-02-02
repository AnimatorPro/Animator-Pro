;diag.asm - map a diagonal line to a horizontal line.  The core of
;the raster rotating/stretching machinery in Vpaint.

	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;dto_table(Raster *sv, Pixel *dtable,int dsize,int x0,int y0,int x1,int y1)
	public dto_table
dto_table proc near

lvarspace equ 16

dttp	struc	;dto_table stack and argument structure

; start of local variables 

	dt_incx dd ?
	dt_incy dd ?
	dt_deltax dd ?
	dt_deltay dd ?

; end local variables

	dt_edi dd ?	;what's there from pushad
	dt_esi dd ?
	dt_ebp dd ?
	dt_esp dd ?
	dt_ebx dd ?
	dt_edx dd ?
	dt_ecx dd ?
	dt_eax dd ?

; pushad variables above

	dt_ret 	dd ?	;return address for function
	dt_sv	dd ?	;1st parameter - source screen
	dt_dtable	dd ?
	dt_dsize	dd ?
	dt_x0	dd ?
	dt_y0	dd ?
	dt_x1	dd ?
	dt_y1	dd ?

dttp ends

	pushad
	sub esp,lvarspace	;local space
	mov ebp,esp

	mov edi,[esp].dt_sv

;compute address of x0/y0 into ds:si

	mov esi,[edi].bym_p
	mov eax,[esp].dt_y0
	mul [edi].bym_bpr
	add esi,eax	;add in y0 component
	mov eax,[esp].dt_x0   ;fetch x0
	add esi,eax   ;add in x0 component

;compute  [esp].dt_deltax and [esp].dt_incx

	mov edx,-1  ;edx = [esp].dt_incx
	mov ebx,[esp].dt_x1
	sub ebx,eax  ;ebx = x1-x0
	js nnx
	neg edx
	neg ebx
nnx: 
	mov [esp].dt_incx,edx
	mov  [esp].dt_deltax,ebx  ; [esp].dt_deltax = -|x1-x0|

;compute [esp].dt_deltay and [esp].dt_incy

	mov edx,[edi].bym_bpr  ;edx = [esp].dt_incy
	neg edx
	mov ebx,[esp].dt_y1
	sub ebx,[esp].dt_y0  ;ebx = y1-y0
	js nny
	neg edx
	neg ebx
nny: 
	mov [esp].dt_incy,edx
	mov [esp].dt_deltay,ebx	;[esp].dt_deltay = -|y1-y0|

	mov edx,[esp].dt_dsize ;get dsize into handy register for inner loop

	;eax = xerr = dsize + [esp].dt_deltax/2
	mov eax, [esp].dt_deltax
	sar eax,1
	add eax,edx

	;ebx = yerr = dsize + [esp].dt_deltay/2
	mov ebx,[esp].dt_deltay
	sar ebx,1
	add ebx,edx

	mov ecx,edx	;dot count into dx

	dec  [esp].dt_deltax
	dec [esp].dt_deltay

	mov edi,[esp].dt_dtable

innerlp:

	movsb	;move that babe!
	dec	esi	;oops didn't mean to increment source here

	add eax, [esp].dt_deltax
	jg	ystep

nextx: add esi,[esp].dt_incx
	add eax,edx
	jle nextx

ystep:
	add ebx,[esp].dt_deltay
	jg nextlp

nexty: add esi,[esp].dt_incy
	add ebx,edx
	jle nexty

nextlp:
	loop innerlp

	add esp,lvarspace
	popad
	ret
dto_table endp

code	ends
	end
