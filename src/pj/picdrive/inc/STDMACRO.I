;*****************************************************************************
;* STDMACRO.I - Some macros and equates to allow shorthand assembler coding.
;*****************************************************************************

bptr	equ	byte ptr
wptr	equ	word ptr
dptr	equ	dword ptr

;-----------------------------------------------------------------------------
; MACRO LocalProc - Set up the proc statement for a local routine.
;-----------------------------------------------------------------------------

LocalProc macro procname
&procname proc near
#PROCSCOPEGLOBAL = 0
	endm

;-----------------------------------------------------------------------------
; MACRO PublicProc - Set up the proc statement for a global routine.
;-----------------------------------------------------------------------------

PublicProc macro procname
	public	&procname
	public	_&procname
&procname  proc near
_&procname proc near
#PROCSCOPEGLOBAL = 1
	endm

;-----------------------------------------------------------------------------
; MACRO EndProc - Set up the endp statement for either global or local.
;-----------------------------------------------------------------------------

EndProc macro procname
  if #PROCSCOPEGLOBAL
_&procname endp
  endif
&procname  endp
	endm

;-----------------------------------------------------------------------------
; MACRO Entry - Set up equates used by other macros.
;
;	Code one of these right after your PROC statement for each routine
;	that wants to use the Lclvars, Args, BArgs, Save, or Restore macros.
;-----------------------------------------------------------------------------

Entry	macro
#ARGOFFS = 4
#LCLSIZE = 0
#BASED_ARGS = 0
	endm

;-----------------------------------------------------------------------------
; MACRO Exit - Undo stack base, then return.
;
;	Code one of these at the point you want your RET instruction.
;	You can code more than one of these in a proc if you want.
;-----------------------------------------------------------------------------

Exit	macro
 if #BASED_ARGS
	leave
 endif
	ret
	endm

;-----------------------------------------------------------------------------
; MACRO Lclvars - Name local stack-based vars, but don't alloc space for them.
;
;	Each variable named is assumed to be a dword, other-sized local vars
;	have to be handled manually.  This equates each local name to a
;	stack offset, but the space is allocated by the Save macro, after
;	registers have been pushed to the stack.  The first variable named
;	should be the most-frequently accessed; it will be addressed as
;	[esp], which generates shorter instructions than [esp+nn].
;
;	If you want to name local vars and are not going to use the Save
;	macro, manually allocate the space by coding 'sub esp,#LCLSIZE'
;	following the Lclvars macro; be sure to manually de-alloc the
;	space before the Exit macro.
;-----------------------------------------------------------------------------

Lclvars macro	var1,var2,var3,var4,var5,var6,var7,var8,var9,var10,var11,var12
  ifnb <var1>
var1 equ  [esp+00]
#LCLSIZE = #LCLSIZE + 4
  endif
  ifnb <var2>
var2 equ  [esp+04]
#LCLSIZE = #LCLSIZE + 4
  endif
  ifnb <var3>
var3 equ  [esp+08]
#LCLSIZE = #LCLSIZE + 4
  endif
  ifnb <var4>
var4 equ  [esp+12]
#LCLSIZE = #LCLSIZE + 4
  endif
  ifnb <var5>
var5 equ  [esp+16]
#LCLSIZE = #LCLSIZE + 4
  endif
  ifnb <var6>
var6 equ  [esp+20]
#LCLSIZE = #LCLSIZE + 4
  endif
  ifnb <var7>
var7 equ  [esp+24]
#LCLSIZE = #LCLSIZE + 4
  endif
  ifnb <var8>
var8 equ  [esp+28]
#LCLSIZE = #LCLSIZE + 4
  endif
  ifnb <var9>
var9 equ  [esp+32]
#LCLSIZE = #LCLSIZE + 4
  endif
  ifnb <var10>
var10 equ  [esp+36]
#LCLSIZE = #LCLSIZE + 4
  endif
  ifnb <var11>
var11 equ  [esp+40]
#LCLSIZE = #LCLSIZE + 4
  endif
  ifnb <var12>
var12 equ  [esp+44]
#LCLSIZE = #LCLSIZE + 4
  endif
	endm

;----------------------------------------------------------------------------
; MACRO Args - Build equates to access each of the function parameters.
;
;	This macro will build equates relating the specified names to
;	locations of the function parameters on the stack, addressed via
;	offset from [esp].  When used in conjunction with the Save and
;	Lclvars macros, the correct stack offsets are maintained internally
;	within the equates.
;----------------------------------------------------------------------------

Args	macro	arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12
  ifnb <arg1>
