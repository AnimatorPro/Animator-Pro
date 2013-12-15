@echo off
	if "%1"=="watcom"  goto watcom
	if "%1"=="turboc"  goto turboc
	if "%1"=="tc"      goto turboc
	if "%1"=="archive" goto archive
	goto default
:watcom
	shift
:default
	call setwc386
	tcmake -fmakefile.wc %1 %2 %3 %4
	goto OUT
:turboc
	shift
:archive
	call set_bc
	tcmake -fmakefile.tc %1 %2 %3 %4
	goto OUT
:OUT
@echo on
