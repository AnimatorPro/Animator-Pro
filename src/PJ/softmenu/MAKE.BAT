@echo off
	call setwc386
:default
	tcmake %1 %2 %3 %4 %5 %6 %7 %8 %9
	if ERRORLEVEL 1 goto out
:out
	beep 1
	@echo on
