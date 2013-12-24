@echo off
if "%1"=="wvideo" goto wvideo
run386 ..\torture\torture /c hgsc %1 %2 %3 %4 %5 %6 %7 %8 %9
goto exit
:wvideo
wvideo ..\torture\torture /c hgsc %2 %3 %4 %5 %6 %7 %8 %9
:exit
