@echo off
	if NOT "%1"=="force" goto do_it
	shift
	set -opts=-f
:do_it
	cpynewer %-opts% -v1 -i \paa\fli\ @objlist.inc
	set -opts=
