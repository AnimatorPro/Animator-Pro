@echo off
	if NOT "%1"=="force" goto do_it
	shift
	set -opts=-f
:do_it
	cpynewer %-opts% -v1 -i \paa\util\	 @obj1list.inc
	cpynewer %-opts% -v1 -i \paa\input\  @obj2list.inc
	cpynewer %-opts% -v1 -i \paa\pjhigh\ @obj3list.inc
	set -opts=
