#!/bin/bash

encodings=( "scripts/encodings/specification.lp" "scripts/encodings/misc2012.lp" ) 
cudf2lp="/home/kaminski/svn/potassco/trunk/aspcud/build/static/bin/cudf2lp"
clasp="/home/kaminski/svn/potassco/trunk/clasp/build/static/bin/clasp"
unclasp="/home/wv/bin/linux/64/unclasp"
gringo="/home/wv/bin/linux/64/gringo"
version="1.6"

cd "$(dirname "$0")"
cd ..

rm -rf submissions
mkdir -p "submissions/aspcud/encodings"

cp "${encodings[@]}" "submissions/aspcud/encodings/"
cp "${gringo}" "${clasp}" "${unclasp}" "${cudf2lp}" "submissions/aspcud/"

strip "submissions/aspcud/gringo"
strip "submissions/aspcud/clasp"
strip "submissions/aspcud/unclasp"
strip "submissions/aspcud/cudf2lp"

cp "scripts/aspcud.sh" "submissions/aspcud/"
cd "submissions/aspcud/"
for system in "aspcud" "aspuncud"; do
	for track in "paranoid" "basic" "full"; do
		ln -s aspcud.sh "${system}-${track}"
	done
done
cd ..

for system in "aspcud" "aspuncud"; do
	for track in "paranoid" "basic" "full"; do
		dst="${system}-${track}-${version}"
		ftrack="${track}"
		test "${track}" != "paranoid" && ftrack="${track} user"
		cp -r aspcud "${dst}"
		echo "Solver for ${ftrack} track." > "${dst}/README"
		tar -cf "${dst}.tar" "${dst}"
		cp "${dst}.tar" /home/wv/WWW/aspcud/files/
		chmod g+w "/home/wv/WWW/aspcud/files/${dst}.tar"
		mail="misc-competition@inria.fr"
		#mail="kaminski@cs.uni-potsdam.de"
		mail "$mail" -s "Solver Submission" <<q
http://www.cs.uni-potsdam.de/aspcud/files/${dst}.tar
$(md5sum "${dst}.tar")
${ftrack}
q
	done
done

cd ..
