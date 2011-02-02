@echo off
	if NOT "%1"=="force" goto do_it
	set -opts=-f
:do_it
	cpynewer %-opts% -v1 -i \paa\inc\		   @main_inc.cpy
	cpynewer %-opts% -v1 -i \paa\drivekit\inc\ @vdrv_inc.cpy
	set -opts=
