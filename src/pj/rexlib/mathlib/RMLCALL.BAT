if "%2"=="" goto samename
	copy rmljump.i %2.asm
	rep XXXX %1 %2.asm -b
	goto out
:samename
	copy rmljump.i %1.asm
	rep XXXX %1 %1.asm -b
:out
