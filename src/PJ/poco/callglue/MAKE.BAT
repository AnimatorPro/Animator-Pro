@echo off
	if "%1" == "turboc" goto turbo
	call setwc386
	if "%1" == "link" goto link
	echo >make.inc !include "makewc.inc"
	tcmake %1 %2 %3 %4 %5 %6 %7 %8 %9
	if ERRORLEVEL 1 goto out
	if NOT "%1" == "" goto out
:link
	wlink @test.lnk option map
	goto out
:turbo
	call set_tc
	shift
	if "%1" == "link" goto tlink
	echo >make.inc !include "maketc.inc"
	tcmake -fmakefile.tc %1 %2 %3 %4 %5 %6 %7 %8 %9
	if NOT "%1" == "" goto out
:tlink
	del tctest.exe
	tlink c:\tc\c0l @tctest.lnk, test.exe, tctest, c:\tc\emu c:\tc\mathl c:\tc\cl
	ren test.exe tctest.exe
	goto out
:out
	beep 3
	@echo on
