;clipline.asm - this file isn't linked.  Probably won't even assemble.
;It's some line clipping that I ended up not using yanked out of clipit.asm

;clipline - pass it with bp pointing to the parameters suggested 
;immediately below.  Trashes registers ax,bx,cx,dx,si,di.  Returns
;with carry bit set if the line is clipped out, otherwise with
;the parameters adjusted to clipped values.
;
;This isn't the Evans/Sutherland algorithm, but one which clips one
;side at a time developed originally in 1984 for the Aegis Animator
;by Jim Kent.  (It's just pretty easy linear math though....) Really
;don't know if this one is better or not for single lines like this.
;You pretty much do have to clip one side at a time for closed filled
;polygons.
;
	public clipline
x1	equ word ptr [bp+8+2]
y1	equ word ptr [bp+10+2]
x2	equ word ptr [bp+12+2]
y2	equ word ptr [bp+14+2]
clipline proc far

;1st clip against left side (x1,x2 < 0)
	mov cx,x1	
	mov bx,x2
	test cx,cx
	jns x1leftok
	test bx,bx
	js  lclipped1
;if have arrived here x1 is out to left but x2 is in
;so zero out x1 and calculate y1 to be in corresponding position...
	mov ax,y1		
	mov si,y2
	sub ax,si
	imul bx
	mov di,bx
	sub di,cx
	idiv di
	add ax,si
	mov y1,ax		;y1 = y2 + (y1-y2)*x2/(x2-x1)
	mov x1,0		;x1 = 0
	jmp clipright
;here we know x1 is not gonna be left clipped
x1leftok:
	test bx,bx	;but what about x2?
	jns clipright	;no left clipping at all
;if here x2 is out to left but x1 is in...
	mov ax,y2
	mov si,y1
	sub ax,si
	imul cx
	mov di,cx
	sub di,bx
	idiv di
	add ax,si
	mov y2,ax		;y2 = y1 + (y2-y1)*x1/(x1-x2)
	mov x2,0		;x2 = 0

clipright:
;now clip against the right side... x1,x2 >= 320...
;cx = x1 and bx = x2 still...
	cmp cx,320
	js  x1rightok
	cmp bx,320
	jns lclipped1
;if have arrived here x1 is out to right but x2 is ok.
	mov ax,y1
	mov si,y2
	sub ax,si    ;ax = (y2-y1)
	mov dx,319
	sub dx,bx
	imul dx
	sub cx,bx
	idiv cx
	add ax,si
	mov y1,ax	 ;y1 = y2 + (y1-y2)*(319-x2)/(x1-x2)
	mov x1,319
	jmp clipup
lclipped1: ;8086 conditional branches are only +-128 bytes so we have
           ;this here in the middle for clip-outs.
	jmp lclipped
x1rightok:
	cmp bx,320
	js  clipup	;hey, no right clipping at all...yeah!
;here x1 is ok, but x2 is too far to the right
;cx = x1 and bx = x2 still...
	mov ax,y2
	mov si,y1
	sub ax,si
	mov dx,319
	sub dx,cx
	imul dx
	sub bx,cx
	idiv bx
	add ax,si
	mov y2,ax	 ;y2 = y1 + (y2-y1)*(319-x1)/(x2-x1)
	mov x2,319

clipup:
;clip against top side (y1,y2 < 0)
	mov cx,y1	
	mov bx,y2
	test cx,cx
	jns y1upok
	test bx,bx
	js  lclipped1
;if have arrived here y1 is out to left but y2 is in
;so zero out y1 and calculate x1 to be in corresponding position...
	mov ax,x1		
	mov si,x2
	sub ax,si
	imul bx
	mov di,bx
	sub di,cx
	idiv di
	add ax,si
	mov x1,ax		;x1 = x2 + (x1-x2)*y2/(y2-y1)
	mov y1,0		;y1 = 0
	jmp clipright
;here we know y1 is not gonna be up clipped
y1upok:
	test bx,bx	;but what about y2?
	jns clipdown	;no up clipping at all
;if here y2 is out to left but y1 is in...
	mov ax,x2
	mov si,x1
	sub ax,si
	imul cx
	mov di,cx
	sub di,bx
	idiv di
	add ax,si
	mov x2,ax		;x2 = x1 + (x2-x1)*y1/(y1-y2)
	mov y2,0		;y2 = 0

clipdown:
;now clip against the down side... y1,y2 >= 200...
;cx = y1 and bx = y2 still...
	cmp cx,200
	js  y1downok
	cmp bx,200
	jns lclipped
;if have arrived here y1 is out to right but y2 is ok.
	mov ax,x1
	mov si,x2
	sub ax,si
	mov dx,199
	sub dx,bx
	imul dx
	sub cx,bx
	idiv cx
	add ax,si
	mov x1,ax	 ;x1 = x2 + (x1-x2)*(199-y2)/(y1-y2)
	mov y1,199
	jmp noclip
y1downok:
	cmp bx,200
	js  noclip	;hey, no down clipping at all...yeah!
;here y1 is ok, but y2 is too far down
;cx = y1 and bx = y2 still...
	mov ax,x2
	mov si,x1
	sub ax,si
	mov dx,199
	sub dx,cx
	imul dx
	sub bx,cx
	idiv bx
	add ax,si
	mov x2,ax	 ;x2 = x1 + (x2-x1)*(199-y1)/(y2-y1)
	mov y2,199
noclip:
	clc
	ret
lclipped:
	stc
	ret
clipline endp

