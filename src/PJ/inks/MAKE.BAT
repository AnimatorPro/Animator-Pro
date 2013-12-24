@echo off
	call setwc386
	pushdir
	cd \paa\inc
	tcmake ilib
	if ERRORLEVEL 1 goto out
	popdir
	tcmake %1 %2 %3 %4 %5 %6 %7 %8 %9
:out
	beep 1
@echo on
