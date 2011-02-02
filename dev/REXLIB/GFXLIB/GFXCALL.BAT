@if "%2"=="" goto onename
@if "%3"=="" goto twoname
:threename
	copy gfxjump.i %2.asm
	rep XXXX %1 %2.asm -b -s
	rep YYYY %3 %2.asm -b -s
	goto out
:twoname
	copy gfxjump.i %2.asm
	rep XXXX %1 %2.asm -b -s
	rep YYYY %1 %2.asm -b -s
	goto out
:onename
	copy gfxjump.i %1.asm
	rep XXXX %1 %1.asm -b -s
	rep YYYY %1 %1.asm -b -s
	goto out
:out
