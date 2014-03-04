#!/bin/bash

location="$(cd "$(dirname $0)"; echo "$PWD")"
encoding="$location"/../share/aspcud/misc2012.lp
#encoding="$location"/../scripts/encodings/specification.lp

clasp=clasp
gringo=gringo-4
check=cudf-sol-check
cudf="$location/../build/debug/bin/cudf2lp"
aspcud="$location/../build/debug/bin/aspcud"
unclasp="$location/../scripts/unclasp-wrapper.sh"

for x in "$location"/enumerate-all/*.cudf; do
    echo "================== $(basename $x) ================="
    "$cudf" < "$x" 2>/dev/null | "$gringo" - "$encoding" 2>/dev/null | "$clasp" 0 --outf=1 -V0 -q0,0 | grep -v "A" |\
    while read line; do
        if [[ -n "$line" ]]; then
            echo "$line" | tr " " "\n" | sort | tr -d "\n"
            echo
        fi
    done | sort | diff - "$(dirname "$x")/$(basename "$x" .cudf)".sol && echo "passed" || echo "FAILED"
done

for x in "$location"/*/*.cudf.xz; do
    crit=$(echo "$(basename "$(dirname "$x")")" | tr "PMLRC" '\+\-(),')
    echo "================== $(basename $x) with $crit ================="
    xzcat "$x" > problem.cudf
    for solver in "$clasp" "$unclasp"; do
        for encoding in "$location/../share/aspcud/misc2012.lp" "$location/../share/aspcud/specification.lp"; do
            start=$(date +%s)
            "$aspcud" -e "$encoding" -s "$solver" -g "$gringo" -l "$cudf" "${extra[@]}" problem.cudf solution.cudf "$crit" > /dev/null
            end=$(date +%s)
            "$check" -cudf problem.cudf -sol solution.cudf -crit "$crit" > solution.opt
            diff "${x%.cudf.xz}.opt" solution.opt && echo "passed ($[$end-$start]s, $(basename "$solver"), $(basename "$encoding"))" || echo "FAILED ($encoding/$solver)"
            rm -f solution.cudf solution.opt
        done
    done
    rm -f problem.cudf
done
