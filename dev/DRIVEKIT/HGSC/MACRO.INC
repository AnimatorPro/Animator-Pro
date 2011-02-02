wptr	  equ		word ptr
dptr	  equ		dword ptr
wpds	  equ		word ptr ds
wpes	  equ		word ptr es

Entry	  macro
		  push		ebp
		  mov		ebp,esp
		  endm

Exit	  macro
		  leave
		  ret
		  endm

Save	  macro 	reg1,reg2,reg3,reg4,reg5,reg6
  ifnb <reg1>
		  push		reg1
  endif
  ifnb <reg2>
		  push		reg2
  endif
  ifnb <reg3>
		  push		reg3
  endif
  ifnb <reg4>
		  push		reg4
  endif
  ifnb <reg5>
		  push		reg5
  endif
  ifnb <reg6>
		  push		reg6
  endif
		  endm

Restore   macro 	reg1,reg2,reg3,reg4,reg5,reg6
  ifnb <reg6>
		  pop	   reg6
  endif
  ifnb <reg5>
		  pop	   reg5
  endif
  ifnb <reg4>
		  pop	   reg4
  endif
  ifnb <reg3>
		  pop	   reg3
  endif
  ifnb <reg2>
		  pop	   reg2
  endif
  ifnb <reg1>
		  pop	   reg1
  endif
		  endm
