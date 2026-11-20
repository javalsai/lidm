builtins.elemAt (
	builtins.match
		"VERSION[[:blank:]]*:?=[[:blank:]]*([^[:space:]]+)"
		(builtins.readFile ../../../Makefile)
) 0
