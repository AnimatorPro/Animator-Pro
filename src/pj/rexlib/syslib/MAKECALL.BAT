if "%2"=="" goto samename
	copy sljump.i %2.asm
	rep XXXX %1 %2.asm -b
	goto out
:samename
	copy sljump.i %1.asm
	rep XXXX %1 %1.asm -b
:out
