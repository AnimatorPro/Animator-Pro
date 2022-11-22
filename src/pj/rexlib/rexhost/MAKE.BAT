@echo off
	call setwc386
	echo >\paa\inc\make.inc !include "\paa\inc\makewc.inc"
rem	pushdir
rem	cd \paa\inc
rem	tcmake ilib
rem	if ERRORLEVEL 1 goto out
rem	popdir
	tcmake %1 %2 %3 %4 %5 %6 %7 %8 %9
:out
	beep 3
@echo on
