if "%1"=="" goto noout
	run386 \paa\test.exp >%1
	goto out
:noout
	run386 \paa\test.exp >h:\out
:out
