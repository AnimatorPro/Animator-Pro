@echo off
call setwc386
rem pushdir
rem md ..\fileio
rem tcmake libs
rem cd ..\rlib
rem tcmake libs
rem popdir
tcmake %1 %2 %3 %4 %5 %6 %7 %8 %9
if ERRORLEVEL 1 goto ERROR
goto OUT
:ERROR
beep 4
:OUT
@echo on
