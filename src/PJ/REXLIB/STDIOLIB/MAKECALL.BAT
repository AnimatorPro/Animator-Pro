if "%2"=="" goto samename
	copy siojump.i %2.asm
	rep XXXX %1 %2.asm
	goto out
:samename
	copy siojump.i %1.asm
	rep XXXX %1 %1.asm
:out
