#!/bin/bash

encodings=( "scripts/encodings/specification.lp" ) 
cudf2lp="build/static/bin/cudf2lp"
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
		# send the mail automatically???
		echo "To: misc-competition@inria.fr"
		echo "Subject: Submission of '$dst' for track '$ftrack'"
		echo "http://www.cs.uni-potsdam.de/aspcud/files/${dst}.tar"
		md5sum "${dst}.tar"
		echo "${ftrack}"
		echo
	done
done

cd ..