arg1 equ  [esp+#ARGOFFS+#LCLSIZE+00]
  endif
  ifnb <arg2>
arg2 equ  [esp+#ARGOFFS+#LCLSIZE+04]
  endif
  ifnb <arg3>
arg3 equ  [esp+#ARGOFFS+#LCLSIZE+08]
  endif
  ifnb <arg4>
arg4 equ  [esp+#ARGOFFS+#LCLSIZE+12]
  endif
  ifnb <arg5>
arg5 equ  [esp+#ARGOFFS+#LCLSIZE+16]
  endif
  ifnb <arg6>
arg6 equ  [esp+#ARGOFFS+#LCLSIZE+20]
  endif
  ifnb <arg7>
arg7 equ  [esp+#ARGOFFS+#LCLSIZE+24]
  endif
  ifnb <arg8>
arg8 equ  [esp+#ARGOFFS+#LCLSIZE+28]
  endif
  ifnb <arg9>
arg9 equ  [esp+#ARGOFFS+#LCLSIZE+32]
  endif
  ifnb <arg10>
arg10 equ  [esp+#ARGOFFS+#LCLSIZE+36]
  endif
  ifnb <arg11>
arg11 equ  [esp+#ARGOFFS+#LCLSIZE+40]
  endif
  ifnb <arg12>
arg12 equ  [esp+#ARGOFFS+#LCLSIZE+44]
  endif
	endm

;----------------------------------------------------------------------------
; MACRO BArgs - Build [ebp]-based equates for the function parameters.
;
;	This macro is similar to the Args macro, except that the parameters
;	are addressed via offset from [ebp].  (Code to save/load ebp is
;	generated as well).  The only time you need this macro instead of
;	the Args macro is when you need to access parms after/while you
;	have stacked other things.  For example:
;
;	   BArgs #xcoor,#ycoor,#width
;
;	   push  #width
;	   push  #ycoor
;	   push  #xcoor
;	   call  _something_
;
;	In this example, 'Args' type equates would fail after the first
;	push instruction modifies esp.	The BArgs equates will work because
;	they are based upon ebp, which doesn't change when pushing things.
;----------------------------------------------------------------------------

BArgs	macro	arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12
#BASED_ARGS = 1
	push	ebp
	mov	ebp,esp
  ifnb <arg1>
arg1 equ  [ebp+08]
  endif
  ifnb <arg2>
arg2 equ  [ebp+12]
  endif
  ifnb <arg3>
arg3 equ  [ebp+16]
  endif
  ifnb <arg4>
arg4 equ  [ebp+20]
  endif
  ifnb <arg5>
arg5 equ  [ebp+24]
  endif
  ifnb <arg6>
arg6 equ  [ebp+28]
  endif
  ifnb <arg7>
arg7 equ  [ebp+32]
  endif
  ifnb <arg8>
arg8 equ  [ebp+36]
  endif
  ifnb <arg9>
arg9 equ  [ebp+40]
  endif
  ifnb <arg10>
arg10 equ  [ebp+44]
  endif
  ifnb <arg11>
arg11 equ  [ebp+48]
  endif
  ifnb <arg12>
arg12 equ  [ebp+52]
  endif
	endm

;-----------------------------------------------------------------------------
; MACRO Save - Save specified registers, and alloc space for local vars.
;
;	Code one of these (following any Lclvars and/or Args macros) to
;	save registers to the stack.  This macro automatically adjusts the
;	offsets of arguments declared with the Args macro so that arguments
;	remain addressable without needing the EBP register, and without
;	needing massive code changes if you change the list of registers
;	to be saved.  If local vars were named with the Lclvars macro, space
;	for them will be allocated on the stack below the saved registers.
;
;	Do not code more than one of these in a routine!
;-----------------------------------------------------------------------------

Save	  macro     reg1,reg2,reg3,reg4,reg5,reg6
  ifnb <reg1>
	  push	    reg1
#ARGOFFS  = #ARGOFFS+4
  endif
  ifnb <reg2>
#ARGOFFS  = #ARGOFFS+4
	  push	    reg2
  endif
  ifnb <reg3>
#ARGOFFS  = #ARGOFFS+4
	  push	    reg3
  endif
  ifnb <reg4>
#ARGOFFS  = #ARGOFFS+4
	  push	    reg4
  endif
  ifnb <reg5>
#ARGOFFS  = #ARGOFFS+4
	  push	    reg5
  endif
  ifnb <reg6>
#ARGOFFS  = #ARGOFFS+4
	  push	    reg6
  endif
  if #LCLSIZE
	  sub	    esp,#LCLSIZE
  endif
	  endm

;-----------------------------------------------------------------------------
; MACRO Restore - Remove local vars (if any) and restore registers.
;
;	Code the register names on this macro IN THE SAME ORDER AS THEY WERE
;	CODED ON THE Save MACRO!  (Not in reverse order like you do with
;	push/pop sequences.  This macro reverses the order automatically.)
;	Do not code more than one of these in a routine!
;-----------------------------------------------------------------------------

Restore   macro     reg1,reg2,reg3,reg4,reg5,reg6
  if #LCLSIZE
	  add	   esp,#LCLSIZE
  endif
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

;-----------------------------------------------------------------------------
; some standardized segment definitions...
;-----------------------------------------------------------------------------

_DATA	segment dword public use32 'DATA'
_DATA	ends

_BSS	segment dword public use32 'BSS'
_BSS	ends

_TEXT	segment dword public use32 'CODE'
_TEXT	ends

CGROUP	group	_TEXT
DGROUP	group	_DATA,_BSS
	assume	cs:CGROUP,ds:DGROUP

; end of stdmacro.i
