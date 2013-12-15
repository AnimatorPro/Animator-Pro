@if "%1" == "" goto ERROR
c:\phar386\binb\bind386 c:\phar386\binb\run386b %1.exp -exe %1.exe
@goto OUT
:ERROR
	@echo bind what?
	@echo usage:  bind program
	@echo this will bind386 program.exp into program.exe
:OUT
