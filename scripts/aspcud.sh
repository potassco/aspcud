#!/bin/bash

function usrtrap()
{
	kill $cudf_pid   2> /dev/null
	kill $gringo_pid 2> /dev/null
	kill $clasp_pid  2> /dev/null
}

function enc()
{
	for x in "." "$base" "$base/encodings" "$base/../../Encodings"; do
		[[ -e "$x/$1" ]] && { echo "$x/$1"; return 0; }
	done
	echo "$x"
	return 1
}

function cleanup()
{
	[[ -d "$tmp" ]] && rm -rf "$tmp"
}

function dumperr()
{
	for x in "$tmp/cudf_err" "$tmp/gringo_err" "$tmp/clasp_err" "$tmp/parser_err"; do
		[[ -e "$x" ]] && cat $x >&2
	done
}

function die()
{
	[[ -n "$wrapper_out" ]] && echo FAIL > "$wrapper_out"
	echo "error: $1" >&2
	dumperr
	usrtrap
	wait
	exit 0
}

function usage()
{
	echo "Usage: ${0} [OPTION]... CUDFIN CUDFOUT [CRITERIA]"
	echo "  -h       print this help"
	echo "  -c OPT   append clasp option OPT"
	echo "  -e ENC   append encoding ENC"
	echo
	echo "Default commandline:"
	echo -n "$(basename "${0}")"
	for x in "${clasp_opts_def[@]}";  do
		echo " \\"
		echo -n "    -c $x"
	done
	for x in "${gringo_opts_def[@]}";  do
		echo " \\"
		echo -n "    -e $x"
	done
	echo
}

base="$(dirname "$(readlink -f "$0")")"
PATH=".:$base:$base/../build/release/bin:$PATH"

# default options
clasp_opts_def=( "--opt-he=1" "--sat" "--restarts=32" "--heu=VSIDS" "--restart-o" "--opt-hi=2" )
gringo_opts_def=( "$(enc configuration.lp)" "$(enc optimize-define.lp)" )

cudf_opts=( )
clasp_opts=( )
gringo_opts=( )

unset wrapper_out
unset cudf_pid
unset gringo_pid
unset clasp_pid
unset tmp

while getopts  "hc:e:" flag
do
	case "$flag" in
		"e") gringo_opts=( "${gringo_opts[@]}" "$(enc "$OPTARG")" ) ;;
		"c") clasp_opts=( "${clasp_opts[@]}" "$OPTARG" ) ;;
		"h") usage; exit 0 ;;
		"?") exit 1 ;;
	esac
done

shift $((OPTIND-1))

[[ ${#clasp_opts[*]} -eq 0 ]] && clasp_opts=( "${clasp_opts_def[@]}" )
[[ ${#gringo_opts[*]} -eq 0 ]] && gringo_opts=( "${gringo_opts_def[@]}" )
if [[ $# -eq 3 ]]; then
	cudf_opts=( "-c" "$3" )
elif echo $(basename "$0") | grep -q "trendy"; then
	[[ $# -ne 2 ]] && { die "error: exactly two arguments expected"; }
	cudf_opts=( "-c" "trendy" )
elif echo $(basename "$0") | grep -q "paranoid"; then
	[[ $# -ne 2 ]] && { die "error: exactly two arguments expected"; }
	cudf_opts=( "-c" "paranoid" )
else
	die "error: exactly three arguments expected"
fi

wrapper_out="$2"

clasp_opts=( "${clasp_opts[@]}" "--stats=2" "--quiet=1,2"  )

trap cleanup EXIT
#tmp="$(mktemp -d /tmp/outXXXXXX)"
tmp="$(mktemp -d outXXXXXX)"

mkfifo $tmp/cudf_out $tmp/gringo_out

clasp   "${clasp_opts[@]}"  > "$tmp/clasp_out"  2> "$tmp/clasp_err"    < "$tmp/gringo_out" &
clasp_pid=$!
gringo  "${gringo_opts[@]}" > "$tmp/gringo_out" 2> "$tmp/gringo_err" - < "$tmp/cudf_out" &
gringo_pid=$!
cudf2lp "${cudf_opts[@]}"   > "$tmp/cudf_out"   2> "$tmp/cudf_err"   - < "$1" &
cudf_pid=$!

trap usrtrap USR1 TERM INT

wait $cudf_pid
[[ $? -ne 0 ]] && die "cudf2lp failed"
wait $gringo_pid
[[ $? -ne 0 ]] && die "gringo failed"
wait $clasp_pid

trap "" USR1 TERM INT
wait

# TODO: could be run in background too
touch $tmp/parser_err
found=0
while read line; do
	case "$found" in
		0) echo "$line" | grep -q "^Answer:" && found=1 || echo "$line" >> "$tmp/parser_err" ;;
		1) 
			found=2
			echo -n "$line" | sed -e 's/in("/package: /g' -e 's/",/\nversion: /g' -e 's/)[ ]\?/\ninstalled: true\n\n/g'
			;;
		2) echo "$line" >> $tmp/parser_err ;;
	esac
done < "$tmp/clasp_out" > "$wrapper_out"

(( found != 2 )) && die "no answer printed"
	
dumperr

exit 0

