builtins.elemAt (
	builtins.match
		"VERSION[[:blank:]]*:?=[[:blank:]]*([^[:space:]]+)\n.*"
		(builtins.readFile ../../../Makefile)
) 0
