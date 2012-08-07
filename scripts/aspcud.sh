#!/bin/bash

clasp_bin=clasp
unclasp_bin=unclasp
gringo_bin=gringo-3.0.3
cudf2lp_bin=cudf2lp

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
	dumperr
	echo "error: $1" >&2
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
	echo "  -p OPT   append cudf2lp option OPT"
	echo "  -s SOL   choose solver {clasp,unclasp}"
	echo
	echo "Default commandline for clasp:"
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
	echo
	echo "Default commandline for unclasp:"
	echo -n "$(basename "${0}")"
	for x in "${unclasp_opts_def[@]}";  do
		echo " \\"
		echo -n "    -c $x"
	done
	for x in "${ungringo_opts_def[@]}";  do
		echo " \\"
		echo -n "    -e $x"
	done
	echo
}

base="$(dirname "$(readlink -f "$0")")"
PATH=".:$base:$base/../build/release/bin:$PATH"

# default options
solver=""
clasp_opts_def=( "--opt-heu=1" "--sat-prepro" "--restarts=128" "--heuristic=VSIDS" "--solution-recording" "--opt-hierarch=1" "--local-restarts" )
unclasp_opts_def=( )
gringo_opts_def=( "$(enc configuration.lp)" "$(enc optimize-define.lp)" )
ungringo_opts_def=( "$(enc configuration-plain.lp)" "$(enc optimize-define.lp)" )

cudf_opts=( )
clasp_opts=( )
gringo_opts=( )

unset wrapper_out
unset cudf_pid
unset gringo_pid
unset clasp_pid
unset tmp

while getopts "hc:e:p:s:" flag
do
	case "$flag" in
		"e") gringo_opts=( "${gringo_opts[@]}" "$(enc "$OPTARG")" ) ;;
		"c") clasp_opts=( "${clasp_opts[@]}" "$OPTARG" ) ;;
		"p") cudf_opts=( "${cudf_opts[@]}" "$OPTARG" ) ;;
		"s") solver="$OPTARG" ;;
		"h") usage; exit 0 ;;
		"?") exit 1 ;;
	esac
done

shift $((OPTIND-1))

if [[ -z ${solver} ]]; then
	if echo $(basename "$0") | grep -q "aspuncud"; then
		solver="unclasp"
	elif echo $(basename "$0") | grep -q "aspcud"; then
		solver="clasp"
	fi
fi

case "${solver}" in 
	clasp)
		solver_bin="${clasp_bin}"
		clasp_opts_implicit=( "--stats=2" "--quiet=1,2" )
		;;
	unclasp)
		clasp_opts_def=( "${unclasp_opts_def[@]}" )
		gringo_opts_def=( "${ungringo_opts_def[@]}" )
		solver_bin="${unclasp_bin}"
		clasp_opts_implicit=( "--stats" )
		;;
	*)
		die "error: solver clasp or unclasp expected"
		;;
esac

[[ ${#clasp_opts[*]} -eq 0 ]] && clasp_opts=( "${clasp_opts_def[@]}" )
[[ ${#gringo_opts[*]} -eq 0 ]] && gringo_opts=( "${gringo_opts_def[@]}" )
clasp_opts=( "${clasp_opts[@]}" "${clasp_opts_implicit[@]}" )

if [[ $# -eq 3 ]]; then
	cudf_opts=( "${cudf_opts[@]}" "-c" "$3" )
elif echo $(basename "$0") | grep -q "trendy"; then
	[[ $# -ne 2 ]] && { die "error: exactly two arguments expected"; }
	cudf_opts=( "${cudf_opts[@]}" "-c" "trendy" )
elif echo $(basename "$0") | grep -q "paranoid"; then
	[[ $# -ne 2 ]] && { die "error: exactly two arguments expected"; }
	cudf_opts=( "${cudf_opts[@]}" "-c" "paranoid" )
else
	die "error: exactly three arguments expected"
fi

wrapper_out="$2"

trap cleanup EXIT
test -n "${TMPDIR}" && tmpdir="${TMPDIR}/"
tmp="$(mktemp -d "${tmpdir}outXXXXXX")"

mkfifo $tmp/cudf_out $tmp/gringo_out

"${solver_bin}"  "${clasp_opts[@]}"  > "$tmp/clasp_out"  2> "$tmp/clasp_err"    < "$tmp/gringo_out" &
clasp_pid=$!
"${gringo_bin}"  "${gringo_opts[@]}" > "$tmp/gringo_out" 2> "$tmp/gringo_err" - < "$tmp/cudf_out" &
gringo_pid=$!
"${cudf2lp_bin}" "${cudf_opts[@]}"   > "$tmp/cudf_out"   2> "$tmp/cudf_err"   - < "$1" &
cudf_pid=$!

trap usrtrap USR1 TERM INT

wait $cudf_pid
[[ $? -ne 0 ]] && die "cudf2lp failed"
wait $gringo_pid
[[ $? -ne 0 ]] && die "gringo failed"
wait $clasp_pid

trap "" USR1 TERM INT
wait

grep -A 1 ^Answer "$tmp/clasp_out" | tail -n 1 | sed -e 's/in("/package: /g' -e 's/",/\nversion: /g' -e 's/)[ ]\?/\ninstalled: true\n\n/g' > "$wrapper_out"
cat "$tmp/clasp_out" | colrm 81 > "$tmp/parser_err"
grep -q ^Answer "$tmp/clasp_out" || die "no answer printed"

dumperr

exit 0

