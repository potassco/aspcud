#!/bin/bash

clasp_bin=clasp
unclasp_bin=unclasp
gringo_bin=gringo
cudf2lp_bin=cudf2lp

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

function die()
{
	echo "error: $1" >&2
	exit 0
}

function usage()
{
	echo "Usage: ${0} [OPTION]... CUDFIN CUDFOUT [CRITERIA]"
	echo "  -h       print this help"
	echo "  -c OPT   append clasp option OPT"
	echo "  -e ENC   append encoding ENC"
	echo "  -p OPT   append cudf2lp option OPT"
    echo "  -s SOL   path to solver (clasp or unclasp)"
    echo "  -g GRD   path to grounder (gringo)"
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
	for x in "${gringo_opts_def[@]}";  do
		echo " \\"
		echo -n "    -e $x"
	done
	echo
}

base="$(dirname "$(readlink -f "$0")")"
PATH=".:$base:$base/../build/release/bin:$PATH"

# default options
clasp_opts_def=( "--opt-heu=1" "--sat-prepro" "--restarts=L,128" "--heuristic=VSIDS" "--opt-hierarch=1" "--local-restarts" "--del-max=200000,250" "--save-progress=0" )
unclasp_opts_def=( )
gringo_opts_def=( "$(enc misc2012.lp)" )

cudf_opts=( )
clasp_opts=( )
gringo_opts=( )

solver_bin=

unset wrapper_out
unset tmp

while getopts "hc:e:p:s:g:" flag
do
	case "$flag" in
		"e") gringo_opts=( "${gringo_opts[@]}" "$(enc "$OPTARG")" ) ;;
		"c") clasp_opts=( "${clasp_opts[@]}" "$OPTARG" ) ;;
		"p") cudf_opts=( "${cudf_opts[@]}" "$OPTARG" ) ;;
		"s") solver_bin="$OPTARG" ;;
		"g") gringo_bin="$OPTARG" ;;
		"h") usage; exit 0 ;;
		"?") exit 1 ;;
	esac
done

shift $((OPTIND-1))

if [[ -z "${solver_bin}" ]]; then
	if echo $(basename "$0") | grep -q "aspuncud"; then
		solver_bin="${unclasp_bin}"
        clasp_opts_def=( "${unclasp_opts_def[@]}" )
        clasp_opts_implicit=( "--stats" )

	elif echo $(basename "$0") | grep -q "aspcud"; then
		solver_bin="${clasp_bin}"
        clasp_opts_implicit=( "--stats=2" "--quiet=1,2" )
	fi
elif echo ${solver_bin} | grep -q "unclasp"; then
    clasp_opts_def=( "${unclasp_opts_def[@]}" )
    clasp_opts_implicit=( "--stats" )
else
    clasp_opts_implicit=( "--stats=2" "--quiet=1,2" )
fi

[[ ${#clasp_opts[*]} -eq 0 ]] && clasp_opts=( "${clasp_opts_def[@]}" )
[[ ${#gringo_opts[*]} -eq 0 ]] && gringo_opts=( "${gringo_opts_def[@]}" )
clasp_opts=( "${clasp_opts[@]}" "${clasp_opts_implicit[@]}" )

if [[ $# -eq 3 ]]; then
	cudf_opts=( "${cudf_opts[@]}" "-c" "$3" )
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

# note this is probably unecessary...
function usrtrap() {
:
}
trap usrtrap USR1 TERM INT

cat <<EOF > "$tmp/parse.py"
#!/usr/bin/python
import signal, re, sys

def ignore(x, y): pass

signal.signal(signal.SIGUSR1, ignore)
signal.signal(signal.SIGTERM, ignore)
signal.signal(signal.SIGINT,  ignore)
signal.signal(signal.SIGPIPE, ignore)

out = open("$wrapper_out", "w")

keep = False
solution = None
try:
	for line in sys.stdin:
		sys.stdout.write(line[:-1][:81])
		sys.stdout.write("\n")
		if keep: 
			solution = line
			keep = False
		if line.startswith("Answer"): keep = True
except: pass

if solution == None: out.write("FAIL\n")
else:
        for m in re.finditer(r'in\\("(?P<pkg>[^,]+)",(?P<ver>[^)]+)\\)', solution):
                out.write("package: " + m.group("pkg") + "\\n")
                out.write("version: " + m.group("ver") + "\\n")
                out.write("installed: true\\n\\n")
out.close()

sys.stdout.write(open("$tmp/clasp_err").read())
sys.stdout.write(open("$tmp/gringo_err").read())
sys.stdout.write(open("$tmp/cudf_err").read())
EOF
chmod +x "$tmp/parse.py"

"${cudf2lp_bin}" "${cudf_opts[@]}"   > "$tmp/cudf_out"   2> "$tmp/cudf_err"   < "$1"
"${gringo_bin}"  "${gringo_opts[@]}" > "$tmp/gringo_out" 2> "$tmp/gringo_err" "$tmp/cudf_out"
"${solver_bin}"  "${clasp_opts[@]}"                      2> "$tmp/clasp_err"  "$tmp/gringo_out" | "$tmp/parse.py"

