@echo off
	if NOT "%1"=="force" goto do_it
	shift
	del *.obj
	del *.lib
:do_it
	make %1 %2 %3 %4
