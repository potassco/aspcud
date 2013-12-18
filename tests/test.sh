#!/bin/bash

location="$(readlink -f "$(dirname $0)")"
encoding="$location"/encoding_new.lp

clasp=clasp
gringo=gringo-4
cudf="$location"/../build/debug/bin/cudf2lp

if [[ -z "$1" ]]; then
	for x in "$location"/*.cudf; do
		echo "================== $(basename $x) ================="
		"$cudf" < "$x" 2>/dev/null | $gringo - "$encoding" 2>/dev/null | $clasp 0 --out=1 | grep -v "ANSWER SET FOUND" |\
		while read line; do
			if [[ -n "$line" ]]; then
				echo "$line" | tr " " "\n" | sort | tr -d "\n"
				echo
			fi
		done | sort | diff - "$(dirname "$x")/$(basename "$x" .cudf)".sol && echo "passed" || echo "FAILED"
	done
else
	"$cudf" < "$1" | $gringo - "$encoding" | $clasp 0 --out=1 | \
	while read line; do
		if [[ -n "$line" ]]; then
			echo "$line" | tr " " "\n" | sort | tr -d "\n"
			echo
		fi
	done | sort
fi


