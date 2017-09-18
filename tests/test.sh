#!/bin/bash

location="$(cd "$(dirname $0)"; echo "$PWD")"
encoding="$location"/../encodings/misc2012.lp
#encoding="$location"/../encodings/specification.lp

clasp=clasp
gringo=gringo
check=cudf-sol-check
cudf="$location/../build/debug/bin/cudf2lp"
aspcud="$location/../build/debug/bin/aspcud"

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
    for solver in "$clasp"; do
        for encoding in "$location/../encodings/misc2012.lp" "$location/../encodings/specification.lp"; do
            start=$(date +%s)
            # echo "\"$aspcud\" -e \"$encoding\" -S \"$solver\" -G \"$gringo\" -P \"$cudf\" problem.cudf solution.cudf \"$crit\""
            "$aspcud" -e "$encoding" -S "$solver" -G "$gringo" -P "$cudf" "${extra[@]}" problem.cudf solution.cudf "$crit" > /dev/null
            end=$(date +%s)
            "$check" -cudf problem.cudf -sol solution.cudf -crit "$crit" > solution.opt
            diff "${x%.cudf.xz}.opt" solution.opt && echo "passed ($[$end-$start]s, $(basename "$solver"), $(basename "$encoding"))" || echo "FAILED ($encoding/$solver)"
            rm -f solution.cudf solution.opt
        done
    done
    rm -f problem.cudf
done
