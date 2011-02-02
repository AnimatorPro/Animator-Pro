@echo off

	if not "%1"=="hc" goto watcom
	shift
	call set_hc
	echo >make.inc !include "\paa\inc\makehc.inc"
	c:\bin\make %1
	call setwc386
	goto out
:watcom
	call setwc386
	echo >make.inc !include "\paa\inc\makewc.inc"
	c:\bin\make %1
	goto out
:out
	beep 3
	@echo on
