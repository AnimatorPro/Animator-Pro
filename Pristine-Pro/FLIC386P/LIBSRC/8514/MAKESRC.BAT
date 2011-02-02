@echo off
	if NOT "%1"=="force" goto do_it
	shift
	set -opts=-f
:do_it
	cpynewer %-opts% -v1 -i \paa\drivekit\8514\ *.h *.i @objlist.inc
	set -opts=
