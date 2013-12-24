@echo off
	call setwc386
	cd \paa\inc
	tcmake ilib
	if ERRORLEVEL 1 goto out
	cd \paa\rastlib
	tcmake %1 %2 %3 %4 %5 %6 %7 %8 %9
:out
	beep 3
@echo on
