#!/usr/bin/env bash

IFS=$'\n'

memcheck() {
	local BIN={1:-$DEFAULT_BIN}

	if command -v valgrind &> /dev/null; then
		valgrind --leak-check=full -v ./$BIN
	else
		echo -e "[-] Binary Valgrind not found - skipping\n"
	fi
}

main() {
	memcheck
}

# stop if being sourced
return 2>/dev/null

set -o errexit
set -o nounset

DEFAULT_BIN='cnano'

main $*
