@echo off
	if "%1"=="highc" goto highc
	if "%1"=="watcom" shift
:watcom
	copy ..\inc\makewc.inc ..\inc\make.inc
	tcmake %1 %2 %3 %4 %5 %6 %7 %8 %9
	goto OUT
:highc
	shift
	copy ..\inc\makehc.inc ..\inc\make.inc
	tcmake %1 %2 %3 %4 %5 %6 %7 %8 %9
	goto OUT
:OUT
@echo on
