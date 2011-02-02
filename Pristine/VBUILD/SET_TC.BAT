@if "%1"=="" goto USAGE
path %1\bin;%1\tc
set LIB=%1\tc\
set INCLUDE=%1\tc\include\
@goto OUT
:USAGE
@echo This sets up environment variables and the path so that
@echo you can compile the Animator, Converter, and AAplay.
@echo Usage: set_tc device:
@echo Example: set_tc c:
@beep 1
@goto OUT
:OUT
