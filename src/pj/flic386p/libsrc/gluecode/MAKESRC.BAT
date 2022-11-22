@echo off
	if NOT "%1"=="force" goto do_it
	shift
	set -opts=-f
:do_it
	@echo (Nothing gets copied into the gluecode dir!)
	set -opts=
